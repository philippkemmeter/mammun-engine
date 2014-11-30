<?

require_once(dirname(__FILE__).'/BuildingOnMap.inc');
require_once(dirname(__FILE__).'/Buildings.inc');

/**
 * Motherfungus-Klasse. Soll später im Prinzip eine Tribes-Klasse werden. Soll also
 * enthalten, welche Tribes sie beherbergt, wieviele davon etc.
 *
 * entsprechend würde ich auch eine Techcenter-Klasse entwerfen, welche weiß, wieviele Scientists an was arbeiten.
 *
 * Der Motherfungus ist von Natur aus ein Fugus und daher davon abgeleitet.
 */

class Motherfungus extends BuildingOnMap {
	private $max_inhabitants;
	private $inhabitants;
	private $children;
	private $free_space;

	private static $uid_map_id_to_id_mapping;
	protected static $INSTANCE;

	const RESOURCE1_CAPACITY_NORMAL = 20000;
	const RESOURCE1_LEVEL_CAPACITY_NORMAL = 10000;
	const RESOURCE2_CAPACITY_NORMAL = 20000;
	const RESOURCE2_LEVEL_CAPACITY_NORMAL = 10000;
	const RESOURCE3_CAPACITY_NORMAL = 20000;
	const RESOURCE3_LEVEL_CAPACITY_NORMAL = 10000;
	const RESOURCE1_CAPACITY_SP = 5000;
	const RESOURCE1_LEVEL_CAPACITY_SP = 5000;
	const RESOURCE2_CAPACITY_SP = 5000;
	const RESOURCE2_LEVEL_CAPACITY_SP = 5000;
	const RESOURCE3_CAPACITY_SP = 5000;
	const RESOURCE3_LEVEL_CAPACITY_SP = 5000;

	/**
	 * Diese Funktion als Konstuktor benutzen
	 *
	 * @param int $uid		Die UID
	 * @param uint $world_id
	 * @param int $map_id	Die ID der Karte, wo der MF des Users sich
	 * 						befindet. Wird false (default) übergeben, wird
	 * 						die Karte gewählt, wo der User aktuell drauf ist.
	 */
	public static function get_instance($uid, $world_id=false, $map_id=false) {
		if (!$world_id) {
			$world_id = User::get_instance_by_id($uid)->get_world_id();
			$map_id = User::get_instance_by_id($uid)->get_map_id();
		}
		$id = self::$uid_map_id_to_id_mapping[$uid][$world_id][$map_id];
		if (!$id) {
			$m = new Motherfungus($uid, $world_id, $map_id);
			self::$uid_map_id_to_id_mapping[$uid][$world_id][$map_id] =
				$m->get_id();
			$id = self::$uid_map_id_to_id_mapping[$uid][$world_id][$map_id];
		}
		if (!self::$INSTANCE || !self::$INSTANCE[$id])
			self::$INSTANCE[$id] = new Motherfungus($id);
		return self::$INSTANCE[$id];
	}

	/**
	 * Konstruktor. Initialisiert alle Variablen (s. Building für den Rest der Liste!)
	 * Motherfungus kann initialisiert werden, durch Angabe
	 * der User-ID und der aktuellen Map_id.
	 * sp, max_sp, att und def werden (durch Building) aus der Session heraus geladen.
	 */
	private function __construct($uid_or_id, $world_id=false, $map_id=false) {
		if (!$world_id) {
			$id = $uid_or_id;
			$this->init_from_id($id);
		}
		else {
			$uid = $uid_or_id;
			$this->uid = $uid;
			$data = mysql_fetch_assoc(query(
				"SELECT * FROM building WHERE uid='{$this->uid}' AND ".
					"type=".Buildings::MOTHER_FUNGUS." AND ".
					"world_id=$world_id AND map_id=$map_id")
			);
			foreach ($data as $key => $value)
				$this->$key = $value;
			unset($this->ulc_x);
			unset($this->ulc_y);
			$this->ulc = new Point2D($data['ulc_x'], $data['ulc_y']);
		}


		$this->max_inhabitants = $this->calc_max_inhabitants();

		$tribes = Tribes::get_instance($this->uid)->get_tribes();
		$inh = 0;
		$children = 0;
		foreach ($tribes as $tribe) {
			$inh += $tribe->get_count();
			$children += $tribe->get_morphing_count();
		}
		$this->inhabitants = round($inh);
		$this->children = round($children);
		$this->free_space = $this->max_inhabitants - $this->inhabitants - $this->children;
	}

