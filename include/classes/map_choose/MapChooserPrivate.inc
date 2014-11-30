<?php
require_once(dirname(__FILE__).'/MapChooserAuthKey.inc');
require_once(dirname(__FILE__).'/../World.inc');
require_once(dirname(__FILE__).'/../host/HostControl.inc');
require_once(dirname(__FILE__).'/../exceptions/HackattackException.inc');
require_once(dirname(__FILE__).'/../../shared/classes/exceptions/'.
										'IllegalArgumentException.inc');

/**
 * Die Klasse ist zuständig dafür, dass der User eine FFA-Karte im
 * privaten Modus wählen kann, dass diese sich füllt und, wenn voll, gestartet
 * wird.
 *
 * Beim privaten Modus ist der erste Spieler, der die Karte betrifft, der
 * Karten-Master. Als solcher kann er die Regeln beim Start der Karte bestimmen.
 * Eine private Karte ist standartmäßig abgeschlossen und man kann sie nur
 * mit Einladung betreten. Da der Einladungsschlüsel pro Karte und nicht pro
 * Spieler existiert, können eingeladene Spieler selbst wieder Spieler einladen
 * (also das Kartenpasswort weitergeben).
 *
 * Der Master kann auch genau das Kartentemplate wählen, also die Inselform, aus
 * einer vom System vorgegebenen Auswahl.
 *
 * Künftige Features:
 * - Passwort/Auth-Key-Schutz abschaltbar (auch zur Laufzeit der Karte)
 * - Kartengeschwindigkeit einstellbar
 * - Forschungslimits
 * - Gebäudelimits
 * - .....
 */
class MapChooserPrivate extends MapChooserAuthKey
{
	/**
	 * Wieviele Spieler auf einer Karte sein dürfen. Wenn z.B. 4 in
	 * $ALLOWED_TYPES enthalten ist, dann heißt das, dass 4-Spieler-Karten
	 * auswählbar sind.
	 * @var int[]
	 */
	protected static $ALLOWED_TYPES = array(4,6,8);

	/**
	 * Alle Templates, die für eine neue Karte verfügbar sind, werden hier
	 * als einfaches Array von IDs gespeichert.
	 * @var uint[]
	 */
	protected $available_templates;

	const DEBUG_VERBOSE = 0;

	private static $INSTANCES;

	/**
	 * Gibt die Instanz für den User mit der angegebenen ID zurück.
	 *
	 * Wird zusätzlich <code>$world_id</code> angegeben, so wird jede
	 * Wahlmöglichkeit usw. auf diese Welt beschränkt. Ansonsten ist die Auswahl
	 * Weltübergreifend und jede Rückgabe nach Welt sortiert.
	 *
	 * @param uint $uid
	 * @param uint $world_id
	 * @return MapChooserPrivate
	 */
	public static function get_instance($uid, $world_id=0) {
		if ($uid <= 0) {
			throw new IllegalArgumentException("uid", "N1", $uid);
		}
		$uid = (int)$uid;
		if ($world_id < 0) {
			throw new IllegalArgumentException("world_id", "N0", $world_id);
		}
		$world_id = (int)$world_id;
		if (!isset(self::$INSTANCES[$uid][$world_id]))
			self::$INSTANCES[$uid][$world_id] = new self($uid, $world_id);

		return self::$INSTANCES[$uid][$world_id];
	}

	/**
	 * @param uint $uid
	 * @return MapChooserPrivate
	 */
	protected function __construct($uid, $world_id) {
		parent::__construct($uid, $world_id);
		$this->direct_start = true;
		$this->auth_key = null;	// Verzögert
	}

