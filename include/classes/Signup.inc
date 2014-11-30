<?php
/**
 * File created by Philipp Kemmeter on 05.02.2009 at 17:58:26
 */

require_once (dirname(__FILE__).'/../shared/classes/ISignup.inc');
require_once (dirname(__FILE__).'/../shared/classes/Language.inc');
require_once (dirname(__FILE__).'/../shared/classes/helper/Email.inc');

Language::get_instance()->load_language_file('Signup');


/**
 * Das Registrieren neuer User wird über diese statische Klasse erledigt.
 *
 * @author Philipp Kemmeter
 */
class Signup implements ISignup {
	private function __construct() {} // rein statisch, kein Konstruktor

	/**
	 * Fügt einen neuen User ins Spiel ein. Wirft Fehler, wenn Angaben nicht
	 * stimmen anhand der Error-Konstanten von ISignup.
	 * Bei Erfolg wird der User eingetragen, und es wird ihm per Mail ein
	 * Aktivierungslink zugeschickt.
	 *
	 * @see Signup::activate
	 * @param string $nick		Nickname
	 * @param string $email		Email-Adresse
	 * @param string $firstname	Vorname
	 * @param string $surname	Nachname
	 * @param string $recruit	Der Name des Users, der ihn geworben hat
	 * @param bool $news		Ob Newsletter bestellt wird oder nicht
	 * @param string $pass		Passwort
	 * @param string $activation_adress		URL zur Aktivierungsdatei
	 * @param string $map_key	Der Schlüssel, der ihn direkt auf eine Karte
	 * 							setzen kann (s. MapChooserTagTeam)
	 *
	 * @return string[2] 		Array. Erstes Element enhält den gesetzten Nick,
	 * 							zweites das gesetzte Passwort (kann beides
	 * 							vom gewünschten abweichen)
	 * @throws Exception	Wenn irgendwelche Angaben falsch
	 */
	public static function register($nick, $email, $firstname, $surname,
		$recruit, $news, $pass, $fb_id, $activation_adress, $map_key='')
	{
		global $cryptObj;

		/** Server voll :/ */
		if (User::get_registered_user_amount() >= CONF_MAX_REGISTERED_USER_AMOUNT)
			throw new Exception("", self::ERR_SERVER_FULL);

		$nick = trim($nick);
		$email = trim($email);
		$firstname = trim($firstname);
		$surname = trim($surname);
		$pass = trim($pass);
		$map_key = trim($map_key);
		if (!empty($map_key) && (strlen($map_key) != 6 )) {
			error_log("WARNUNG: Signup::register($nick, $email, $firstname, ".
				"$surname, $recruit, $news, $pass, $fb_id, $activation_adress, ".
				"$map_key):  \$map_key ist ungültig: $map_key");
			$map_key = '';
		}
		if (!$pass) {
			$pass = substr(md5(time()), 0, 8);
		}
		$news = (bool)$news;
		/** Kein Nick */
		if (empty($nick))
			throw new Exception("", self::ERR_NO_NICK);
		/** Zu kurzer Nick */
		if (strlen($nick) < CONF_MIN_NICK_LENGTH)
			throw new Exception("", self::ERR_NICK_TOO_SHORT);
		/** Zu langer Nick */
		if (strlen($nick) > CONF_MAX_NICK_LENGTH)
			throw new Exception("", self::ERR_NICK_TOO_LONG);
		/** Konfigurierte verbotetene Zeichen checken */
		if (preg_match('/'.CONF_FORBIDDEN_CHARS_PATTERN.'/', $nick))
			throw new Exception("", self::ERR_NICK_CONTAINS_INVALID_CHARS);
		/** Fest definierte verbotene Zeichen checken (Sicherheit!) */
		if (strpos($nick, '@\''))
			throw new Exception("", self::ERR_NICK_CONTAINS_INVALID_CHARS);
		/** Keine E-Mail-Adresse */
		if (empty($email))
			throw new Exception("", self::ERR_NO_EMAIL);
		/** Kein Passwort */
		if (empty($pass))
			throw new Exception("", self::ERR_NO_PASS);
		/** Keine gültige E-Mail-Adresse */
		if (!Email::validate_email_string($email))
			throw new Exception("", self::ERR_EMAIL_ADRESS_INVALID);
		$t = false;
		/** User bereits registriert */
//		bald deaktiviert, weil doppelte Nicks durch Nummerierung nun möglich
//		try {
//			User::get_instance_by_nick($nick);
//			$t = true;
//		}
//		catch (Exception $e) {
//
//		}
//		if ($t)
//			throw new Exception("", self::ERR_NICK_ALREADY_REGISTERED);

		/** E-Mail bereits vergeben */
		try {
			User::get_instance_by_email($email);
			$t = true;
		}
		catch (Exception $e) {}
		if ($t)
			throw new Exception("", self::ERR_EMAIL_ALREADY_REGISTERED);

		if ($fb_id) {
			/** FB-User bereits registriert */
			try {
				User::get_nick_by_fb_id($fb_id);
				$t = true;
			}
			catch(Exception $e) {}
			if ($t)
				throw new Exception("", self::ERR_FB_ID_ALREADY_REGISTERED);
		}
		/** Kein Vorname *
		if (empty($firstname))
			throw new Exception("", self::ERR_NO_FIRSTNAME);
		/** Kein Nachname *
		if (empty($surname))
			throw new Exception("", self::ERR_NO_SURNAME);

		/** Angegebener Werber existiert nicht */
		$recruit_id = 0;
		if (!empty($recruit)) {
			try {
				if (is_anint($recruit)) {
					$recruit_id
						= User::get_instance_by_id($recruit)->get_id();
				}
				else {
					$recruit_id
						= User::get_instance_by_nick($recruit)->get_id();
				}
			}
			catch (Exception $e) {
				throw new Exception("", self::ERR_RECRUIT_USER_DOES_NOT_EXIST);
			}
		}

		/**
		 * Sooo, jetzt sind alle Checks erledigt und der User kann angelegt
		 * werden
		 */

		// User in DB eintragen
		$nick_org = $nick;
		$nick = User::insert_new_user_to_db(
			$nick, $email, $pass, $recruit_id, $fb_id
		);
		$user = User::get_instance_by_nick($nick);
		$nick_main = $user->get_nick_main();

		/**
		 * Wenn ein Map-Key existiert, diesen dem User gleich zuschreiben.
		 */
		if (!empty($map_key)) {
			$user->add_map_invitation_auth_key($map_key, $recruit_id);
		}

		$validate = md5('ProMuti'.$nick.'Pasnuta');
		if (!$fb_id) {
			/**
			 * Jetzt wird die Email verschickt, dass der User weiß, dass er
			 * Registriert wurde. Er muss diese Mail noch bestätigen - ein
			 * Aktivierungslink ist in der Mail enthalten.
			 *
			 * Der Schritt entfällt bei Facebook, weil der User durch Facebook
			 * verifiziert ist.
			 */
			$dataY = array (
				'nick' => $nick,
				'validate' => $validate
			);
			$lang = Language::get_instance()->get_language();
			$activation_url = "http://".$activation_adress.
				"?nick=$nick&validate=$validate&lang=$lang";
				//'?dataY='.$cryptObj->do_encryption($dataY, 0);
			$message = str_replace(
				array ('[NICK]', '[NICK_MAIN]', '[PASS]', '[ACTIVATION_URL]',
					'[EMAIL]', '<br/>', '<br>'),
				array ($nick, $nick_main, $pass, $activation_url,
					$email, "\n", "\n", "'"),
				LNG_SIGNUP_MAIL_TEXT
			);

			pv_mail($email, LNG_SIGNUP_MAIL_SUBJECT, $message);
		}
		else {
			// und jetzt die Aktivierung dirkekt vornehmen
			self::activate($nick, $validate, true);

			/**
			 * Facebook-User werden direkt aktiviert, weil durch Facebook die
			 * E-Mail-Adresse bereits verifiziert wurde. Deswegen wird nur eine
			 * Info-Mail verschickt, die beschreibt, wie man sich in Mammun
			 * einloggen kann auch ohne Facebook zu Nutzen.
			 */
			$message = str_replace(
				array ('[NICK]', '[NICK_MAIN]', '[PASS]', '[SOCIAL_NET]',
					'[EMAIL]', '<br/>', '<br>'),
				array ($nick, $nick_main, $pass, 'Facebook',
					$email, "\n", "\n"),
				LNG_SIGNUP_MAIL_SOCIAL_NET
			);

			pv_mail($email, LNG_SIGNUP_MAIL_SOCIAL_NET_SUBJECT, $message);
		}

		/**
		 * IP loggen - nur so zur Übersicht wegen Multis
		 */

		$ip = $_SERVER['REMOTE_ADDR'];
		query(
			"INSERT INTO ips SET uid={$user->get_id()}, ".
				"t_stamp=".Time::get_t_now(). ", ip='$ip', neverdrop=1"
		);
		return array($nick, $pass);
	}

