<?

require_once(dirname(__FILE__) . '/../../consts.inc');
require_once(dirname(__FILE__) . '/../map/Mapsettings.inc');
require_once(dirname(__FILE__)
	. '/../../shared/classes/buildings/AbstractBuilding.inc');

/**
 * Jedes Gebäude hat gewisse Eigenschaften, die jetzt durch ein Objekt vereint
 * werden sollen (z.B. Att/Def/SP etc.. aber auch die Größe usw).
 * Das Building-Objekt repräsentiert hierbei ein Gebäude i.a., das aber keiner
 * Instanz auf der Karte zu Grund liegt, sondern die Eigenschaften gewisser
 * Gebäude-Typen bei bestimmten Stufen i.a. darstellt.
 */

class Building extends AbstractBuilding {
	const ERR_BUILDING_DOES_NOT_EXIST = 1;

	private static $INSTANCE = NULL;

	public static function get_instance($type, $level) {
		if (!isset(self::$INSTANCE[$type][$level]))
			self::$INSTANCE[$type][$level] = new Building($type, $level);
		return self::$INSTANCE[$type][$level];
	}

	/**
	 * Konstruktor. Initialisiert alle Variablen.
	 * sp, max_sp, att und def werden aus der Session heraus geladen.
	 *
	 * @param int $type		Gebäudetyp
	 * @param int $level	Stufe des Gebäudes
	 */
	private function __construct($type=0, $level=0) {
		$this->init_from_type_and_level($type, $level);
	}

	public function init_from_type_and_level($type, $level) {
		$res = query(
			"SELECT att, def, hitpoints as sp, type, cost, level, grow, ".
				"req_sc_id, req_level, pic FROM buildings ".
			"WHERE type=$type AND level=$level"
		);
		if (mysql_num_rows($res) == 0) {
			throw new Exception("Building does not exist: $type, $level",
				self::ERR_BUILDING_DOES_NOT_EXIST);
		}
		else {
			$fungus_array = mysql_fetch_array($res);
			foreach ($fungus_array as $key => $value)
				$this->$key = $value;
		}

		$this->size = Buildings::get_b_area_size($this->type);
		$this->height = Buildings::get_b_height($this->type);
		$this->const_key = Buildings::get_b_const_key($this->type);
	}

	/***********************************************************************************
	 * Getter ..Start..
	 **********************************************************************************/

	public function get_grow() {
		return $this->grow / Mapsettings::get_instance()->get_timefactor();
	}

	/**
	 * Gibt die Kosten für das nächste Upgrade zurück
	 *
	 * @return int Kosten
	 * @throws Exception
	 */
	public function get_upgrade_cost() {
		$next_fungus = Building::get_instance($this->type, $this->level +1);
		return $next_fungus->get_cost();
	}

	public function is_custom_building() {
		return (($this->type >= Buildings::CUSTOM_BUILDINGS_BEGIN)
			&& ($this->type <= Buildings::CUSTOM_BUILDINGS_END));
	}

	public function to_php_std_class() {
		$result = new stdClass();
		$result->req_sc_id = $this->get_req_sc_id();
		$result->req_level= $this->get_req_level();
		$result->cost = $this->get_cost();
		try {
			$result->upgrade_cost = $this->get_upgrade_cost();
		}
		catch (Exception $e) {
			$result->upgrade_cost = 0;
		}
		$result->grow = $this->get_grow();
		$result->type = $this->get_type();
		$result->level = $this->get_level();
		$result->height = $this->get_height();
		$result->size = $this->get_size();
		$result->const_key = $this->get_const_key();
		$result->att = $this->get_att();
		$result->def = $this->get_def();
		$result->sp = $this->get_sp();
		$result->pic = $this->get_pic();
		$result->extra = $this->get_extra();
		$result->custom = $this->is_custom_building();
		return $result;
	}


	/***********************************************************************************
	 * Getter ...End...
	 **********************************************************************************/

	/***********************************************************************************
	 * Setter ..Start..
	 **********************************************************************************/

	public function set_level($value) {
		if (is_anint($value)) {
			$this->init_from_type_and_level($this->type, $value);
		}
	}

	public function set_type($value) {
		if (is_anint($value)) {
			$this->init_from_type_and_level($value, $this->level);
		}
	}

	/***********************************************************************************
	 * Setter ...End...
	 **********************************************************************************/
}

?>