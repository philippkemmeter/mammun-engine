<?php

require_once (dirname(__FILE__).'/MapTemplate.inc');
require_once(dirname(__FILE__).'/../Db.inc');
require_once (dirname(__FILE__).'/../../shared/classes/exceptions/IllegalArgumentException.inc');

/**
 * Diese Klasse repräsentiert die Beschreibung einer jeden Karte. Es können
 * alle möglichen allgemeinen Anfragen gestellt werden, wie z.B. "Wieviele
 * betretbare Sektoren hat die Karte" oder "wann ist sie gestartet?" etc.
 * Anfragen, die sich direkt auf einen Sektor beziehen, werden hiervon nur
 * teilweise beantwortet, dafür ist primär/immer die Sector-Klasse zuständig.
 *
 * @author Philipp Kemmeter
 */
class MapDescription {
	const ERR_MAP_DOES_NOT_EXIST = 1;

	const STATE_MAP_RUNNING = 0;
	const STATE_MAP_IDLE = 1;
	const STATE_MAP_FINISHED = 2;

	const MAP_FILL_TIMEOUT = Time::FOUR_WEEKS;
	const MAP_FILL_TIMEOUT_MENTOR = Time::FOUR_WEEKS;


	private /*MapTemplate*/ $template;
	private /*int*/ $world_id;
	private /*int*/ $map_id;
	private /*int*/ $type;
	private /*int*/ $state;
	private /*tstamp*/ $inserted;
	private /*tstamp*/ $started;
	private /*tstamp*/ $finished;
	private /*int*/ $winner_id;
	private /*int*/ $winner_homesector_id;
	private /*float*/ $avg_player_level;
	private /*int*/ $time_factor;
	private /*bool*/ $mentor;

	private /*bool*/ $db_need_update;

	private static $INSTANCES = array();


	/**
	 * Gibt die entsprechende Instanz zurück.
	 *
	 * @param uint $map_id
	 * @return MapDescription
	 */
	public static function get_instance($world_id, $map_id) {
		$map_id_old = $map_id;
		$map_id = (int)$map_id;
		if ($map_id < 0) {
			error_log(print_r(debug_backtrace(false),true));
			throw new IllegalArgumentException("map_id", 'N0', $map_id_old);
		}
		$world_id_old = $world_id;
		$world_id = (int)$world_id;
		if ($world_id < 0) {
			error_log(print_r(debug_backtrace(false),true));
			throw new IllegalArgumentException("world_id", 'N0', $world_id_old);
		}
		if (!isset(self::$INSTANCES[$world_id][$map_id]))
			self::$INSTANCES[$world_id][$map_id] = new MapDescription($world_id,
				$map_id);
		return self::$INSTANCES[$world_id][$map_id];
	}

	private function __construct($world_id, $map_id) {
		$res = query (
			"SELECT * FROM maps WHERE world_id=$world_id AND map_id=$map_id"
		);
		if (mysql_num_rows($res) == 0) {
			error_log(print_r(debug_backtrace(false),true));
			throw new Exception("Map '$world_id, $map_id' does not exist",
				self::ERR_MAP_DOES_NOT_EXIST);

		}
		$data = mysql_fetch_array($res);

		$this->map_id = $data['map_id'];
		$this->world_id = $data['world_id'];
		$this->type = $data['type'];
		$this->state = $data['state'];
		$this->inserted = $data['inserted'];
		$this->started = $data['started'];
		$this->finished = $data['finished'];
		$this->winner_id = $data['winner'];
		$this->winner_homesector_id = $data['winner_homesector'];
		$this->avg_player_level = $data['avg_player_level'];
		$this->time_factor = $data['timefactor'];
		$this->max_unit_limit = $data['max_unit_limit'];
		$this->mentor = $data['mentor'];
		$this->template = MapTemplate::get_instance($data['template_id']);

		$this->db_need_update = false;
	}

	public function __destruct() {
		if ($this->db_need_update) {
			query(
				"UPDATE maps SET type={$this->type},".
				"	state={$this->state}, inserted={$this->inserted},".
				"	started={$this->started}, finished={$this->finished},".
				"	winner={$this->winner_id}, ".
				"	winner_homesector={$this->winner_homesector_id},".
				"	avg_player_level={$this->avg_player_level},".
				"	timefactor={$this->time_factor}, ".
				"	template_id={$this->template->get_id()} ".
				"WHERE world_id={$this->world_id} AND map_id={$this->map_id}"
			);
		}
	}

