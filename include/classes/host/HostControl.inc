<?php

require_once(dirname(__FILE__).'/HostTribe.inc');
/**
 * Zuständig für den neutralen Spieler "Host". Hierüber können neue Einheiten
 * kreiert, an strategischen Positionen besonderer Widerstand generiert, und
 * Bewegungen ausgeführt werden. Das hier ist also sowas wie die Schaltzentrale
 * des Hosts.
 * Pro Karte gibt es eine Instanz dieser Host-Kontrolle.
 */

class HostControl {
	private $world_id;
	private $map_id;
	private $tribe_ids = array();	// Alle Tribes, die der Host auf der Karte besitzt

	const USER_HOST_ID = 1;
	/**
	 * ACHTUNG: Die namen MÜSSEN unterschiedlich sein!
	 */
	const HOST_TRIBE_NAME1 = 'Buba';
	const HOST_TRIBE_NAME2 = 'Tibu';
	const HOST_TRIBE_NAME3 = 'Muna';
	const HOST_TRIBE_NAME4 = 'Funop';
	const HOST_TRIBE_NAME5 = 'Nagot';
	const HOST_TRIBE_NAME6 = 'Naja';
	const ARTIFACT_GUARD_DROP_STRENGTH = 4;
	const DOMPOINT_GUARD_DROP_STRENGTH = 4;

	private static $INSTANCES = array();

	/**
	 * @param uint $world_id
	 * @param uint $map_id
	 * @return HostControl
	 */
	public static function get_instance($world_id, $map_id) {
		if (!isset(self::$INSTANCES[$world_id][$map_id]))
			self::$INSTANCES[$world_id][$map_id]
				= new HostControl($world_id, $map_id);
		return self::$INSTANCES[$world_id][$map_id];
	}

	private function __construct($world_id, $map_id) {
		$this->world_id = $world_id;
		$this->map_id = $map_id;
		$this->init();
	}

	public function init() {
		$this->tribe_ids = HostTribe::get_tribe_ids_of_host($this->get_id());
		if (empty($this->tribe_ids))
			$this->tribe_ids = $this->create_tribes();
	}

