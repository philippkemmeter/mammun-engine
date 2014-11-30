<?php

require_once(dirname(__FILE__).'/../../score/GameScore.inc');

class MapCompute__ComputedSectorData {
	const SECTOR_STATUS_NONE = 0;
	const SECTOR_STATUS_FIGHT = 1;
	const SECTOR_STATUS_EXPLORE = 2;

	private $computed_sector_data = array();
	private $compute_id;
	private $sector_dominators = array();
	private $status = array();
	private $t_stamp_start = array();
	/**
	 * @var MapCompute
	 */
	private $outer = NULL;
	private $world_id;
	private $map_id;

	private static $INSTANCES = array();

	/**
	 * @param MapCompute $outer
	 * @param bool $force
	 * @return MapCompute__ComputedSectorData
	 */
	public static function get_instance(MapCompute $outer, $force = false)
	{
		if (!$outer) {
			throw new Exception("Outer Class must not be NULL");
		}
		$map_id = $outer->get_id();
		$world_id = $outer->get_world_id();
		if (!isset(self::$INSTANCES[$world_id][$map_id]))
			self::$INSTANCES[$world_id][$map_id] =
				new MapCompute__ComputedSectorData($outer);
		else if ($force)
			self::$INSTANCES[$world_id][$map_id]->init_from_db();
		return self::$INSTANCES[$world_id][$map_id];
	}

	private function __construct(MapCompute $outer) {
		$this->outer = $outer;
		$this->map_id = $outer->get_id();
		$this->world_id = $outer->get_world_id();
		$this->compute_id = $outer->get_last_compute_id();
		$this->init_from_db();
	}

	public function save($compute_id) {
		$this->compute_id = $compute_id;

		$sql =
			"INSERT INTO computed_extra ".
			"(world_id, map_id, compute_id, sector_id, user_id,".
				"sector_resource1, sector_resource2) ".
			"VALUES ";
		/**
		 * Alle Sektoren durchgehen und für jeden Sektor alle Spieler. Diese
		 * Daten werden dann gespeichert.
		 */
		$first_value = true;
		foreach ($this->computed_sector_data as $sector_id => $u_datas) {
			foreach ($u_datas as $uid => $u_data) {
				if ($first_value) {
					$first_value = false;
				}
				else {
					$sql .= ',';
				}
				$sql .=
					"({$this->world_id}, ".
					"{$this->map_id}, ".
					"$compute_id,".
					"$sector_id,".
					"$uid,".
					"{$u_data['sector_resource'][0]},".
					"{$u_data['sector_resource'][1]})";
			}
		}
		/**
		 * Wenn mindestens ein Eintrag gemacht wurde, dann Query ausführen.
		 */
		if (!$first_value)
			query ($sql);

		/**
		 * Nun dasselbe Prozedere für die Daten, die NICHT user-bezogen
		 * gespeichert werden. Also die Daten, die für alle User gleich
		 * aussehen.
		 */
		$sql =
			"INSERT INTO computed_sector_data ".
			"(world_id, map_id, compute_id, sector_id, owner, status, ".
				"t_stamp_start) ".
			"VALUES ";
		$first_value = true;

		for ($sector_id = 0; $sector_id < 100; $sector_id++) {
			$dominator = (isset($this->sector_dominators[$sector_id]))
				? (int)$this->sector_dominators[$sector_id] : 0;

			$status = isset($this->status[$sector_id])
					? (int) $this->status[$sector_id] : 0;

			$t_stamp = isset($this->t_stamp_start[$sector_id])
					? (int) $this->t_stamp_start[$sector_id] : 0;

			if ($first_value) {
				$first_value = false;
			}
			else {
				$sql .= ',';
			}

			$sql .=
				"({$this->world_id}, ".
				"{$this->map_id}, ".
				"$compute_id, ".
				"$sector_id, ".
				"$dominator, ".
				"$status, ".
				"$t_stamp)";

		}
		if (!$first_value)
			query ($sql);

	}

