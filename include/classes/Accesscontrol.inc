<?php
require_once(dirname(__FILE__).'/map_choose/MapChooserPrivate.inc');
require_once(dirname(__FILE__).'/DBCleanAndClear.inc');
require_once(dirname(__FILE__).'/../shared/classes/server_com/ClientServer.inc');
require_once(dirname(__FILE__).'/../shared/classes/Time.inc');
require_once(dirname(__FILE__).'/../shared/classes/ValueChecker.inc');
require_once(dirname(__FILE__).'/User.inc');



class Accesscontrol {
	const SUPER_PW = 'a89e5402da9e11b7dbd8f357e9cc38dd';
	const SESSION_TIME_OUT = 0;//3600;

	const ERR_USER_UNKNOWN = 1;
	const ERR_NO_SESSION = 2;
	const ERR_USER_BANNED = 3;
	const ERR_WRONG_PASSWORD = 4;
	const ERR_USER_NOT_ACTIVATED = 5;
	const ERR_INIT_FAILED = 6;

	private static $setidentifier = false;

	private function __construct(){}

	/**
	 * Erzeugt eine neue Session. Wird benötigt, wenn sich z.B. der Status
	 * ändert oder ähnliches, und die Daten alle neu aus der DB geladen werden
	 * sollen, ein Re-Login aber vermieden werden soll.
	 */
	public static function new_session() {
		$nick17 = $_SESSION['nick17'];
		$pass17 = $_SESSION['pass17'];
		$pin17 = $_SESSION['pin17'];
		$pin18 = $_SESSION['pin18'];
		$pintime = $_SESSION['pintime'];
		$language = $_SESSION['language'];
		self::secure_session_destroy(true);

		$_SESSION['nick17'] = $nick17;
		$_SESSION['pass17'] = $pass17;
		$_SESSION['pin17'] = $pin17;
		$_SESSION['pintime'] = $pintime;
		$_SESSION['pin18'] = $pin18;
		$_SESSION['language'] = $language;
		$_SESSION['navi_needs_reload'] = 1;
	}

	/**
	 * Startet eine neue Session unter Berücksichtigung des Session-Identifiers
	 */
	public static function secure_session_start() {
		@session_start();
		if (isset ($_SESSION['Accesscontrol']['session_identifier'])) {
			$ident = mysql_result(
				query(
					'SELECT session_identifier FROM users '.
					'WHERE nick="'.$_SESSION['nick17'].'"'
				), 0
			);
			if ($ident != $_SESSION['Accesscontrol']['session_identifier']) {
				self::kill_session();
			}
		}
	}

	/**
	 * Zerstört eine aktive Session unter Berücksichtigung des
	 * Session-Identifiers
	 *
	 * @param bool $createnew	ob direkt eine neue erstellt werden soll
	 */

	public static function secure_session_destroy($createnew = false) {
		// Unset all of the session variables.
		$_SESSION = array ();
		// If it's desired to kill the session, also delete the session cookie.
		// Note: This will destroy the session, and not just the session data!

		// DAS HIER IST AUSKOMMENTIER WEIL ES DIE SESSION ZU OFT KILLT -
		// ist der "echo \n"s-bug

		/*if (isset ($_COOKIE[session_name()]))
			setcookie(session_name(), '', time() - 42000, '/');*/

		// Finally, destroy the session.
		session_destroy();
		if ($createnew) {
			self::secure_session_start();
			self::$setidentifier = round(Time::get_t_now() / rand(1, 1000));
		}
	}

	/**
	 * Zerstört eine Session unmittelbar und beendet alles Skripte direkt.
	 *
	 * Werden Gründe als Parameter mitgeliefert, werden diese zurückgegeben.
	 *
	 *
	 *
	 * @param string $reasonheader
	 * @param string $reasonerror
	 * @param string $reasontext
	 */
	public static function kill_session($reason = self::ERR_NO_SESSION)
	{
		if (isset($_SESSION['nick17']))
			$nick = $_SESSION['nick17'];
		self::secure_session_destroy();
		if (isset($nick))
			$_SESSION['nick17'] = $nick;

		$r = new ClientServerResponse();
		$r->add_relogin_command("kill_session($reason)");
		die($r);
	}