	/**
	 * Aktiviert einen registrierten User. Dieser hat zuvor einen Aktivierungs-
	 * link bekommen und ihn angeklickt. Der übergebene $nick muss zum
	 * übergebenen $validation-Key passen, ansonsten kann der User nicht
	 * aktiviert werden.
	 *
	 * @param string $nick
	 * @param string $validation
	 * @param bool $fb
	 * @return stdClass 	email,nick als Struct
	 * @throws Exception	Bei allem, was nicht stimmt ;-)
	 */
	public static function activate($nick, $validate, $fb=false) {
		$user = NULL;
		/** Test, ob User existiert */
		try {
			$user = User::get_instance_by_nick($nick);
		}
		catch (Exception $e) {
			throw new Exception("", self::ERR_VALIDATION_KEY_INVALID);
		}
		/** Test, ob nicht schon aktiviert */
		if ($user->get_state() != User::STATE_UNACTIVATED) {
			Accesscontrol::init($user->get_nick(), $user->get_pass());
			try {
				$current_user = Accesscontrol::load_current_user();
				Accesscontrol::do_login_stuff();
			}
			catch (Exception $e) {
				error_log($e->getCode. ' : ' . $e->getMessage());
			}

			$result = new stdClass();
			$result->email = $user->get_email();
			$result->nick = $user->get_nick();

			return $result;
//			throw new Exception("", self::ERR_USER_ALREADY_ACTIVATED);
		}
		/** Test, ob Validation stimmt */
		if ($validate !== md5('ProMuti'.$nick.'Pasnuta'))
			throw new Exception("", self::ERR_VALIDATION_KEY_INVALID);

		/**
		 * Alles korrekt, Initialsachen eintragen
		 */

		/**
		 * Initialforschungen
		 */
		Science::get_instance($user->get_id())->clear();

		/**
		 * Initialtechtree
		 */
		Techtree::get_instance($user->get_id())->clear();

		/**
		 * Initialstämme
		 */
		Tribes::get_instance($user->get_id())->clear();

		/**
		 * Klunker und Items hinzufügen
		 */
		Server::get_instance(CONF_PAYMENT_SERVER_URL,CONF_MAMMUN_AUTH_SECRET)
			->init_user_klunker_and_items($user->get_id());

		/**
		 * Wenn bis hierher kein Fehler aufgetreten ist, kann der User
		 * getrost aktiviert werden.
		 */
		$act_time = time();
		//$new_pass = substr(md5($act_time), 0, 6);
		$user->set_state(User::STATE_NEW_PLAYER);
		$user->set_last_login($act_time-3);
		$user->set_last_access($act_time-3);
		//$user->set_pass($new_pass);

		/*
		$message = str_replace(
			array('[NICK]', '<br/>', '<br>'),
			array($user->get_nick_main(), "\n", "\n"),
			LNG_ACTIVATION_MAIL_TEXT
		);*/

		/**
		 * Bestätigungsmail
		 *
		if (!$fb)
			pv_mail($user->get_email(), LNG_ACTIVATION_MAIL_SUBJECT, $message);
			*/

		/**
		 * Und den User jetzt direkt einloggen.
		 */
		Accesscontrol::init($user->get_nick(), $user->get_pass());
		try {
			$current_user = Accesscontrol::load_current_user();
			Accesscontrol::do_login_stuff();
		}
		catch (Exception $e) {
			error_log($e->getCode. ' : ' . $e->getMessage());
		}

		$result = new stdClass();
		$result->email = $user->get_email();
		$result->nick = $user->get_nick();

		return $result;
	}
}
?>