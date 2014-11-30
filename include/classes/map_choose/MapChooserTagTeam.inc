<?php
require_once(dirname(__FILE__).'/MapChooserAuthKey.inc');
require_once(dirname(__FILE__).'/../World.inc');
require_once(dirname(__FILE__).'/../host/HostControl.inc');
require_once(dirname(__FILE__).'/../exceptions/HackattackException.inc');
require_once(dirname(__FILE__).'/../../shared/classes/exceptions/'.
										'IllegalArgumentException.inc');

/**
 * Die Klasse ist zuständig dafür, dass der User eine FFA-Karte im
 * Tag-Team-Modus wählen kann, dass diese sich füllt und, wenn voll, gestartet
 * wird.
 *
 * Beim Tag-Team-Modus sind nur die Hälfte der Startplätze freigegeben, die
 * andere Hälfte ist reserviert für je einen Team-Partner, den der Besetzer
 * bestimmt. Auch externe können anhand der E-Mail-Adresse oder Facebook-ID
 * bestimmt werden.
 */
class MapChooserTagTeam extends MapChooserAuthKey
{
	/**
	 * Wieviele Spieler auf einer Karte sein dürfen. Wenn z.B. 4 in $ALLOWED_TYPES
	 * enthalten ist, dann heißt das, dass 4-Spieler-Karten auswählbar sind.
	 * @var int[]
	 */
	protected static $ALLOWED_TYPES = array(4,6,8);
	const MAPS_PER_ALLOWED_TYPE = 2;	// wie viele Karten pro Typ gleichzeitig
										// angeboten werden sollen

	const RESERVATION_TIMEOUT = 6480000;// Wann eine Reservierung verfällt in s

	/**
	 * Im Gegensatz zu $available_maps werden hier diejenigen gespeichert, die
	 * öffentlich zugänglich sind, in $available_maps dagegen alle.
	 *
	 * Das muss hier unterschieden werden, weil es verfügbare Karten gibt, die
	 * nur über Auth-Key erreichbar sind.
	 * @var assoc[]
	 */
	protected $public_available_maps;

	const DEBUG_VERBOSE = 0;

	private static $INSTANCES;