	/**
	 * Initialisiert available_maps
	 */
	protected function init_available_maps() {
		$this->available_maps = array();
		/**
		 * Alle Karten anbieten, die entweder keinen Auth-Key benötigen, oder
		 * zu denen der User mit der ID $this->uid einen passenden Auth-Key
		 * besitzt.
		 */

		/**
		 * Diese Query funktioniert so:
		 * Erst werden alle Auth-Keys des Users geholt, dann die passenden
		 * reservierten Karten gejoint und dann die Daten durch joining von
		 * maps und templates abgerufen.
		 * Um festzustellen, dass die Karte nicht schon voll ist, wird die
		 * Anzahl der maximalen Spieler noch mit derer auf der Karte verglichen
		 * mithilfe einer Subquery auf players_on_map.
		 *
		 * Dann kommt der UNION. Hier werden nun alle Karten auf ähnliche Weise
		 * erfragt, die keinen Auth-Key benötigen.
		 *
		 * (Anm.: Es ohne UNION in die erste Quers miteinzuflicken durch OR bei
		 * Auth-Key führt zu einer ALL-Abfrage in der map_reserved_maps-
		 * Tabelle, weswegen das vermeitlich langsamer aussehende UNION für 
		 * große Tabellen an dieser Stelle deutlich schneller ist).
		 *
		 * (Anm.: Durch das UNION liegt das Ergebnis so sortiert vor, dass
		 * zunächst alle privaten und dann alle öffentlichen Karten kommen).
		 */
		$sql =
			"SELECT m.world_id, m.map_id, m.template_id, t1.num_players, ".
				"ukeys.auth_key, 0 AS public ".
			"FROM users_map_keys AS ukeys ".
			"LEFT JOIN map_reserved_maps AS rmaps ".
				"ON ukeys.auth_key=rmaps.auth_key ".
			"LEFT JOIN maps AS m ".
				"ON rmaps.world_id=m.world_id AND rmaps.map_id=m.map_id ".
			"LEFT JOIN template_maps_1 AS t1 ".
				"ON m.template_id=t1.template_id ".
			"WHERE ukeys.uid={$this->uid} AND rmaps.world_id IS NOT NULL ".
				"AND rmaps.public=0 ".
				"AND (SELECT COUNT(*) FROM players_on_map WHERE ".
							"map_id=m.map_id AND world_id=m.world_id) ".
															"< t1.num_players ".
				"AND t1.num_players IN ("
							.implode(', ', self::$ALLOWED_TYPES) . ") ".
				"AND m.started > (".(Time::get_t_now()
							- MapDescription::MAP_FILL_TIMEOUT).") ".
			"UNION SELECT m.world_id, m.map_id, m.template_id, " .
				"t1.num_players, rmaps.auth_key, 1 AS public ".
			"FROM map_reserved_maps AS rmaps ".
			"LEFT JOIN maps AS m ON ".
				"rmaps.world_id=m.world_id AND rmaps.map_id=m.map_id ".
			"LEFT JOIN template_maps_1 AS t1 ".
				"ON m.template_id=t1.template_id ".
			"WHERE rmaps.public=1 ".
				"AND (SELECT COUNT(*) FROM players_on_map WHERE ".
							"world_id=m.world_id AND map_id=m.map_id) ".
															"< t1.num_players ".
				"AND t1.num_players IN ("
							.implode(', ', self::$ALLOWED_TYPES) . ") ".
				"AND m.started > (".(Time::get_t_now()
							- MapDescription::MAP_FILL_TIMEOUT).")";

		if ($this->world_id)
			$sql .= " AND rmaps.world_id={$this->world_id}";

		$result = query($sql);

		/**
		 * User aus der players_on_map-Tabelle holen, da die Karte
		 * (wenn nicht leer) schon läuft und wir die Daten aller Spieler der
		 * Karte gesammelt haben wollen sortiert nach gekillt und noch am Leben.
		 */
		while ($row = mysql_fetch_assoc($result)) {
			$res = query (
				'SELECT user_id,kill_time FROM players_on_map '.
				"WHERE world_id={$row['world_id']} AND map_id={$row['map_id']}"
				// AND kill_time=0
			);
			$users = array();
			$dead_ones = array();
			if (mysql_num_rows($res) > 0) {
				while ($p_row = mysql_fetch_assoc($res)) {
					if ($p_row['kill_time'] <= 0)
						$users[] = $p_row['user_id'];
					else
						$dead_ones[] = $p_row['user_id'];
				}
			}

			$map_data = array (
				'num_players'	=>	$row['num_players'],
				'users_on_map'	=>	$users,
				'dead_users'	=>	$dead_ones,
				'template_id'	=>	$row['template_id'],
				'world_id'		=>	$row['world_id'],
				'map_id'		=>	$row['map_id'],
				'public'		=>	(bool)$row['public']
			);
			$this->available_maps[] = $map_data;
		}
	}
	/**
	 * Initialisiert alle Member-Variablen außer available_maps.
	 */
	protected function init() {
		$user = User::get_instance_by_id($this->uid);
		$cur_level = $user->get_cur_level();


		/**
		 * Jetzt holen wir alle Templates aus der Datenbank, die passen.
		 */
// TODO CACHE

		$sql =
			'SELECT template_id, num_players FROM template_maps_1 '.
			'WHERE type=(type|'.MapTemplate::TYPE_SUITABLE_FOR_FFA.') '.
				'AND num_players IN ('.
								implode(', ', self::$ALLOWED_TYPES) . ') ';
		$result = query($sql);
		while ($row = mysql_fetch_assoc($result)) {
			$this->available_templates[] = array(
				'template_id'	=>	$row['template_id'],
				'num_players'	=>	$row['num_players']
			);
		}
	}

