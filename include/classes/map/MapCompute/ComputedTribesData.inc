<?php

require_once (dirname(__FILE__).'/ComputedMapObjectsData.inc');
/**
 * Repräsentiert die Daten eines Tribes auf der Karte, die Kartenberechnungen
 * zugrunde liegen. Das sind alle die Daten, die nicht nur vom Tribe alleine,
 * sondern von irgendeiner Interaktion des Tribes mit irgendwas auf der Karte.
 * Beispiele sind: Hitpoints, die durch Kampf verloren gehen, Attack, der durch
 * "Auren" gesteigert werden kann oder auch statistische Daten, wie z.B. die
 * Anzahl der Einheiten, die der Tribe bereits umgeochst hat :)
 */
class MapCompute__ComputedTribesData
	extends MapCompute__ComputedMapObjectsData
{
	private static $INSTANCES = array();

	/**
	 * Gibt die Instanz zurück. Das ist eine Nested-Class (fake) und somit
	 * muss eine Referenz auf die Outer-Class mitübergeben werden.
	 *
	 * @param MapCompute $outer
	 * @return MapCompute__ComputedTribesData
	 */
	public static function get_instance(MapCompute $outer,
		$force = false)
	{
		if (!$outer) {
			throw new Exception("Outer Class must not be NULL");
		}
		if (!$outer->_get_moving_tribes_data()) {
			throw new Exception("Moving tribes has to be set before");
		}
		$map_id = $outer->get_id();
		$world_id = $outer->get_world_id();
		if (!isset(self::$INSTANCES[$world_id][$map_id])) {
			self::$INSTANCES[$world_id][$map_id] =
				new MapCompute__ComputedTribesData($outer);
		}
		else if ($force)
			self::$INSTANCES[$world_id][$map_id]->init_from_db();

		return self::$INSTANCES[$world_id][$map_id];
	}

	private function __construct(MapCompute $outer) {
		$this->outer_class = $outer;
		$this->compute_id = $outer->get_last_compute_id();
		$this->map_id = $outer->get_id();
		$this->world_id = $outer->get_world_id();

		$this->init_from_db();
	}

	public function save($compute_id) {
		$this->compute_id = $compute_id;

		$sql =
			"INSERT INTO computed_tribes ".
			"(world_id, map_id, compute_id, amount, killed, died, ".
				"camouflaged, sector_id, tribe_id) ".
			"VALUES ";

		/**
		 * Alle Sektoren durchgehen und darin alle Tribes. Diese Werte
		 * werden dann gespeichert.
		 */
		$first_value = true;
		foreach ($this->computed_results as $sector_id => $t_datas) {
			foreach ($t_datas as $tid => $t_data) {
				if ($t_data->amount > 0) {
					if ($first_value) {
						$first_value = false;
					}
					else {
						$sql .= ',';
					}
					$cam = (int)$t_data->camouflaged;
					$sql .=
						"({$this->world_id}, ".
						"{$this->map_id},".
						"{$this->compute_id}, ".
						"{$t_data->amount},".
						"{$t_data->killed},".
						"{$t_data->died},".
						"$cam,".
						"{$sector_id},".
						"$tid)";
				}
			}
		}
		/**
		 * Nur ausführen, wenn mindestens ein Datum eingefügt werden soll.
		 */
		if (!$first_value)
			query($sql);
	}

	private function init_from_db() {
		$map_id = $this->map_id;
		$world_id = $this->world_id;

		$this->data_to_compute = array();
		$compute_time = $this->outer_class->get_last_compute_time();
		$this->appearances = array();
		$this->computed_results = array();
		if ($this->compute_id) {
			/**
			 * Zuerst die bereits berechnenten (also die alten) Tribes holen.
			 * Gibt es keine $this->compute_id, ist klar, dass es noch keine
			 * Berechnung gab und wir sparen uns diese Query.
			 */

			/**
			 * Bevor wir die alten Jungs holen, testen wir die DB auf
			 * Konsistenzen, dass keine Einheit an zwei Orten gleichzeitig ist.
			 */
			$res = query (
				'SELECT DISTINCT c.tribe_id FROM tribes_on_map AS tmap, '.
					'computed_tribes AS c '.
				'WHERE c.tribe_id=tmap.id AND tmap.world_id='.$world_id.' '.
					'AND tmap.map_id='.$map_id.' '.
					'AND c.sector_id!=SUBSTRING(tmap.path, LENGTH(tmap.path)-1)'
			);
			while ($row = mysql_fetch_assoc($res)) {
				query (
					"DELETE FROM computed_tribes ".
					"WHERE tribe_id={$row['tribe_id']}"
				);
			}

			/**
			 * Jetzt können wir die alten auslesen.
			 */
			$res = query (
				'SELECT t.uid, s.tribe_id, s.sector_id,'.
					's.amount, s.killed, s.camouflaged, s.died '.
				'FROM computed_tribes AS s, tribes AS t, tribes_on_map AS tmap '.
				'WHERE s.tribe_id=tmap.id AND tmap.tribe_id=t.id '.
					"AND s.world_id={$this->world_id} ".
					"AND s.map_id={$this->map_id} ".
					'AND s.compute_id='.$this->compute_id
			);

			while ($row = mysql_fetch_assoc($res)) {
				// bei den alten tun wir so, als wären sie zur Computezeit
				// platziert worden
				$row['time_placed'] = $compute_time;
				$this->insert_db_row_in_data_arrays($row, 1);
			}
		}

		/**
		 * Jetzt kommen die neuen Tribes. Diese sind nach der computezeit oder
		 * zur computezeit entstanden. An das Entstehen neuer Tribes können
		 * Ereignisse geknüpft sein, deswegen muss das hier unterschieden
		 * werden.
		 * Neue Tribes entstehen vorallem durch das Produzieren von Einheiten,
		 * durch das Ankommen sich vorher bewegter Einheiten,
		 * aber auch durch das plötzliche Auftauchen von NSC-Einheiten.
		 */
		$res = query(
			'SELECT tmap.id AS tribe_id, tmap.path, t.uid, '.
				'tmap.amount, tmap.t_appearance AS time_placed, '.
				'IF(ta.ability_id IS NOT NULL, TRUE, FALSE) AS camouflaged '.
			'FROM tribes AS t '.
			'LEFT JOIN tribes_abilities AS ta '.
				'ON ta.tribe_id=t.id '.
				'AND ta.ability_id=4001 '.
			'INNER JOIN tribes_on_map AS tmap '.
				'ON tmap.tribe_id=t.id '.
			'LEFT JOIN computed_tribes AS c '.
				'ON c.tribe_id=tmap.id '.
				'AND c.compute_id='.$this->compute_id.' '.
			'WHERE tmap.world_id='.$world_id.' '.
				'AND tmap.map_id='.$map_id.' '.
				'AND tmap.amount > 0 '.
				'AND tmap.t_movement_started=0 '. // Keine Bewegung
				'AND tmap.t_appearance <= '.Time::get_t_now(). ' '.
				'AND c.tribe_id IS NULL'	// <= IS NULL :)
		);
		while ($row = mysql_fetch_assoc($res)) {
			$row['sector_id'] = (int)substr($row['path'], -2);
			$row['killed'] = $row['died'] = 0;
			$cycle = $this->outer_class->time_to_cycle(
							$row['time_placed'], $compute_time);

			$this->insert_db_row_in_data_arrays($row, $cycle);
			// Triggers? Momentan noch keine eingebaut
		}

		// Jetzt noch die sich einst bewegenden holen
		$this->merge_arrived_moved_tribes();

		ksort($this->appearances, SORT_NUMERIC);
	}

	/**
	 * Fügt die Einheiten, die sich bis eben in der Bewegung befanden und jetzt
	 * angehalten wurden, zu den computed tribes daten hinzu. Diese sind bisher
	 * in MovingTribesData registriert, weil dort entschieden wird, wann eine
	 * Einheit die Bewegung beendet hat.
	 * Da die sich nicht bewegenden Einheiten, egal, warum sie sich aktuell
	 * nicht bewegen, hier in dieser Klasse verwaltet werden, ist dieser merge
	 * sinnvoll.
	 */
	private function merge_arrived_moved_tribes() {
		$compute_time = $this->outer_class->get_last_compute_time();

		for ($sector_id = 0; $sector_id < 100; $sector_id++) {
			$move_appearances = $this->outer_class->_get_moving_tribes_data()
											->get_appearances($sector_id);

			if ($move_appearances) {
				foreach ($move_appearances as $time_placed => $tribes) {
					for ($i = 0, $n = count($tribes); $i < $n; $i++) {
						$t_obj = $tribes[$i];
						$this->insert_db_row_in_data_arrays(
							array(
								'tribe_id'		=>	$t_obj->id,
								'amount'		=>	$t_obj->amount,
								'time_placed'	=>	$time_placed,
								'sector_id'		=>	$sector_id,
								'camouflaged'	=>	$t_obj->can_camouflage,
								'uid'			=>	$t_obj->uid,
								'killed'		=>	0,
								'died'			=>	0
							),
							$this->outer_class->time_to_cycle(
								$time_placed, $compute_time
							)
						);
					}
				}

			}
		}

	}

	private function insert_db_row_in_data_arrays($row, $cycle) {
		if (MapCompute::DEBUG_VERBOSE > 3) {
			error_log("ComputedTribesData::insert_db_row_in_data_arrays(row,\
				 cycle=$cycle)\n row = ".print_r($row,true));
		}
		$unit_id = $row['tribe_id'];
		$sector_id = $row['sector_id'];

		$t1 = new stdClass();
		$t1->amount = $row['amount'];
		$t1->time_placed = $row['time_placed'];
		$this->data_to_compute[$cycle][$sector_id][$unit_id] = $t1;

		$t2 = new stdClass();
		$t2->amount = $row['amount'];
		$t2->id = $unit_id;
		$t2->type = self::T_TRIBE; // Markierung nötig wegen Joins in MapCompute
		$this->appearances[$sector_id][$row['time_placed']][] = $t2;

		$t3 = new stdClass();
		$t3->amount = $row['amount'];
		$t3->max_amount = 1000;
		$t3->killed = $row['killed'];
		$t3->died = $row['died'];
		$t3->camouflaged = $row['camouflaged'];
		$this->computed_results[$sector_id][$unit_id] = $t3;

		if (!isset($this->uids_in_sector[$sector_id])
			|| !in_array($row['uid'], $this->uids_in_sector[$sector_id]))
		{
			$this->uids_in_sector[$sector_id][] = $row['uid'];
		}
	}

//	/**
//	 * Gibt die alle Tribes im Sektor zum übergebenen Cycle als Array zurück,
//	 * und zwar als "$array[Tribe-ID] = Anzahl".
//	 *
//	 * @param [0,1,..99] $sector_id		Die ID des Sektors
//	 * @param uint $cycle				In welchem Zyklus (wenn false, dann alle)
//	 * @return int[]
//	 */
//	public function get_tribes_in_sector($sector_id, $cycle=false) {
//		$result = array();
//		if ($cycle == false) {
//			foreach ($this->data_to_compute as $cycle => $data_per_cycle) {
//				if ($data_per_cycle[$sector_id]) {
//					foreach ($data_per_cycle[$sector_id] as $tid => $jnk) {
//						$result[$tid] +=
//							$this->get_amount($tid, $sector_id, $cycle);
//					}
//				}
//			}
//		}
//		else {
//			if ($this->data_to_compute[$cycle][$sector_id]) {
//				foreach ($this->data_to_compute[$cycle][$sector_id] as $tid => $jnk) {
//					$result[$tid] +=
//							$this->get_amount($tid, $sector_id, $cycle);
//				}
//			}
//		}
//		return $result;
//	}

	/**
	 * Gibt alle berechneten Tribes im Sektor als Array zurück
	 *
	 * @param [0,1,...99] $sector_id	Die ID des Sektors
	 * @return TribeOnMap[]
	 */
	public function get_computed_tribes_in_sector($sector_id) {
		$result = array();
		$sort_ids = array();
		if (isset($this->computed_results[$sector_id])) {
			foreach ($this->computed_results[$sector_id] as $tid => $data) {
				if ($data->amount > 0) {
					try {
						$result[] = TribeOnMap::get_instance($tid, $sector_id,
							$this->outer_class->get_id());
						$sort_ids[] = $tid;
					}
					catch(Exception $e){
						// tribe existiert nicht mehr
					}
				}
			}
		}
		array_multisort($sort_ids, $result);
		return $result;
	}

	/**
	 * Gibt zurück, ob in dem Sektor zu irgendeinem Rechenzyklus zwischen
	 * Einheiten gekämpft wird.
	 *
	 * @param [0,1,...99] $sector_id
	 * @return bool
	 */
	public function is_fighting($sector_id) {
		return (isset($this->uids_in_sector[$sector_id])
			&& (count($this->uids_in_sector[$sector_id]) >= 2));
	}


	/**
	 * MapCompute setzt hierüber die Tarnung des Tribes.
	 *
	 * @param uint $tid
	 * @param [0,1,...99] $sector_id
	 * @param bool $camouflaged
	 */
	public function set_tribe_is_camouflaged($tid, $sector_id, $camouflaged) {
		$this->computed_results[$sector_id][$tid]->camouflaged = $camouflaged;
	}

	/**
	 * Gibt zurück, ob der Tribe getarnt ist
	 *
	 * @param uint $tid		Die Tribe-ID
	 * @param [0,1,...99] $sector_id
	 * @return bool
	 */
	public function get_tribe_is_camouflaged($tid, $sector_id) {
		return $this->computed_results[$sector_id][$tid]->camouflaged;
	}

	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::clear_cache();
		query ("DELETE FROM computed_tribes");
	}

	public static function clear_cache() {
		self::$INSTANCES = array();
	}
}

?>