	/**
	 * @param uint $uid
	 * @return MapChooserTagTeam
	 */
	public static function get_instance($uid, $world_id=0) {
		if ($uid <= 0) {
			error_log(print_r(debug_backtrace(false),true));
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
	 * @return MapChooserTagTeam
	 */
	protected function __construct($uid, $world_id) {
		parent::__construct($uid, $world_id);
		$this->direct_start = true;
		$this->auth_key = null;	// Verzögert
	}

	/**
	 * Initialisiert alle Member-Variablen.
	 */
	protected function init() {
		$user = User::get_instance_by_id($this->uid);
		$cur_level = $user->get_cur_level();
		$this->available_maps = array();

		/**
		 * Wenn keine Welt angegeben wurde, wird die gewählt, die am besten
		 * passt
		 */
		if (!$this->world_id)
			$this->world_id = World::get_instance_by_level($cur_level,
				World::MAP_TYPE_TAG_TEAM)->get_id();

		/**
		 * Alle Karten anbieten, die nicht voll sind, egal, ob sie bereits
		 * gestartet sind, oder nicht.
		 *
		 * Hier ist zu beachten, dass Karten bereits dann voll sind, wenn die
		 * nicht reservierten Slots voll sind (denn die anderen Karten sollen
		 * nicht aufgeführt werden bei der Wahl). Leider habe ich keine Query
		 * gefunden, die mir genau das zurückgibt, aber eine, aus der man das
		 * dann direkt auslesen kann.
		 */

		/**
		 * Diese Query funktioniert so: Sie liest alle Heimatsektoren aus
		 * template_maps_teams aus, die als erstes besiedelt werden müssen
		 * (teams_size=2 AND fill_order=1), JOINT alle Karten, die diese
		 * Templates benutzen und JOINT alle Spieler auf den Karten, die
		 * auf einem solchen Homesector sitzen. Zurückgegeben wird dann die
		 * Anzahl der möglichen Erstsiedlungssektoren und die Anzahl der
		 * bereits belegten.
		 * Wenn der auth_key-Timeout abgelaufen ist (m.started<NOW-TIMEOUT),
		 * dann ist es egal, ob die fill_order=1 ist, denn dann ist die Karte
		 * komplett freigegeben. Dieser Fall wird durch das OR in der Query
		 * ausgedrückt.
		 * Beim durchlaufen der Ergebnisse werden dann die rausgefiltert, bei
		 * denen beide Werte gleich sind, denn dann sind alle
		 * Erstsiedlungssektoren der Karte bereits besetzt und man kann nur
		 * mit Auth-Key joinen.
		 * Ein weiterer LEFT JOIN auf die template_maps_1 ist nötig, um die
		 * Kartengröße in Mitspielern ermitteln zu können (num_players).
		 */

		$sql =
			"SELECT t.template_id, t1.num_players, m.world_id, m.map_id, ".
				"COUNT(*) AS total_first_hs_amount, ".
				"COUNT(p.homesector) AS occ_first_hs_amount ".
			"FROM template_maps_teams AS t ".
			"LEFT JOIN template_maps_1 AS t1 ".
				"ON t1.template_id=t.template_id ".
			"LEFT JOIN maps AS m ".
				"ON m.template_id=t.template_id ".
			"LEFT JOIN players_on_map AS p ".
				"ON p.world_id=m.world_id AND p.map_id=m.map_id ".
					"AND p.homesector=t.sector_id ".
			"WHERE (t.fill_order=1 OR ".
				"(m.started!=0 AND m.started<".
					"(".(Time::get_t_now()-self::RESERVATION_TIMEOUT)."))) ".
				"AND m.started <= (".(Time::get_t_now()
							- MapDescription::MAP_FILL_TIMEOUT).")".
				"AND m.world_id={$this->world_id} ".
				"AND t.team_size=2 ".
				"AND t1.num_players IN ("
							.implode(', ', self::$ALLOWED_TYPES) . ") ".
				"GROUP BY m.world_id*10000000+m.map_id";
		$result = query($sql);

		$not_existing_maps = array();
		foreach (self::$ALLOWED_TYPES as $num_players) {
			/**
			 * Zunächst wird einmal davon ausgegangen, dass keine Karte
			 * in der DB exisitert
			 */
			$not_existing_maps[$num_players] = array();
			for ($i = 0; $i < self::MAPS_PER_ALLOWED_TYPE; $i++) {
				$not_existing_maps[$num_players][$i] = true;
			}
		}

		/**
		 * User aus der players_on_map-Tabelle holen, da die Karte
		 * (wenn nicht leer) schon läuft.
		 */
		while ($row = mysql_fetch_assoc($result)) {

			$res = query (
				'SELECT user_id,kill_time FROM players_on_map '.
				'WHERE world_id='.$row['world_id'].
					' AND map_id='.$row['map_id']	// AND kill_time=0
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
				'map_id'		=>	$row['map_id']
			);
			/**
			 * Wie bei der Query-Beschreibung angekündigt werden die Karten
			 * rausgefiltert, deren Erstsiedlungssektoren gesamt so viele sind
			 * wie die bereits besetzten; denn dann ist die Karte nur noch
			 * über Einladungen zu erreichen.
			 */
			$map_data['public'] = ($row['total_first_hs_amount']
				> $row['occ_first_hs_amount']);

			$this->available_maps[] = $map_data;

			if ($map_data['public']) {
				/**
				 * Eine $row['num_players']-Spieler-Karte existiert doch (ist
				 * öffentlich verfügbar), also poppen wir :)
				 */
				array_pop($not_existing_maps[$row['num_players']]);
			}
		}

		/**
		 * Es kann vorkommen, dass nicht alle verfügbaren Karten in der DB
		 * eingetragen sind. Früher wurden z.B. die Karten nur dann eingetragen,
		 * wenn irgendwann einmal ein User darauf war. Heute ist es so, dass
		 * diese zwar direkt eingetragen werden, aber erst, wenn eine World
		 * zum erstenmal betreten wird. Werden die $ALLOWED_TYPES modifiziert,
		 * werden auch Lücken existieren.
		 * Daher werden hier nun alle nicht existenten Karten nachgetragen.
		 */
		foreach ($not_existing_maps as $num_players => $bool_array) {
			$sql =
				'SELECT template_id FROM template_maps_1 '.
				'WHERE state=2 '.
				'AND type=(type|'.MapTemplate::TYPE_SUITABLE_FOR_TAG_TEAM.') '.
					"AND num_players=$num_players";
			$res = query($sql);
			if (mysql_num_rows($res) == 0) {
				error_log("WARNING: No suitable template found for ".
						"$num_players players in Tag-Team-Mode.");
				continue;
			}
			$matching_template_ids = array();
			while ($row = mysql_fetch_assoc($res))
				$matching_template_ids[] = $row['template_id'];

			foreach ($bool_array as $jnk) {
				$pair = random_value_key_pair($matching_template_ids);
				if (count($matching_template_ids) > 1)
					unset($matching_template_ids[$pair['key']]);
				$template_id = $pair['value'];

				$map_id = World::get_instance($this->world_id)
					->insert_map($template_id, World::MAP_TYPE_TAG_TEAM);

				$map_data = array (
					'num_players'	=>	$num_players,
					'users_on_map'	=>	array(),
					'dead_users'	=>	array(),
					'template_id'	=>	$template_id,
					'map_id'		=>	$map_id,
					'world_id'		=>	$this->world_id,
					'public'		=>	true
				);
				$this->available_maps[] = $map_data;
			}
		}
		usort($this->available_maps, array('MapChooserFFA',
			'compare_num_players'));

		// Abschließend wird der chosen_map_index rekonstruiert.
		// FIXME: ge_twaiting_map_id is obsolete
		$chosen_map_id =
			User::get_instance_by_id($this->uid)->get_waiting_map_id()%100;
		$chosen_world_id =
			floor(User::get_instance_by_id($this->uid)->get_waiting_map_id()/100);
		if ($chosen_map_id) {
			foreach ($this->available_maps as $index => $map) {
				if (($map['world_id'] == $chosen_world_id)
					&& ($map['map_id'] == $chosen_map_id))
				{
					$this->chosen_map_index = $index;
					break;
				}
			}
		}
	}

	/**
	 * Gibt die Daten zurück, die dem entsprechenden Auth-Key zugeordnet sind.
	 *
	 * Die Rückgabe ist ein Standartobjekt folgender Struktor:
	 *
	 * $o->world_id
	 * $o->map_id		ID der Karte, für die der Key gilt
	 * $o->sector_ids	Liste der Sektoren, die in Frage kommen. Ist die Liste
	 * 					leer, sind entweder alle möglichen Sektoren belegt, oder
	 * 					die Reservierungsdauer ist ausgelaufen.
	 * 					Dann kann höchstens versucht werden, ohne Authkey normal
	 * 					der Karte beizutreten (falls sie nicht voll ist).
	 * $o->res_by_uid	ID des Users, der den Authkey gesetzt hat
	 * $o->min_level	Die Stufe, die der User mindestens braucht, um hier
	 * 					mitspielen zu dürfen
	 * $o->max_level	Die Stufe, die der User maximal haben darf, um hier ohne
	 * 					Handicap mitspielen zu dürfen
	 *
	 * @param string $auth_key
	 * @return stdClass[]
	 */
	public static function get_data_by_auth_key($auth_key) {

		$res = query(
			"SELECT r.res_by_uid, t.sector_id, m.map_id, r.world_id ".
			"FROM map_reserved_slots AS r ".
			"LEFT JOIN maps AS m ".
				"ON m.map_id=r.map_id AND m.world_id=r.world_id ".
			"LEFT JOIN template_maps_teams AS t ".
				"ON t.team_id=r.team_id ".
					"AND m.template_id=t.template_id ".
					"AND t.team_size=2 ".
					"AND t.fill_order>1 ".
			"WHERE auth_key='$auth_key'"
		);
		if (mysql_num_rows($res) == 0) {
			throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);
		}

		$result = new stdClass();
		$row = mysql_fetch_object($res);

		$result->min_level =
			World::get_instance($row->world_id)->get_minimum_level();
		$result->max_level =
			World::get_instance($row->world_id)->get_maximum_level();
		$result->map_id = $row->map_id;
		$result->world_id = $row->world_id;
		$result->res_by_uid = $row->res_by_uid;
		$result->sector_ids[] = $row->sector_id;
		while ($row = mysql_fetch_object($res)) {
			$result->sector_ids[] = $row->sector_id;
		}

		return $result;
	}

	/**
	 * Gibt den Authentifizierungsschlüssel zurück, den der angebene User für
	 * die Karte, auf der er gerade spielt, zugewiesen bekommen hat.
	 *
	 * Mit diesem Schlüssel kann er weitere Spieler in sein Team auf der Karte
	 * einladen.
	 *
	 * @param {1, 2, ...} $uid
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
			"SELECT auth_key FROM map_reserved_slots ".
			"WHERE res_by_uid=$uid ".
				"AND world_id=$world_id AND map_id=$map_id"
		),0);
	}

	/**
	 * Gibt zurück, welche Sektoren der angegebene User auf seiner aktuellen
	 * Karte reserviert hat.
	 *
	 * Ist die Reservierungszeit abgelaufen, wird ein leeres Array
	 * zurückgegeben, sonst ein Array mit den entsprechenden Sektor-IDs
	 *
	 * @param uint $uid
	 * @return {0,1,...99}[]
	 */
	public static function get_reserved_sectors_for_current_map($uid) {
		if ($uid <= 0) {
			throw new IllegalArgumentException("uid", "N1", $uid);
		}
		$uid = (int)$uid;
		$map_id = User::get_instance_by_id($uid)->get_map_id();
		$world_id = User::get_instance_by_id($uid)->get_world_id();

		$started = MapDescription::get_instance($world_id, $map_id)
			->get_time_started();

		if ($started < (Time::get_t_now() - self::RESERVATION_TIMEOUT))
			return array();

		$res = query(
			"SELECT t.sector_id FROM map_reserved_slots AS r ".
			"LEFT JOIN maps AS m ".
				"ON m.map_id=(100*r.world_id+r.map_id) ".
			"LEFT JOIN template_maps_teams AS t ".
				"ON t.team_id=r.team_id AND t.team_size=2 ".
				"AND t.template_id=m.template_id ".
			"LEFT JOIN players_on_map AS p ".
				"ON p.map_id=m.map_id AND p.homesector=t.sector_id ".
			"WHERE r.res_by_uid=$uid AND r.map_id=$map_id AND ".
				"r.world_id=$world_id AND p.homesector IS NULL ".
				"AND t.sector_id IS NOT NULL"
		);
		$result = array();
		while ($row = mysql_fetch_row($res)) {
			$result[] = $row[0];
		}
		return $result;
	}

	/**
	 * Hilfsfunktion zum Festlegen des Authentifizierungsschlüssels.
	 *
	 * @param uint $map_id
	 * @param uint $team
	 * @param char[6] $auth_key
	 */
	private function insert_auth_key($map_id, $team, $auth_key) {
		$world_id = $this->world_id;
		query(
			"INSERT INTO map_reserved_slots SET ".
				"res_by_uid={$this->uid},".
				"world_id=$world_id, ".
				"map_id=$map_id, ".
				"team_id=$team, ".
				"auth_key='$auth_key', ".
				"t_stamp=".Time::get_t_now(). " ".
			"ON DUPLICATE KEY UPDATE ".
				"world_id=$world_id, ".
				"map_id=$map_id, ".
				"team_id=$team, ".
				"auth_key='$auth_key', ".
				"t_stamp=".Time::get_t_now()
		);
		$this->auth_key = $auth_key;
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

		try {
			$this->unchoose_map();
			// Die folgende clevere Query bestimmt alle Karten, die noch freie
			// Heimatsektoren haben, für die kein Auth-Key nötig ist.
			$res = query (
				"SELECT m.map_id, m.started, t.sector_id, t.team_id ".
				"FROM maps AS m ".
				"LEFT JOIN template_maps_teams AS t ".
					"ON m.template_id=t.template_id ".
				"LEFT JOIN players_on_map AS p ".
					"ON p.map_id=m.map_id AND p.homesector=t.sector_id ".
				"WHERE t.team_size=2 AND (t.fill_order=1 OR ".
					"(m.started!=0 AND m.started<".
						"(".(Time::get_t_now()-self::RESERVATION_TIMEOUT)."))) ".
					"AND m.map_id<".($this->world_id*100+100)." ".
					"AND m.map_id>=".($this->world_id*100)." ".
					"AND p.homesector IS NULL"
			);
			if (mysql_num_rows($res) == 0) {
				throw new ExceptionWithErrorlog(
					"No suitable map found in random_map",
					$this->uid
				);
			}

			/**
			 * Die gefundenen Karten / Heimatsektoren durchtesten. Der
			 * einzige Fehler, der passieren kann, ist, dass der Spieler schon
			 * auf der Karte war. Das hab ich in die Query nicht mehr
			 * reinbekommen. Die Zeile, für die das nicht zutrifft, wird
			 * genommen.
			 */
			$row_found = false;
			while (($row = mysql_fetch_assoc($res)) && !$row_found) {
				$map_id = $row['map_id'];
				$sector_id = $row['sector_id'];

				if (($row['started'] == 0) || ($row['started'] >=
						(Time::get_t_now() - self::RESERVATION_TIMEOUT)))
				{
					/**
					 * Ob eine Platzreservierung noch möglich ist
					 * @var bool
					 */
					$reservation_possible = true;
					$team = $row['team_id'];
				}
				else {
					/**
					 * Ob eine Platzreservierung noch möglich ist
					 * @var bool
					 */
					$reservation_possible = false;
				}


				// $map_index bestimmen (s. choose_map()):
				$n = count($this->available_maps);
				$map_index = 0;
				while (($map_index < $n) &&
						($this->available_maps[$map_index]['map_id'] != $map_id))
				{
					$map_index++;
				}

				$map = $this->available_maps[$map_index];

				if (!in_array($this->uid, $map['users_on_map']) &&
					!in_array($this->uid, $map['dead_users']))
				{
					$row_found = true;
				}
			}

			/**
			 * Wenn keine Karte gefunden wurde, ist der Grund klar: Der User
			 * war schon auf allen potentiellen Karten!
			 * => Exception
			 */
			if (!$row_found) {
				throw new Exception("", self::ERR_USER_IS_OR_WAS_ON_MAP);
			}

			$free_home =  $sector_id + $map['map_id']*100;

			$user = User::get_instance_by_id($this->uid);
			// Wartestatus: Er wartet auf ne volle Map
			$user->set_waiting_homesector($free_home);
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

			if ($reservation_possible) {
				/**
				 * Jetzt kommt der reservierte Slot ins Spiel. Der User darf einen
				 * weiteren einladen.
				 */
				$auth_key = self::create_new_auth_key();
				$map_id = $map['map_id'] % 100;
				$this->insert_auth_key($map_id, $team, $auth_key);
			}

			// MUTEX
			sem_release($sem_id);

			/**
			 * Beim Direkten Start schalten wir von "wartend" direkt auf
			 * "wartend und fertig" um, da es keinen Warteraum gibt, wo Spieler
			 * markieren müssen, dass sie fertig sind und auch wirklich sicher.
			 */
			$this->toggle_ready_state();

			return $new_home;
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
	 * @return uint				Die komplette Sektor-ID des Heimatsektors, der
	 * 							dem User zugewiesen wurde.
	 */
	public function choose_map_by_auth_key($auth_key) {
		// MUTEX: Keine zwei Kartenzusagen gleichzeitig!!
		// (sonst würden Homesektoren doppelt vergeben)
		$sem_id = sem_get(MUTEX_MAP_CHOOSE, 1, 0600, -1);
		sem_acquire($sem_id);

		try {
			$res = query(
				"SELECT world_id, map_id, team_id FROM map_reserved_slots ".
				"WHERE auth_key='$auth_key' LIMIT 1"
			);
			if (mysql_num_rows($res) == 0) {
				throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);
			}
			$row = mysql_fetch_assoc($res);
			$map_id = $row['world_id']*100 + ($row['map_id'] % 100);
			$team_id = $row['team_id'];

			// $map_index bestimmen (s. choose_map()):
			$n = count($this->available_maps);
			$map_index = 0;
			while (($map_index < $n) &&
					($this->available_maps[$map_index]['map_id'] != $map_id))
			{
				$map_index++;
			}

			if ($map_index == $n){	// Map ist nicht bekannt
				throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);
			}

			$this->unchoose_map();

			$map = $this->available_maps[$map_index];

			if (in_array($this->uid, $map['users_on_map']) ||
				in_array($this->uid, $map['dead_users']))
			{
				throw new Exception("", self::ERR_USER_IS_OR_WAS_ON_MAP);
			}

			/* Jetzt schauen, ob in dem Team, für das er sich beworben hat
			   noch ein Heimatsektor frei ist */
			$sql =
				"SELECT sector_id FROM template_maps_teams ".
				"WHERE template_id={$map['template_id']} ".
					"AND team_size=2 ".
					"AND team_id=$team_id ".
					"AND fill_order>1";	// Erster HS sollte ja besetzt sein

			$res = query($sql);
			if (mysql_num_rows($res) < 1)
				throw new Exception("", self::ERR_NO_HS_FOUND);
			$homes = array();
			// und in $homes speichern
			while ($row = mysql_fetch_assoc($res))
				$homes[$row['sector_id']] = true;

			/**
			 * Beim Direkten Start wird nicht gewartet, die homesectoren sind
			 * nicht in waiting_homesectors gespeichert, sondern in homesectors.
			 * Nun kann es aber sein, dass User auf der KArte bereits diese
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
				"SELECT homesector FROM players_on_map ".
				"WHERE map_id={$map['map_id']}";
			$res = query($sql);
			// und aus $homes wieder rauswerfen
			while ($row = mysql_fetch_assoc($res))
				unset($homes[$row['homesector']]);

			if (count($homes) == 0) {
				throw new Exception("", self::ERR_MAP_ALREADY_FULL);
			}
			// Abschließend von den verbleibenden Homes zufällig eins aussuchen
			$sector_id = random_key($homes);
			/**
			 * Okay, der User darf hier siedeln :)
			 */
			$new_home = $sector_id + $map['map_id']*100;
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
			// MUTEX
			sem_release($sem_id);

			/**
			 * Beim Direkten Start schalten wir von "wartend" direkt auf
			 * "wartend und fertig" um, da es keinen Warteraum gibt, wo Spieler
			 * markieren müssen, dass sie fertig sind und auch wirklich sicher.
			 */
			$this->toggle_ready_state();

			/**
			 * Wenn der User den letzten freien Platz belegt hat, kann der
			 * Auth-Key aus der DB gelöscht werden.
			 */
			if (count($homes) == 1) {
				/**
				 * Kann er nicht, weil er die einzige Möglichkeit darstellt,
				 * dass man die Karte über den Auth-Key wiederfinden kann.
				 */
			/*	query(
					"DELETE FROM map_reserved_slots ".
					"WHERE auth_key='$auth_key'"
				);*/
			}

			return $new_home;
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
	 * Setzt den User auf die Map mit der angegebenen ID
	 *
	 * @param uint $map_index	Der User wird auf die
	 * 							Karte available_maps[$map_index] gesetzt
	 * @return uint				Die komplette Sektor-ID des Heimatsektors, der
	 * 							dem User zugewiesen wurde.
	 */
	public function choose_map($map_index, $auth_key='') {
		if ($map_index < 0) {
			throw new IllegalArgumentException("map_index", "N0", $map_index);
		}
		// MUTEX: Keine zwei Kartenzusagen gleichzeitig!!
		// (sonst würden Homesektoren doppelt vergeben)
		$sem_id = sem_get(MUTEX_MAP_CHOOSE, 1, 0600, -1);
		sem_acquire($sem_id);

		if (self::DEBUG_VERBOSE > 0) {
			error_log("MapChooserTagTeam::\$INSTANCES[{$this->uid}]->choose_map($map_index) called");
		}
		try {
			$this->unchoose_map();

			$map = $this->available_maps[$map_index];
			if (empty($map) || empty($map['template_id']) || empty($map['map_id']))
				throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);

			if (in_array($this->uid, $map['users_on_map']) ||
				in_array($this->uid, $map['dead_users']))
			{
				throw new Exception("", self::ERR_USER_IS_OR_WAS_ON_MAP);
			}

			$started =
				MapDescription::get_instance($map['map_id'])->get_time_started();

			/**
			 * Ob eine Platzreservierung noch möglich ist
			 * @var bool
			 */
			$reservation_possible = true;
			/**
			 * Wenn die Reservierungszeit abgelaufen ist, dann ist die Karte
			 * freigegeben für alle...
			 */
			if (($started > 0) &&
				($started <	(Time::get_t_now() - self::RESERVATION_TIMEOUT)))
			{
				$sql =
					"SELECT sector_id FROM template_maps ".
					"WHERE template_id={$map['template_id']} AND home>0";
				$reservation_possible = false;
			}
			/**
			 * Sonst nur alle "ersten" Heimatsektoren der Karte holen
			// (erster HS bedeutet, dass er vom Team her zuerst gefüllt wird)
			 */
			else {
				$sql =
					"SELECT sector_id, team_id AS team FROM template_maps_teams ".
					"WHERE template_id={$map['template_id']} ".
						"AND team_size=2 ".
						"AND fill_order=1";
			}

			$res = query($sql);
			if (mysql_num_rows($res) < 1)
				throw new Exception("", self::ERR_NO_HS_FOUND);
			$homes = array();
			if ($reservation_possible)
				$teams = array();
			// und in $homes speichern
			while ($row = mysql_fetch_assoc($res)) {
				if (!$row['team'] && $reservation_possible)
					throw new Exception("", self::ERR_NO_HS_FOUND);
				$homes[$row['sector_id']] = true;
				if ($reservation_possible)
					$teams[$row['sector_id']] = $row['team'];
			}

			/**
			 * Beim Direkten Start wird nicht gewartet, die homesectoren sind
			 * nicht in waiting_homesectors gespeichert, sondern in homesectors.
			 * Nun kann es aber sein, dass User auf der KArte bereits diese
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
				"SELECT homesector FROM players_on_map ".
				"WHERE map_id={$map['map_id']}";
			$res = query($sql);
			// und aus $homes wieder rauswerfen
			while ($row = mysql_fetch_assoc($res))
				unset($homes[$row['homesector']]);

			if (count($homes) == 0) {
				throw new Exception("", self::ERR_MAP_ALREADY_FULL);
			}
			// Abschließend von den verbleibenden Homes zufällig eins aussuchen
			$sector_id = random_key($homes);
			$free_home =  $sector_id + $map['map_id']*100;

			$user = User::get_instance_by_id($this->uid);
			// Wartestatus: Er wartet auf ne volle Map
			$user->set_waiting_homesector($free_home);
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

			if ($reservation_possible) {
				/**
				 * Jetzt kommt der reservierte Slot ins Spiel. Der User darf einen
				 * weiteren einladen.
				 */
				$team = $teams[$sector_id];
				$auth_key = self::create_new_auth_key();
				$map_id = $map['map_id'] % 100;
				$this->insert_auth_key($map_id, $team, $auth_key);
			}

			// MUTEX
			sem_release($sem_id);

			/**
			 * Beim Direkten Start schalten wir von "wartend" direkt auf
			 * "wartend und fertig" um, da es keinen Warteraum gibt, wo Spieler
			 * markieren müssen, dass sie fertig sind und auch wirklich sicher.
			 */
			$this->toggle_ready_state();
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
	 * Gibt den aktuellen Auth-Key zurück
	 *
	 * @return char[6]
	 */
	public function get_auth_key() {
		if ($this->auth_key === null) {
			$this->auth_key = @mysql_result(query (
				"SELECT auth_key FROM map_reserved_slots ".
				"WHERE res_by_uid={$this->uid}"
			), 0);
		}
		return $this->auth_key;
	}

	/**
	 * Gibt alle ÖFFENTLICH verfügbaren Karten zurück.
	 *
	 * @see include/classes/MapChooser#get_available_maps()
	 */
	public function get_available_maps() {
		$result = null;
		foreach ($this->available_maps as $key => $value) {
			if ($value['public'])
				$result[$key] = $value;
		}
		return $result;
	}

}
?>