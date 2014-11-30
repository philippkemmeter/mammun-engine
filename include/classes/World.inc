<?php
require_once(dirname(__FILE__).'/../shared/classes/IWorld.inc');

/**
 * Repräsentiert eine World. Eine World ist eine Gruppe von Karten, die alle
 * für die gleiche Spielerstärke (ungefähr) freigegeben ist. Eine World ist
 * definiert durch die maximale und minimale Stufe, die ein User haben darf,
 * um eine Karte innerhalb dieser Gruppe betreten zu drüfen, und anderen
 * zusätzlichen Eigenschaften.
 * Worlds können jederzeit neu erzeugt werden. Sie erzeugen sich auch selbst neu,
 * sobald eine Situation dies erfordert (keine World für einen User verfügbar)
 */

class World implements IWorld {
	private $id;
	private $free_maps;	// Jede Welt fasst 100 Karten. Sind keine mehr frei,
						// muss eine neue Welt erschaffen werden
	private $wait_maps;	// Wieviele Karten geöffnet, aber nicht gestartet wurden
	private $minimum_level;	// Welche Stufe ein Nutzer minimal haben muss, um
							// einer Karte dieser Welt beizutreten
	private $maximum_level;	// Welche Stufe ein Nutzer maximal haben darf, um
							// einer Karte dieser Welt beizutreten
	private $type;			// Um welchen Kartentyp handelt es sich
	private $name_index;	// Index, der den Namen der Insel bestimmt
	/**
	 * Die folgenden Variablen sind nötig, um die Welten zu verknüpfen in eine
	 * verkettete Liste. Dies ist dann nötig, wenn eine Welt voll von Karten
	 * wurde, und eine neue, gleichwertige eröffnet werden muss. Der Nutzer
	 * merkt keine Unterscheidung zwischen den Welten - dies ist rein intern.
	 * Somit werden Welten unbegrenzt groß und sind nicht mehr auf die 100 Karten
	 * limitiert, wie es die ursprüngliche Kodierung vorsieht.
	 */
	private $cur_joint_world;	// welche Welt die aktuell letzte Welt der Liste ist(ID)
	private $next_joint_world;	// welche Welt die nächste in der Liste ist (ID)
	private $first_joint_world;	// welche Welt die erste in der Liste ist (ID)
	private $map_id_offset;		// Offset der Karten-ID bei der Anzeige

	private $data_changed = false;	// Ob sich die Daten geändert haben seit
									// Objekterzeugung. Wenn ja, wird beim
									// Zerstören in die DB gesichert.

	private static $INSTANCES = array();

	/**
	 *
	 * @param uint $id
	 * @return World
	 * @throws Exception
	 */
	public static function get_instance($id) {
		if (!isset(self::$INSTANCES[$id]))
			self::$INSTANCES[$id] = new self($id);
		return self::$INSTANCES[$id];
	}

	/**
	 * Jede Welt kann auch durch eine Level-Typ-Anfrage bestimmt werden,
	 * da kein Level zwei Welten eines Typs zugeordnet ist.
	 * @param int $level
	 * @param int $type
	 * @return World
	 * @throws Exception
	 */
	public static function get_instance_by_level($level, $type) {
		$sql =
			"SELECT id FROM worlds ".
			"WHERE minimum_level<=$level AND maximum_level>=$level ".
				"AND type=$type AND id=cur_joint_world";
		$res = query($sql);
		if (mysql_num_rows($res) == 0) {
			throw new ExceptionWithErrorlog(
				"Error while trying to reach a world...",
				User::get_current_user()->get_id(),
				'Query: '.$sql."\n".
				'backtrace: '.print_r(debug_backtrace(false),true)
			);
		}

		$id = mysql_result($res, 0);

		return self::get_instance($id);
	}

	/**
	 * Gibt ein Array von Welt-Daten-Objekten zurück, die dem angegebenen Typ
	 * entsprechen (s. MAP_TYPE-Konstanten).
	 *
	 * Der Rückgabe-Wert ist dabei ein Array von Objekten folgender Struktur:
	 * <code>
	 * $o->id               uint    Welt-ID
	 * $o->name_index		uint	Index, der den Weltnamen bestimmt
	 * $o->minimum_level    uint    Welches Level minimal benötigt wird
	 * $o->maximum_level    uint    Welches Level maximal erlaubt ist
	 * </code>
	 *
	 * @param int $type
	 * @return stdClass[]
	 */
	public static function get_world_ids_and_range($type)
	{
		$res = query(
			"SELECT id, name_index, minimum_level, ".
				"maximum_level FROM worlds ".
			"WHERE type=$type AND id=cur_joint_world ".
			"ORDER BY minimum_level"
		);

		$result = array();
		while ($row = mysql_fetch_object($res)) {
			$result[] = $row;
		}
		return $result;
	}


	private function __construct($id) {
		$this->data_changed = false;
		$this->init_world_by_id((int)$id);
	}

