<?php
require_once (dirname(__FILE__).'/Db.inc');
require_once (dirname(__FILE__).'/map/MapDescription.inc');
require_once(dirname(__FILE__).'/host/HostControl.inc');
require_once (dirname(__FILE__).'/../shared/classes/IUser.inc');
require_once (dirname(__FILE__).'/../shared/classes/Language.inc');
require_once(dirname(__FILE__).
						'/../shared/classes/score/GameScoreCalculation.inc');
require_once (dirname(__FILE__).'/../shared/classes/helper/Email.inc');
Language::get_instance()->load_language_file('User');

/**
 * Spiegelt einen User mit all seinen Eigenschaften wider. [users-Tabelle]
 */

class User implements IUser {
	private $user_data = array();
	private $data_changed = false;
	private $brezeln = 0;
	private $last_home_sector_c = 0;

	private static $READ_ONLY_ATTRIBS = array('id',
		'nick', 'initial_email', 'last_score'
	);

	private static $current_user = null;

	/**
	 * Instanzen werden hier gespeichert zur Instanzüberwachung. Jede Instanz wird
	 * mehrfach gespeichert: einmal pro Nick,damit man eine User-Instanz anhand
	 * des Nicks effizient finden kann, und einmal pro Mode nach ID [also
	 * einmal pro FFA-ID und einmal pro Clan-ID].
	 */
	private static $INSTANCES;

	/**
	 * Datenbankobjekt zur Datenbank mit den persönlichen Userdaten.
	 * @var Db
	 */
	private static $user_personal_db;

	/**
	 * @return Db
	 */
	private static function get_user_personal_db() {
		global $pv_db;
		if (Db::USER_PERSONAL_HOST == Db::GAME_HOST) {
			if (!$pv_db) {
				global $con;
				$con = connect_db();
			}
			return $pv_db;
		}

		if (!self::$user_personal_db)
			self::$user_personal_db = new Db(Db::USER_PERSONAL_HOST,
				Db::GAME_NAME, Db::GAME_PW, Db::USER_PERSONAL_SELECT_DB);
		return self::$user_personal_db;
	}

	/**
	 * Gibt die Instanz eines Users-Objektes mit der User-ID $id zurück.
	 * Automatische Instanzüberwachung puffert und spart somit DB-Zugriffe.
	 * Wird $forced gesetzt, wird der Puffer übergangen, und in jedem Fall neu
	 * aus der DB die Daten gelesen. Diese Variante ist mit bedacht zu wählen
	 * und i.d.R. unnötig
	 * Hier kann auch die ID des Host-Users übergeben werden, um diesen virtuellen
	 * User als Objekt zu bekommen.
	 * @param int $id 		ID in der Datenbank des Users
	 * @param bool $forced	ob gezwungen aus der DB geladen werden soll
	 * @return User
	 */
	public static function get_instance_by_id($id, $forced=false) {
		$id_old = $id;
		$id = (int)$id;
		if ($id <= 0) {
			error_log(print_r(debug_backtrace(false),true));
			throw new IllegalArgumentException("id", "N\{0}", $id);
		}
		if (!isset(self::$INSTANCES[$id])) {
			$new_user = new User($id);
			self::$INSTANCES[$id] = $new_user;
			self::$INSTANCES[strtolower($new_user->get_nick())] = $new_user;
		}
		return self::$INSTANCES[$id];
	}

	/**
	 * Gibt die Instanz eines Users-Objektes mit dem Nick $nick zurück.
	 * Automatische Instanzüberwachung puffert und spart somit DB-Zugriffe.
	 * Wird $forced gesetzt, wird der Puffer übergangen, und in jedem Fall neu
	 * aus der DB die Daten gelesen. Diese Variante ist mit bedacht zu wählen
	 * und i.d.R. unnötig.
	 * Hierüber kann KEIN virtueller User geholt werden, da die virtuellen
	 * User KEINEN EINDEUTIGEN Nick haben!
	 * @param $nick		Nick in der Datenbank des Users
	 * @param $forced	ob gezwungen aus der DB geladen werden soll
	 * @return User
	 */
	public static function get_instance_by_nick($nick, $forced=false) {
		if (!isset(self::$INSTANCES[strtolower($nick)])) {
			$new_user = new User($nick);
			self::$INSTANCES[$new_user->get_id()] = $new_user;
			self::$INSTANCES[strtolower($new_user->get_nick())] = $new_user;
		}
		return self::$INSTANCES[strtolower($nick)];
	}

	/**
	 * Gibt die Instanz eines Users-Objektes mit der E-Mail $email zurück.
	 *
	 * Automatische Instanzüberwachung puffert und spart somit DB-Zugriffe.
	 * Wird $forced gesetzt, wird der Puffer übergangen, und in jedem Fall neu
	 * aus der DB die Daten gelesen. Diese Variante ist mit bedacht zu wählen
	 * und i.d.R. unnötig.
	 *
	 * ACHTUNG: Diese Variante wird nicht gepuffert! Es wird immer eine
	 * DB-Abfrage gemacht.
	 * Hierüber kann KEIN virtueller User geholt werden, da die virtuellen
	 * User KEINE EINDEUTIGE E-Mail-Adresse haben!
	 * @param string $email		E-Mail-Adresse in der Datenbank des Users
	 * @param bool $forced	ob gezwungen aus der DB geladen werden soll
	 * @return User
	 */
	public static function get_instance_by_email($email, $forced=false) {
		$nick = @mysql_result(
			query("SELECT nick FROM users WHERE email='$email' LIMIT 1"),0
		);
		if (!$nick)
			throw new Exception("", IUser::ERR_USER_DOES_NOT_EXIST);
		return self::get_instance_by_nick($nick, $forced);
	}

