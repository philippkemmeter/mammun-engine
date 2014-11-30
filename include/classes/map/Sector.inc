<?php


require_once (dirname(__FILE__).'/MapDescription.inc');
require_once (dirname(__FILE__).'/../Resource.inc');
require_once (dirname(__FILE__).'/../buildings/Buildings.inc');
require_once (dirname(__FILE__).'/../../shared/classes/Tribe.inc');
require_once (dirname(__FILE__).'/../../shared/classes/Direction.inc');
require_once (dirname(__FILE__).'/../../shared/classes/datastructures/'
	.'WorldCoordinates.inc');
require_once (dirname(__FILE__).'/MapCompute/MapCompute.inc');
/**
 * Klasse repräsentiert einen Sektor auf der Karte. Enthalten sind Attribute
 * wie Einheiten, Gebäude, ob sichtbar etc etc...
 * Alle Daten beziehen sich immer auf den User, der gerade eingeloggt ist!
 *
 */
// VORSICHT!!!: Session-Zugriff! NUR AKTUELLER USER!
// TODO: Sektorklasse in zwei Klassen aufteilen: 1. Sector und 2. SectorUsersView
class Sector {
	private /* WorldCoordinates */ $sector_c = null;

	private $host_activity = 0;
	private $bio = 0;
	private $artifact = 0;
	private $last_fight_duration = 0;
	private $last_stats_start = 0;


	/**
	 * Singleton: 1 Instanz pro Sektor-ID
	 */
	static private $INSTANCE = array();

	/**
	 * Diese statische Funktion als Konstruktor benutzen
	 *
	 * @param uint $map_id
	 * @param WorldCoordinates $sector_c
	 * @return Sector	Das gewünschte Sector-Objekt :)
	 */

 	public static function get_instance(WorldCoordinates $sector_c) {
 		$world_id = $sector_c->get_world_id();
 		$map_id = $sector_c->get_map_id();
 		$sector_id = $sector_c->get_sector_id();
 		if (!isset(self::$INSTANCE[$world_id][$map_id][$sector_id])) {
 			self::$INSTANCE[$world_id][$map_id][$sector_id]
 				= new Sector($sector_c);
 		}
 		return self::$INSTANCE[$world_id][$map_id][$sector_id];
 	}

 	/**
 	 * Diese Funktion ist dazu da, dass eine Instanz mit Daten gefüttert werden
 	 * kann.
 	 * Das ist nützlich, wenn man eine Abfrage à la "gib mir alle Sektoren, die
 	 * das und das erfüllen" an die DB stellt, und dann nicht nochmal für jede
 	 * Zeile alles extra auslesen will. Spart also u.U. 100Queries.
 	 *
 	 * @param WorldCoordinates $sector_c
 	 * @param stdClass $data
 	 * @return Sector
  	 * @throws Exception
	 * @throws ExceptionWithErrorlog
 	 */
 	private static function update_instance(WorldCoordinates $sector_c,
 		stdClass $data)
 	{
 		$world_id = $sector_c->get_world_id();
 		$map_id = $sector_c->get_map_id();
 		$sector_id = $sector_c->get_sector_id();
 		if (!isset(self::$INSTANCE[$world_id][$map_id][$sector_id])) {
 			// Leere Instanz erstellen, wenn noch keine da, um DB-Zugriff zu
 			// sparen (kein init aus DB)
 			self::$INSTANCE[$world_id][$map_id][$sector_id] = new Sector();
 		}
 		// jetzt die leere Instanz befüllen mit allen Daten
 		self::$INSTANCE[$world_id][$map_id][$sector_id]->init_from_db($data);
 		self::$INSTANCE[$world_id][$map_id][$sector_id]
 			->set_sector_c($sector_c);
 		return self::$INSTANCE[$world_id][$map_id][$sector_id];
 	}

	/**
	 * Konstruktor. Übergebene ID identifiert den Sektor.
	 *
	 * @param WorldCoordinates $sector_c
	 * @return Sector
	 * @throws Exception
	 * @throws ExceptionWithErrorlog
	 */
	private function __construct(WorldCoordinates $sector_c=null) {
		if ($sector_c) {
			$this->sector_c = $sector_c;
			$world_id = $sector_c->get_world_id();
			$map_id = $sector_c->get_map_id();
			$sector_id = $sector_c->get_sector_id();
			$res = query(
				"SELECT * FROM sectors ".
				"WHERE world_id=$world_id AND map_id=$map_id ".
					"AND sector_id=$sector_id"
			);
			if (mysql_num_rows($res) > 0) {
				$data = mysql_fetch_object($res);
				$this->init_from_db($data);
			}
		}
	}