	private function init_world_by_id($id) {
		query("START TRANSACTION");
		$sql = "SELECT * FROM worlds WHERE id=$id";
		$res = query($sql);
		if (mysql_num_rows($res) == 0) {
			query("ROLLBACK");
			throw new ExceptionWithErrorlog(
				"Error while trying to reach a world...",
				User::get_current_user()->get_id(),
				'Query: '.$sql."\n".
				'backtrace: '.print_r(debug_backtrace(false),true)
			);
		}
		$data = mysql_fetch_assoc($res);
		/**
		 * Wenn wir einen Verweis auf eine andere World-ID finden, ist diese World
		 * mit (mindestens) einer anderen verknüpft. Wir wollen immer die Daten
		 * der letzten verknüpften Welt haben, also folgen wir dem Verweis zur
		 * letzten Welt.
		 */
		if ($data['cur_joint_world'] != $data['id']) {
			query("ROLLBACK");
			return self::init_world_by_id($data['cur_joint_world']);
		}
		/**
		 * Die aktuelle Welt ist voll. Es muss eine neue Kopie erstellt und
		 * verknüpft werden, damit man wieder Platz für Karten hat.
		 */
		else if ($data['free_maps'] < 2) {
			$sql =
				"INSERT INTO worlds SET ".
				"	name_index={$data['name_index']}, ".
				"	free_maps=100, wait_maps=0, ".
				"	minimum_level={$data['minimum_level']},".
				"	maximum_level={$data['maximum_level']},".
				"	type={$data['type']}, next_joint_world=0,".
				"	first_joint_world={$data['first_joint_world']},".
				"	map_id_offset=".($data['map_id_offset']+100);
			query($sql);
			$id = mysql_result(query(
				"SELECT id FROM worlds WHERE ".
				"first_joint_world={$data['first_joint_world']} ".
				"AND map_id_offset=".($data['map_id_offset']+100). " ".
				"AND next_joint_world=0"
			),0);
//			$id = mysql_insert_id();
			// cur_joint_world bei allen im Verbund neu setzen
			$sql =
				"UPDATE worlds SET cur_joint_world=$id ".
				"WHERE first_joint_world={$data['first_joint_world']}";
			query($sql);
			// next_joint_world bei der gerade voll gewordenen Welt auf die neue setzen
			$sql =
				"UPDATE worlds SET next_joint_world=$id ".
				"WHERE id={$data['id']}";
			query($sql);

			$data['id'] = $id;
			$data['free_maps'] = 100;
			$data['wait_maps'] = 0;
			$data['next_joint_world'] = 0;
			$data['cur_joint_world'] = $id;
			$data['map_id_offset']+=100;
			//$this->data_changed wird nicht angefasst, da schon alles on the
			//fly sync gehalten wurde.
		}
		query("COMMIT");
		foreach ($data as $key => $value)
			$this->$key = $value;
	}