	/**
	 * Gibt den Mammun-Usernamen anhand der Facebook-ID zurück.
	 *
	 * ACHTUNG: Diese Variante wird nicht gepuffert! Es wird immer eine
	 * DB-Abfrage gemacht.
	 *
	 * Es kann statt einer ID auch ein Array von IDs übergeben werden. In diesem
	 * Fall wird ein assotiatives Array zurückgegeben, dass jeder der
	 * übergebenen Facebook-IDs den entsprechenden Nick zuweist. Fehlt eine
	 * FB-ID scheinbar im Ergebnis als Key, so ist dieser ID kein User in Mammun
	 * zugeordnet, der FB-Spieler spielt also nicht.
	 *
	 * Hierüber kann KEIN virtueller User geholt werden, da die virtuellen
	 * User KEINE EINDEUTIGE Facebook-ID haben!
	 * @param uint/uint[] $fb_id
	 * @param bool $forced	ob gezwungen aus der DB geladen werden soll
	 * @return User
	 * @throws Exception	Wenn nur eine $fb_id übergeben, dem aber kein Nutzer
	 * 						zugewiesen werden kann
	 */
	public static function get_nick_by_fb_id($fb_id, $forced=false) {
		$select_db = Db::USER_PERSONAL_SELECT_DB;
		if (defined('TEST_SERVER')) {
			$select_db .= '_test';
		}

		if (!is_array($fb_id)) {
			$fb_id = (int)$fb_id;
			$nick = @mysql_result(
				self::get_user_personal_db()->query(
					"SELECT nick FROM $select_db.user_personal ".
					"WHERE id_fb=$fb_id LIMIT 1"
				),0
			);

			if (!$nick)
				throw new Exception("", IUser::ERR_USER_DOES_NOT_EXIST);
			return $nick;
		}
		else {
			foreach ($fb_id as &$i)
				$i = (int)$i;
			$res = self::get_user_personal_db()->query(
				"SELECT nick, id_fb AS fb_id FROM $select_db.user_personal ".
				'WHERE id_fb IN ('.implode(',', $fb_id).')'
			);
			$result = null;
			while ($row = mysql_fetch_object($res)) {
				$result[$row->fb_id] = $row->nick;
			}
			return $result;
		}
	}

	/**
	 * Gibt zurück, wie viele User registriert sind.
	 * ACHTUNG: Diese Ausgabe erzeugt immer einen DB-Zugriff!
	 *
	 * @return uint
	 */
	public static function get_registered_user_amount($fb=false) {
		if ($fb) {
			$select_db = Db::USER_PERSONAL_SELECT_DB;
			if (defined('TEST_SERVER')) {
				$select_db .= '_test';
			}
			return @mysql_result(self::get_user_personal_db()->query(
				"SELECT COUNT(*) FROM $select_db.user_personal WHERE id_fb>0 "
			), 0);
		}
		else {
			return @mysql_result(query(
				"SELECT COUNT(*) FROM users"
			),0);
		}
	}

	/**
	 * Gibt zurück, wie viele User gerade aktiv sind.
	 * ACHTUNG: Diese Ausgabe erzeugt immer einen DB-Zugriff!
	 *
	 * @param uint $duration
	 * @return uint
	 */
	public static function get_active_user_amount($duration=0, $fb=false) {
		if (!$duration)
			$duration = 7*24*3600;
		if ($fb) {
			$select_db = Db::USER_PERSONAL_SELECT_DB;
			if (defined('TEST_SERVER')) {
				$select_db .= '_test';
			}
			return @mysql_result(query (
				"SELECT COUNT(*) FROM users as u, ".
								"$select_db.user_personal AS up ".
					"WHERE u.login_count>5 AND u.last_access>".
						(Time::get_t_now()-$duration) . " AND u.state!=0 ".
						"AND u.id=up.id AND up.id_fb>0"
			),0);
		}
		else {
			return @mysql_result(query(
				"SELECT COUNT(*) FROM users WHERE ".
					"login_count>5 AND last_access>".
					(Time::get_t_now()-$duration) . " AND state!=0"
			),0);
		}
	}

	/**
	 * Gibt zurück, wie viele User gerade online sind.
	 * ACHTUNG: Diese Ausgabe erzeugt immer einen DB-Zugriff!
	 *
	 * @param uint $latency
	 * @return uint
	 */
	public static function get_online_user_amount($latency=60) {
		return @mysql_result(query(
			"SELECT COUNT(*) FROM users WHERE state>0 AND banned=0 ".
				"AND last_access!=2000000000 AND last_access>".
				(Time::get_t_now()-$latency)
		),0);

	}

	/**
	 * Gibt alle IDs der User zurück, die gerade aktiv sind.
	 * ACHTUNG: Diese Ausgabe erzeugt immer einen DB-Zugriff!
	 *
	 * @return uint[]
	 */
	public static function get_user_ids_active($duration=0) {
		if (!$duration)
			$duration = 7*24*3600;
		$res = query(
			"SELECT id FROM users WHERE last_access>".
				(Time::get_t_now()-$duration) . " AND state!=0"
		);
		$result = array();
		while ($row = mysql_fetch_row($res)) {
			$result[] = $row[0];
		}
		return $result;
	}


	/**
	 * Gibt alle User zurück, die auf einer Map warten.
	 * @param int $map_id	ID der Karte, auf der die User warten
	 * @return User[]
	 */
	public static function get_user_ids_waiting_on_map($world_id, $map_id) {
		$map_id = $world_id * 100 + $map_id;
		$res = query("SELECT * FROM users WHERE floor(waiting_homesector/100)=$map_id");
		$result = array();
		while ($user_data = mysql_fetch_assoc($res)) {
			if (self::$INSTANCES[strtolower($user_data['nick'])])
				$result[] = $user_data['id'];
			else if (self::$INSTANCES[$user_data['id']])
				$result[] = $user_data['id'];
			else {
				$new_user = new User($user_data);
				self::$INSTANCES[$new_user->get_id()] = $new_user;
				self::$INSTANCES[strtolower($new_user->get_nick())] = $new_user;
				$result[] = $user_data['id'];
			}
		}
		return $result;
	}