	/**
	 * Gibt die Daten zurück, die dem entsprechenden Auth-Key zugeordnet sind.
	 *
	 * Die Rückgabe ist ein Array mit Standartobjekten folgender Struktor:
	 *
	 * $o->world_id		ID der Welt, in der der Key gilt
	 * $o->map_id		ID der Karte, für die der Key gilt
	 * $o->sector_ids	Liste der Sektoren, die in Frage kommen. Ist die Liste
	 * 					leer, sind entweder alle möglichen Sektoren belegt, 
	 *					oder die Reservierungsdauer ist ausgelaufen.
	 * 					Dann kann höchstens versucht werden, ohne Authkey 
	 *					normal der Karte beizutreten (falls sie nicht voll 
	 *					ist).
	 * $o->min_level	Die Stufe, die der User mindestens braucht, um hier
	 * 					mitspielen zu dürfen
	 * $o->max_level	Die Stufe, die der User maximal haben darf, um hier 
	 *					ohne Handicap mitspielen zu dürfen
	 *
	 * @param string[] $auth_keys
	 * @return stdClass[]
	 */
	public static function get_data_by_auth_keys(array $auth_keys) {
		// TODO Alle abgelaufenen Auth-Keys löschen?!
		$res = query(
			'SELECT rmaps.auth_key, m.map_id, rmaps.world_id, t.sector_id '.
			'FROM map_reserved_maps AS rmaps '.
			'LEFT JOIN maps AS m '.
				'ON m.map_id=rmaps.map_id '.
				'AND m.world_id=rmaps.world_id '.
			'LEFT JOIN template_maps AS t '.
				'ON m.template_id=t.template_id '.
			'LEFT JOIN players_on_map AS p '.
				'ON p.map_id=m.map_id AND t.sector_id=p.homesector '.
				'AND m.world_id=p.world_id '.
			"WHERE t.home=1 AND rmaps.auth_key IN ('".
									implode("','",$auth_keys)."') ".
				"AND p.map_id IS NULL ".
				"AND m.started > (".(Time::get_t_now()
							- MapDescription::MAP_FILL_TIMEOUT).") ".
			"ORDER BY m.world_id, m.map_id"
		);
		if (mysql_num_rows($res) == 0) {
			throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);
		}

		$result = array();

		$last_world_id = 0;
		$last_map_id = 0;
		while ($row = mysql_fetch_object($res)) {
			// Zunächst alle Daten, die in allen Zeilen der Map gleich sind:
			$o = new stdClass();
			$o->min_level =
				World::get_instance($row->world_id)->get_minimum_level();
			$o->max_level =
				World::get_instance($row->world_id)->get_maximum_level();
			$o->world_id = $row->world_id;
			$o->map_id = $row->map_id;
			$o->auth_key = $row->auth_key;
			$o->sector_ids[] = $row->sector_id;

			$last_world_id = $row->world_id;
			$last_map_id = $row->map_id;
			// Und jetzt der Rest
			while (($row = mysql_fetch_object($res))
				&& ($last_world_id == $row->world_id)
				&& ($last_map_id== $row->map_id))
			{
				$o->sector_ids[] = $row->sector_id;
			}
			$result[] = $o;
		}