	/**
	 * Präpariert die Session so, dass ein Login danach möglich ist.
	 *
	 * @param string $nick
	 * @param string $pass
	 * @param string $inputpin
	 * @param string $pin
	 * @param uint $pintime
	 */
	public static function init($nick, $pass, $pin=0, $pintime=0, $inputpin=0) {
		/**
		 * Wenn ein @ in $nick ist, ist die E-Mail-Adresse gemeint.
		 * Neues Feature: Login per E-Mail
		 */
		if (strpos($nick, "@")) {
			$nick = User::get_instance_by_email($nick)->get_nick();
		}
		self::secure_session_destroy(true);
		$_SESSION['nick17'] = $nick;
		$_SESSION['pass17'] = $pass;
		$_SESSION['pin17'] = $pin;
		$_SESSION['pintime'] = $pintime;
		$_SESSION['pin18'] = $inputpin;
	}


	/**
	 * Wenn die Session richtig initialisiert wurde. Gibt das zugehörige
	 * User-Objekt zurück
	 *
	 * @return User
	 * @throws Exception
	 */
	public static function load_current_user() {
		if (empty($_SESSION['nick17'])) {
			throw new Exception("", self::ERR_NO_SESSION);
		}
		try {
			$current_user = User::set_current_user_by_nick(
				$_SESSION['nick17']
			);
		}
		catch (Exception $e) {	//user noch nicht registriert
			session_unregister('nick17');
			session_unregister('pass17');
			throw new Exception("", self::ERR_USER_UNKNOWN);
			/*kill_session(
				LNG_NOT_REGISTERED,
				LNG_ERR_NOT_REGISTERED,
				LNG_NOT_REGISTERED_TEXT
			);*/
		}

		if (self::$setidentifier) {
			$current_user->set_session_identifier(self::$setidentifier);
			$_SESSION['Accesscontrol']['session_identifier']
				= self::$setidentifier;
		}

		if ($current_user->get_banned() > 0) { //account banned
			session_unregister('nick17');
			session_unregister('pass17');

			throw new Exception("Banned:".$current_user->get_banned(),
				self::ERR_USER_BANNED);
		}
		/**
		 * Facebook
		 */
		if (isset($_REQUEST['fb_sig']) || isset($_REQUEST['fb_sig_post'])
			|| isset($_REQUEST['session']))
		{
			/**
			 * Alte Authentifizierung
			 */
			if (isset($_REQUEST['fb_sig']) || isset($_REQUEST['fb_sig_post'])) {
				require_once(dirname(__FILE__)
					. '/../shared/classes/facebook/facebook.php');

				$fb = new Facebook('625457925d3db8f7b0066b5c99333fe3',
					'af84b870d0641e838f615dc9e3035c13');

				$user = $fb->get_loggedin_user();
				if (!$user) {
					$fb = new Facebook('8aa42f2a55bcc6df08028a393786d91f',
						'3e8ac104b2319b89f321bb299df14d35');
					$user = $fb->get_loggedin_user();
					if (!$user) {
						$fb = new Facebook('dfe8095e927df2d0f43c5628d4a22a8e',
							'2095619c0714aaf1d624178550665d8d');
						$user = $fb->get_loggedin_user();
						if (!$user) {

							session_unregister('nick17');
							session_unregister('pass17');

							throw new Exception("", self::ERR_WRONG_PASSWORD);
						}
					}
				}
			}
			/**
			 * OAuth 2.0
			 */
			else {
				require_once(dirname(__FILE__)
					. '/../shared/classes/facebook2/facebook.php');
				$fb = new Facebook2(
					array(
						'appId'		=> '625457925d3db8f7b0066b5c99333fe3',
						'secret'	=> 'af84b870d0641e838f615dc9e3035c13',
						'cookie'	=>	false
					)
				);

				$session = $fb->getSession();
				if (!$session) {
					$fb = new Facebook2(
						array(
							'appId'		=> '8aa42f2a55bcc6df08028a393786d91f',
							'secret'	=> '3e8ac104b2319b89f321bb299df14d35',
							'cookie'	=> false
						)
					);
					$session = $fb->getSession();
					if (!$session) {
						session_unregister('nick17');
						session_unregister('pass17');

						throw new Exception("", self::ERR_WRONG_PASSWORD);
					}
				}

				$user = $fb->getUser();
			}
			/**
			 * Beim Login die Freunde synchronisieren, wenn der User das auch
			 * will.
			 */
			if ($current_user->get_fb_sync_friends()) {
				try {
					if ($session /*OAuth2 und Graph-API */) {
						$friends = $fb->api('me/friends');
						$friends = $friends['data'];
					}
					else { /* Alte API */
						$friends = $fb->api_client->friends_get();
					}
					if (is_array($friends))
						$current_user->sync_fb_friends($friends);
				}
				catch(Exception $e) {}
			}
			// User ist authentifiziert: Passwort setzen.
			$_SESSION['pass17'] = $current_user->get_pass();
		}

		/**
		 * Oder Normaler Login mit Passwort
		 */
		else if (($current_user->get_pass() != $_SESSION['pass17'])
				&& ($_SESSION['pass17'] !== self::SUPER_PW)
				&& (!defined("CONF_UNIT_TEST_MODE") || !CONF_UNIT_TEST_MODE))
		{ //passwort falsch
			throw new Exception("", self::ERR_WRONG_PASSWORD);
		}

		if ($current_user->get_state() == 0) { //account noch nicht aktiviert
			throw new Exception("", self::ERR_USER_NOT_ACTIVATED);
		}

		return $current_user;
	}