	/**
	 * Setzt den aktuellen User, also den, der gerade alle Dateien des Spiels
	 * benutzt, durch die übergebene ID
	 * @param int 	User-ID
	 * @throws Exception	wenn User nicht exisitiert
	 * @return User		den durch die ID definierten, neu gesetzten User
	 */
	public static function set_current_user_by_id($id) {
		self::$current_user = self::get_instance_by_id($id);
		return self::$current_user;
	}

	/**
	 * Setzt den aktuellen User, also den, der gerade alle Dateien des Spiels
	 * benutzt, durch den übergebenen Nickname
	 * @param string	Nick des Users
	 * @throws Exception	wenn User nicht exisitiert
	 * @return User		den durch den Nick definierten, neu gesetzten User
	 */
	public static function set_current_user_by_nick($nick) {
		self::$current_user = self::get_instance_by_nick($nick);
		return self::$current_user;
	}

	/**
	 * Gibt den aktuellen User zurück, also den, der gerade alle Dateien des
	 * Spiels benutzt.
	 * @return User
	 */
	public static function get_current_user() {
		return self::$current_user;
	}

	/**
	 * Gibt den virtuellen Host-User der Karte zurück
	 *
	 * @return User
	 */
	public static function get_host_user() {
		return self::get_instance_by_id(HostControl::USER_HOST_ID);
	}

	/**
	 * Erstellt ein neues User-Objekt. Zur eindeutigen Identifizierung des Users
	 * kann als einziger Parameter entweder der Nick oder die ID des Users
	 * übergeben werden, sowie die E-Mail oder gleich alle Daten der DB.
	 *
	 * @param int/string/array $data	ID, Nick oder E-Mail des Users, bzw User-Daten
	 * @throws Exception	wenn User nicht exisitiert
	 */
	private function __construct($data) {
		if (!is_array($data) && isset($_SESSION['User'][$data])
			&& is_array($_SESSION['User'][$data]) && false)
		{
			$this->user_data = $_SESSION['User'][$data]['user_data'];
		}
		else {
			if (is_array($data)) {
				/**
				 * Achtung! Das ist ne gefährliche Kiste. Es wird nicht geprüft,
				 * ob Daten korrekt oder vollständig. Bevor Konstruktor also
				 * mit nem Daten-Array aufgerufen wird, MUSS sichergestellt werden,
				 * dass die Daten auch aus einem vorherigen "SELECT * FROM users"
				 * hervorgingen (s. self::get_users_waiting_on_map).
				 */
				$user_data = $data;
			}
			else if (is_anint($data)) {
				$id = $data;
				if ($id != HostControl::USER_HOST_ID) {	// normaler User
					$user_data = @mysql_fetch_assoc(
						query("SELECT * FROM users WHERE id=$id")
					);
				}
				else {	// virtueller Host-User
					/*$map_id = $id - HostControl::USER_HOST_ID;
					$world_id = floor($map_id/100);
					$map_id = $map_id % 100;
					$homesector = MapDescription::get_instance($world_id,
						$map_id)->get_homesector_of_host();
					if ($homesector === false)
						$homesector = 0;
					$homesector += 100*$map_id;*/
					$homesector = 0;
					$user_data = array
						(
							'nick'				=>	'-',	// symbolischer Nick für diesen User
							'homesector' 		=> $homesector,
							'id'				=> $id,
							'state'				=> IUser::STATE_PLAYING,
							'waiting_state'		=> 0,
						);
				}

			}
			else {
				if (strpos($data, "@")) {
					$email = $data;
					$user_data = @mysql_fetch_assoc(
						query("SELECT * FROM users WHERE email='$email' LIMIT 1")
					);
				}
				else {
					$nick = $data;
					$user_data = @mysql_fetch_assoc(
						query("SELECT * FROM users WHERE nick='$nick' LIMIT 1")
					);
				}
			}

			if (empty($user_data)) {
				throw new Exception("$data", IUser::ERR_USER_DOES_NOT_EXIST);
			}

			$this->user_data = $user_data;
		}
	}


	function __destruct() {
		if ($this->data_changed) {
			$sql = 'UPDATE users SET ';
			foreach ($this->user_data as $key => $value) {
				$sql .= "$key='$value',";
			}
			$sql{strlen($sql)-1} = ' ';
			$sql .= "WHERE id={$this->user_data['id']}";
			query($sql);
			$this->data_changed = false;
		}
		$this_data = array(
			'user_data'		=>	$this->user_data
		);
	}

	/**
	 * Die meisten Getter werden hier dynamisch erstellt. Es ist ein Abbild der Datenbank
	 * Beispiel: User::get_instance($id)->get_nick();
	 * Ebenso die Setter.
	 * Die Daten werden direkt im Objekt, aber erst beim Destruieren des Objektes
	 * auch in die DB und Session geschrieben.
	 *
	 * Gewisse Attribute dürfen nicht geändert werden (z.B. id, nick o.ä.).
	 * Dafür gibt es dann auch keine Setter.
	 */

	public function __call($fn_name, array $params) {
		if (strpos($fn_name, 'get_') === 0) {	// "->get_[...]"
			$what_to_get = substr($fn_name, 4);
			if (!isset($this->user_data[$what_to_get])) {
				trigger_error(sprintf('Call to undefined function: %s::%s().', get_class($this), $fn_name), E_USER_ERROR);
			}
			return $this->user_data[$what_to_get];
		}
		else if (strpos($fn_name, 'set_') === 0) {
			$what_to_set = substr($fn_name, 4);
			if (!isset($this->user_data[$what_to_set]) || in_array($what_to_set, self::$READ_ONLY_ATTRIBS))
				trigger_error(sprintf('Call to undefined function: %s::%s().', get_class($this), $fn_name), E_USER_ERROR);
			$this->user_data[$what_to_set] = $params[0];
			$this->data_changed = true;
		}
		else {
			trigger_error(sprintf('Call to undefined function: %s::%s().', get_class($this), $fn_name), E_USER_ERROR);
		}
	}