		return $result;
	}
	public static function get_data_by_auth_key($auth_key) {
		$x = self::get_data_by_auth_keys(array($auth_key));
		return $x[0];
	}

	/**
	 * Gibt den Authentifizierungsschlüssel zurück, den der angebene User für
	 * die Karte, auf der er gerade spielt, zugewiesen bekommen hat.
	 *
	 * Mit diesem Schlüssel kann er weitere Spieler in sein Team auf der Karte
	 * einladen.
	 *
	 * @param ℕ₁ $uid
	 * @return string
	 */
	public static function get_auth_key_for_current_map($uid) {
		if ($uid <= 0) {
			throw new IllegalArgumentException("uid", "N1", $uid);
		}
		$uid = (int)$uid;
		$map_id = User::get_instance_by_id($uid)->get_map_id();
		$world_id = User::get_instance_by_id($uid)->get_world_id();

		return @mysql_result(query(
			"SELECT auth_key FROM map_reserved_maps ".
			"WHERE world_id=$world_id AND map_id=$map_id"
		),0);
	}

	/**
	 * Hilfsfunktion zum Festlegen des Authentifizierungsschlüssels.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param char[6] $auth_key
	 * @param bool $public	Ob die Karte öffentlich sein soll (default: false)
	 * @throws Exception	Wenn auth_key bereits belegt
	 */
	private static function insert_auth_key($world_id, $map_id, $auth_key,
		$public = false)
	{
		$world_id = ValueChecker::id($world_id, 'world_id');
		$map_id = ValueChecker::id($map_id, 'map_id');
		$auth_key = ValueChecker::string($auth_key, 'auth_key', false, 6, 6);
		$public = (int)ValueChecker::bool($public, 'public');

		query("START TRANSACTION");
		$res = query(
			"SELECT world_id, map_id FROM map_reserved_maps ".
			"WHERE auth_key='$auth_key'"
		);
		if (mysql_num_rows($res) > 0) {
			query("ROLLBACK");
			throw new Exception("Key in use");
		}

		query(
			"INSERT INTO map_reserved_maps SET ".
				"world_id=$world_id, ".
				"map_id=$map_id, ".
				"auth_key='$auth_key', ".
				"public=$public, ".
				"t_stamp=".Time::get_t_now() . " ".
			"ON DUPLICATE KEY UPDATE auth_key='$auth_key'"
		);
		query("COMMIT");
	}

	/**
	 * Setzt für die angegebene Karte den zufälligen Auth-Key und gibt diesen
	 * zurück.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param bool $public	Ob die Karte öffentlich sein soll. Default: False.
	 * @return char[6]
	 * @throws Exception	Wenn der Auth-Key nicht gesetzt werden konnte, weil
	 * 						z.B. kein eindeutiger Key gefunden werden konnte
	 * 						oder weil der aktuelle User versucht eine Karte
	 * 						zu reservieren, die ein anderer User bereits
	 * 						reserviert hat oder ähnliches.
	 */
	private function insert_random_auth_key($world_id, $map_id, $public=false)
	{
		$max_tries = 10;
		$i = 0;
		do {
			$auth_key = self::create_new_auth_key();
			try {
				$this->insert_auth_key($world_id, $map_id, $auth_key, $public);
				$need_new_auth_key = false;
			}
			catch(Exception $e) {
				$need_new_auth_key = true;
			}
			$i++;
		} while ($need_new_auth_key && ($i < $max_tries));
		if ($i >= $max_tries)
			throw new Exception("Auth-Key could not set!");

		return $auth_key;
	}

	/**
	 * Der User wird auf die nächste Karte gesetzt, die einen freien Platz hat.
	 *
	 * @return uint				Die komplette Sektor-ID des Heimatsektors, der
	 * 							dem User zugewiesen wurde.
	 */
	public function choose_random_map() {
		// MUTEX: Keine zwei Kartenzusagen gleichzeitig!!
		// (sonst würden Homesektoren doppelt vergeben)
		$sem_id = sem_get(MUTEX_MAP_CHOOSE, 1, 0600, -1);
		sem_acquire($sem_id);

		$this->init_available_maps();	// Alle Daten müssen frisch sein
		$sector_id = false;
		try {
			$this->unchoose_map();

			/**
			 * Die gefundenen Karten / Heimatsektoren durchtesten. Der
			 * einzige Fehler, der passieren kann, ist, dass der Spieler schon
			 * auf der Karte war. Das hab ich in die Query nicht mehr
			 * reinbekommen. Die Zeile, für die das nicht zutrifft, wird
			 * genommen.
			 */
			$n = count($this->available_maps);
			for ($map_index = 0; $map_index < $n; $map_index++) {
				$map = $this->available_maps[$map_index];

				if (!in_array($this->uid, $map['users_on_map']) &&
					!in_array($this->uid, $map['dead_users'])
					&& $map['public'])
				{
					$sql =
						'SELECT t.sector_id FROM template_maps AS t '.
						'LEFT JOIN players_on_map AS p '.
							"ON p.map_id={$map['map_id']} ".
								"AND p.world_id={$map['world_id']} ".
							"AND t.sector_id=p.homesector ".
						"WHERE t.home=1 AND t.template_id={$map['template_id']} ".
							"AND p.map_id IS NULL";

					$res = query($sql);
					if (mysql_num_rows($res) == 0) {
						error_log("MUTEX! No free sector could be found in ".
							__FILE__." line ".__LINE__." using the following ".
							"query:\n$sql");
						continue;	// Angeblich wäre hier platz, aber kein
									// freier Sektor ist zu finden.
									// => MUTEX-Bug!
					}
					else {
						$sector_id = @mysql_result($res, 0);
						break;
					}
				}
			}

			/**
			 * Wenn keine Karte gefunden wurde, ist der Grund klar: Der User
			 * war schon auf allen potentiellen Karten!
			 *
			 * Also wird eine neue eröffnet.
			 */
			if ($sector_id === false) {
				$template = $this->available_templates[
								rand(0, count($this->available_templates)-1)];

				$rules = new stdClass();
				$rules->public = true;
				$free_home = $this->open_new_map_logic(
					$template['template_id'], $rules
				);
			}
			else {
				/**
				 * Ansonsten wird das siedeln wie üblich von Hand durchgeführt
				 */
				$free_home =  $sector_id + $map['world_id']*10000
					+ $map['map_id']*100;

				$user = User::get_instance_by_id($this->uid);
				// Wartestatus: Er wartet auf ne volle Map
				$user->set_waiting_homesector($free_home);
				$user->set_waiting_state(IUser::STATE_WAITING_ON_MAP);

				/**
				 * Der normale State wird nicht angetastet, da der Waiting-
				 * State höherwertig ist. Der normale State wird nun genutzt,
				 * um den Status des Trainingsmapspiels zu speichern, das nur
				 * gespielt werden kann, während ein Spieler wartet.
				 * Dasselbe gilt für den Homesektor, der ebenso nicht 
				 * angetastet wird, damit man ihn für eine Trainingskarte
				 * nutzen kann.
				 */
				// User der gewählten Karte hinzufügen:
				$this->available_maps[$map_index]['users_on_map'][]
					= $user->get_id();
				$this->chosen_map_index = $map_index;

				/**
				 * Authkey-technisch muss man nix machen, weil das pro Map und 
				 * nicht pro User gespeichert wird bei Private Maps
				 */

				/**
				 * Beim Direkten Start schalten wir von "wartend" direkt auf
				 * "wartend und fertig" um, da es keinen Warteraum gibt, wo 
				 * Spieler markieren müssen, dass sie fertig sind usw
				 */
				$this->toggle_ready_state_logic();
			}
			// MUTEX
			sem_release($sem_id);
			return $free_home;
		}
		catch (IllegalArgumentException $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
		catch (Exception $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
	}

	private function choose_map_by_index_logic($map_index, $home=0, $color=0) {
		$this->unchoose_map();

		$map = $this->available_maps[$map_index];

		if (in_array($this->uid, $map['users_on_map']) ||
			in_array($this->uid, $map['dead_users']))
		{
			throw new Exception("", self::ERR_USER_IS_OR_WAS_ON_MAP);
		}

		/**
		 * Beim Direkten Start wird nicht gewartet, die homesectoren sind
		 * nicht in waiting_homesectors gespeichert, sondern in homesectors.
		 * Nun kann es aber sein, dass User auf der Karte bereits diese
		 * verlassen haben - dann wäre homesector nicht mehr aufzufinden und
		 * ein zwar aktuell nicht mehr vom Spieler beanspruchter, aber
		 * wahrscheinlich mit Einheiten vollgepumpter Sektor würde mehrfach
		 * vergeben. Das will niemand.
		 * Deswegen werden die Homesectoren nochmal in players_on_map
		 * gespeichert (was auch die Suchgeschwindigkeit erhöht). Die dort
		 * gespeicherten Werte meinen nicht, welchen Homesektor der User
		 * JETZT innehat, sondern, welcher ihm auf der Karte mit der ID
		 * $map_id für diese Karte zugewiesen worden ist. Und genau das
		 * wollen wir hier haben. Beim alten Modell ist der aktuelle HS
		 * immer derselbe, wie der, der ihm zugewiesen wurde, da man
		 * nachträglich nicht auf die Karte drauf konnte.
		 */
		$sql =
			'SELECT t.sector_id FROM template_maps AS t '.
			'LEFT JOIN players_on_map AS p '.
				"ON p.map_id={$map['map_id']} ".
				"AND p.world_id={$map['world_id']} ".
				"AND t.sector_id=p.homesector ".
			"WHERE t.home=1 AND t.template_id={$map['template_id']} ".
				"AND p.map_id IS NULL";
		$res = query($sql);
		if (mysql_num_rows($res) == 0)
			throw new Exception("", self::ERR_MAP_ALREADY_FULL);


		/**
		 * Wir prüfen jetzt, ob die Heimatsektoren auch alle leer sind, also,
		 * dass sie keine dominiert. Wenn die Karte noch nicht gestartet ist,
		 * dann ist das sowieso der Fall und wir können den erst besten Sektor
		 * wählen. Sonst müssen wir jeden möglichen Sektor einzeln prüfen.
		 */
		$homes = array();
		if (MapDescription::get_instance($map['world_id'], $map['map_id'])
			->get_state() == MapDescription::STATE_MAP_IDLE)
		{
			while ($row = mysql_fetch_assoc($res)) {
				$homes[$row['sector_id']] = true;
			}
		}
		else {
			while ($row = mysql_fetch_assoc($res)) {
				$sector_id = $row['sector_id'];
				/**
				 * Forced recompute, damit der Spieler sicher die richtige
				 * Information bekommt, ob der Sektor leer ist, oder nicht.
				 */
				$dom = MapCompute::get_instance($map['world_id'],
					$map['map_id'], 0)->get_dominator($sector_id);
				if ($dom == 0) { // Leerer Sektor => Siedeln ist hier möglich
					$homes[$row['sector_id']] = true;
				}
			}
			if (count($homes) == 0) {
				throw new Exception("", self::ERR_MAP_ALREADY_FULL);
			}
		}

		/**
		 * Okay, der User darf hier siedeln :)
		 */

		/**
		 * Wenn ein Wunsch-HS gewählt wurde, dann versuchen diesen zu
		 * wählen.
		 */
		if ($home) {
			if (!$homes[$home])
				throw new Exception("", self::ERR_HS_NOT_AVAILABLE);
			$new_home = $home + $map['world_id']*10000 + $map['map_id']*100;
		}
		/**
		 * Sonst wählen wir zufällig einen.
		 */
		else {
			$new_home = random_key($homes) + $map['world_id']*10000
				+ $map['map_id']*100;
		}
		$user = User::get_instance_by_id($this->uid);
		// Wartestatus: Er wartet auf ne volle Map
		$user->set_waiting_homesector($new_home);
		$user->set_waiting_state(IUser::STATE_WAITING_ON_MAP);

		/**
		 * Der normale State wird nicht angetastet, da der Waiting-State
		 * höherwertig ist. Der normale State wird nun genutzt, um den
		 * Status des Trainingsmapspiels zu speichern, das nur gespielt werden,
		 * während ein Spieler wartet.
		 * Dasselbe gilt für den Homesektor, der ebenso nicht angetastet wird,
		 * damit man ihn für eine Trainingskarte nutzen kann.
		 */
		// User der gewählten Karte hinzufügen:
		$this->available_maps[$map_index]['users_on_map'][] = $user->get_id();
		$this->chosen_map_index = $map_index;

		if ($color)
				$this->wished_color = $color;

		return $new_home;
	}

	/**
	 * Versucht den Spieler auf die Karte zu setzen, die sein Auth-Key angibt.
	 *
	 * Je nach Auth-Key bewirbt sich der Spieler als Tag-Team-Partner eines
	 * Spielers. Er wurde also eingeladen. Dieser Tag-Team-Platz ist jedoch
	 * nicht sicher, da sich ggf. mehrere Leute auf den Platz beworben haben.
	 * In dem Fall wird die Exception "self::ERR_MAP_ALREADY_FULL" geworfen.
	 *
 	 * @param string $auth_key	Die Hälfte der Heimatsektoren sind reserviert.
	 * 							Der $auth_key entscheidet, für welchen dieser
	 * 							Sektoren er sich bewirbt (könnte schon belegt
	 * 							sein)
	 * @param string $no_invitation		Wenn das TRUE ist, dann wird kein
	 * 							"Lade jetzt Freunde ein!"-Ereignis geschickt.
	 * @return uint				Die komplette Sektor-ID des Heimatsektors, der
	 * 							dem User zugewiesen wurde.
	 */
	public function choose_map_by_auth_key($auth_key, $no_invitation=false,
		$home=0, $color=0)
	{
		// MUTEX: Keine zwei Kartenzusagen gleichzeitig!!
		// (sonst würden Homesektoren doppelt vergeben)
		$sem_id = sem_get(MUTEX_MAP_CHOOSE, 1, 0600, -1);
		sem_acquire($sem_id);

		$this->init_available_maps();	// Alle Daten müssen frisch sein
		try {
			$res = query(
				"SELECT world_id, map_id FROM map_reserved_maps ".
				"WHERE auth_key='$auth_key' LIMIT 1"
			);
			if (mysql_num_rows($res) == 0) {
				throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);
			}
			$row = mysql_fetch_assoc($res);
			$map_id = $row['map_id'];
			$world_id = $row['world_id'];

			// $map_index bestimmen (s. choose_map()):
			$n = count($this->available_maps);
			$map_index = 0;
			while (($map_index < $n) &&
					($this->available_maps[$map_index]['world_id']!=$world_id)
					&& ($this->available_maps[$map_index]['map_id']!=$map_id))
			{
				$map_index++;
			}

			if ($map_index == $n){	// Map ist nicht bekannt
				throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);
			}
			$new_home = $this->choose_map_by_index_logic($map_index, $home,
				$color);

			/**
			 * Beim Direkten Start schalten wir von "wartend" direkt auf
			 * "wartend und fertig" um, da es keinen Warteraum gibt, wo Spieler
			 * markieren müssen, dass sie fertig sind und auch wirklich sicher.
			 */
			$this->toggle_ready_state_logic();
			// MUTEX
			sem_release($sem_id);

			// Dem Spieler entfernen wir jetzt den Authkey, weil er ihn benutzt
			// hat
			User::get_instance_by_id($this->uid)
				->remove_map_invitation_auth_key($auth_key);


			/**
			 * Der Spieler ist per Auth-Key einer Insel beigetreten, also ist
			 * sie nicht öffentlich. Deswegen muss er jetzt auch Spieler
			 * einladen.
			 */
			if (!$no_invitation) {
				try {
					EventMessages::get_instance($this->uid)->add_entry(
						EventMessages::TYPE_OPEN_INVITATION_DIALOG
					);
				}
				catch(Exception $e) {
					error_log("Warning: {$e->getFile()} line {$e->getLine()}:"
						. $e->getMessage());
				}
			}
			return $new_home;
		}
		catch (IllegalArgumentException $e) {
			// MUTEX
			sem_release($sem_id);
			/**
			 * Fehlerhaften Authkey gleich entfernen
			 */
			User::get_instance_by_id($this->uid)
				->remove_map_invitation_auth_key($auth_key);
			throw ($e);
		}
		catch (Exception $e) {
			// MUTEX
			sem_release($sem_id);
			/**
			 * Fehlerhaften Authkey gleich entfernen
			 * Nur wenn HS_NOT_AVAILABLE, dann hat er was falsches
			 * ausgewählt und kann nochmal probieren => hier
			 * Authkey lassen
			 */
			if ($e->getCode() != self::ERR_HS_NOT_AVAILABLE) {
				User::get_instance_by_id($this->uid)
					->remove_map_invitation_auth_key($auth_key);
			}
			throw ($e);
		}
	}

	private function open_new_map_logic ($template_id, $rules,
		$home=0, $color=0)
	{
		if ($rules && is_object($rules)) {
			$time_factor =
				(isset($rules->time_factor) && ($rules->time_factor > 0))
					? $rules->time_factor
					: 1;
			$public = (bool)$rules->public;
			$map_key = !empty($rules->map_key) ? $rules->map_key : NULL;
		}
		else {
			$time_factor = 1;
			$public = false;
			$map_key = NULL;
		}
		$this->unchoose_map();

		$this->init_available_maps();	// Alle Daten müssen frisch sein

		$n = count($this->available_templates);
		$found = false;
		for ($i = 0; ($i < $n) && !$found; $i++) {
			if ($this->available_templates[$i]['template_id']
				== $template_id)
			{
				$template = $this->available_templates[$i];
				$found = true;
			}
		}
		if (!$found) {
			throw new IllegalArgumentException(
				"template_id",
				"{".implode(",", $this->available_templates)."}", 
				$template_id
			);
		}

		/**
		 * Die Welt ermitteln, wenn keine angegeben wurde
		 */
		if (!$this->world_id) {
			/**
			 * Menkarten starten üblicherweise in der untersten Welt
			 */
			if ($rules->mentor) {
				$world = World::get_instance_by_level(0,
					World::MAP_TYPE_FFA);
			}
			/**
			 * Bei allen anderen wird die höchst mögliche Welt angenommen
			 */
			else {
				$user = User::get_instance_by_id($this->uid);
				$cur_level = $user->get_cur_level();
				$world = World::get_instance_by_level($cur_level,
					World::MAP_TYPE_FFA);
			}
		}
		else {
			$world = World::get_instance($this->world_id);
		}

		$map_id = $world->insert_map($template['template_id'],
			World::MAP_TYPE_FFA, $time_factor, $rules->mentor);
		$map_index = count($this->available_maps);
		$this->available_maps[$map_index] = array(
			'num_players'	=> $template['num_players'],
			'users_on_map'	=> array(),
			'dead_users'	=> array(),
			'template_id'	=> $template['template_id'],
			'world_id'		=> $world->get_id(),
			'map_id' 		=> $map_id,
			'public'		=> $public
		);
			
		if (!$map_key) {
			$this->insert_random_auth_key(
				$world->get_id(), $map_id, $public
			);
		}
		else {
			$this->insert_auth_key(
				$world->get_id(), $map_id, $map_key, $public
			);
		}
		$free_home = $this->choose_map_by_index_logic(
			$map_index, $home, $color
		);

		/**
		 * Beim Direkten Start schalten wir von "wartend" direkt auf
		 * "wartend und fertig" um, da es keinen Warteraum gibt, wo Spieler
		 * markieren müssen, dass sie fertig sind und auch wirklich sicher.
		 */
		$this->toggle_ready_state_logic();

		return $free_home;
	}

	/**
	 * Setzt den User auf die Map mit der angegebenen ID
	 *
	 * @param uint $map_index	Der User wird auf die
	 * 							Karte available_maps[$map_index] gesetzt
	 * @return uint				Die komplette Sektor-ID des Heimatsektors, der
	 * 							dem User zugewiesen wurde.
	 */
	public function choose_map($map_index, $auth_key='', $home=0, $color=0) {
		if ($map_index < 0) {
			throw new IllegalArgumentException("map_index", "N0", $map_index);
		}

		// MUTEX: Keine zwei Kartenzusagen gleichzeitig!!
		// (sonst würden Homesektoren doppelt vergeben)
		$sem_id = sem_get(MUTEX_MAP_CHOOSE, 1, 0600, -1);
		sem_acquire($sem_id);

		$this->init_available_maps();	// Alle Daten müssen frisch verfügbar sein}
		if (self::DEBUG_VERBOSE > 0) {
			error_log("MapChooserTagTeam::\$INSTANCES[{$this->uid}]->choose_map($map_index) called");
		}
		try {
			$this->unchoose_map();

			$map = $this->available_maps[$map_index];
			if (empty($map) || empty($map['template_id']) ||
					empty($map['world_id']) || empty($map['map_id']))
				throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);

			$free_home = $this->choose_map_by_index_logic($map_index, $home,
				$color);

			/**
			 * Beim Direkten Start schalten wir von "wartend" direkt auf
			 * "wartend und fertig" um, da es keinen Warteraum gibt, wo Spieler
			 * markieren müssen, dass sie fertig sind und auch wirklich sicher.
			 */
			$this->toggle_ready_state_logic();
			// MUTEX
			sem_release($sem_id);

			return $free_home;
		}
		catch (IllegalArgumentException $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
		catch (Exception $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
	}

	/**
	 * Der Spieler kann hiermit, statt einer laufenden Karte beizutreten, eine
	 * neue eröffnen und mit seinen Regeln konfigurieren.
	 *
	 * <code>$rules</code> bestimmt die Regeln und muss von folgendes Struktur
	 * sein:
	 * <code>
	 * $rules->public bool		Ob die Karte öffentlich zugänglich (TRUE) oder
	 * 							mit auth-key abgeschlossen sein soll (FALSE)
	 * 							[default: FALSE]
	 * $rules->mentor			Ob es eine Mentoreninsel sein soll
	 * $rules->map_key			Map-Key vorschreiben. Wenn leer, dann random.
	 * $rules->time_factor uint	Geschwindigkeitsfaktor [default: 1]
	 * </code>
	 *
	 * @param uint $template_id
	 * @param stdClass $rules
	 * @param bool $no_invitations		Einladungszwang abschalten
	 * @return {10000, 10001, ...} Homesector (lange ID)
	 */
	public function open_new_map($template_id, $rules=NULL,
		$no_invitations=false, $home=0, $color=0)
	{
		// MUTEX: Keine zwei Kartenzusagen gleichzeitig!!
		// (sonst würden Homesektoren doppelt vergeben)
		$sem_id = sem_get(MUTEX_MAP_CHOOSE, 1, 0600, -1);
		sem_acquire($sem_id);
		try {
			$free_home = $this->open_new_map_logic($template_id, $rules,
				$home, $color);

			// MUTEX
			sem_release($sem_id);

			/**
			 * Wenn er eine neue Karte eröffnet hat, dann muss er jetzt neue
			 * Leute auf die Insel einladen als ersten Schritt.
			 * Das kann abgeschaltet werden, indem $no_initations gesetzt wird.
			 *
			 * Bei öffentlichen Karten muss er natürlich keinen einladen!
			 */
			if (!$no_invitations && !$rules->public) {
				try {
					EventMessages::get_instance($this->uid)->add_entry(
						EventMessages::TYPE_OPEN_INVITATION_DIALOG
					);
				}
				catch(Exception $e) {
					error_log("Warning in {$e->getFile()} line {$e->getLine()}:"
						. $e->getMessage());
				}
			}

			return $free_home;
		}
		catch (IllegalArgumentException $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
		catch (Exception $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
	}

	/**
	 * Gibt alle verfügbaren Karten zurück
	 *
	 * @see include/classes/map_choose/MapChooser#get_available_maps()
	 * @return assoc[]
	 */
	public function get_available_maps() {
		if (empty($this->available_maps))
			$this->init_available_maps();
		return $this->available_maps;
	}

	/**
	 * Gibt alle wählbaren Templates zurück
	 *
	 * Der Rückgabewert ist ein Array von assotiatives Arraya mit folgenden
	 * Keys:
	 * <code>
	 * $a[]->template_id		Template-ID
	 * $a[]->num_players		Anzahl der Spieler, die auf einer Insel eines
	 * 							solchen Templates spielen können und sollen.
	 * </code>
	 *
	 * @return assoc[]
	 */
	public function get_available_templates() {
		return $this->available_templates;
	}
}
?>
