<?php
/**
 * Diese Datei beinhaltet alle Hilfsfunktionen für die Engine, die primär
 * aus historischen Gründen noch keiner Klasse zugeordnet sind.
 *
 * Ziel ist es, diese Datei komplett zu entfernen, sie mit der main_funcs
 * zusammenzulegen und daraus eine PHP-Standardfunktionserweiterungsdatei zu
 * erstellen, die als Frameworkkern dienen kann.
 */
require_once (dirname(__FILE__).'/classes/map/Mapsettings.inc');
require_once (dirname(__FILE__).'/consts.inc');
require_once (dirname(__FILE__).'/shared/main_funcs.inc');

function do_login_stuff($redo=0)
{
	$current_user = User::get_current_user();
	require_once (dirname(__FILE__).'/classes/Resource.inc');

	/* IP log - eventuell zu aufwendig mit dem FIFO */
	$ip = $_SERVER['REMOTE_ADDR'];
	$ips = query(
		'SELECT id, ip, t_stamp FROM ips '.
			'WHERE uid='.$current_user->get_id().' AND neverdrop=0'
	);

	$first_ip = mysql_fetch_array($ips);
	if ($first_ip['ip'] == $ip) // keine IP doppelt speichern
		$just_saved = true;
	else {
		$just_saved = false;
		while ($temp_ip = mysql_fetch_array($ips)) {
			if ($temp_ip['ip'] == $ip) {
				$just_saved = 1;
				break;
			}
			if ($temp_ip['t_stamp'] < $first_ip['t_stamp'])
			$first_ip = $temp_ip;
		}
	}
	if (!$just_saved) {
		if (mysql_num_rows($ips) >= 10) // bei mehr als 10 => FIFO
		query('DELETE FROM ips WHERE id='.$first_ip['id']);
		query('INSERT INTO ips SET uid='.$current_user->get_id().', t_stamp='.time().', ip=\''.$ip.'\'');
	}
	/* IP log Ende */

	$mapsettings = Mapsettings::get_instance();

	$result=query('SELECT type FROM worlds WHERE id='.$current_user->get_world_id());
	if (mysql_num_rows($result))
	{
		$data=mysql_fetch_assoc($result);
		$_SESSION['map_type']=$data['type'];
	}
	$mapsettings->set_timefactor(
		mysql_result(
			query(
				'SELECT timefactor FROM maps '.
				"WHERE map_id={$current_user->get_map_id()} ".
				"AND world_id={$current_user->get_world_id()}"
			),0));
}

function errormail($message, $to='All', $topic='undefined')
{
	return;	// dekativiert
}

/**
 * Diese Wrapper-Funktion benutzen um Mails zu versenden, anstatt mail().
 * Wird $from nicht gesetzt, oder mit false belegt, dann wird die Mail
 * über den Certified Server von TMC-MM gesandt.
 *
 * ACHTUNG: Mails über den TMC-MM-Server können ggf mitgelesen werden. Keine
 * sensiblen Daten wie Errormails über TMC-MM schicken!
 *
 * Zudem wird für TMC-MM-Server-Mails fopen benutzt. Das heißt, in der php.ini muss allow_url_fopen=On
 * sein.
 *
 * @param string $receiver_address	Adresse an den man schickt
 * @param string $topic				Titel der Mail
 * @param string $message			Nachricht der Mail
 * @param string $from				Absender. Wenn false oder nicht gesetzt, dann wird über TMC-MM gesandt
 */
function pv_mail($receiver_address, $topic, $message, $from=false) {
	require_once("shared/classes/php_mailer/class.phpmailer.php");

	$mail = new PHPMailer(true);
	$mail->IsSMTP();

	try {
		$mail->Host = "smtp.variomedia.de";
		$mail->SMTPAuth = true;
		$mail->Port = 587;
//		$mail->Username = "donotreply@gausus.com";
//		$mail->Password = "b44R9TkLrUhf2WZL";
		$mail->Username = "noreply@mammun.com";
		$mail->Password = "N3f0pifbNFP3";
//		$mail->SetFrom("donotreply@gausus.com");
		$mail->SetFrom("noreply@mammun.com");
		$mail->CharSet = 'utf-8';
		if ($from !== false) {
			try {
				$mail->AddReplyTo($from);
			}
			catch (phpmailerException $e) {}
		}
		else
			//$mail->AddReplyTo("donotreply@gausus.com");
			$mail->AddReplyTo("noreply@mammun.com");

		$mail->AddAddress($receiver_address);
		$mail->Subject = $topic;
		$mail->Body = $message;
		$mail->Send();
	}
	catch (phpmailerException $e) {
		error_log("pv_mail: ".$e->errorMessage());
		throw new Exception($e->getMessage(), $e->getCode());
	}
	catch (Exception $e) {
		error_log("pv_mail: ".$e->getMessage());
		throw $e;
	}

/*	if ($from === false) {
		$url = 'http://www.tmc-multimedia.de/pv/mail.php?';
		$url .= 'addr='.urlencode($receiver_address).'&';
		$url .= 'betr='.urlencode($topic).'&';
		$url .= 'text='.urlencode($message);
		$checkcon = @fopen($url, 'r');
		if (!$checkcon) {
			$checkcon = @fopen('http://www.tmc-multimedia.de/pv/mail.php?addr='.urlencode("support@project-virus.de").
			'&betr=could not send message&text=Message could not be send, maybe it\'s too long?\n\nOriginal topic: '.$topic, 'r');
			if (!$checkcon) {
				echo "<b>Warning:</b> Error message could not be sent!";
				echo $message;
			}
		}
		if ($checkcon) {
			$buffer = @fgets($checkcon, 4096);
			@fclose($checkcon);
		}
	}
	else {
		mail($receiver_address, $topic, $message, $from);
	}*/
}
?>