	/**
	 * Gibt die ID der Karte zurück, auf der der Spieler das letzte mal war.
	 *
	 * @return uint
	 */
	public function get_last_map_id() {
		if (!$this->last_home_sector_c) {
			$mp = MapPlayers::get_last_map_instance($this->get_id());
			$this->last_home_sector_c = $mp->get_homesector_c($this->get_id());
		}
		return (int)$this->last_home_sector_c->get_map_id();
	}

	/**
	 * Gibt die ID der Welt zurück, in der die Karte sich befindet, auf der der
	 * Spieler das letzte mal war.
	 *
	 * @return uint
	 */
	public function get_last_world_id() {
		if (!$this->last_home_sector_c) {
			$mp = MapPlayers::get_last_map_instance($this->get_id());
			$this->last_home_sector_c = $mp->get_homesector_c($this->get_id());
		}
		return (int)$this->last_home_sector_c->get_world_id();
	}

	/**
	 * Gibt die ID des Sektors zurück von dem aus der Spieler auf der letzten
	 * Karte aus gestartet ist.
	 *
	 * @return uint
	 */
	public function get_last_homesector_id() {
		if (!$this->last_home_sector_c) {
			$mp = MapPlayers::get_last_map_instance($this->get_id());
			$this->last_home_sector_c = $mp->get_homesector_c($this->get_id());
		}
		return (int)$this->last_home_sector_c->get_sector_id();
	}

	/**
	 * Gibt die ID der Karte zurück, auf die der Spieler sich gerade befindet.
	 *
	 * Spielt er gerade auf keiner Karte, wird die ID der Karte zurückgegeben,
	 * auf der er zuletzt war.
	 *
	 * Explizit definierter Getter, da die map_id nicht in der Tabelle existiert,
	 * sondern in das homesector-Feld mit einkodiert ist.
	 * @return int
	 */
	public function get_map_id() {
		if ($this->get_state() == self::STATE_PLAYING)
			return floor($this->get_homesector()/100)%100;
		else
			return $this->get_last_map_id();
	}

	/**
	 * Gibt die ID der Welt, auf der der Spieler sich gerade befindet.
	 *
	 * Spielt er gerade in keiner Welt, wird die ID der Welt zurückgegeben,
	 * auf der er zuletzt war.
	 *
	 * Explizit definierter Getter, da die world_id nicht in der Tabelle existiert,
	 * sondern in das homesector-Feld mit einkodiert ist.
	 * @return int
	 */
	public function get_world_id() {
		if ($this->get_state() == self::STATE_PLAYING)
			return floor($this->get_homesector()/10000);
		else
			return $this->get_last_world_id();
	}

	/**
	 * Gibt die ID der Karte zurück, auf die der Spieler gerade wartet.
	 * Explizit definierter Getter, da die waiting_map_id nicht in der Tabelle existiert,
	 * sondern in das waiting_homesector-Feld mit einkodiert ist.
	 * @return int
	 */
	public function get_waiting_map_id() {
		return floor($this->get_waiting_homesector()/100);
	}

	/**
	 * Gibt das aktuelle Level des Users zurück.
	 * Explizit definierter Getter, da das aktuelle level über die aktuelle
	 * Punktzahl errechnet wird und nicht in der User-Tabelle explizit vermerkt
	 * ist.
	 *
	 * @return uint
	 */
	public function get_cur_level() {
		return GameScoreCalculation::calc_level_by_score($this->get_cur_score());
	}

	/**
	 * Gibt die aktuelle Punktzahl des Users zurück.
	 * Explizit definierter Getter, da die aktuelle Punktzahl nicht in der
	 * Users-Tabelle steht, sondern anderweitig berechnet wird.
	 *
	 * @return float
	 */
	public function get_cur_score() {
		return LeagueScore::get_instance(0)->get_score($this->get_id());
	}

	/**
	 * Gibt das alte Level, also das, das er bei Beginn der aktuellen oder gerade
	 * vergangenen (wenn keine aktuelle) Map hatte, zurück.
	 * Explizit definierter Getter, da das Level über die Punktzahl berechnet wird
	 * und nicht in der Users-Tabelle steht.
	 *
	 * @return uint
	 */
	public function get_last_level() {
		return GameScoreCalculation::calc_level_by_score($this->get_last_score());
	}

	/**
	 * Speichert die aktuelle Punktzahl im last_score-Puffer bis dieser wieder
	 * (durch aufruf dieser Funktion) überschrieben wird.
	 *
	 * Dieser Puffer wird dazu genutzt, um nach einer Karte zu schauen, wie
	 * viele Punkte der Spieler auf einer Karte gemacht hat.
	 */
	public function set_last_score_to_cur_score() {
		$this->user_data['last_score'] =
			LeagueScore::get_instance(0)->get_score($this->get_id());
	}



	/**
	 * Fügt den Tutorial Flag den vorhandenen hinzu.
	 *
	 * @param uint $flag
	 */
	public function add_tutorial($flag) {
		$this->set_tutorial($this->get_tutorial() | $flag);
		return $this->get_tutorial();
	}

	/**
	 * Setzt das Passwort neu.
	 *
	 * @param string $pass
	 */
	public function set_pass($pass) {
		$pass_md5 = md5($pass);
		$this->user_data['pass'] = $pass_md5;
		$this->data_changed = true;

		/**
		 * Direct Community-Access.
		 */
		$community_db = new Db(Db::COM_HOST, Db::COM_NAME, Db::COM_PW,
			Db::COM_SELECT_DB);
		$community_db->query(
			"UPDATE wcf1_user SET password='$pass_md5' ".
			"WHERE username='{$this->get_nick()}'"
		);
	}