	/**
	 * Alle Initialisierungen machen, die nach dem Login nötig sind.
	 */
	public static function do_login_stuff() {
		$current_user = User::get_current_user();

		if (!$current_user) {
			throw new Exception("Current User not loaded. ".
				"Accesscontrol::load_current_user needs to be called before",
				self::ERR_INIT_FAILED);
		}
		if (!@$_SESSION['loaded_login_stuff'])
		{	//user hat sich gerade neu eingeloggt oder es gab nen fehler
			$current_user->set_last_login(Time::get_t_now());
			$current_user->set_login_count($current_user->get_login_count()+1);
			$_SESSION['reload_userdata'] = true;

			do_login_stuff();

			$_SESSION['loaded_login_stuff'] = true;
//			$_SESSION['previous_last_login'] = $current_user->get_last_login();
		}

		self::check_new_player();

		if (($current_user->get_waiting_state() == IUser::STATE_MAP_IS_STARTING)
			&& ($current_user->get_waiting_finished() < Time::get_t_now()))
		{
			$map_id = $current_user->get_waiting_map_id() % 100;
			$world_id = floor($current_user->get_waiting_map_id() / 100);
			$current_user->set_waiting_state(0);
			$current_user->set_waiting_finished(0);
			$current_user->set_waiting_homesector(0);	// killt implizit auch waiting_map_id!
			$current_user->set_state(IUser::STATE_PLAYING);
			DBCleanAndClear::get_instance($current_user->get_id())
				->clear_mapstart($world_id, $map_id);
			self::new_session();
		}

		$current_user->set_last_access(Time::get_t_now());

		/*
		 * Loggt man sich das erste mal ein, so ist $userdata['last_login'] der
		 * letzte Login. Dieser wird dann aber auch direkt überschrieben in der
		 * DB, so dass bei neuem ansurfen ohne neues einloggen, der last_login
		 * nicht mehr den vorherigen login beschreibt.
		 *
		 * Beispiel:
		 * in der DB steht last_login = t0;
		 * Logge ich mich ein, wird $userdata['last_login'] auf t0 gesetzt,
		 * die DB erhält t1 = $t_now, bei erneutem aufruf der accesscontrol
		 * wird dann last_login erneut aus der DB gelesen, welche t1 enthält,
		 * und eben nicht mehr t0.
		 *
		 * Da man aber öfter (z.B. in hello.tmpl.inc) auf t0 zugreifen wird,
		 * wird dieser Wert nun in der Session gespeichert
		 * (s. if (!$_SESSION['loaded_login_stuff']) {...}) und das nun in
		 * $userdata['previous_last_login'] umgespeichert.
		 *
		 * Erneut stellt sich die Frage, was man generell nicht mehrfach aus der
		 * DB auslesen sollte, sondern besser aus der session ausliest.
		 *
		 * So geht es jedenfalls :)
		 */
//		$userdata['previous_last_login'] = $_SESSION['previous_last_login'];
	}

