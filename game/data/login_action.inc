<?php
require_once('../include/shared/classes/server_com/ClientServer.inc');
$r = new ClientServerResponse();

define('LOGIN_ACTIVE',	1);

if (!LOGIN_ACTIVE) {
	$nick = $_REQUEST['nick'];

	$users_may_login = array(
		'philipp',
		'robert',
		'matthias',
		'annika',
		'david',
		'daniel',
		'anna',
		'alex'
	);
	if (isset($users_may_login) && !in_array(strtolower($nick), $users_may_login)) {
		$r->add_value("Login aktuell nicht möglich, alle Karten werden aktuell ".
			"zurückgesetzt und das Spielsystem unter Berücksichtigung aller ".
			"eingegangenen Berichte umstrukturiert und neu ausbalanciert.\n".
			"\n".
			"Diese Umstrukturierungen können insgesamt eine Woche in Anspruch nehmen.\n".
			"\n".
			"Wenn diese Umstellungen erfolgt sind, erhält jeder registrierte ".
			"Beta-Nutzer eine E-Mail benachrichtigung mit ggf. neuen Zugangsdaten.\n".
			"\n".
			"Vielen Dank für das fleißige Testen bislang.\n".
			"	Euer Gausus-Team");
		$r->add_session();
		die($r);
	}
}
@session_destroy();
@session_unset();
@session_start();
require_once ('../include/accesscontrol2.inc');
$r = new ClientServerResponse();
$r->add_value(1);
$r->add_session();
echo $r;

?>