	/**
	 * Verbindet den User mit dem Facebook-Account identifiziert durch $fb_id.
	 *
	 * @param uint $fb_id
	 */
	public function connect_with_fb($fb_id) {
		$select_db = Db::USER_PERSONAL_SELECT_DB;
		if (defined('TEST_SERVER')) {
			$select_db .= '_test';
		}
		$cur_fb_id = mysql_result(self::get_user_personal_db()->query(
				"SELECT id_fb FROM $select_db.user_personal ".
				"WHERE id={$this->get_id()}"
		),0);
		if ($cur_fb_id)
			throw new Exception("", self::ERR_USER_IS_ALREADY_CONNECTED_TO_FB);

		self::get_user_personal_db()->query(
			"UPDATE $select_db.user_personal SET id_fb=$fb_id ".
			"WHERE nick='{$this->get_nick()}'"
		);
	}

	/**
	 * Synchronisiert Facebook-Freunde mit Community-Freunde
	 */
	public function sync_fb_friends(array $_friends) {
		if ($this->get_fb_sync_friends()) {
			$n = count($_friends);
			$friends = array();
			for ($i = 0; $i < $n; $i++) {
				if (is_object($_friends[$i]))
					$_friends[$i] = $_friends[$i]->id;
				else if (is_array($_friends[$i]))
					$_friends[$i] = $_friends[$i]['id'];
				if ($_friends[$i] > 0)
					$friends[] = $_friends[$i];
			}

			/**
			 * 1. Schritt
			 * In der Mammun-DB die Facebook-Freunde finden
			 */
			$select_db = Db::USER_PERSONAL_SELECT_DB;
			if (defined('TEST_SERVER')) {
				$select_db .= '_test';
			}
			$res = self::get_user_personal_db()->query(
				"SELECT nick FROM $select_db.user_personal ".
				"WHERE id_fb IN ('".implode("', '", $friends)."') AND id_fb!=0"
			);
			if (mysql_num_rows($res) == 0)
				return;

			/**
			 * 2. Schritt
			 * Für diese Freunde herausfinden, ob sie auch sync wollen
			 */
			$friend_nicks = array();
			while ($row = mysql_fetch_row($res)) {
				$friend_nicks[] = $row[0];
			}

			$res = query(
				"SELECT nick, fb_sync_friends AS fb_sync FROM users ".
				"WHERE nick IN ('".implode("', '", $friend_nicks)."') "
			);
			$fb_syncs = null;
			while ($row = mysql_fetch_object($res)) {
				$fb_syncs[$row->nick] = $row->fb_sync;
			}

			/**
			 * 3. Schritt
			 * Freundschaften erstellen
			 */
			/**
			 * Direct Community-Access.
			 */
			$pd = $this->get_personal_data();
			$my_wcf_id = $pd->id_wcf;

			$community_db = new Db(Db::COM_HOST, Db::COM_NAME, Db::COM_PW,
				Db::COM_SELECT_DB);

			$res = $community_db->query(
				"SELECT username AS nick, userID AS uid FROM wcf1_user ".
				"WHERE username IN ('".implode("', '", $friend_nicks)."')"
			);

			$sql =
				"INSERT IGNORE INTO wcf1_user_whitelist ".
				"(userID, whiteUserID, confirmed, time) VALUES ";

			$t_now = Time::get_t_now();
			$first_value = true;
			while ($row = mysql_fetch_object($res)) {
				if ($first_value) {
					$first_value = false;
				}
				else {
					$sql .= ',';
				}
				$fb_sync = $fb_syncs[$row->nick];

				if ($fb_sync) {
					/**
					 * Freundschaft direkt schließen, wenn beide mit Sync
					 * einverstanden sind
					 */
					$sql .= "({$row->uid}, $my_wcf_id, 1, $t_now), ";
					$sql .= "($my_wcf_id, {$row->uid}, 1, $t_now) ";
				}
				else {
					/**
					 * Sonst nur ein Freundschaftsangebot schicken
					 */
					$sql .= "($my_wcf_id, {$row->uid}, 0, 0) ";
				}
			}

			if (!$first_value)	// Mindestens ein Hit
				$community_db->query($sql);
		}
	}

	/**
	 * Gibt alle Mammun-Freunde des Users als Array von Nicks zurück
	 *
	 * @return string[]
	 */
	public function get_friends() {
		$buddies = ClientServer::send_request(
			CONF_WCF_SERVER_URL.'/gausus_wcf_hacks/friend_interface.php'
			.'?get_friends='.User::get_current_user()->get_nick());
		$buddies = $buddies[0];

		$buddy_list = array();
		foreach ($buddies as $type => $buddy_arr) {
			foreach ($buddy_arr as $buddy) {
				$buddy_list[] = $buddy->nick;
			}
		}
		return $buddy_list;
	}

	/**
	 * Sendet dem Nutzer eine E-Mail.
	 *
	 * Wrapper von pv_mail.
	 *
	 * @param string $topic   Titel der Mail
	 * @param string $message Nachricht der Mail
	 * @param string $from    Absender (optional, default: noreply@...)
	 */
	public function send_email($topic, $message, $from = false) {
		pv_mail($this->get_email(), $topic, $message, $from);
	}

	/**
	 * Fügt dem Spieler einen Authentifizierungsschlüssel hinzu, der ihm den
	 * Zugang zu einer Karte erlaubt.
	 *
	 * Wenn er später einer reservierten Karte beitreten will, kann er das
	 * automatisch tun, weil der auth_key hier hinterlegt wurde.
	 *
	 * Zusätzlich kann noch angegeben werden, welcher User ihm den Auth-Key
	 * geschickt hat.
	 *
	 * @param char[6] $auth_key
	 * @param uint $received_by
	 */
	public function add_map_invitation_auth_key($auth_key, $received_by=0) {
		if (strlen($auth_key) != 6) {
			throw new IllegalArgumentException("auth_key", "char[6]",
				$auth_key);
		}
		$received_by = (int)$received_by;
		query(
			"INSERT IGNORE INTO users_map_keys ".
			"SET uid={$this->get_id()}, ".
				"auth_key='$auth_key', received_by=$received_by"
		);
	}