	private function create_tribes() {
		$host_id = $this->get_id();
//		if ($average_player_level > 200) //alle auf ultimate level
//		{
//			$average_player_level += 30;
//			$spc_abils = array (SC_ID_TIMEBURST, SC_ID_CTRLABLE_STICKY_SLIME);
//			$off_abils = array (SC_ID_ULT_STRENGTH);
//			$def_abils = array (SC_ID_DEADLY_SLIME, SC_ID_ORB_OF_PROTECTION);
//			$gen_abils = array (SC_ID_INVISIBILITY, SC_ID_MIGHTY_BLOCK, SC_ID_INSANE_RAGE);
//			$num_tribes = 6;
//		}
//		elseif ($average_player_level > 150) //knapp auf ultimate level
//		{
//			$average_player_level += 20;
//			$spc_abils = array (SC_ID_TIMEBURST, SC_ID_CTRLABLE_STICKY_SLIME);
//			$off_abils = array (SC_ID_STR_OF_THE_SUN, SC_ID_ULT_STRENGTH, SC_ID_AGRESSIVE_SLIME);
//			$def_abils = array (SC_ID_DANGEROUS_SLIME, SC_ID_DEADLY_SLIME, SC_ID_ORB_OF_PROTECTION);
//			$gen_abils = array (SC_ID_INVISIBILITY, SC_ID_MIGHTY_BLOCK, SC_ID_DANGEROUS_SLIME);
//			$num_tribes = 5;
//		}
//		elseif ($average_player_level > 100) //alle auf advanced level
//		{
//			$average_player_level += 15;
//			$spc_abils = array (SC_ID_TIMEBURST, SC_ID_RAMPANT_STICKY_SLIME);
//			$off_abils = array (SC_ID_STR_OF_THE_SUN, SC_ID_AGRESSIVE_SLIME);
//			$def_abils = array (SC_ID_DANGEROUS_SLIME, SC_ID_ORB_OF_PROTECTION);
//			$gen_abils = array (SC_ID_INVISIBILITY);
//			$num_tribes = 4;
//		}
//		elseif ($average_player_level > 80) //knapp auf advanced level
//		{
//			$average_player_level += 10;
//			$spc_abils = array (SC_ID_TIMEWARP, SC_ID_FREEZE, SC_ID_TIMEBURST);
//			$off_abils = array (SC_ID_STR_OF_THE_FIRE, SC_ID_DEVOUR);
//			$def_abils = array (SC_ID_DANGEROUS_SLIME, SC_ID_SHIELD);
//			$gen_abils = array (SC_ID_HIDE, SC_ID_LIFE_BOOST, SC_ID_LIFE_INCREASE);
//			$num_tribes = 3;
//		}
//		elseif ($average_player_level > 60) //hohes low-level
//		{
//			$average_player_level += 5;
//			$spc_abils = array (SC_ID_TIMEWARP, SC_ID_FREEZE, SC_ID_TIMEBURST);
//			$off_abils = array (SC_ID_STR_OF_THE_FIRE, SC_ID_STR_OF_THE_SUN);
//			$def_abils = array (SC_ID_DEFENSIVE_SLIME, SC_ID_DANGEROUS_SLIME, SC_ID_SHIELD);
//			$gen_abils = array (SC_ID_MIGHTY_BLOCK, SC_ID_RAGE, SC_ID_HIDE);
//			$num_tribes = 3;
//		}
//		elseif ($average_player_level > 30) //low-level
//		{
//			$spc_abils = array (SC_ID_TIMEWARP, SC_ID_FREEZE);
//			$off_abils = array (SC_ID_MIGHTY_BLOCK, SC_ID_RAGE, SC_ID_STR_OF_THE_FIRE);
//			$def_abils = array (SC_ID_DEFENSIVE_SLIME);
//			$num_tribes = 3;
//		}
//		elseif ($average_player_level > 20) //niedriges low level
//		{
//			$average_player_level -= 5;
//			$off_abils = array (SC_ID_MIGHTY_BLOCK, SC_ID_RAGE, SC_ID_STR_OF_THE_FIRE);
//			$def_abils = array (SC_ID_HULL, SC_ID_DEFENSIVE_SLIME);
//			$num_tribes = 2;
//		}
//		else //anfänger
//		{
			if ($average_player_level < 5)
				$average_player_level = 5;
			$off_abils = array ();
			$def_abils = array ();
			$num_tribes = 2;
//		}
		$lev = round(pow($average_player_level, 0.4))*0.8;
		$tribe_ids = array();
		$tribe_ids[] = HostTribe::add_tribe($host_id, self::HOST_TRIBE_NAME1,
			HostTribe::TRIBE_TYPE_AGRESSIVE, $lev, array(), $off_abils);
		if ($num_tribes > 1) {
			$tribe_ids[] = HostTribe::add_tribe($host_id, self::HOST_TRIBE_NAME2,
				HostTribe::TRIBE_TYPE_DEFENSIVE, $lev, array(), $def_abils);
		}
		if ($num_tribes > 2) {
			$tribe_ids[] = HostTribe::add_tribe($host_id, self::HOST_TRIBE_NAME3,
				HostTribe::TRIBE_TYPE_EQUAL, $lev*0.7, array(), $spc_abils);
		}
		if ($num_tribes > 3) {
			$tribe_ids[] = HostTribe::add_tribe($host_id, self::HOST_TRIBE_NAME4,
				HostTribe::TRIBE_TYPE_AGRESSIVE, $lev*1.5, $gen_abils, $off_abils);
		}
		if ($num_tribes > 4) {
			$tribe_ids[] = HostTribe::add_tribe($host_id, self::HOST_TRIBE_NAME5,
				HostTribe::TRIBE_TYPE_DEFENSIVE, $lev*1.5, $gen_abils, $def_abils);
		}
		if ($num_tribes > 5) {
			$tribe_ids[] = HostTribe::add_tribe($host_id, self::HOST_TRIBE_NAME6,
				HostTribe::TRIBE_TYPE_EQUAL, $lev*1.1, $gen_abils, $spc_abils);
		}
		return $tribe_ids;
	}