	/**** GETTER / SETTER ****/
	public function get_map_id() {
		return $this->map_id;
	}
	public function get_world_id() {
		return $this->world_id;
	}
	public function get_type() {
		return $this->type;
	}
	public function get_state() {
		return $this->state;
	}
	/**
	 * Gibt zurück, wann die Karte in die Datenbank gesetzt wurde.
	 *
	 * @return unsigned int
	 */
	public function get_time_inserted() {
		return $this->inserted;
	}
	/**
	 * Gibt zurück, wann die Karte gestartet wurde, also den Zeitpunkt, wann
	 * alle Spieler auf der Karte aktiv werden durften.
	 *
	 * @return unsigned int
	 */
	public function get_time_started() {
		return $this->started;
	}
	/**
	 * Gibt zurück, wann die Karte beendet wurde (meistens, weil ein Sieger
	 * festgestellt wurde).
	 *
	 * @return unsigned int
	 */
	public function get_time_finished() {
		return $this->finished;
	}
	/**
	 * Gibt das maximale Einheitenlimit für diese Karte zurück
	 *
	 * @return unsigned int
	 */
	public function get_max_unit_limit() {
		return $this->max_unit_limit ? $this->max_unit_limit : 50;
	}
	/**
	 * Gibt den User zurück, der die Karte gewonnen hat. Hat niemand die Karte
	 * (bislang) gewonnen, wird NULL zurückgegeben.
	 *
	 * @return User
	 */
	public function get_winner() {
		if (!$this->winner_id)
			return null;
		else
			return User::get_instance_by_id($this->winner_id);
	}
	/**
	 * Gibt die ID des Homesectors (0-99) des Gewinners zurück.
	 *
	 * @return unsigned int
	 */
	public function get_winner_homesector_id() {
		return $this->winner_homesector_id%100;
	}
	/**
	 * Gibt das aktuelle Durchschnittslevel aller Spieler, die gerade auf der
	 * Karte spielen, zurück.
	 *
	 * @return float
	 */
	public function get_avg_player_level() {
		return $this->avg_player_level;
	}
	/**
	 * Gibt den Kartenspezifischen Zeitfaktor zurück, also um wieviel schneller
	 * diese Karte läuft, als normale Karten (=> 1 = normale Geschwindigkeit).
	 *
	 * @return unsigned float
	 */
	public function get_time_factor() {
		return $this->time_factor;
	}
	/**
	 * Gibt den Namen der Kartenvorlage (Template) zurück. Ist kein Name gesetzt,
	 * wird der leere String zurückgegeben.
	 *
	 * @return string
	 */
	public function get_template_name() {
		return $this->template->get_name();
	}
	/**
	 * Gibt die ID der Kartenvorlage (Template) zurück.
	 *
	 * @return uint
	 */
	public function get_template_id() {
		return $this->template->get_id();
	}
	/**
	 * Gibt zurück, wie viele Spieler maximal auf der Karte gleichzeitig spielen
	 * dürfen, d.h. Eine Karte ist voll, wenn diese Anzahl Spieler auf der Karte
	 * sind.
	 *
	 * @return unsigned int
	 */
	public function get_max_player_amount() {
		return $this->template->get_num_players();
	}
	/**
	 * Gibt zurück, ob die Karte frühzeitig geschlossen / gestartet wurde, also
	 * ob die Karte schon jetzt als voll gelten soll, auch wenn die maximale
	 * Spielerzahl nicht erreicht wurde.
	 *
	 * <b>Eine geschlossene Karte wird für immer geschlossen bleiben.</b>
	 *
	 * @return bool
	 */
	public function is_closed() {
		$timeout = ($this->mentor)
			? self::MAP_FILL_TIMEOUT_MENTOR
			: self::MAP_FILL_TIMEOUT;
		return ($this->get_time_started() > 0) &&
				(($this->get_time_started() + $timeout)	<= Time::get_t_now());
	}
	/**
	 * Gibt zurück, welcher Sektor der Heimatsektor des Hosts ist, oder false,
	 * falls er keinen hat.
	 *
	 * @return int/bool
	 */
	public function get_homesector_of_host() {
		return $this->template->get_homesector_of_host();
	}
	/**
	 * Gibt ein Array von den Sektor-IDs (0-99) zurück, die ein Heimatsektor
	 * sind.
	 *
	 * @return unsigned int[]
	 */
	public function get_homesectors() {
		return $this->template->get_homesectors();
	}
	/**
	 * Gibt ein Array von den Sektor-IDs (0-99) zurück, in denen ein Artefakt
	 * zu finden ist.
	 *
	 * @return unsigned int[]
	 */
	public function get_artifact_sectors() {
		return $this->template->get_artifact_sectors();
	}