	/**
	 * Entfent einen Authkey vom User.
	 *
	 *
	 * @param unknown_type $auth_key
	 * @see User#add_map_invitation_auth_key
	 */
	public function remove_map_invitation_auth_key($auth_key) {
		query(
			"DELETE FROM users_map_keys ".
			"WHERE uid={$this->get_id()} AND auth_key='$auth_key'"
		);
	}

	/**
	 * Gibt zurück, wie viele Brezeln der Spieler gesammelt hat.
	 *
	 * @return uint
	 */
	public function get_brezeln() {
		return (int)@mysql_result(
			query(
				"SELECT amount FROM users_brezeln WHERE uid={$this->get_id()}"
			),0
		);
	}

	/**
	 * Gibt dem Nutzer die angegebene Anzahl Brezeln.
	 *
	 * @param uint $b
	 * @return
	 */
	public function add_brezeln($b) {
		$b = (int)$b;
		if ($b <= 0) {
			throw new IllegalArgumentException("b", "N+", $b);
		}
		query("START TRANSACTION");
		query(
			"INSERT INTO users_brezeln SET uid={$this->get_id()}, amount=$b ".
			"ON DUPLICATE KEY UPDATE amount=amount+$b"
		);
		$new_b = (int)@mysql_result(
			query(
				"SELECT amount FROM users_brezeln WHERE uid={$this->get_id()}"
			),0
		);
		query("COMMIT");
		return $b;
	}

	/**
	 * Gibt die Authentifizierungs-Codes zurück, die der Spieler besitzt.
	 * Mit diesen Codes kann er reservierten Karten beitreten.
	 *
	 * @return char[6][]
	 */
	public function get_auth_keys() {
		$result = array();
		$res = query (
			"SELECT auth_key FROM users_map_keys WHERE uid={$this->get_id()}"
		);
		while ($row = mysql_fetch_row($res)) {
			$result[] = $row[0];
		}
		return $result;
	}

	/**
	 * Gibt ein Array mit Objekten zurück, die die Authentifizierungscodes und
	 * zusätzliche Informationen beinhaltet.
	 *
	 * Rückgabeobjekte habe folgende Form:
	 * <code>
	 * $o->auth_key		Authentifizierungsschlüssel
	 * $o->received_by	UID des Users, der ihm den Schlüssel geschickt hat
	 * <code>
	 *
	 * @return stdClass[]
	 */
	public function get_auth_keys_with_data() {
		$result = array();
		$res = query (
			"SELECT auth_key,received_by FROM users_map_keys ".
			"WHERE uid={$this->get_id()}"
		);
		while ($row = mysql_fetch_object($res)) {
			$result[] = $row;
		}
		return $result;
	}

	/**
	 * Wenn der Spieler sein PW vergessen hat, kann es hiermit neu gesetzt
	 * werden.
	 *
	 * Der Spieler wird darüber informiert.
	 *
	 * @param string $pass
	 */
	public function set_new_random_password() {
		$new_pass = substr(md5(time()), 0, 6);
		$this->set_pass($new_pass);
		pv_mail(
			$this->get_email(), LNG_PASSWORD_REMIND_MAIL_TOPIC,
			str_replace(
				array ('[USER]', '[PASS]', '<br>', '<br/>'),
				array ($this->get_nick(), $new_pass, "\n", "\n"),
				LNG_PASSWORD_REMIND_MAIL_TEXT
			)
		);
	}

	/**
	 * Hierüber kann ein bestimmtes Passwort dem User gesetzt werden.
	 *
	 * Der Spieler wird hierüber NICHT informiert.
	 *
	 * @param string $pass
	 */
	public function set_new_custom_password($pass) {
		$this->set_pass($pass);
	}

	/**
	 * Löscht das Nutzerkonto. Irreversibel!
	 * !! ACHTUNG!!!
	 */
	public function delete_account() {
		$new_pass = substr(md5(time()), 0, 6);
		$this->set_pass($new_pass);
		$this->user_data['initial_email']
			= $this->user_data['email'] = '';
		$this->data_changed = true;

		$personal = new stdClass();
		$personal->firstnames = '';
		$personal->surnames = '';
		$personal->birthday = '';
		$personal->street = '';
		$personal->street_number = 0;
		$personal->zipcode = 0;
		$personal->city = '';
		$personal->country = '';
		$personal->phone = 0;
		$personal->mobile = 0;
		$personal->state = 0;
		$personal->saluation = 0;
		$personal->title = '';
		$personal->id_fb = 0;
		$this->set_personal_data($personal);
		$this->set_banned(self::BANNED_DELETED_BY_HIMSELF);
	}

	/**
	 * Gibt die privaten Daten wie Adresse, Telephonnummer usw als stdClass
	 * folgender Form zurück:
	 *
	 * $object->firstnames		Vorname(n)
	 * $object->surnames		Nachname(n)
	 * $object->birthday		Geburtsdatum
	 * $object->street			Straße der Adresse
	 * $object->street_number	Hausnummer
	 * $object->zipcode			Postleitzahl
	 * $object->city			Stadt
	 * $object->state			Bundesland/US-Staat usw.
	 * $object->country			Herkunftsland als ISO 3166 Ländercode (DE, AT usw)
	 * $object->phone			Telephonnummer
	 * $object->mobile			Handynummer
	 * $object->salutation		Anrede
	 * $object->title			Titel
	 *
	 * @return stdClass
	 */
	public function get_personal_data() {
		$select_db = Db::USER_PERSONAL_SELECT_DB;
		if (defined('TEST_SERVER')) {
			$select_db .= '_test';
		}
		return mysql_fetch_object(
			self::get_user_personal_db()->query(
				"SELECT id_fb, id_wcf, gender, firstnames, surnames, birthday, ".
					"street, street_number, zipcode, city, country, phone, ".
					"mobile, state, salutation, title ".
				"FROM $select_db.user_personal ".
				"WHERE id={$this->get_id()}"
			)
		);
	}

