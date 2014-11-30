<?php

require_once(dirname(__FILE__).'/../shared/classes/Tribe.inc');
require_once (dirname(__FILE__).'/../shared/classes/helper/IConvertablePHPAssocArray.inc');
require_once (dirname(__FILE__).'/../shared/classes/helper/IConvertablePHPStdClass.inc');
require_once (dirname(__FILE__).'/map/MapCompute/MapCompute.inc');
require_once (dirname(__FILE__).'/../shared/classes/exceptions/ExceptionWithErrorlog.inc');
require_once(dirname(__FILE__).'/medals/Medals.inc');
require_once(dirname(__FILE__).'/medals/UserMedals.inc');


/**
 * Repräsentiert einen Stamm in einem Sektor auf der Karte, also einen, der
 * nicht mehr im MF ist. Dieser kann z.B. bewegt werden usw.
 *
 * @author Philipp Kemmeter
 */

class TribeOnMap extends AbstractTribe
	implements IConvertablePHPAssocArray, IConvertablePHPStdClass
{
	const SSP_PARTY_AMOUNT = 3;	// wieviele beim SSP-Spiel mitmachen ;-)
	private $sector_id;
	private $world_id;
	private $map_id;
	private $t_appearance;
	private $t_movement_started;
	private $path;
	private $moved_by;
	private $sex;
	private $outerwear;
	private $trousers;
	private $shoes;
	private $jewelry;
	private $hairhat;
	private $weapon_style;
	private static $INSTANCES = array();

	/**
	 * Gibt das TribeOnMap-Objekt zurück, das durch die gegebene ID
	 * identifiziert wird.
	 *
	 * @param uint $tmap_id
	 * @param N mod 100 $sector_id
	 * @param uint $map_id	(wenn false, wir current_user->get_map_id() genommen)
	 * @param uint $world_id (wenn false, wir current_user->get_world_id() genommen)
	 * @return TribeOnMap
	 * @throws Exception
	 */
	public static function get_instance($tmap_id, $sector_id=0, $map_id=0,
		$world_id=0)
	{
		if (!isset(self::$INSTANCES[$tmap_id])) {
			self::$INSTANCES[$tmap_id]
				= new TribeOnMap($world_id, $map_id, $sector_id, $tmap_id);
		}
		return self::$INSTANCES[$tmap_id];
	}

	public static function get_all_of_user($world_id, $map_id, $uid) {
		$world_id = (int)$world_id;
		$map_id = (int)$map_id;
		$uid = (int)$uid;
		$res = query (
			"SELECT tmap.id FROM tribes_on_map AS tmap, tribes AS t ".
			"WHERE tmap.map_id=$map_id AND tmap.world_id=$world_id ".
				"AND tmap.tribe_id=t.id AND t.uid=$uid"
		);
		$result = array();
		while ($row = mysql_fetch_assoc($res))
			$result[] = self::get_instance($row['id']);
		return $result;
	}

	/**
	 * Erzeugt ein neues TribeOnMap-Objekt zurück, das durch die gegebene ID
	 * identifiziert wird.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param [0,1,...99] $sector_id
	 * @param uint $tid
	 * @return TribeOnMap
	 * @throws Exception
	 */
	protected function __construct($world_id, $map_id, $sector_id, $tid) {
		if (!$tid)
			debug_print_backtrace();
		$this->id = $tid;
		$this->sector_id = $sector_id;
		$this->map_id = $map_id;
		$this->world_id = $world_id;

		try {
			$this->init_from_db();
		}
		catch (ExceptionWithErrorlog $e) {
			throw new Exception($e->getMessage());
		}
	}

	private function save() {
		$path_str = '';	// Der Pfad muss als String umgewandelt werden für die DB
		foreach ($this->path as $sector) {
			if ($sector < 10)
				$path_str .= '0';
			$path_str .= $sector;
		}
		query (
			'UPDATE tribes_on_map SET '.
				'path="'.$path_str.'", t_appearance='.$this->t_appearance.','.
				't_movement_started='.$this->t_movement_started.', '.
				'moved_by='.$this->moved_by.', amount='.$this->amount.' '.
			'WHERE id='.$this->id
		);
	}

	protected function init_from_db(array $db_data=null) {
		if (!$db_data) {
			$res = query (
				'SELECT t.*, tmap.t_appearance, tmap.amount, '.
					'tmap.t_movement_started, tmap.world_id, tmap.map_id, tmap.path, '.
					'tmap.moved_by, tmap.name AS unit_name, '.
					'tmap.sex, tmap.outerwear, tmap.trousers, tmap.shoes, '.
					'tmap.jewelry, tmap.hairhat, tmap.weapon_style '.
				'FROM tribes_on_map AS tmap, tribes AS t '.
				'WHERE tmap.tribe_id=t.id AND tmap.id='.$this->id
			);
			if (mysql_num_rows($res) == 0) {
				throw new Exception("Tribe does not exist!");
			}
			$data = mysql_fetch_assoc($res);
		}
		else {
			$data = $db_data;
		}

		$this->tribe_id = $data['id'];
		$this->uid = $data['uid'];
		$this->name = $data['unit_name'] ? $data['unit_name'] : $data['name'];
		$this->race = $data['race'];
		$this->cost = $data['cost'];
		$this->state = $data['state'];
		$this->extra = $data['extra'];
		$this->hp = $data['hp'];
		$this->att = $data['att'];
		$this->def = $data['def'];
		$this->speed = $data['speed'];
		$this->att_boon = 0;
		$this->hp_boon = 0;
		$this->def_boon = 0;
		$this->speed_boon = 0;
		$this->amount = $data['amount'];
		$this->t_appearance = $data['t_appearance'];
		$this->t_movement_started = $data['t_movement_started'];
		$this->sex = $data['sex'];
		$this->outerwear = $data['outerwear'];
		$this->trousers = $data['trousers'];
		$this->shoes = $data['shoes'];
		$this->jewelry = $data['jewelry'];
		$this->hairhat = $data['hairhat'];
		$this->weapon_style = $data['weapon_style'];
		// TODO: Remove this here and let the GUI handle the pics
		$pic_core = substr($data['pic_url'], 0,strrpos($data['pic_url'],'.'));

		$this->pic_url = "$pic_core/{$pic_core}_{$this->sex}_".
			"{$this->outerwear}_{$this->trousers}_{$this->shoes}_".
			"{$this->jewelry}_{$this->hairhat}_{$this->weapon_style}.png";

		$this->path = str_split($data['path'], 2);
		for ($i = 0; $i < count($this->path); $i++)
			$this->path[$i] = (int)$this->path[$i];
		$this->moved_by = $data['moved_by'];

		if (!$this->map_id)
			$this->map_id = $data['map_id'];

		if (!$this->world_id)
			$this->world_id = $data['world_id'];

		$this->abilities = array();
		$res_abil = query(
			'SELECT ability_id FROM tribes_abilities '.
			'WHERE tribe_id='.$this->tribe_id
		);
		while ($abil_row = mysql_fetch_row($res_abil)) {
			$this->abilities[] = $abil_row[0];
		}
	}

	/**
	 * Setzt die Einheit in Bewegung. Der angegebene Pfad wird geprüft und,
	 * wenn möglich, genommen.
	 *
	 * @param [0,1,...99][] $path
	 * @return uint vorraussichtliche Ankunftszeit
	 */
	public function move(array $path) {
		/**
		 * 1. Pfad prüfen.
		 */
		if ($path[0] != $this->get_current_position())
			throw new IllegalArgumentException('path[0]; '.
				'$this->get_current_position()', 'EQUAL',
				"{$path[0]}; ".$this->get_current_position());

		$m = MapCompute::get_instance($this->world_id, $this->map_id,0);

		for ($i = 0; $i < count($path)-1; $i++) {
			/**
			 * 1.1: Existiert der Sektor?
			 */
			if (!Sector::get_instance(
				new WorldCoordinates($this->world_id, $this->map_id, $path[$i])
			)->exists())
				throw new IllegalArgumentException(
					"Sector {$path[$i]} does not exist."
				);
			/**
			 * 1.2: TODO: Sektoren im Pfad zusammenhängend?
			 */
			$diff = $path[$i] - $path[$i+1];
			//if (($diff != 1) && ($diff != 10) && ($diff != -10) && ($diff != -1))
		}


		/**
		 * 3. Bewegen :)
		 */

		// MUTEX: Während Einheiten bewegt werden, darf kein Compute stattfinden
		$sem_id = sem_get(MUTEX_MAP_COMPUTE_OFFSET+$this->id, 1, 0600, -1);
		sem_acquire($sem_id);
		try {
			$movement_price = count($path)-1;

			$amount = ceil($m->get_amount_of_tribe($this->id,
				$this->get_current_position()));
			if ($amount > 0)
				$this->amount = $amount;
			else
				$this->amount = 1;
			$time_needed = floor($movement_price * CIBal::MOVEMENT_TIME /
				$this->get_speed());
			$this->t_appearance = Time::get_t_now() + $time_needed;
			$this->t_movement_started = Time::get_t_now();
			$this->moved_by = $this->get_uid();
			$this->path = $path;
			$this->save();

			$m->register_movement($this);
			$m->mark_sector_as_dirty($path[0]);
		}
		catch (IllegalArgumentException $e) {
			sem_release($sem_id);
			throw $e;
		}
		catch (Exception $e) {
			sem_release($sem_id);
			throw $e;
		}
		// MUTEX
		sem_release($sem_id);
		$m = MapCompute::get_instance($this->world_id, $this->map_id,0);// force recompute
	}

	/**
	 * Stoppt die Einheit dort, wo sie sich gerade befindet.
	 *
	 * @return uint		Sektor, wo sie gestoppt wurde
	 */
	public function stop() {
		$m = MapCompute::get_instance($this->world_id, $this->map_id,0);
		// MUTEX: Während Einheiten gestoppt werden, darf kein Compute stattfinden
		$sem_id = sem_get(MUTEX_MAP_COMPUTE_OFFSET+$this->id, 1, 0600, -1);
		sem_acquire($sem_id);
		try {
			$sector = $this->get_current_position();

			$this->t_appearance = Time::get_t_now()-1;
			$this->t_movement_started = 0;

			$n = count($this->path);
			for ($i = 0; $i < $n; $i++) {
				if ($this->path[$i] == $sector) {
					array_splice($this->path, $i+1, $n);
					break;
				}
			}
			$this->save();
			$m->register_stop($this);
			$m->mark_sector_as_dirty($sector);
		}
		catch (IllegalArgumentException $e) {
			sem_release($sem_id);
			throw $e;
		}
		catch (Exception $e) {
			sem_release($sem_id);
			throw $e;
		}
		// MUTEX
		sem_release($sem_id);
		$m = MapCompute::get_instance($this->world_id, $this->map_id,0);// force recompute

		return $sector;
	}

	/**
	 * Beam alle sich in der Bewegengung befindlichen Einheiten des Users
	 * mit der ID <code>$uid</code> auf der Karte <code>$map_id</code> zu ihrem
	 * Zielort.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param uint $uid
	 */
	public static function beam_moving_units($world_id, $map_id, $uid) {
		if ($world_id <= 0) {
			throw new IllegalArgumentException("world_id", "N+", $world_id);
		}
		$world_id = (int)$world_id;
		if ($map_id <= 0) {
			throw new IllegalArgumentException("map_id", "N+", $map_id);
		}
		$map_id = (int)$map_id;
		$uid = (int)$uid;
		if ($uid <= 0) {
			throw new IllegalArgumentException("uid", "N+", $uid);
		}

		$hs = User::get_instance_by_id($uid)->get_homesector() % 100;
		query(
			"UPDATE tribes_on_map AS tmap, tribes AS t ".
				"SET tmap.t_appearance=".
				(Time::get_t_now()-1)." ".// Einheiten sind gerade angekommen
			"WHERE tmap.tribe_id=t.id AND t.uid=$uid ".
				"AND tmap.world_id=$world_id ".
				"AND tmap.map_id=$map_id ".
				"AND path!='$hs'"	// Trainierende Einheiten ignorieren
		);
		foreach (self::$INSTANCES as $key => $foo)
			unset(self::$INSTANCES[$key]);

		MapCompute::get_instance($world_id, $map_id, 0);
	}

	/**
	 * ACHTUNG: IRREVERSIBEL!
	 * Löscht den Tribe komplett und irreversibel aus der Datenbank.
	 */
	public function remove_from_db() {
		query(
			"DELETE FROM tribes_on_map WHERE id={$this->id}"
		);
		unset(self::$INSTANCES[$this->id]);

		/**
		 * Auskommentiert, dass alle Werte gelöscht werden. Denn, was wenn noch
		 * ein Pointer auf diesen Tribe zeigt? Ich glaube es ist besser, wenn
		 * der Tribe erstmal weiterbenutzt werden kann, auch wenn er nicht mehr
		 * existiert.
		 * Ansich wäre es besser, das gelöschte als solches intern zu markieren
		 * und bei jedem Zugriff eine Exception zu werfen. Naja...
		 */
//		$this->id = NULL;
//		$this->tribe_id = NULL;
//		$this->uid = NULL;
//		$this->name = NULL;
//		$this->race = NULL;
//		$this->pic_url = NULL;
//		$this->cost = NULL;
//		$this->state = NULL;
//		$this->abilities = array();
//		$this->extra = NULL;
//		$this->hp = NULL;
//		$this->att = NULL;
//		$this->def = NULL;
//		$this->speed = NULL;
//		$this->att_boon = NULL;
//		$this->hp_boon = NULL;
//		$this->def_boon = NULL;
//		$this->speed_boon = NULL;
//		$this->amount = NULL;
//		$this->t_appearance = NULL;
//		$this->t_movement_started = NULL;
//		$this->path = NULL;
//		$this->moved_by = NULL;
//		$this->map_id = NULL;
	}

	/**
	 * Verändert die Kleidung des Mammuns.
	 *
	 * Der Mammun wird dann die übergebenen Kleider tragen. Falls die
	 * angegebenen Kleider nicht alle in seiner Schatzkiste sind, werden sie
	 * automatisch nachgekauft aus der Schatzkiste von andern (diese müssen
	 * davor gelockt worden sein. => s. PAYMENT_SERVER->cmd_use_items)
	 *
	 * Gibt die Klunker zurück, die der Spieler nach der Aktion noch besitzt.
	 *
	 * @param UnitClothes $new_cloths
	 * @return UnitClothes
	 */
	public function change_clothes(UnitClothes $new_clothes) {
		$old_clothes = $this->get_clothes();
		/**
		 * Kleidung, die er schon anhat, rauslöschen, damit sie nicht doppelt
		 * verbraucht wird (bzw. sonst wird versucht sie von der Schatzkiste
		 * zu verbrauchen und dann das alte zurückgelegt, was keinen Verlust
		 * darstellt, aber genau dann scheitert, wenn der Spieler nicht das
		 * Item, das der Mammun schon trägt, nochmal in der Kiste hat. Also:
		 * wir müssen hier nullen :))
		 */
		$clothes_to_consume = new UnitClothes($new_clothes);
		foreach (UnitClothes::$cloth_type_strings as $type_str) {
			if ($old_clothes->$type_str == $clothes_to_consume->$type_str) {
				$clothes_to_consume->$type_str = 0;
				$old_clothes->$type_str = 0;
			}
		}

		try {
			$kkr = Server::get_instance(CONF_PAYMENT_SERVER_URL,
				CONF_MAMMUN_AUTH_SECRET)->cmd_use_items(
					$this->uid, $clothes_to_consume->to_php_array(), true);
		}
		catch (Exception $e) {
			throw new Exception($e->getMessage(),
				ITribes__TribeInMF::ERR_NOT_ENOUGH_KLUNKER);
		}
		foreach (UnitClothes::$cloth_type_strings as $type_str) {
			$this->$type_str = (int)$new_clothes->$type_str;
		}

		query (
			'UPDATE tribes_on_map SET '.
				"outerwear={$this->outerwear}, trousers={$this->trousers}, ".
				"shoes={$this->shoes}, jewelry={$this->jewelry}, ".
				"hairhat={$this->hairhat}, weapon_style={$this->weapon_style} ".
			'WHERE id='.$this->id
		);

		$clothes_array = $old_clothes->to_php_array();
		$amount_of_clothes = 0;
		foreach ($clothes_array as $type => $id) {
			if (isset($id)) {
				if ($id > 0)
					$amount_of_clothes++;
				else
					unset($clothes_array[$type]);
			}
		}
		if ($amount_of_clothes > 0) {
			/*error_log("Trying to put clothes of Tribe ".
							"{$tribe->get_id()} back to chest");*/
			Server::get_instance(CONF_PAYMENT_SERVER_URL,
				CONF_MAMMUN_AUTH_SECRET)
					->put_items_into_chest(
						$this->get_uid(),
						$clothes_array
					);
			//error_log("done.");
		}

		return $kkr;
	}


	/**
	 * Gibt zurück, wo sich die Einheit zum Zeitpunkt $current_time befindet.
	 *
	 * @param uint $current_time [default: jetzt]
	 * @return uint
	 */
	public function get_current_position($current_time=false)
	{
		if ($current_time === false)
			$current_time = Time::get_t_now();

		/**
		 * Wenn sich die Einheit nicht bewegt, ist es einfach: Der letzte
		 * Eintrag im Pfad
		 */
		if (!$this->is_moving()) {
			return $this->path[count($this->path)-1];
		}

		if ($current_time < $this->t_movement_started)
			throw new Exception("\$current_time too old - cannot give an answer.");

		$travel_duration = $this->t_appearance - $this->t_movement_started;
		$count_of_path = count($this->path);
		$duration_per_sector = ($count_of_path > 1)
			? $travel_duration / ($count_of_path - 1)
			: 0;
		$t = $this->t_movement_started + $duration_per_sector;
		$i = 0;
		while ($i < $count_of_path && $t < $current_time) {
			$i++;
			$t += $duration_per_sector;
		}
		return $this->path[$i];
	}

	/**
	 * Gibt zurück, ob sich die Einheit noch bewegt.
	 *
	 * @return bool
	 */
	public function is_moving() {
		// Wenn Ankunftszeitpunkt in der Zukunft, bewegt sich die Einheit noch
		return ($this->t_appearance >= Time::get_t_now());
	}

	/**
	 * Gibt zurück, in welche Richtung die Einheit sich bewegt, oder FALSE,
	 * wenn sie sich gar nicht bewegt.
	 *
	 * @param bool code01234
	 * @return uint
	 */
	public function get_heading($code01234=false) {
		if (!$this->is_moving()) {
			$p = $this->get_path();
			if (count($p) < 2)
				return ($code01234) ? 4 : Direction::TELEPORT;

			$sector_now = $p[count($p)-1];
			$sector_last = $p[count($p)-2];
			$diff = $sector_now - $sector_last;
		}
		else {
			$sector_now = $this->get_current_position();
			$x = array_search($sector_now, $this->path);
			$sector_next = $this->path[$x+1];
			$diff = $sector_next - $sector_now;
		}
		if ($diff == 1)
			return ($code01234) ? 0 : Direction::RIGHT;
		if ($diff == -1)
			return ($code01234) ? 1 : Direction::LEFT;
		if ($diff == 10)
			return ($code01234) ? 2 : Direction::DOWN;
		if ($diff == -10)
			return ($code01234) ? 3 : Direction::UP;
		else
			return ($code01234) ? 4 : Direction::TELEPORT;
	}

	/**
	 * Gibt zurück, wann die Einheit ankommt, oder FALSE, wenn sie sich nicht
	 * bewegt.
	 *
	 * @return uint
	 */
	public function get_arrival_time() {
		if (!$this->is_moving())
			return false;
		return $this->t_appearance;
	}

	/**
	 * Gibt zurück, wann die Einheit losgelaufen ist, oder FALSE, wenn sie
	 * sicht nicht bewegt.
	 *
	 * @return uint
	 */
	public function get_start_time() {
		if (!$this->is_moving())
			return false;
		return $this->t_movement_started;
	}

	/**
	 * Gibt zurück, wie viele Einheiten-HP die Einheit hat.
	 *
	 * @return float
	 * @deprecated Bitte get_amount verwenden
	 */
	public function get_count() {
		// Befindet sich die Einheit in der Bewegung, weiß sie die Anzahl selbst
		if ($this->is_moving())
			return $this->get_amount_last_moving();

		// Ansonsten könnte ein Kampf o.ä. im Gange sein, und man muss fragen
		return MapCompute::get_instance($this->world_id, $this->map_id)
			->get_amount_of_tribe($this->id, $this->get_current_position());
	}
	/**
	 * Gibt die Anzahl zurück. Dasselbe wie get_count().
	 */
	public function get_amount() {
		return $this->get_count();
	}

	/**
	 * Gibt zurück, wie viel HP die Einheit bei der letzten Bewegung hatte.
	 */
	public function get_amount_last_moving() {
		return $this->amount;
	}

	public function get_att() {
		return $this->att + $this->get_att_boon();
	}
	public function get_att_no_boon() {
		return $this->att;
	}
	public function get_att_boon() {
		return $this->att_boon;
	}

	public function get_def() {
		return $this->def + $this->get_def_boon();
	}
	public function get_def_no_boon() {
		return $this->def;
	}
	public function get_def_boon() {
		return $this->def_boon;
	}

	public function get_hp() {
		return $this->hp + $this->get_hp_boon();
	}
	public function get_hp_no_boon() {
		return $this->hp;
	}
	public function get_hp_boon() {
		return $this->hp_boon;
	}

	public function get_speed() {
		return $this->speed + $this->get_speed_boon();
	}
	public function get_speed_no_boon() {
		return $this->speed;
	}
	public function get_speed_boon() {
		return $this->speed_boon;
	}

	public function get_path() {
		return $this->path;
	}

	public function get_world_id() {
		return $this->world_id;
	}

	public function get_map_id() {
		return $this->map_id;
	}

	/**
	 * Gibt die Kleidung des Mammuns zurück.
	 *
	 * @return UnitClothes
	 */
	public function get_clothes() {
		return new UnitClothes($this->outerwear, $this->trousers, $this->shoes,
			$this->jewelry, $this->hairhat, $this->weapon_style);
	}

	/**
	 * Gibt zurück, ob der Tribe die angegebene Fähigkeit besitzt.
	 *
	 * @param uint $abil_id
	 * @return bool
	 */
	public function has_ability($abil_id) {
		return (in_array($abil_id, $this->abilities));
	}

	/**
	 * Gibt das Geschlecht der Einheit zurück.
	 *
	 * 0: Männlich
	 * 1: Weiblich
	 *
	 * @return {0; 1}
	 */
	public function get_sex() {
		return $this->sex;
	}

	/**
	 * Gibt zurück, ob die Einheit völlig unindividualisiert ist.
	 *
	 * @return bool
	 */
	public function is_naked() {
		return (!$this->outerwear && !$this->trousers && !$this->shoes &&
			!$this->jewelry && !$this->hairhat && !$this->weapon_style);
	}

	/**
	 * Gibt den Index des Stein-Schere-Papier-Systems zurück. Dabei hat ein
	 * Tribe mit SSP-Index i den SSP-Vorteil gegen
	 * i+1 mod self::SSP_PARTY_AMOUNT [da self::SSP_PARTY_AMOUNT Parteien].
	 *
	 * @return [0,1,..,self::SSP_PARTY_AMOUNT-1]
	 */
	private function get_ssp_index() {
		return 0;	// SSP-System abgeschaltet

		if ($this->get_race() == SC_ID_BOW
			|| $this->get_race() == SC_ID_ADV_BOW
			|| $this->get_race() == SC_ID_ADV_BOW_SHARPENED)
		{
			return 0;
		}
		if ($this->get_race() == SC_ID_JAVELIN
			|| $this->get_race() == SC_ID_ADV_JAVELIN
			|| $this->get_race() == SC_ID_ADV_JAVELIN_SHARPENED)
		{
			return 1;
		}
		if ($this->get_race() == SC_ID_CLUB
			|| $this->get_race() == SC_ID_AXE_BEATEN
			|| $this->get_race() == SC_ID_AXE_BEATEN_SHARPENED
			|| $this->get_race() == SC_ID_AXE_SHARPENED)
		{
			return 2;
		}

	}

	/**
	 * Gibt zurück, ob dieser Tribe einen SSP-Vorteil gegenüber dem übergebenen
	 * Gegner hat.
	 *
	 * @param TribeOnMap $enemy		Der Gegner
	 * @return bool
	 */
	public function has_ssp_advantage_against(TribeOnMap $enemy) {
		return
			( (($this->get_ssp_index() + 1) % self::SSP_PARTY_AMOUNT)
				==	$enemy->get_ssp_index() );
	}


	/**
	 * Gibt zurück, ob der Tribe sich tarnen kann
	 *
	 * @return bool
	 */
	public function can_camouflage() {
		return $this->has_ability(SC_ID_CAMOUFLAGE);
	}

	/**
	 * Gibt zurück, wie viele Einheiten pro Stunde ein ruhender Tribe
	 * regeneriert.
	 *
	 * @return {0,1,...,10}
	 */
	public function get_healing_speed() {
		return ($this->has_ability(SC_ID_HERBS)) ? 10 : 1;
	}

	/**
	 * Gibt zurück, ob der Tribe getarnt ist, oder nicht
	 *
	 * @return bool
	 */
	public function is_camouflaged() {
		return MapCompute::get_instance($this->world_id, $this->map_id)
			->get_tribe_is_camouflaged($this->id, $this->get_current_position());
	}

	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		query ("DELETE FROM tribes_on_map");
	}


	/////////// CONVERTER /////////////
	public function to_php_assoc_array() {
		$result = array();
		$result['uid'] = $this->get_uid();
		$result['amount'] = $this->get_amount();
		$result['race'] = $this->get_race();
		$result['speed'] = $this->get_speed();
		$tmp = $this->get_clothes()->to_php_array();
		$clothes = array();
		for ($i = 1; $i <= count($tmp); $i++) {
			$clothes[$i-1] = $tmp[$i];
		}
		$result['clothes'] = $clothes;
		$result['pic_url'] = $this->get_pic_url();
		$result['heading'] = $this->get_heading();
		$result['path'] = $this->get_path();
		$result['hp'] = $this->get_hp();
		$result['att'] = $this->get_att();
		$result['def'] = $this->get_def();
		$result['id'] = $this->get_id();
		$result['abilities'] = $this->get_abilities();
		$result['name'] = $this->get_name();
		$result['start_time'] = $this->get_start_time();
		$result['arrival_time'] = $this->get_arrival_time();
		$result['sex'] = $this->get_sex();
		$result['naked'] = $this->is_naked();
		return $result;
	}

	public function to_php_std_class() {
		$x = new stdClass();
		$x->abilities = $this->get_abilities();
		$x->amount = $this->get_amount();
		$x->att = $this->get_att();
		$x->def = $this->get_def();
		$x->hp = $this->get_hp();
		$x->id = $this->get_id();
		$x->name = $this->get_name();
		$x->heading = $this->get_heading();
		$x->path = $this->get_path();
		$tmp = $this->get_clothes()->to_php_array();
		$clothes = array();
		for ($i = 1; $i <= count($tmp); $i++) {
			$clothes[$i-1] = $tmp[$i];
		}
		$x->clothes = $clothes;
		$x->pic_url = $this->get_pic_url();
		$x->race = $this->get_race();
		$x->speed = $this->get_speed();
		$x->uid = $this->get_uid();
		$x->start_time = $this->get_start_time();
		$x->arrival_time = $this->get_arrival_time();
		for ($i = 0; isset($this->abilities[$i]); $i++) {
			$x->{'ability_'.$i} = $this->abilities[$i];
		}
		$x->sex = $this->get_sex();
		$x->naked = $this->is_naked();
		return $x;
	}
}


?>