	/**
	 * Initialisiert die Instanz mit den übergebenen Daten, die besser aus der
	 * Db kommen sollten... ;-)
	 *
	 * @param stdClass $data
	 */
	private function init_from_db(stdClass $data) {
		$this->host_activity = $data->host_activity;
		$this->artifact = $data->artifact;
		$this->bio = $data->bio;
		$this->last_stats_start = $data->last_stats_start;
		$this->last_fight_duration = $data->last_fight_length;
	}

	/**
	 * Setzt die komplette Sektor-ID.
	 * Wichtig: Es ist natürlich überhaupt keine gute Idee, die nachträglich
	 * nochmal zu ändern. Dann gibt's auch ne Exception.
	 *
	 * @param WorldCoordinates $sector_c
	 * @throws Exception
	 */
	private function set_sector_c(WorldCoordinates $sector_c) {
		if ($this->sector_c)
			throw new Exception("You cannot reset sector_c.");
		$this->sector_c = $sector_c;
	}

	/**
	 * Gibt die Sektoren auf der gewählten Karte zurück, die ein Artefakt
	 * verbergen.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @return Sector[]
	 * @throws Exception
	 * @throws ExceptionWithErrorlog
	 */
	public static function get_sectors_with_artifact(
		$world_id, $map_id)
	{
		$res = query(
			'SELECT * FROM sectors '.
			"WHERE artifact!=0 AND world_id=$world_id AND map_id=$map_id"
		);

		$result = array();
		if (mysql_num_rows($res) == 0)
			return $result;

		while ($data = mysql_fetch_object($res)) {
			$result[] = self::update_instance(
				new WorldCoordinates($world_id, $map_id, $data->sector_id),
				$data
			);
		}

		return $result;
	}

	/**
	 * Gibt das Artefakt zurück, dass in dem Sektor verborgen liegt.
	 *
	 * @return uint
	 */
	public function get_artifact() {
		return $this->artifact;
	}

	public function get_max_resources($which=false) {
		if ($which !== false)
			return ceil($this->get_bio()*Resource::get_bio_to_resource_rate($which));
		else
			return array(
				ceil($this->get_bio()*Resource::get_bio_to_resource_rate(0)),
				ceil($this->get_bio()*Resource::get_bio_to_resource_rate(1))
			);
	}

	/**
	 * Gibt zurück, wie viel Resource der angegebene User in diesem Sektor
	 * abbaut. Ist ein Wrapper der Resource::get_resource_gain-Funktion.
	 *
	 * @param uint $uid
	 * @param uint $which
	 * @return ufloat
	 */
	public function get_resource_gain($uid, $which=false) {
		$uid_old = $uid;
		$uid = (int)$uid;
		if ($uid <= 0)
			throw new IllegalArgumentException("\$uid has to be a positive ".
				"integer value.", $uid_old);

		if ($which !== false)
			return Resource::get_instance($uid)->get_resource_gain($which,
				$this->sector_id);
		else
			return array (
				Resource::get_instance($uid)->get_resource_gain(0,
					$this->sector_id),
				Resource::get_instance($uid)->get_resource_gain(1,
					$this->sector_id),
				Resource::get_instance($uid)->get_resource_gain(2,
					$this->sector_id)
			);
	}

	/**
	 * Gibt die Höhe der Wirtsaktivität in ganzaligen Prozent zurück.
	 *
	 * @return [0,1,...100]
	 */
	public function get_host_activity() {
		return $this->host_activity;
	}

	/**
	 * Gibt zurück, ob dieser Sektor ein Domination-Point ist.
	 *
	 * @return bool
	 */
	public function is_domination_point() {
		return MapDescription::get_instance($this->sector_c->get_world_id(),
			$this->sector_c->get_map_id())
		->get_sector_is_domination_point($this->sector_c->get_sector_id());
	}

	/**
	 * Gibt zurück, ob dieser Sektor ein Homesector ist.
	 */
	public function is_homesector() {
		return MapDescription::get_instance($this->sector_c->get_world_id(),
			$this->sector_c->get_map_id())
		->get_sector_is_homesector($this->sector_c->get_sector_id());
	}