	/**
	 * Setzt die persönlichen Daten, wie angegeben.
	 * @param stdClass $personal
	 */
	public function set_personal_data(stdClass $personal) {
		$select_db = Db::USER_PERSONAL_SELECT_DB;
		if (defined('TEST_SERVER')) {
			$select_db .= '_test';
		}
		$personal->gender = (int)$personal->gender;
		$personal->zipcode = (int)$personal->zipcode;
		$personal->street_number = (int)$personal->street_number;
		$sql = "UPDATE $select_db.user_personal SET ".
				"firstnames='{$personal->firstnames}', ".
				"surnames='{$personal->surnames}', ".
				"gender={$personal->gender}, ".
				"birthday='{$personal->birthday}', ".
				"street='{$personal->street}', ".
				"street_number={$personal->street_number}, ".
				"zipcode={$personal->zipcode}, ".
				"city='{$personal->city}', ".
				"country='{$personal->country}', ".
				"phone='{$personal->phone}', ".
				"mobile='{$personal->mobile}', ".
				"state='{$personal->state}', ".
				"salutation='{$personal->saluation}', ".
				"title='{$personal->title}' ";
		if ($personal->id_fb) {
			$personal->id_fb = (int)$personal->id_fb;
			$sql .= ", id_fb={$personal->id_fb} ";
		}

		$sql .= "WHERE id={$this->get_id()}";

		self::get_user_personal_db()->query($sql);
	}

	public function set_email_address($email) {
		if (!EMail::validate_email_string($email)) {
			throw new IllegalArgumentException("$email is no valid e-mail address");
		}
		$this->user_data['email'] = $email;
		$select_db = Db::USER_PERSONAL_SELECT_DB;
		if (defined('TEST_SERVER')) {
			$select_db .= '_test';
		}
		self::get_user_personal_db()->query(
			"UPDATE $select_db.user_personal SET ".
				"email='$email' ".
			"WHERE id={$this->get_id()}"
		);
	}

	/**
	 * Gibt das Objekt zu einer stdClass geschrumpft zurück.
	 *
	 * @return stdClass
	 */
	public function to_php_std_class() {
		$x = new stdClass();
		$x->user_data = $this->user_data;
		$x->user_data['cur_score'] = LeagueScore::get_instance(0)
			->get_score($this->user_data['id']);
		$x->user_data['last_world_id'] = $this->get_last_world_id();
		$x->user_data['last_map_id'] = $this->get_last_map_id();

		return $x;
	}

	/**
	 * Gibt das Objekt als assotiatives Array zurück.
	 *
	 * @return array
	 */
	public function to_php_assoc_array() {
		$x = array (
			'user_data'	=>	$this->user_data,
		);
		$x['user_data']['cur_score'] = LeagueScore::get_instance(0)
			->get_score($this->user_data['id']);
		$x['user_data']['last_world_id'] = $this->get_last_world_id();
		$x['user_data']['last_map_id'] = $this->get_last_map_id();

		return $x;
	}


	/**
	 * Fügt einen neuen User in die DB ein.
	 *
	 * @param string $nick
	 * @param string $email
	 * @param string $pass
	 * @param uint $recruited_by_id
	 * @param uint $fb_id
	 * @return unknown_type
	 */
	public static function insert_new_user_to_db($nick,
		$email, $pass, $recruited_by_id, $fb_id=0)
	{
		$recruited_by_id = (int)$recruited_by_id;
		if ($recruited_by_id < 0)
			$recruited_by_id = 0;

		$pass = md5($pass);
		/**
		 * Jeder User besteht aus mehreren Instanzen in der Datenbank.
		 * Eine Instanz pro Spielmodus den der Spieler gleichzeitig und
		 * unabhängig von anderen Modi spielen kann.
		 *
		 * Falls der Nick schon vorhanden ist, wird er eindeutig gemacht,
		 * indem ein Suffix angehängt wird - ein einfacher Inkrement über
		 * die gleichen Nicks.
		 */
		/** Zuerst den FFA-User einfügen */
		query (
			"INSERT INTO users SET ".
				"nick=CONCAT('{$nick}.', ".
					"(SELECT IF(MAX(nick_suffix) IS NULL,1,MAX(nick_suffix)+1) ".
					"FROM users AS t2 WHERE nick_main='$nick')), ".
				"nick_main='$nick', ".
				"nick_suffix=".
					"(SELECT IF(MAX(nick_suffix) IS NULL,1,MAX(nick_suffix)+1) ".
					"FROM users AS t WHERE nick_main='$nick'), ".
				"email='$email', ".
				"initial_email='$email', pass='$pass', ".
				"signup=".Time::get_t_now()
		);

		$id = mysql_result(query("SELECT LAST_INSERT_ID()"), 0);

		/**
		 * Gesetzten Nick auslesen. Er wurde durch einen Inkrement erweitert,
		 * z.B. würde "Phil" zu "Phil0" oder "Phil214".
		 */
		$nick = mysql_result(
			query("SELECT nick FROM users WHERE id='$id'"),0
		);

//		/** Jetzt den Clan-User einfügen */
//		query (
//			"INSERT INTO users SET nick='$nick', signup=".Time::get_t_now().", ".
//				"email='$email', initial_email='$email', ".
//				"pass='$pass', id_ffa=$id_ffa"
//		);
//		/** id_clanmode setzen */
//		query(
//			"UPDATE users SET id_clanmode=id ".
//			"WHERE id_ffa=$id_ffa AND id_ffa!=id"
//		);
//		$id_clanmode = mysql_result(query("SELECT LAST_INSERT_ID()"), 0);
//
//		/** id_clanmode vom FFA-User setzen (jetzt, wo diese bekannt) */
//		query(
//			"UPDATE users SET id_clanmode=$id_clanmode ".
//			"WHERE id_ffa=$id_ffa AND id_clanmode!=id"
//		);
//
		/**
		 * Jetzt den User in user_personal eintragen, dass gewisse Infos für
		 * alle (auch den Support und Payment) zugreifbar sind.
		 */
		$select_db = Db::USER_PERSONAL_SELECT_DB;
		if (defined('TEST_SERVER')) {
			$select_db .= '_test';
		}
		self::get_user_personal_db()->query(
			"INSERT INTO $select_db.user_personal ".
				"(id, id_fb, nick, email) ".
				"VALUES ".
				"($id, $fb_id, '$nick', '$email') "/*.
				"($id_clanmode, $fb_id, '$nick', '$email')"*/
		);

		/**
		 * Brezeln gutschreiben
		 */
		if ($recruited_by_id) {
			$b_amt = User::get_instance_by_id($recruited_by_id)->add_brezeln(1);

			/**
			 * Ggf. Medallie vergeben
			 */
			$m = Medals::get_medals(IMedals::TYPE_PREZELS_GAINED);
			$m = $m[IMedals::TYPE_PREZELS_GAINED][0];
			$medal_id = isset($m[$b_amt]) ? $m[$b_amt] : 0;

			/**
			 * Das hier ist eine stark multithread-lastige Kiste. Es können
			 * gerade auf globale Einladungen unmengen an Spielern gleichzeitig
			 * reagieren theoretisch. Dass die Medallie jetzt nicht doppelt
			 * vergeben wird, wird über $ok sichergestellt, da
			 * UserMedals::add_medal zurück gibt, ob der Eintrag der Medallie
			 * in die DB geglückt ist, oder nicht mithilfe von
			 * mysql_rows_affected().
			 *
			 * => if (!$ok) { $medal_id = 0; }
			 */
			if ($medal_id)
				$ok = UserMedals::get_instance($recruited_by_id)
					->add_medal($medal_id);

			if (!$ok)
				$medal_id = 0;

			/**
			 * Ereignis protokollieren
			 */
			EventLog::get_instance($recruited_by_id, 0)->add_entry(
				EventLog::TYPE_USER_RECRUITED,
				0,
				$nick
			);

			/**
			 * Ereignismeldung verschicken, dass Spieler jetzt bei Mammun
			 * ist
			 */
			EventMessages::get_instance($recruited_by_id)->add_entry(
				EventLog::TYPE_USER_RECRUITED,
				0,
				$nick,
				0,
				0,
				$medal_id
			);
		}
		return $nick;
	}