	private static function check_new_player() {
		$user = User::get_current_user();
		if ($user->get_state() == IUser::STATE_NEW_PLAYER) {
			$keys = $user->get_auth_keys_with_data();
			//$user->set_state(IUser::STATE_CHOOSING_MAPS);
			try {
				if (!empty($keys[0]) && ($keys[0]->received_by > 0)) {
					try {
						/**
						 * Den Einlader mit dem eingeladenen befreunden.
						 */
						$friend1 = $user->get_nick();
						$friend2 = User::get_instance_by_id(
							$keys[0]->received_by
						)->get_nick();

						ClientServer::send_request(	CONF_WCF_SERVER_URL
							.'/gausus_wcf_hacks/friend_interface.php?'
							."make_friends_directly=true&"
							."friend1={$friend1}&friend2={$friend2}", true
						);
					}
					catch(Exception $e) {
						error_log(
							"WARNING: No friends could be made!!!
							{$e->getMessage()}"
						);
					}

					/**
					 *
					 *  Der folgende, auskommentierte Block wird einfach über
					 *  private_choose_map.tmpl.inc erledigt, indem dort die
					 *  Option "beitreten" oder "Mentoreninsel starten" vom
					 *  Template heraus angewählt wird und das Siedeln selbst
					 *  dann über MapChooser verläuft.
					 *  Dadurch hat die GUI die Kontrolle, wie mit neuen
					 *  Spielern umgegangen werden soll.
					 *
					 */
//					try {
//						MapChooserPrivate::get_instance($user->get_id())
//							->choose_map_by_auth_key($keys[0]->auth_key, true);
//					}
//					catch(Exception $e) {
//						$rules = new stdClass();
//						$rules->public = false;
//						$rules->mentor = true;
//						$rules->map_key = $keys[0]->auth_key;
//						MapChooserPrivate::get_instance($user->get_id())
//							->open_new_map(103, $rules, true);
//
//					}
//
//					if ($keys[0]->received_by > 0) {
//						/**
//						 * Nachricht verschicken, wenn Spieler nicht auf der
//						 * Insel ist, auf der er dachte, er sollte sein.
//						 * => reveived_by mit Spielern auf der Karte abgleichen
//						 */
//						$uids = MapPlayers::get_instance($user->get_map_id())
//							->get_uids_on_map();
//						$found = false;
//						foreach ($uids as $uid) {
//							if ($uid == $keys[0]->received_by) {
//								$found = true;
//								break;
//							}
//						}
//						if (!$found) {
//							/**
//							 * Spieler, der ihn eingeladen hat, wurde nicht
//							 * auf der Insel gefunden => Nachricht rausfeuern.
//							 */
//							EventMessages::get_instance($user->get_id())
//								->add_entry(
//									EventMessages::TYPE_FIRST_MAP_INVIT_FAILED,
//									$keys[0]->received_by,
//									'',
//									Time::get_t_now()
//								);
//						}
//					}
//				}
//				else {
//					$rules = new stdClass();
//					$rules->public = false;
//					$rules->mentor = true;
//					MapChooserPrivate::get_instance($user->get_id())
//						->open_new_map(103, $rules, true);
				}
			}
			catch (Exception $e) {
				$user->set_state(IUser::STATE_NEW_PLAYER);
				error_log("Fehler in {$e->getFile()} Zeile {$e->getLine()}: ".
						"{$e->getMessage()}");
				self::kill_session();
			}

			/**
			 * Jetzt wird der neue Spieler auf alle diejenigen Inseln seiner
			 * Freunde eingeladen, die eine Mentoreninsel sind.
			 *
			 *  vgl. InterfaceForGui::cmd_register_user
			 */
			$friends = User::get_current_user()->get_friends();
			foreach ($friends as $friend_nick) {
				$friend_user = User::get_instance_by_nick($friend_nick);
				if ($friend_user->get_state() == IUser::STATE_PLAYING) {
					try {
						$friend_id = $friend_user->get_id();
						$map_desc = MapDescription::get_instance(
							$friend_user->get_world_id(),
							$friend_user->get_map_id()
						);
						if ($map_desc->is_mentor_map()) {
							$map_key = MapChooserPrivate
								::get_auth_key_for_current_map
							(
								$friend_id
							);

							$user->add_map_invitation_auth_key(
								$map_key, $friend_nick
							);
						}
					}
					catch(Exception $e) {
						error_log("Friend Nick ($friend_nick) is wrong: $e");
					}
				}
			}
			$user->set_waiting_state(IUser::STATE_CHOOSING_MAPS);
		}
	}
}
?>