	/**
	 * Fügt eine neue Map in diese World hinzu.
	 * @param int $template_id		Welches Template die Karte benutzen soll
	 * @param MAP_TYPE $type		Welchen Typ die Karte haben soll
	 * @param int $time_factor		Mit welchem Zeitfaktor die Karte versehen
	 * 								werden soll
	 * @param bool $mentor			Ob die Karte eine Mentorenkarte sein soll
	 * @return int 					Gibt die ID der neuen Karte zurück
	 */
	public function insert_map($template_id, $type, $time_factor=1,
		$mentor=false)
	{

		if ($type < 0 || $type > self::MAP_TYPE_MAX)
			throw new IllegalArgumentException("type",
							"{0, 1, ..., ".self::MAP_TYPE_MAX."}", $type);
		if ($template_id <= 0)
			throw new IllegalArgumentException("template_id", "N\{0}",
																$template_id);

		// MUTEX: Keine zwei Karteneinfügungen gleichzeitig!!
		// (sonst läuft die Welt über!)
		$sem_id = sem_get(MUTEX_WORLD_MUTEX, 1, 0600, -1);
		sem_acquire($sem_id);

		try {
			// Daten müssen ganz frisch sein!
			$this->init_world_by_id($this->get_id());

			$template_id = (int)$template_id;
			$time_factor = (int)$time_factor;
			if ($time_factor < 1)
				$time_factor = 1;
			$t_now = Time::get_t_now();

			$mentor = ($mentor) ? 1 : 0;

			$world_id = $this->get_id();
			$map_id = 100 - $this->free_maps;
			$sql =
				"INSERT INTO maps SET map_id=$map_id, world_id=$world_id, ".
					"timefactor=$time_factor, ".
					"type=$type, state=1, template_id=$template_id, ".
					"inserted=$t_now, mentor=$mentor";
			query($sql);

			$result = query(
				"SELECT * FROM template_maps WHERE status=0 ".
					"AND template_id=$template_id"
			);
			while ($tmpl = mysql_fetch_assoc($result)) {
				$sector_id = $tmpl['sector_id'];
//				if ($tmpl['home']) {
//					/**
//					 * Bei SinglePlayer-Maps wird gegen den Host gespielt. Seine
//					 * Gebäude werden nun in seinem Heimatsektor platziert.
//					 */
//					if (($tmpl['home'] == 2) && ($type == self::MAP_TYPE_SP)) {
//						$host_id = HostControl::USER_HOST_ID;
//						$sql =
//							"INSERT INTO building SET uid=$host_id, time_built=$t_now,".
//							"	finish_time=$t_now, ".
//							"	time_build_started=$t_now, type=".Buildings::MOTHER_FUNGUS.",".
//							"	cur_level=1, ulc='00', sector_id=$sector_id";
//						query($sql);
//						$sql =
//							"INSERT INTO building SET uid=$host_id, time_built=$t_now,".
//							"	finish_time=$t_now, ".
//							"	time_build_started=$t_now, type=".Buildings::TECHCENTER.",".
//							"	cur_level=1, ulc='04', sector_id=$sector_id";
//						query($sql);
//					}
//
//				}
//
				/**
				 * Hier wird erstmal nur markiert, ob ein Artefakt im Sektor ist.
				 * Welches Artefakt es dann wird, wird erst beim Start der Karte
				 * entschieden (s. MapChooser::start_map())
				 */
				$artifact = ($tmpl['artifact']) ? 1 : 0;
				$sql =
					"INSERT INTO sectors SET world_id=$world_id, ".
						"map_id=$map_id, ".
						"sector_id={$tmpl['sector_id']}, ".
						"artifact=$artifact, ".
						"host_activity={$tmpl['host_activity']}, ".
						"bio={$tmpl['bio']}, ".
						"domination_point={$tmpl['domination_point']}";
				query($sql);
			}
			$this->free_maps--;
			$this->wait_maps++;
			/**
			 * DIREKT AKTUALLISIEREN!
			 * SONST WIRD MUTEX UMGANGEN!
			 */
			$sql =
				"UPDATE worlds SET free_maps={$this->free_maps}, ".
				"	wait_maps={$this->wait_maps} ".
				"WHERE id={$this->id}";
			query($sql);
			// MUTEX
			sem_release($sem_id);
			return $map_id;
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
	 * Beendet eine Karte
	 *
	 * @param $finish_time
	 * @param $winner_uid
	 * @param $winner_hs_id
	 * @return unknown_type
	 */
	public function finish_map($map_id, $finish_time, $winner_uid,
		$winner_hs_id)
	{
		$map_desc = MapDescription::get_instance($this->id, $map_id);
		$map_desc->set_state(2);
		$map_desc->set_winner($winner_uid);
		$map_desc->set_time_finished($finish_time);
		$map_desc->set_winner_homesector_id($winner_hs_id);

		$map_players = MapPlayers::get_instance($this->id, $map_id);

		/**
		 * Nun wird geschaut, ob noch Spieler auf der Karte sind. Das kann
		 * sein, weil eine Karte ggf. gerade abgrebrochen wird.
		 * Alle Spieler, die noch auf der Karte sind, sollen diese jetzt
		 * verlassen.
		 */

		$users_still_on_map = $map_players->get_users_on_map(true);
		foreach ($users_still_on_map as $user) {
			if ($user->get_state() == User::STATE_PLAYING) {
				error_log("Calling leave_map({$user->get_id()}, false)".
							" in World.inc line 340.");
				$map_players->leave_map($user->get_id(), false);
			}
		}
	}

	/**
	 * Gibt die ID der World zurück.
	 *
	 * @return uint
	 */
	public function get_id() {
		return $this->id;
	}

	/**
	 * Gibt die Stufe zurück, die ein Spieler mindestens haben muss, um diese
	 * Welt betreten zu dürfen.
	 *
	 * @return uint
	 */
	public function get_minimum_level() {
		return $this->minimum_level;
	}

	/**
	 * Gibt die Stufe zurück, die ein Spieler höchstens haben darf, um diese
	 * Karte handycap-frei betreten zu dürfen.
	 *
	 * @return uint
	 */
	public function get_maximum_level() {
		return $this->maximum_level;
	}

	/**
	 * Gibt den Namensindex zurück.
	 *
	 * Jeder Weltverbund, der aus mehreren Welten in der DB bestehen kann, hat
	 * einen gemeinsamen Weltnamensindex, der angibt, welcher Name der Welt
	 * gegeben werden soll. Die Indizes sind durchnummeriert.
	 *
	 * @return uint
	 */
	public function get_name_index() {
		return $this->name_index;
	}

	/**
	 * Gibt den Typ der Welt zurück.
	 *
	 * Siehe IWorld-Konstanten.
	 * @return {0, 1,... IWorld::MAP_TYPE_MAX}
	 * @see include/shared/classes/IWorld.inc
	 */
	public function get_type() {
		return $this->type;
	}


	/**
	 * !!!ACHTUNG: Setzt alle Kartendaten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		/* Alle Maps freigeben, sowie die Joints aufheben */
		query (
			"UPDATE worlds SET free_maps=100, wait_maps=0, ".
				"next_joint_world=0, cur_joint_world=id"
		);

		/* Alle dynamisch hinzugekommen, verbundenen Welten (Joints) löschen */
		query ("DELETE FROM worlds WHERE map_id_offset>0");
	}
}


?>