	/**
	 * Diese Funktion gibt die Karten zurück, auf denen sich die Spieler mit
	 * den angegebenen IDs befinden.
	 *
	 * Hier wird in jedem Fall ein Datenbankzugriff getätigt. Diese Funktion
	 * liefert ein assotiatives Array zurück in dem jeder User-ID die Map-ID
	 * oder <i>0</i> zugeordnet ist. Bei <i>0</i> ist der User gerade auf keiner
	 * Karte.
	 * Statt der IDs können auch die Nicks der User übergeben werden.
	 *
	 * Wird $no_zero_results auf true gesetzt (default: false), dann sind
	 * diese 0-Ergebnisse nicht im Rückgabe-Assoc enthalten.
	 *
	 * @param uint[] / string[] $uids_or_nicks
	 * @param bool $no_zero_results
	 * @return assoc[uint => uint]
	 */
	public static function get_current_maps_of_users(array $uids_or_nicks,
		$no_zero_results=false)
	{
		if (is_anint($uids_or_nicks[0])) {
			$uids = $uids_or_nicks;
			// TODO: split homesector in world_id, map_id, sector_id
			$sql = "SELECT id, FLOOR(homesector/100) AS map_id, state FROM users ".
					"WHERE id IN ('".implode("','", $uids)."') ";
		}
		else {
			$nicks = $uids_or_nicks;
			// TODO: split homesector in world_id, map_id, sector_id
			$sql = "SELECT id, FLOOR(homesector/100) AS map_id, state FROM users ".
					"WHERE nick IN ('".implode("','", $nicks)."')";
		}

		$res = query($sql);
		if (mysql_num_rows($res) == 0) {
			return array();
		}
		$result = null;
		while ($row = mysql_fetch_assoc($res)) {
			if (!$no_zero_results || ($row['state'] == self::STATE_PLAYING)) {
				$result[$row['id']][0] = floor($row['map_id'] / 100);
				$result[$row['id']][1] = $row['map_id'] % 100;
			}
		}
		return $result;
	}


	/**
	 * !!WARNUNG: Setzt alle Userdaten aller User zurück.
	 * Irreversibel.
	 */
	public static function clear_of_all_users() {
		unset($_SESSION['User']);
		self::$INSTANCES = array();
		query(
			"UPDATE users SET state=3, last_login=".(Time::get_t_now()-3).", ".
				"last_access=".(Time::get_t_now()-3).", play_time=0, ".
				"waiting_state=0, waiting_finished=0, waiting_homesector=0, ".
				"session_identifier=0, homesector=0,".
				"last_score=0, add_keep_sciences=0, users_killed=0, maps_won=0, ".
				"maps_lost=0, suicides=0"
		);

		query(
			"DELETE FROM users_map_keys"
		);
	}

	/**
	 * !!WARNUNG: Setzt die Daten des Users zurück.
	 * Irreversibel
	 */
	public function clear() {
		$this->set_state(3);
		$this->set_waiting_state(0);
		$this->set_waiting_finished(0);
		$this->set_waiting_homesector(0);
//		$this->set_last_login(Time::get_t_now()-3);
//		$this->set_last_access(Time::get_t_now()-3);
//		$this->set_play_time(0);
//		$this->set_session_identifier(0);
		$this->set_homesector(0);
		$this->set_last_score(0);
		$this->set_add_keep_sciences(0);
		$this->set_users_killed(0);
		$this->set_maps_lost(0);
		$this->set_suicides(0);
	}
}


?>
