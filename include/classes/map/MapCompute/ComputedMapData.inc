<?php

class MapCompute__ComputedMapData {
	private $computed_map_data = array();
	private $compute_id;
	private $world_id;
	private $map_id;
	/**
	 *
	 * @var MapCompute
	 */
	private $outer;

	private static $INSTANCES = array();
	/**
	 * Gibt die Instanz zurück. Das ist eine Nested-Class (fake) und somit
	 * muss eine Referenz auf die Outer-Class mitübergeben werden.
	 *
	 * @param MapCompute $outer
	 * @param bool $force
	 * @return MapCompute__ComputedMapData
	 */
	public static function get_instance(MapCompute $outer, $force=false) {
		if (!$outer) {
			throw new Exception("Outer Class must not be NULL");
		}
		$map_id = $outer->get_id();
		$world_id = $outer->get_world_id();
		if (!isset(self::$INSTANCES[$world_id][$map_id]))
			self::$INSTANCES[$world_id][$map_id] =
				new MapCompute__ComputedMapData($outer);
		else if ($force)
			self::$INSTANCES[$world_id][$map_id]->init_from_db();
		return self::$INSTANCES[$world_id][$map_id];
	}

	private function __construct(MapCompute $outer) {
		$this->outer = $outer;
		$this->compute_id = $outer->get_last_compute_id();
		$this->map_id = $outer->get_map_id();
		$this->world_id = $outer->get_world_id();
		$this->init_from_db();
	}

	private function init_from_db() {
		$this->computed_map_data = array();
		$res = query (
			'SELECT user_id, user_units_died, user_buildings_destroyed, ' .
				'user_bio_destroyed, user_sp_destroyed, enemy_units_killed, '.
				'enemy_buildings_destroyed, enemy_bio_destroyed, '.
				'enemy_sp_destroyed FROM computed_map_data '.
			"WHERE world_id={$this->world_id} AND map_id={$this->map_id} ".
				"AND compute_id={$this->compute_id}"
		);

		while ($row = mysql_fetch_assoc($res)) {
			$uid = $row['user_id'];
			unset($row['user_id']);
			$this->computed_map_data[$uid] = $row;
		}
	}

	public function save($new_compute_id) {
		$this->compute_id = $new_compute_id;


		$sql =
			"INSERT INTO computed_map_data ".
			"(world_id, map_id, compute_id, user_id, user_units_died, ".
				"user_buildings_destroyed, user_bio_destroyed,".
				"user_sp_destroyed, enemy_units_killed,".
				"enemy_buildings_destroyed, enemy_bio_destroyed,".
				"enemy_sp_destroyed) ".
			"VALUES ";

		$first_value = true;
		foreach ($this->computed_map_data as $uid => $data) {
			if ($first_value) {
				$first_value = false;
			}
			else {
				$sql .= ',';
			}
			$sql .=
				"({$this->world_id}, ".
				"{$this->map_id}, ".
				"$new_compute_id,".
				"$uid,".
				(isset($data['user_units_died'])
					? $data['user_units_died'] : 0).','.
				(isset($data['user_buildings_destroyed'])
					? $data['user_buildings_destroyed'] : 0). ','.
				(isset($data['user_bio_destroyed'])
					? $data['user_bio_destroyed'] : 0).	','.
				(isset($data['user_sp_destroyed'])
					? $data['user_sp_destroyed'] : 0).','.
				(isset($data['enemy_units_killed'])
					? $data['enemy_units_killed'] : 0).','.
				(isset($data['enemy_buildings_destroyed'])
					? $data['enemy_buildings_destroyed'] : 0).','.
				(isset($data['enemy_bio_destroyed'])
					? $data['enemy_bio_destroyed'] : 0).','.
				(isset($data['enemy_sp_destroyed'])
					? $data['enemy_sp_destroyed'] : 0).')';
		}
		/**
		 * Nur ausführen, wenn mindestens ein Datum eingefügt werden soll.
		 */
		if (!$first_value)
			query($sql);
	}

	/**
	 * Gibt die zugehörige $compute_id zurück, die dieses Objekt identifizieren.
	 *
	 * @return unsigned int
	 */
	public function get_compute_id() {
		return $this->compute_id;
	}
	/**
	 * Shortcut für get_compute_id()
	 *
	 * @return unsigned int
	 */
	public function get_id() {
		return $this->get_compute_id();
	}
	/**
	 * Gibt zurück, wieviel Bio die Karte an den User $of_user_id ausgeschüttet
	 * hat. Ist der User mit der ID $of_user nicht auf der Karte, wird FALSE
	 * zurückgegeben.
	 *
	 * @param int $of_user_id	Von welchem User
	 * @return unsigned float
	 */
	public function get_map_bio($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['map_bio']
			: false;
	}
	/**
	 * Gibt zurück, wie viele Einheiten vom User mit der ID $of_user_id
	 * insgesamt im Verlauf der Karte gestorben sind. Ist dieser User nicht
	 * auf der Karte vorhanden, wird FALSE zurückgegeben.
	 *
	 * @param unsigned int $of_user_id	Welcher User
	 * @return unsigned float
	 */
	public function get_units_lost($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['user_units_died']
			: false;
	}
	/**
	 * Gibt zurück, wie viel Bio die Einheiten, die der User mit der ID
	 * $of_user_id insgesamt im Verlauf der Karte verloren hat, insgesamt wert
	 * waren. Ist dieser User nicht auf der Karte vorhanden, wird FALSE
	 * zurückgegeben.
	 *
	 * @param unsigned int $of_user_id	Welcher User
	 * @return unsigned float
	 */
	public function get_units_lost_in_bio($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['user_bio_destroyed']
			: false;
	}