	private function init_from_db() {
		$this->computed_sector_data = array();
		// Der Tabellenname computed_extra ist unpassend gewählt.
		$res = query (
			'SELECT * FROM computed_extra '.
			"WHERE world_id={$this->world_id} AND map_id={$this->map_id} ".
				"AND compute_id={$this->compute_id}"
		);
		while ($row = mysql_fetch_assoc($res)) {
			$sid = $row['sector_id'];
			$uid = $row['user_id'];

			for ($r = 0; $r < Resource::NUM_RESOURCES; $r++) {
				$this->computed_sector_data[$sid][$uid]['sector_resource'][$r]
					= $row['sector_resource'.($r+1)];
			}
		}

		$res = query (
			'SELECT * FROM computed_sector_data '.
			"WHERE world_id={$this->world_id} AND map_id={$this->map_id} ".
				"AND compute_id={$this->compute_id}"
		);

		while ($row = mysql_fetch_assoc($res)) {
			$this->sector_dominators[$row['sector_id']] = $row['owner'];
			$this->status[$row['sector_id']] = $row['status'];
			$this->t_stamp_start[$row['sector_id']] = $row['t_stamp_start'];
		}

	}

	/**
	 * Gibt zurück, wie viel Resource eines Typs der User mit der ID $of_user
	 * im Sektor $in_sector_id bereits insgesamt gesammelt hat. Gibt es den User
	 * auf der Karte nicht oder gibt es keinen Eintrag von dem User für den
	 * Sektor, wird FALSE zurückgegeben.
	 *
	 * @param uint $of_user_id			Der User
	 * @param [0,1,..99] $in_sector_id	Der Sektor :)
	 * @param uint $which_resource		Welche Ressource abgefragt werden soll
	 * @return ufloat
	 */
	public function get_resource($of_user_id, $in_sector_id, $which_resource) {
		$in_sector_id %= 100;
		return (isset($this->computed_sector_data[$in_sector_id][$of_user_id]))
			? $this->computed_sector_data[$in_sector_id][$of_user_id]
										['sector_resource'][$which_resource]
			: false;
	}

	/**
	 * Fügt dem User mit der ID $of_user_id die übergebene Anzahl Ressource
	 * des übergebenen Typs für den angegebenen Sektor hinzu.
	 *
	 * @param uint $of_user_id			Der User
	 * @param [0,1,..99] $in_sector_id	Der Sektor :)
	 * @param uint $which_resource		Welche Ressource abgefragt werden soll
	 * @param ufloat $amount			Wieviel
	 */
	public function add_resource($of_user_id, $in_sector_id, $which_resource,
		$amount)
	{
		GameScore::add_score($this->world_id, $this->map_id, $of_user_id,
			$amount, GameScore::TYPE_RESSOURCE_GAIN);
		$this->computed_sector_data[$in_sector_id][$of_user_id]
					['sector_resource'][$which_resource] += $amount;
	}

	/**
	 * Gibt den gespeicherten Dominator des Sektors zurück.
	 *
	 * @param uint $sector_id
	 */
	public function get_dominator($sector_id) {
		return isset($this->sector_dominators[$sector_id])
			? $this->sector_dominators[$sector_id] : 0;
	}

	/**
	 * Setzt den Dominator eines Sektors.
	 *
	 * @param [0,1,..99] $sector_id		Der Sektor :)
	 * @param unsigned int $uid			Der neue Dominator
	 */
	public function set_dominator($sector_id, $uid) {
		$uid = (int)$uid;
		$this->sector_dominators[$sector_id] = $uid;
	}

	/**
	 * Gibt alle Sektoren als IDs zurück, die gerade vom User mit der
	 * angegebenen ID dominiert werden.
	 *
	 * @param uint $uid
	 * @return {0,1,...,99}[]
	 */
	public function get_all_sectors_dominated_by($uid) {
		$result = array();
		foreach ($this->sector_dominators as $sector_id => $_uid) {
			if ($uid == $_uid)
				$result[] = $sector_id;
		}
		return $result;
	}

	/**
	 * Setzt den Status des Sektors, also ob Kampf, Erkundung oder nichts
	 * besonderes.
	 *
	 * @param uint $sector
	 * @param self::SECTOR_STATUS_... $status
	 * @param uint $t_stamp
	 */
	private function set_sector_status($sector_id, $status, $t_stamp=0) {
		switch ($status) {
			case self::SECTOR_STATUS_NONE:
			case self::SECTOR_STATUS_EXPLORE:
			case self::SECTOR_STATUS_FIGHT:
				$this->t_stamp_start[$sector_id]
					= $t_stamp ? $t_stamp : Time::get_t_now();
				$this->status[$sector_id] = (int)$status;
				break;
			default:
				throw new IllegalArgumentException(
					'status', '{0,1,2}', $status
				);
		}
	}

