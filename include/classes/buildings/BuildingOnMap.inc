<?php
require_once (dirname(__FILE__).'/../score/GameScore.inc');
require_once(dirname(__FILE__)
	. '/../../shared/classes/buildings/AbstractBuilding.inc');
require_once (dirname(__FILE__).'/../../shared/classes/datastructures/Point2D.inc');
require_once (dirname(__FILE__).'/../../shared/classes/datastructures/IndividualItem.inc');
require_once (dirname(__FILE__).'/../../shared/classes/datastructures/WorldCoordinates.inc');
require_once (dirname(__FILE__).'/../../shared/classes/helper/IConvertableJSONArray.inc');
require_once (dirname(__FILE__).'/../../shared/classes/helper/IConvertableJSArray.inc');
require_once (dirname(__FILE__).'/../../shared/classes/helper/IConvertablePHPArray.inc');
require_once (dirname(__FILE__).'/../../shared/classes/helper/Converter.inc');
require_once (dirname(__FILE__).'/../../shared/classes/exceptions/ExceptionWithErrormail.inc');
require_once (dirname(__FILE__).'/../map/MapCompute/MapCompute.inc');
require_once (dirname(__FILE__).'/../EventLog.inc');
require_once (dirname(__FILE__).'/../medals/Medals.inc');
require_once (dirname(__FILE__).'/../medals/UserMedals.inc');


/**
 * Repräsentiert ein Gebäude, das auf der Karte platziert wurde.
 */