	/**
	 * Gibt zurück, ob die Karte bereits gestartet ist, oder nicht.
	 *
	 * @return bool
	 */
	public function is_started() {
		return ($this->started > 0);
	}

	/**
	 * Gibt zurück, ob diese Karte eine Mentorenkarte ist.
	 *
	 * @return bool
	 */
	public function is_mentor_map() {
		return (bool)$this->mentor;
	}

///// SEKTOR-SPEZIFISCHE GETTER SIND NUR VON DER SEKTOR-KLASSE AUFZURUFEN ////
	/**
	 * !!Guter Stil: Statt dieser Funktion Sector::is_domination_point()
	 * aufrufen!
	 * Gibt zurück, ob ein Sektor ein Domination-Point ist.
	 *
	 * @param [0,1,...99] $sector_id
	 * @return bool
	 * @throws IllegalArgumentException
	 */
	public function get_sector_is_domination_point($sector_id) {
		if ($sector_id < 0 || $sector_id > 99)
			throw new IllegalArgumentException();
		return $this->template->get_sector_is_domination_point($sector_id);
	}
	/**
	 * !!Guter Stil: Statt dieser Funktion Sector::is_home_sector()
	 * aufrufen!
	 * Gibt zurück, ob ein Sektor ein Homesector ist.
	 *
	 * @param [0,1,...99] $sector_id
	 * @return bool
	 * @throws IllegalArgumentException
	 */
	public function get_sector_is_homesector($sector_id) {
		if ($sector_id < 0 || $sector_id > 99)
			throw new IllegalArgumentException();
		return $this->template->get_sector_is_homesector($sector_id);
	}
///// ENDE: SEKTOR-SPEZIFISCHE GETTER SIND NUR VON DER SEKTOR-KLASSE AUFZURUFEN ////


	/**** SETTER *****/
	public function set_state($state) {
		$this->state = $state;
		$this->db_need_update = true;
	}

	public function set_winner($uid) {
		$this->winner_id = $uid;
		$this->db_need_update = true;
	}

	public function set_winner_homesector_id($homesector) {
		$this->winner_homesector_id = $homesector;
		$this->db_need_update = true;
	}

	public function set_time_finished($time_finished) {
		$this->finished = $time_finished;
		$this->db_need_update = true;
	}


	/**
	 * Gibt alle IDs der Karten zurück, die der Angegebene User gewonnen hat.
	 * Bewirkt einen Datenbankzugriff.
	 *
	 * @param uint $uid
	 * @return unknown_type
	 */
	public static function get_all_map_ids_of_winner($uid) {
		if ($uid <= 0) {
			throw new IllegalArgumentException("uid", "N+", $uid);
		}
		$uid = (int)$uid;
		$res = query("SELECT world_id, map_id FROM maps WHERE winner=$uid");
		$result = array();
		while ($row = mysql_fetch_row($res)) {
			$result[] = array($row[0], $row[1]);
		}
		return $result;
	}

	/**
	 * !!! Löscht alle Kartendaten unwiederbringlich aus der DB!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		unset ($_SESSION['MapDescription']);
		query ("DELETE FROM maps");
		/* Eine Map muss immer da sein, weil die Engine irgendwo sonst hakt
		 * (historisch bedingt => Chris)
		 */
		query ("INSERT INTO maps SET world_id=0, map_id=0");
	}
}

?>