	/**
	 * Gibt zurück, wie viele Gebäude der User mit der ID $of_user_id im Verlauf
	 * der Karte verloren hat.Ist dieser User nicht auf der Karte vorhanden,
	 * wird FALSE zurückgegeben.
	 *
	 * @param unsigned int $of_user_id	Welcher User
	 * @return unsigned int
	 */
	public function get_buildings_lost($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['user_buildings_destroyed']
			: false;
	}

	/**
	 * Gibt zurück, wieviel Bio die Gebäude, die der User mit der ID $of_user_id
	 * im Verlauf der Karte verloren hat, insgesamt wert waren.Ist dieser User
	 * nicht auf der Karte vorhanden, wird FALSE zurückgegeben.
	 *
	 * @param unsigned int $of_user_id	Welcher User
	 * @return unsigned int
	 */
	public function get_buildings_lost_in_bio($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['user_sp_destroyed']
			: false;
	}

	/**
	 * Gibt zurück, wie viele Einheiten der User mit der ID $of_user_id
	 * insgesamt im Verlauf der Karte getötet hat. Ist dieser User nicht
	 * auf der Karte vorhanden, wird FALSE zurückgegeben.
	 *
	 * @param unsigned int $of_user_id	Welcher User
	 * @return unsigned float
	 */
	public function get_enemy_units_killed($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['enemy_units_killed']
			: false;
	}
	/**
	 * Gibt zurück, wie viel Bio die Einheiten, die der User mit der ID
	 * $of_user_id insgesamt im Verlauf der Karte getötet hat, insgesamt wert
	 * waren. Ist dieser User nicht auf der Karte vorhanden, wird FALSE
	 * zurückgegeben.
	 *
	 * @param unsigned int $of_user_id	Welcher User
	 * @return unsigned float
	 */
	public function get_enemy_units_killed_in_bio($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['enemy_bio_destroyed']
			: false;
	}

	/**
	 * Gibt zurück, wie viele gegnerische Gebäude der User mit der ID
	 * $of_user_id im Verlauf der Karte zerstört hat.Ist dieser User nicht auf
	 * der Karte vorhanden, wird FALSE zurückgegeben.
	 *
	 * @param unsigned int $of_user_id	Welcher User
	 * @return unsigned int
	 */
	public function get_enemy_buildings_destroyed($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['enemy_buildings_destroyed']
			: false;
	}

	/**
	 * Gibt zurück, wieviel Bio die Gebäude, die der User mit der ID $of_user_id
	 * im Verlauf der Karte zerstört hat, insgesamt wert waren.Ist dieser User
	 * nicht auf der Karte vorhanden, wird FALSE zurückgegeben.
	 *
	 * @param unsigned int $of_user_id	Welcher User
	 * @return unsigned int
	 */
	public function get_enemy_buildings_destroyed_in_bio($of_user_id) {
		ValueChecker::id($of_user_id, 'of_user_id');

		return ($this->computed_map_data[$of_user_id])
			? $this->computed_map_data[$of_user_id]['enemy_sp_destroyed']
			: false;
	}

	/**
	 * Setzt, wie viele Einheiten der User insgesamt auf der Karte getötet hat.
	 *
	 * @param uint $uid		ID des Users
	 * @param uint $amount	Anzahl
	 */
	public function set_enemy_units_killed($uid, $amount) {
		ValueChecker::id($uid, 'uid');
		ValueChecker::float($amount, 'amount', 0);

		$this->computed_map_data[$uid]['enemy_units_killed'] = $amount;
	}

	/**
	 * Fügt zu der Anzahl der vom User getöteten Einheiten die angegebene Anzahl
	 * hinzu.
	 *
	 * @param uint $uid		ID des Users
	 * @param int $amount	Anzahl
	 */
	public function add_enemy_units_killed($uid, $amount) {
		if (isset($this->computed_map_data[$uid]['enemy_units_killed']))
			$this->computed_map_data[$uid]['enemy_units_killed'] += $amount;
		else
			$this->computed_map_data[$uid]['enemy_units_killed'] = $amount;
	}

	/**
	 * Setzt, wie viele Einheiten der User insgesamt auf der Karte verloren hat.
	 *
	 * @param uint $uid		ID des Users
	 * @param uint $amount	Anzahl
	 */
	public function set_units_lost($uid, $amount) {
		ValueChecker::id($uid, 'uid');
		ValueChecker::float($amount, 'amount', 0);

		$this->computed_map_data[$uid]['user_units_died'] = $amount;
	}

	/**
	 * Fügt zu der Anzahl der verlorenen Einheiten die angegebene hinzu.
	 *
	 * @param uint $uid		ID des Users
	 * @param int $amount	Anzahl
	 */
	public function add_units_lost($uid, $amount) {
		ValueChecker::id($uid, 'uid');
		ValueChecker::float($amount, 'amount', 0);

		if (isset($this->computed_map_data[$uid]['user_units_died']))
			$this->computed_map_data[$uid]['user_units_died'] += $amount;
		else
			$this->computed_map_data[$uid]['user_units_died'] = $amount;
	}

	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		query ("DELETE FROM computed_map_data");
	}
}

?>