	public function drop_initial_guard_into_sectors($avg_player_level = 0) {
		$sectors = Sector::get_sectors_with_artifact(
			$this->world_id, $this->map_id
		);

		for ($i = 0, $n = count($sectors); $i < $n; $i++) {
			$this->drop_host_units_by_strength(
				$sectors[$i]->get_sector_id(),
				self::ARTIFACT_GUARD_DROP_STRENGTH
			);
		}
	}

	public function drop_host_units_by_amount($sector_id, $total_units, $num_tribes=0) {
		$drop_time = Time::get_t_now();	// +60 Sicherheitslatenz
		$sector_id = $sector_id % 100;

// EXISITERT NICHT	$total_units *= get_protector_factor($sector_id);
		if ($num_tribes < 1)
			$num_tribes = 1;
		if ($num_tribes > count($this->tribe_ids))
			$num_tribes = count($this->tribe_ids);

		$sql =
			 'SELECT SUM(c.amount)/4 FROM computed_tribes AS c, tribes AS t '.
			 'WHERE c.sector_id='.$sector_id.' '.
				'AND c.world_id='.$this->world_id.' '.
				'AND c.map_id='.$this->map_id.' '.
			 	'AND t.uid='.self::USER_HOST_ID.' '.
			 	'AND t.id=c.tribe_id';
		$res = query($sql);
		$actual_total_amount = (mysql_num_rows($res) > 0)
			? mysql_result($res,0)
			: 0;
		// Für später in die DB :)
		$path_str = ($sector_id < 10) ? '0'.$sector_id : $sector_id;

		$first = true;
		$sql =
			'INSERT INTO tribes_on_map '.
			'(tribe_id, t_appearance, amount, world_id, map_id, path) '.
			'VALUES ';
		if ($actual_total_amount < 100) {
			for($i = $num_tribes-1; $i > 0; $i--) {
				if ($this->tribe_ids[$i]) {
					$amount = rand($total_units/$num_tribes*0.8,
										$total_units/$num_tribes*1.2);
					switch($i) {
						case 2:
						case 5:
							$fac=0.4;
							break; //special units mit timeburst/warp und sticky slimes etc
						case 1:
						case 4:
							$fac=0.2;
							break; //def units mit def slimes
						default:
							$fac=2.5; //off units
					}
					$amount = $amount*$fac;
					if ($amount > $total_units)
						$amount = $total_units;
					$total_units -= $amount;
					$num_tribes--;
					if ($first)
						$first = false;
					else
						$sql .= ',';

					$sql .=
						"({$this->tribe_ids[$i]}, ".
						"$drop_time,".
						"$amount, ".
						"{$this->world_id}, ".
						"{$this->map_id}, ".
						"'$path_str')";
				}
			}
			if (!$this->tribe_ids[0]) {
				error_log('Units to drop not contain no data (maybe a map has just been finished. Check this bug)');
				return;
			}
			if ($first)
				$first = false;
			else
				$sql .= ',';

			$sql .=
				"({$this->tribe_ids[0]}, ".
				"$drop_time, ".
				"$total_units, ".
				"{$this->world_id}, ".
				"{$this->map_id}, ".
				"'$path_str')";
		}

		/**
		 * Nur ausführen, wenn mindestens ein Datum eingefügt werden soll.
		 */
		if (!$first)
			query($sql);
	}

	public function drop_host_units_by_strength($sector_id, $strength) {
		// $total_units Einheiten werden insgesamt gedroppt
		$total_units = rand($strength*150, $strength*200);
		// $num_tribes verschiedene Tribes werden dabei erstellt (wenn möglich)
		$num_tribes = ceil($strength/15);
		$this->drop_host_units_by_amount($sector_id, $total_units, $num_tribes);
	}



	public function get_id() {
		return self::USER_HOST_ID;
	}
}

?>