	/**
	 * Gibt die den User zurück, der diesen Homesector besitzt, oder false,
	 * wenn der Sektor kein Homesector ist.
	 *
	 * @return uint/FALSE
	 */
	public function get_homesector_uid() {
		$world_id = $this->sector_c->get_world_id();
		$map_id = $this->sector_c->get_map_id();
		if (!$this->is_homesector())
		{
			return false;
		}
		if (MapDescription::get_instance($world_id, $map_id)->
			get_homesector_of_host() == $this->sector_c->get_sector_id())
		{
			return User::get_host_user();
		}
		$users = MapPlayers::get_instance($world_id, $map_id)
			->get_users_on_map();
		foreach ($users as $user) {
			if ($user->get_homesector() == ($world_id*10000
				+ $map_id*100 + $this->sector_c->get_sector_id()))
					return $user;
		}
		return 0;	// War ein Homesektor, mitlerweile ist Besitzer aber besiegt
	}

	/**
	 * Gibt zurück, wie viel Bio im Sektor vorhanden ist.
	 *
	 * @return uint
	 */
	public function get_bio() {
		return $this->bio;
	}

	public function get_dominator_id() {
		return MapCompute::get_instance(
			$this->sector_c->get_world_id(),
			$this->sector_c->get_map_id()
		)->get_dominator($this->sector_c->get_sector_id());
	}

	/**
	 * Gibt die UIDs der Dominatoren der Nachbarsektoren zurück. Dabei ist in
	 * [0] der rechte Nachbar, [1] der linke, [2] der unter und [3] der obere
	 * gespeichert.
	 *
	 * @return uint[4]
	 */
	public function get_neighbor_dominator_ids() {
		$mc = MapCompute::get_instance(
			$this->sector_c->get_world_id(),
			$this->sector_c->get_map_id()
		);
		$result = array();
		$sector_id = $this->sector_c->get_sector_id();
		$s_mod = $sector_id % 10;
		$s_div = floor($sector_id / 10);
		$result[0] = ($s_mod < 9) ? $mc->get_dominator($sector_id + 1) : 0;
		$result[1] = ($s_mod > 0) ? $mc->get_dominator($sector_id - 1) : 0;
		$result[2] = ($s_div < 9) ? $mc->get_dominator($sector_id + 10): 0;
		$result[3] = ($s_div > 0) ? $mc->get_dominator($sector_id - 10): 0;
		return $result;
	}

	public function get_dominator_nick() {
		if ($this->get_dominator_id()) {
			$this->dominator_nick =
				User::get_instance_by_id($this->get_dominator_id())->get_nick();
		}
		else
			$this->dominator_nick = '';

		return $this->dominator_nick;
	}

	public function get_visible_buildings($owned_by_user_id = false,
		$visible_to_user_id = false)
	{
		if (!$visible_to_user_id) {
			if (!$owned_by_user_id)
				$visible_to_user_id = User::get_current_user()->get_id();
			else
				$visible_to_user_id = $owned_by_user_id;
		}
		$sector_buildings = Buildings::get_buildings_in_sector($this->sector_c);
		$vis_buildings = array();
		foreach ($sector_buildings as $building) {
			if ($building->is_visible_to_user($visible_to_user_id)) {
				if (($owned_by_user_id === false) || ($owned_by_user_id
						 == $building->get_uid()))
					$vis_buildings[] = $building;
			}
		}
		return $vis_buildings;
	}

	/**
	 * Ob der gewählte User durch den Sektor hindurch ziehen darf
	 *
	 * @param int $uid	die ID des Users
	 * @return bool		ob er durchziehn darf
	 */
	public function may_move_through($uid) {
		//TODO
		return true;
	}


	/**
	 * Gibt zurück, ob im Sektor gekämpft wird
	 *
	 * @return bool
	 */
	public function get_fight() {
		return MapCompute::get_instance(
			$this->sector_c->get_world_id(),
			$this->sector_c->get_map_id()
		)->get_fight_in_sector($this->sector_c->get_sector_id());
	}

	/**
	 * Gibt zurück, ob der Sektor als solcher existiert, also, ob er generell
	 * betreten werden kann.
	 */
	public function exists() {
		return ($this->get_bio() > 0);
	}

	/**
	 * @return WorldCoordinates
	 */
	public function get_id() {
		return $this->sector_c;
	}

	/**
	 * Gibt die ID des Sektors zurück
	 *
	 * @return [0,1,..99]
	 */
	public function get_sector_id() {
		return $this->sector_c->get_sector_id();
	}

	/**
	 * Gibt die ID der Karte des Sektors zurück
	 *
	 * @return uint
	 */
	public function get_map_id() {
		return $this->sector_c->map_id;
	}

	/**
	 * Gibt die ID der Welt des Sektors zurück
	 */
	public function get_world_id() {
		return $this->sector_c->world_id;
	}

	/**
	 * !!!ACHTUNG: Setzt alle Sektordaten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCE = array();
		query("DELETE FROM sectors");
	}

}

?>