	/**
	 * Berechnet die maximale Anzahl von Einheiten, welche den MF bewohnen können
	 */
	private function calc_max_inhabitants() {
		if ($this->get_specialization()==12)
			$factor=0.5;
		else if ($this->get_specialization()==11)
			$factor=2;
		else
			$factor=1;
		return round(pow($this->cur_level, 0.8) * 200) * 100*$factor;
	}

	/***********************************************************************************
	 * Getter ..Start..
	 **********************************************************************************/

	public function get_resource_capacity($which=0) {
 		if ($which < 0 || $which >= Resource::NUM_RESOURCES)
 			throw new ExceptionWithErrormail("Ressource-Error", $this->uid, 'All',
 				"Trying to access invalid ressources",
 				"\$which = $which in Motherfungus::get_resource_capacity");

		if ($this->get_specialization()==11)
			$factor=0.5;
		else if ($this->get_specialization()==12)
			$factor=2;
		else
			$factor=1;

		if ($_SESSION['map_type']==SP_MAP) {
			if ($which == 0) {
				return(($this->get_level()*self::RESOURCE1_LEVEL_CAPACITY_SP
					+ self::RESOURCE1_CAPACITY_SP)*$factor);
			}
			else if ($which == 1) {
				return(($this->get_level()*self::RESOURCE2_LEVEL_CAPACITY_SP
					+ self::RESOURCE2_CAPACITY_SP)*$factor);
			}
			else if ($which == 2) {
				return(($this->get_level()*self::RESOURCE3_LEVEL_CAPACITY_SP
					+ self::RESOURCE3_CAPACITY_SP)*$factor);
			}
		}
		else {
			if ($which == 0) {
				return(($this->get_level()*self::RESOURCE1_LEVEL_CAPACITY_NORMAL
					+ self::RESOURCE1_CAPACITY_NORMAL)*$factor);
			}
			else if ($which == 1) {
				return(($this->get_level()*self::RESOURCE2_LEVEL_CAPACITY_NORMAL
					+ self::RESOURCE2_CAPACITY_NORMAL)*$factor);
			}
			else if ($which == 2) {
				return(($this->get_level()*self::RESOURCE3_LEVEL_CAPACITY_NORMAL
					+ self::RESOURCE3_CAPACITY_NORMAL)*$factor);
			}
		}
	}

	public function get_inhabitants() {
		return ($this->inhabitants) ? $this->inhabitants : 0;
	}

	public function get_max_inhabitants() {
		return ($this->max_inhabitants) ? $this->max_inhabitants : 0;
	}

	public function get_children() {
		return ($this->children) ? $this->children : 0;
	}

	public function get_free_space() {
		return ($this->free_space) ? $this->free_space : 0;
	}

	/***********************************************************************************
	 * Getter ...End...
	 **********************************************************************************/

	/***********************************************************************************
	 * Setter ..Start..
	 **********************************************************************************/

	public function set_inhabitants($value) {
		if (is_anint($value))
		$this->inhabitants = $value;
	}

	public function set_max_inhabitants($value) {
		if (is_anint($value))
		$this->max_inhabitants = $value;
	}

	public function set_children($value) {
		if (is_anint($value))
		$this->children = $value;
	}

	public function set_free_space($value) {
		if (is_anint($value))
		$this->free_space = $value;
	}

	/***********************************************************************************
	 * Setter ...End...
	 **********************************************************************************/
}

?>