	/**
	 * Gibt den Sektorstatus zurück.
	 *
	 * @param uint $sector_id
	 * @return self::SECTOR_STATUS_...
	 */
	public function get_sector_status($sector_id) {
		return isset($this->status[$sector_id]) ?
			$this->status[$sector_id] : 0;
	}


	/**
	 * Setzt, dass ab <code>$t_stamp</code> gekämpft wird.
	 *
	 * @param uint $sector_id
	 * @param uint $t_stamp
	 */
	public function start_fight($sector_id, $t_stamp=0) {
		if ($this->get_explore_in_sector($sector_id)) {
			$this->end_exploration($sector_id, $t_stamp);
		}
		$this->set_sector_status(
			$sector_id, self::SECTOR_STATUS_FIGHT, $t_stamp
		);
	}

	/**
	 * Setzt, dass ab <code>$t_stamp</code> nicht mehr gekämpft wird.
	 *
	 * @param $sector_id
	 * @param $t_stamp
	 */
	public function end_fight($sector_id, $t_stamp=0) {
		if ($this->get_fight_in_sector($sector_id)) {
			$this->set_sector_status(
				$sector_id, self::SECTOR_STATUS_NONE, $t_stamp
			);
		}
	}

	/**
	 * Gibt zurück, ob im Sektor gekämpft wird.
	 *
	 * Das ist eine Kurzform für <code>get_sector_status($sector_id) ==
	 * MapCompute__ComputedSectorData::SECTOR_STATUS_FIGHT</code>.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return bool
	 */
	public function get_fight_in_sector($sector_id) {
		return (isset($this->status[$sector_id])
			&& ($this->status[$sector_id] == self::SECTOR_STATUS_FIGHT));
	}

	/**
	 * Setzt, dass ab <code>$t_stamp</code> erkundet wird.
	 *
	 * @param uint $sector_id
	 * @param uint $t_stamp
	 */
	public function start_exploration($sector_id, $t_stamp=0) {
		if ($this->get_fight_in_sector($sector_id)) {
			$this->end_fight($sector_id, $t_stamp);
		}
		$this->set_sector_status(
			$sector_id, self::SECTOR_STATUS_EXPLORE, $t_stamp
		);
	}

	/**
	 * Setzt, dass ab <code>$t_stamp</code> nicht mehr gekämpft wird.
	 *
	 * @param $sector_id
	 * @param $t_stamp
	 */
	public function end_exploration($sector_id, $t_stamp=0) {
		if ($this->get_explore_in_sector($sector_id)) {
			$this->set_sector_status(
				$sector_id, self::SECTOR_STATUS_NONE, $t_stamp
			);
		}
	}

	/**
	 * Gibt zurück, ob im Sektor erkundet wird.
	 *
	 * Das ist eine Kurzform für <code>get_sector_status($sector_id) ==
	 * MapCompute__ComputedSectorData::SECTOR_STATUS_EXPLORE</code>.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return bool
	 */
	public function get_explore_in_sector($sector_id) {
		return (isset($this->status[$sector_id])
			&& ($this->status[$sector_id] == self::SECTOR_STATUS_EXPLORE));
	}

	/**
	 * Gibt zurück, wann der letzte Statuswechsel war als Timestamp zurück.
	 *
	 * Wird <code>0</code> zurückgegeben, dann gibt es entweder den Sektor
	 * nicht, oder er hat noch nie einen Statuswechsel erfahren.
	 *
	 * @param uint $sector_id
	 * @return uint
	 */
	public function get_last_state_change_t_stamp($sector_id) {
		return (isset($this->t_stamp_start[$sector_id]))
			? $this->t_stamp_start[$sector_id] : 0;
	}

	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		query ("DELETE FROM computed_extra");
		query ("DELETE FROM computed_sector_data");
	}

}

?>