class BuildingOnMap extends AbstractBuilding
	implements IConvertableJSONArray, IConvertableJSArray,IConvertablePHPArray
{
	const ERR_DEST_SUICIDE_DURING_SIEGE = 1;
	const ERR_BUILDING_DOES_NOT_EXIST = 2;
	const ERR_NO_DOMINATION = 3;
	const ERR_REQ_NOT_MET = 4;
	const ERR_NOT_ENOUGH_RESOURCES = 5;
	const ERR_DONT_DESTROY_MAIN_BUILDINGS = 6;
	const ERR_DONT_DESTROY_ON_UPGRADE = 7;
	const ERR_DONT_DESTROY_TREES = 8;


	protected $max_sp = 0;		// Wieviel SP es maximal haben kann
	protected $uid = 0;			// Wem das Gebäude gehört
	protected $sector_id = 0;	// In welchem Sektor es platziert wurde
	protected $world_id = 0;
	protected $map_id = 0;
	protected $ulc = NULL;		// Wo es sich im Sektor befinden (UpperLeftCorner)
	protected $color = 0;
	protected $painted_by_uid = 0;
	protected $xp_level = 0;

	protected $t_stamp = 0;		// Wann Gebäude(aus)bau gestartet
	protected $to_build = 0;	// Ob es gerade upgradet	TODO TO_BUILD IST REDUNDANT ZU (bool)$t_stamp
	protected $progress = 0;	// Wie weit es bereits ist	TODO PROGRESS SCHEINT OHNE NUTZEN! AUS TABELLE KICKEN
	protected $finish_time = 0;	// Wann ein Upgrade fertig ist

	protected $mapsettings = NULL;	// Die Einstellungen für die Karte (z.B. Zeitfaktor)

	private static $INSTANCE = NULL;

	/**
	 * Gibt die Instanz von BuildingOnMap bezüglich der übergebenen ID zurück.
	 * Diese Funktion statt dem Konstruktor benutzen!
	 *
	 * @param WorldCoordinates $sector_c
	 * @param uint $x
	 * @param uint $y
	 * @return BuildingOnMap
	 */
	public static function get_instance(WorldCoordinates $sector_c, $x, $y)
	{
		if ($x < 0)
			throw new IllegalArgumentException("x", "N0+", $x);
		if ($y < 0)
			throw new IllegalArgumentException("x", "N0+", $y);
		$world_id = $sector_c->get_world_id();
		$map_id = $sector_c->get_map_id();
		$sector_id = $sector_c->get_sector_id();

		if (!isset(self::$INSTANCE[$world_id][$map_id][$sector_id][$x][$y])) {
			self::$INSTANCE[$world_id][$map_id][$sector_id][$x][$y] =
				new BuildingOnMap($sector_c, $x, $y);
		}
		return self::$INSTANCE[$world_id][$map_id][$sector_id][$x][$y];
	}

	/**
	 * Gibt alle Instanzen des User identifiziert durch $uid zurück. Wenn
	 * $type_filter angegeben, werden nur die Gebäude des Users vom übergebenen
	 * Typ zurückgegeben.
	 *
	 * @param uint $uid
	 * @param uint $type_filter
	 * @param uint $world_id
	 * @param uint $map_id
	 * @return BuildingOnMap[]
	 */
	public static function get_all_of_user($uid, $type_filter=0, $world_id=0,
		$map_id=0)
	{
		$uid = (int)$uid;
		$type_filter = (int)$type_filter;
		$world_id=(int)$world_id;
		$map_id=(int)$map_id;
		$sql = "SELECT world_id,map_id,sector_id,ulc_x,ulc_y FROM building ".
			"WHERE uid=$uid";
		if ($type_filter > 0)
			$sql .= " AND type=$type_filter";
		if ($world_id > 0) {
			$sql .= " AND world_id=$world_id";
			$sql .= " AND map_id=$map_id";
		}
		$res = query ($sql);
		$result = array();
		while ($row = mysql_fetch_object($res)) {
			$result[] = self::get_instance(
				new WorldCoordinates($row->world_id, $row->map_id,
					$row->sector_id),
				$row->ulc_x, $row->ulc_y
			);
		}
		return $result;
	}

	private function __construct(WorldCoordinates $sector_c, $x, $y) {
		$this->init_from_id($sector_c, $x, $y);
	}

	protected function init_from_id(WorldCoordinates $sector_c, $x, $y) {
		$world_id = $sector_c->get_world_id();
		$map_id = $sector_c->get_map_id();
		$sector_id = $sector_c->get_sector_id();
		$res = query(
			"SELECT b.*, bs.cost, bs.hitpoints, bs.def, bs.att, bs.req_sc_id, ".
				"bs.req_level, bs.grow, bs.base_size AS size, ".
				"bs.height_factor AS height, ".
				"bs.pic, bs.const_key, l.level AS xp_level ".
			"FROM buildings AS bs, building AS b ".
			"LEFT JOIN level_up_presents AS l ON b.type=l.id AND l.type=10 ".
				"AND l.blueprint=0 ".
			"WHERE b.world_id=$world_id ".
				"AND b.map_id=$map_id ".
				"AND b.sector_id=$sector_id ".
				"AND b.ulc_x=$x ".
				"AND b.ulc_y=$y ".
				"AND bs.type=b.type ".
				"AND bs.level=b.cur_level"
		);
		if (mysql_num_rows($res) == 0) {
			error_log(print_r(debug_backtrace(false),true));
			throw new Exception("Building does not exist!",
				self::ERR_BUILDING_DOES_NOT_EXIST);
		}

		$fungus_array = mysql_fetch_assoc($res);
		foreach ($fungus_array as $key => $value)
			$this->$key = $value;
		unset($this->ulc_x);
		unset($this->ulc_y);
		$this->ulc = new Point2D($x, $y);

		// TODO: Hier den Cache von der Buildingsklasse updaten?
	}

	/**
	 * Gibt zurück, ob der User dieses Gebäude auch upgraden darf
	 *
	 * @param int $uid	Die ID des Users, der den Pilz upgraden will
	 * @return bool		ob er das darf
	 */
	public function upgradable_by_user($uid) {
		// 1. gehört das Gebäude dem User?
		if ($uid != $this->uid)
			return false;

		// 2. Ist gerade bereits ein upgrade im Gange ?
		if ($this->to_build)
			return false;

		// 3. Gibt es überhaupt ein nächstes Level ?
		try {
			$next_building = Building::get_instance($this->type, $this->level+1);
		}
		catch (Exception $e) {
			return false;
		}

		// 4. Darf der User dieses erforschen ?
		$req_sc_id = $next_building->get_req_sc_id();
		$req_level = $next_building->get_req_level();

		$b = Techtree::get_instance($uid)->get_science_cur_level($req_sc_id);
		return ($b >= $req_level);
	}

	/**
	 * Zerstört das Gebäude unwiederbringlich. Nicht direkt aufrufen, wird
	 * von Buildings-Objekt verwaltet! (Leider gibt es in PHP keine Friends)
	 *
	 * Wird $force angegeben, dann wird jeder Fehler usw. ignoriert und es
	 * wird versucht das Ding auf alle Fälle abzureißen - selbst, wenn es sich
	 * dabei um ein Haupthaus handelt!
	 *
	 * @throws Exception bei Fehlern
	 */
	public function _destroy($force=false) {
/*		if ($this->is_upgrading()) {
			throw new Exception("Upgrading on destroy.",
				self::ERR_DONT_DESTROY_ON_UPGRADE);
		}*/
		if (($this->type >= Buildings::NEUTRAL_BUILDINGS_BEGIN)
			&& ($this->type <= Buildings::NEUTRAL_BUILDINGS_END))
		{
			if (!$force) {
				throw new Exception("Trees must not be destroyed.",
					self::ERR_DONT_DESTROY_TREES);
			}
		}
		switch ($this->type) {
			case Buildings::TECHCENTER:
			case Buildings::MOTHER_FUNGUS:
				if (!$force) {
					throw new Exception("Main buildings must not be destroyed.",
						self::ERR_DONT_DESTROY_MAIN_BUILDINGS);
				}
				break;
		}
		query(
			"DELETE FROM building WHERE uid={$this->get_uid()} AND ".
				"world_id={$this->get_world_id()} AND ".
				"map_id={$this->get_map_id()} AND ".
				"sector_id={$this->get_sector_id()} AND ".
				"ulc_x={$this->get_ulc()->x} AND ulc_y={$this->get_ulc()->y}"
		);
		if (mysql_affected_rows() == 0) {
			throw new Exception("Building does not exist on destroy!",
				self::ERR_BUILDING_DOES_NOT_EXIST);
		}

		if ($this->is_custom_building()) {
			Server::get_instance(CONF_PAYMENT_SERVER_URL,
				CONF_MAMMUN_AUTH_SECRET)
					->put_items_into_chest(
						$this->get_uid(),
						array(
							IIndividualization::TYPE_BUILDING
								=> $this->get_type()
						)
					);
		}

	}

	/**
	 * Ändert die Farbe des Gebäudes. Nicht direkt aufrufen, wird
	 * von Buildings-Objekt verwaltet! (Leider gibt es in PHP keine Friends)
	 *
	 * Wird $uid angegeben, wird vermerkt, dass dieser Spieler das Gebäude
	 * bemalt hat.
	 *
	 * @param uint/String $color
	 * @param uint
	 */
	public function _change_color($color, $uid=0) {
		$color = (int)$color;
		$uid = (int)$uid;
		query(
			"UPDATE building SET color=$color, painted_by_uid=$uid ".
			"WHERE uid='{$this->get_uid()}' AND ".
				"world_id={$this->get_world_id()} AND ".
				"map_id={$this->get_map_id()} AND ".
				"sector_id={$this->get_sector_id()} AND ".
				"ulc_x={$this->get_ulc()->x} AND ulc_y={$this->get_ulc()->y}"
		);
		$this->color = $color;
	}

	/**
	 * Führt ein Upgrade des Gebäudes aus. Nicht direkt aufrufen, wird
	 * von Buildings-Objekt verwaltet! (Leider gibt es in PHP keine Friends)
	 *
	 * @param bool $buildtime	Hierüber kann gesteuert werden, wann das Upgrade
	 * 							gestartet wurde (wenn 0, dann wird JETZT genommen)
	 */
	public function _upgrade($buildtime=0) {
		if ($buildtime > Time::get_t_now())
			throw new ExceptionWithErrormail("", User::get_current_user()->get_id(), 'All',
				'$buildtime in future on upgrade',
				"\$buildtime = $buildtime, \$t_now = ".Time::get_t_now());
		if (!$buildtime)
			$buildtime = Time::get_t_now();

		try {
			$next_building = Building::get_instance($this->get_type(),
				$this->get_level()+1);
		}
		catch (Exception $e) {
			// TODO NÄCHSTES LEVEL EXISTIERT NICHT!
		}
		$finish_time = $buildtime + (100 * $next_building->get_grow())
			/ Mapsettings::get_instance()->get_timefactor();

		if ($this->is_custom_building()) {
			$kkr = Server::get_instance(CONF_PAYMENT_SERVER_URL,
				CONF_MAMMUN_AUTH_SECRET)->cmd_use_items(
					$this->uid,
					array(
						IndividualItem::TYPE_BUILDING => $this->type
					)
				);
		}
		else {
			$req_level = $next_building->get_req_level();
			$req_sc_id = $next_building->get_req_sc_id();


			if ($req_level > Techtree::get_instance($this->get_uid())
						->get_science_cur_level($req_sc_id))
				throw new Exception("$req_level-$req_sc_id" ,
					self::ERR_REQ_NOT_MET);
		}

		$r_obj = Resource::get_instance($this->uid);
		$resource1 = $r_obj->get_resource(Resource::RESOURCE1);
		$cost = $this->get_cost();
		if ($resource1 < $cost)
			throw new Exception(Resource::RESOURCE1,
				self::ERR_NOT_ENOUGH_RESOURCES);
		$r_obj->add_resource((-1)*$cost, Resource::RESOURCE1);

		try {
			$sql =
				"UPDATE building ".
				"SET to_build='1', t_stamp='$buildtime', progress='0' ";
			if ($this->get_level() > 0)
				$sql .=
					", finish_time='$finish_time'";
			$sql .=
				"WHERE uid='{$this->get_uid()}' AND ".
					"world_id={$this->get_world_id()} AND ".
					"map_id={$this->get_map_id()} AND ".
					"sector_id={$this->get_sector_id()} AND ".
					"ulc_x={$this->get_ulc()->x} AND ulc_y={$this->get_ulc()->y}";
			query($sql);
		}
		catch (Exception $e) {
			/**
			 * Wenn, warum auch immer, die Query schiefläuft, dann die Resourcen
			 * wieder gutschreiben!
			 */
			$r_obj->add_resource($cost, Resource::RESOURCE1);
			throw $e;
		}
		$this->t_stamp = $buildtime;
		$this->finish_time = $finish_time;

		/**
		 * (künftiges) Bau-Ereignis protokollieren.
		 * (aber nur, wenn kein Custom-Building, weil die haben keine
		 * Bauzeit)
		 */
		if (!$this->is_custom_building()) {
			$ev_log = EventLog::get_instance($this->get_uid(),
				$this->world_id, $this->map_id);
			$ev_m = EventMessages::get_instance($this->get_uid());
			if ($this->get_level() > 0) {
				$ev_log->add_entry(
					EventLog::TYPE_UPGRADE_COMPLETED,
					$this->get_sector_id(),
					$this->get_type(),
					0/*$finish_time*/
				);
				/**
				 * Punkte durch Eventnachricht gutschreiben.
				 */
				$ev_m->add_entry(
					EventLog::TYPE_UPGRADE_COMPLETED,
					$this->get_sector_id(),
					$this->get_type(),
					0,/*$finish_time*/
					GameScore::calc_score($cost, GameScore::TYPE_RESSOURCE_USE)
				);
			}
			else {
				$ev_log->add_entry(
					EventLog::TYPE_CONSTRUCTION_COMPLETED,
					$this->get_sector_id(),
					$this->get_type(),
					0/*$finish_time*/
				);

				/**
				 * Ggf. Medallie vergeben
				 */
				$sc_medals = Medals::get_medals(IMedals::TYPE_BUILDING_CONSTRUCTED);
				$sc_medals = $sc_medals[IMedals::TYPE_BUILDING_CONSTRUCTED];

				$c1 = $ev_log->get_total_event_amount(
					EventLog::TYPE_CONSTRUCTION_COMPLETED);
				$c2 = $ev_log->get_total_event_amount(
					EventLog::TYPE_CONSTRUCTION_COMPLETED, 0, $this->get_type());

				$medal_id = 0;
				if (isset($sc_medals[0][$c1]))
					$medal_id = $sc_medals[0][$c1];
				else if (isset($sc_medals[$this->get_type()][$c2]))
					$medal_id = $sc_medals[$this->get_type()][$c2];

				if ($medal_id) {
					UserMedals::get_instance($this->uid)->add_medal(
						$medal_id, 0/*$finish_time*/
					);
				}

				/**
				 * Punkte durch Eventnachricht gutschreiben und Medallie vermerken
				 */
				$ev_m->add_entry(
					EventLog::TYPE_CONSTRUCTION_COMPLETED,
					$this->get_sector_id(),
					$this->get_type(),
					0,/*$finish_time*/
					GameScore::calc_score($cost, GameScore::TYPE_RESSOURCE_USE),
					$medal_id
				);
			}
		}
	}

	/**
	 * Gibt zurück, ob und von welcher Partei (Alliierte, Feinde, man selbst) das
	 * Gebäude getarnt wurde.
	 *
	 * @return int
	 */
	private function get_cloaked_to_user($uid) {
		return 0;
	}

	private function is_cloaked() {
		return false;
	}

	/////// GETTER//////////

	public function get_t_stamp() {
		return $this->t_stamp;
	}

	/**
	 * Gibt die Structurepoints zurück, die das Gebäude dann hat, wenn es
	 * komplett repariert ist (also der höchst mögliche SP-Wert).
	 *
	 * @return uint
	 */
	public function get_max_sp() {
		return $this->max_sp;
	}

	/**
	 * Gibt den Angriffswert des Gebäudes zurück
	 *
	 * @return uint
	 */
	public function get_att() {
		return $this->att + MapCompute::get_instance(
			$this->world_id, $this->map_id
		)->get_att_boon_of_building($this->get_ulc(), $this->get_sector_id());
	}

	/**
	 * Gibt den Defensivwert des Gebäudes zurück
	 *
	 * @return uint
	 */
	public function get_def() {
		return $this->def + MapCompute::get_instance(
			$this->world_id, $this->map_id
		)->get_def_boon_of_building($this->get_ulc(), $this->get_sector_id());
	}

	/**
	 * Gibt die Aktuelle Anzahl Strukturpunkte zurück.
	 *
	 * @return uint
	 */
	public function get_sp() {
		return MapCompute::get_instance(
			$this->world_id, $this->map_id
		)->get_sp_of_building($this->get_ulc(), $this->get_sector_id());
	}

	/**
	 * Gibt die ID des Besitzers des Gebäudes zurück.
	 *
	 * @return uint
	 */
	public function get_uid() {
		return ($this->uid) ? $this->uid : 0;
	}

	/**
	 * Gibt die Position der oberen linken Ecke des Gebäudes an als Point2D
	 *
	 * @return Point2D	obere linke Ecke
	 */
	public function get_ulc() {
		return $this->ulc;
	}

	/**
	 * Gibt die ID des Sektors zurück, in dem das Gebäude steht
	 *
	 * @return [0,1..99]
	 */
	public function get_sector_id() {
		return ($this->sector_id) ? $this->sector_id : 0;
	}

	/**
	 * Gibt die ID der Karte zurück, auf dem das Gebäude steht
	 * @return uint
	 */
	public function get_map_id() {
		return $this->map_id;
	}

	/**
	 * Gibt die ID der Welt zurück, in dem das Gebäude steht.
	 * @return uint
	 */
	public function get_world_id() {
		return $this->world_id;
	}

	/**
	 * Gibt die Welt-Koordinaten des Sektors zurück, in dem das Gebäude steht.
	 * @return WorldCoordinates
	 */
	public function get_sector_world_coordinates() {
		return new WorldCoordinates(
			$this->get_world_id(),
			$this->get_map_id(),
			$this->get_sector_id()
		);
	}

	/**
	 * Gibt zurück, ob das Gebäude gerade ausgebaut/konstruiert wird.
	 *
	 * @return bool
	 */
	public function is_upgrading() {
		return (bool)(($this->to_build) ? $this->to_build : false);
	}

	/**
	 * Gibt den Aktuellen Ausbausfortschritt an
	 *
	 * @return ufloat
	 */
	public function get_progress() {
		return $this->progress
			+ ((Time::get_t_now() - $this->t_stamp) / $this->get_grow());
	}

	/**
	 * Gibt die Spezialisierung des Gebäudes als Code zurück (ugly!)
	 *
	 * @return uint
	 */
	public function get_specialization()
	{
		return 0;
	}


	public function get_spezi() {
		return 0;
	}

	public function get_spezi_finished() {
		return 0;
	}

	public function get_finish_time() {
		return $this->finish_time;
	}

	public function get_color() {
		return $this->color;
	}

	public function get_xp_level() {
		return ($this->xp_level) ? $this->xp_level : 0;
	}

	public function get_painted_by_uid() {
		return ($this->painted_by_uid)
			? $this->painted_by_uid
			: $this->get_uid();
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

	/////////// CONVERTER /////////////

	/**
	 * Implementierung von IConvertableJSONArray. Gibt Objekt als JSON-Array
	 * zurück. Dabei werden die Array-Elemente wie folgt belegt:
	 * [0]: NAME
	 * [1]: array(ULC->X, ULC->Y)
	 * [2]: PIC_PATH_AND_FILE_NAME
	 * [3]: array(SIZE, HEIGHT)
	 * [4]: ID
	 *
	 * @return string
	 */
	public function to_json_array() {
		return Converter::arrayToJSON($this->to_php_array());
	}

	/**
	 * Implementierung von IConvertableJSArray. Gibt Objekt als JS-Array
	 * zurück. Dabei werden die Array-Elemente wie folgt belegt:
	 * [0]: NAME
	 * [1]: array(ULC->X, ULC->Y)
	 * [2]: PIC_PATH_AND_FILE_NAME
	 * [3]: array(SIZE, HEIGHT)
	 * [4]: encrypted(ID)	// encrypted, da ID sensibel
	 *
	 * @return string
	 */
	public function to_js_array() {
		return Converter::arrayToJSArray($result);
	}
	/**
	 * Implementierung von IConvertableJSArray. Gibt Objekt als JS-Array
	 * zurück. Dabei werden die Array-Elemente wie folgt belegt:
	 * [0]: NAME
	 * [1]: array(ULC->X, ULC->Y)
	 * [2]: PIC_PATH_AND_FILE_NAME
	 * [3]: array(SIZE, HEIGHT)
	 * [4]: array(PROGRESS, GROW, T_STAMP)
	 * [5]: IS_NEUTRAL_BUILDING
	 *
	 * @return string
	 */
	public function to_php_array() {
		$result = array();
		$result[0] = $this->get_const_key();
		$ulc = $this->get_ulc();
		$result[1] = array($ulc->x, $ulc->y);
		$result[2] = $this->get_pic();
		$result[3] = array($this->get_size(), $this->get_height());
		$result[4] = array($this->get_progress(), $this->get_grow(), $this->get_t_stamp());
		$result[5] = (($this->get_type() >= IBuildings::NEUTRAL_BUILDINGS_BEGIN)
				&& ($this->get_type() <=  IBuildings::NEUTRAL_BUILDINGS_END));
		return $result;
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
		$result->ulc = $this->get_ulc();
		$result->uid = $this->get_uid();
		$result->is_upgrading = $this->is_upgrading();
		$result->finish_time = $this->get_finish_time();
		$result->progress = $this->get_progress();
		$result->t_stamp = $this->get_t_stamp();
		$result->max_sp = $this->get_max_sp();
		$result->world_id = $this->get_world_id();
		$result->map_id = $this->get_map_id();
		$result->sector_id = $this->get_sector_id();
		$result->const_key = $this->get_const_key();
		$result->xp_level = $this->get_xp_level();
		$result->color = $this->get_color();
		return $result;
	}

	public function is_visible_to_user($uid) {
		if ($this->get_cloaked_to_user($uid) == 1) {
			return false;	// durch Cloaking nicht sichtbar
		}
		else
			return true;
		// Da kein FOW mehr existiert, war's das :)
	}

	public function is_custom_building() {
		return (($this->type >= Buildings::CUSTOM_BUILDINGS_BEGIN)
			&& ($this->type <= Buildings::CUSTOM_BUILDINGS_END));
	}

	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCE = array();
		query ("DELETE FROM building");
	}
}

?>