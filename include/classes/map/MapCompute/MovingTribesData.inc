<?php

/**
 * Repräsentiert die Daten aller Einheiten, die gerade unterwegs sind.
 * Ankunftszeiten, Pfade usw. können hierüber abgefragt und geändert werden.
 * Da hierüber keine Berechnungen durchgeführt werden, die am Ende gespeichert
 * werden, ist das kein Computed_XY-Objekt. Etwaige Berechnungen die z.B. durch
 * das Kreuzen von Pfaden entstehen können, werden über ComputedTribesData
 * gespeichert, da dann auf der Karte ja wieder Tribes "entstehen".
 */
class MapCompute__MovingTribesData {
	/**
	 * Die Referenz auf die äußere Klasse (Nested-Fake)
	 *
	 * @var MapCompute
	 */
	private $outer_class;

	/**
	 * Enthält alle Daten der derzeitigen Truppenbewegungen auf der Karte, und
	 * zwar mit den Positionen pro Zyklus. Sprich: Abfragen à la "Im
	 * Berechnungszyklus X: Welche Einheiten bewegten sich dann gerade
	 * durch den Sektor Y?" sind hiermit effizient zu beantworten.
	 *
	 * @var uint[][][][]
	 */
	private $movement_data;

	/**
	 * Enthält alle Einheiten, die angekommen sind nach anstrengender Bewegung
	 * sortiert nach Ankunftszeitpunkten. ACHTUNG: Nur die Vergangenheit
	 * zählt: Einheiten, die gerade angekommen würden und in der Zukunft werden
	 * hier nicht berücksichtigt.
	 *
	 * @var uint[][][]
	 */
	private $appearances;

	/**
	 * Enthält nur die IDs der Einheiten, die sich noch bewegen.
	 *
	 * @var uint[]
	 */
	private $moving_units;

	/**
	 * ID der Karte, auf welche sich die Berechnungen beziehen
	 *
	 * @var int
	 */
	private $map_id;

	/**
	 * ID der Welt, in der die Karte sich befindet.
	 *
	 * @var uint
	 */
	private $world_id;

	private static $INSTANCES = array();

	/**
	 * Gibt die Instanz zurück. Das ist eine Nested-Class (fake) und somit
	 * muss eine Referenz auf die Outer-Class mitübergeben werden.
	 *
	 * @param MapCompute $outer
	 * @return MapCompute__MovingTribesData
	 */
	public static function get_instance(MapCompute $outer, $force=false) {
		if (!$outer) {
			throw new Exception("Outer Class must not be NULL");
		}

		if (!isset(self::$INSTANCES[$outer->get_id()])) {
			self::$INSTANCES[$outer->get_id()] =
				new MapCompute__MovingTribesData($outer, $force);
		}
		/**
		 * Die Outer-Class einer MapCompute__ComputedBuildingData-Instanz
		 * kann unter normalen Umständen niemals wechseln, weil die Compute-ID
		 * die Outer-Class eindeutig bestimmt. Also wird, wenn immer dieselbe
		 * Compute-ID übergeben wird, auch immer dieselbe Outer-Class übergeben.
		 * Wenn diese Regel verletzt wird, ist etwas oberfaul! Entweder ist die
		 * DB dann inkonsistent und eine compute_id ist mehreren Karten
		 * zugeordnet worden, oder es gab einen fehlerhaften Aufruf, also, dass
		 * eine MapCompute-Instanz keinen compute von sich, sondern von einer
		 * anderen Instanz übergeben hat, oder nicht sich selbst als Outer-Class,
		 * sondern eine andere Instanz selbst übergeben hat. Dieser fall lässt
		 * sich leicht prüfen: Man suche nach RegExp
		 * "MapCompute__ComputedBuildingData::get_instance\(.*, .*\)"
		 * und schaue, ob überall als zweiter Paramter $this steht.
		 */
		else if (self::$INSTANCES[$outer->get_id()]->outer_class !== $outer)
			throw new ExceptionWithErrormail("Outer class has changed!",
				$current_user?$current_user->get_id():0, 'All',
				'MapCompute__MovingTribesData: Outer class has changed!',
				'See MapCompute__MovingTribesData::get_instance(). The '.
				"expected outer class id is ".
				self::$INSTANCES[$outer->get_id()]->outer_class->get_id().", but ".
				"the given outer class id is {$outer->get_id()}.");
		else if ($force)
			self::$INSTANCES[$outer->get_id()]->init_from_db();
		return self::$INSTANCES[$outer->get_id()];
	}

	private function __construct(MapCompute $outer, $force) {
		$this->outer_class = $outer;
		$this->map_id = $this->outer_class->get_map_id();
		$this->world_id = $this->outer_class->get_world_id();
		$this->init_from_db();
		$this->handle_path_intersections();
	}

	public function save() {
		$tids = array();
		foreach ($this->appearances as $sector_id => $tribes_by_tstamp) {
			foreach ($tribes_by_tstamp as $t_stamp => $t_datas) {
				for ($i = 0, $n = count($t_datas); $i < $n; $i++) {
					$tids[] = $t_datas[$i]->id;
				}
			}
		}

		if (!empty($tids)) {
			// Alle, die bereits erschienen sind, bewegen sich nicht mehr.
			query (
				'UPDATE tribes_on_map SET t_movement_started=0 '.
				'WHERE id IN ('.implode(', ', $tids).')'
			);
		}
	}

	private function init_from_db() {
		$this->appearances = array();
		$this->movement_data = array();
		$this->moving_units = array();
		$compute_time = $this->outer_class->get_last_compute_time();
		$map_id = $this->outer_class->get_map_id();
		$world_id = $this->outer_class->get_world_id();

		/**
		 * Die Einheiten holen, die seit der letzten Berechnung noch nicht
		 * angekommen sind. Die Daten werden eingelesen und danach geschaut,
		 * wo sich die Einheiten zu welchem Zyklus befinden, und ob es zu
		 * Kollissionen kommt etc. Auch sowas wie Trigger bei Sektordurchlauf
		 * kann dann hier gemacht werden.
		 */
		$res = query(
			'SELECT tmap.id, t.uid, tmap.amount, '.
				'tmap.t_appearance AS time_placed, '.
				'tmap.t_movement_started AS time_started, tmap.path, '.
				'IF(ta.ability_id IS NOT NULL, TRUE, FALSE) AS camouflaged '.
			'FROM tribes AS t, tribes_on_map AS tmap '.
			'LEFT JOIN tribes_abilities AS ta '.
				'ON ta.tribe_id=tmap.tribe_id '.
				'AND ta.ability_id=4001 '.
			'WHERE tmap.tribe_id=t.id '.
				'AND tmap.world_id='.$world_id.' '.
				'AND tmap.map_id='.$map_id.' '.
				'AND tmap.t_movement_started>0'
		);

		while ($row = mysql_fetch_assoc($res)) {
			$path = str_split($row['path'],2);
			$t_stamps = self::get_position_t_stamps($path,
				$row['time_started'], $row['time_placed']);
			$unit_id = $row['id'];
			for ($i = 0;	// Alle Sektoren der Vergangenheit durchgehen
				($i < count($path)-1) && ($t_stamps[$i] < Time::get_t_now());
				$i++)
			{
				$cycle = $this->outer_class->time_to_cycle(
					$t_stamps[$i], $compute_time);

				$this->movement_data[$cycle][$path[$i]][$unit_id]
	 				= array (
	 					'amount'		=>	$row['amount'],
						'time_placed'	=>	$t_stamps[$i],//$row['time_placed']
					);
			}

			/**
			 * Der letzte Sektor im Pfad ist der Ankunftssektor. Das speziell
			 * markieren, damit man weiß, dass die Einheit dann im Sektor
			 * erscheinen muss.
			 */
			if ($row['time_placed'] <= Time::get_t_now())
			{	// Nur hinzufügen, wenn Erscheinungszeitpunkt nicht in Zukunft
				$t = new stdClass();
				$t->amount = $row['amount'];
				$t->id = $unit_id;
				$t->can_camouflage = $row['camouflaged'];
				$t->type = MapCompute__ComputedTribesData::T_TRIBE;
				$t->uid = $row['uid'];
				$this->appearances[$path[count($path)-1]][$row['time_placed']][]
					= $t;
			}
			else
			{ // Die Einheit bewegt sich noch
				$this->moving_units[$unit_id] = $row['amount'];
			}
		}
		ksort($this->appearances, SORT_NUMERIC);
	}

	private function handle_path_intersections() {
		return;


		foreach ($this->movement_data as $cycle => $data_per_cycle) {
			foreach ($data_per_cycle as $sector_id => $units) {
				if (count($units) > 1) {
					// Min. Zwei Einheiten haben sich in einem Sektor innerhalb
					// eines Zeitfensters==Berechnungszyklus getroffen
					$uids = array();
					$stop_all = false;
					$tribes = array();
					foreach ($units as $unit_id => $data) {
						$tribe = TribeOnMap::get_instance(
							$unit_id, $sector_id,
							$this->outer_class->get_map_id(),
							$this->outer_class->get_world_id()
						);
						if (!empty($uids) && !in_array($tribe->get_uid(), $uids)) {
							$stop_all = true;
							break;
						}
						$uids[] = $tribe->get_uid();
						$tribes[] = $tribe;
					}
					if ($stop_all) {
						foreach ($tribes as $t) {
							$time_placed = $this->outer_class->cycle_to_time(
								$cycle,
								$this->outer_class->get_last_compute_time()
							);


							// TODO: SIEHE OBEN BEI INIT WIE APPEARANCE AUSSIEHT
							$this->appearances[$sector_id][$time_placed][]
								= array (
									'tribe_id'	=>	$unit_id,
									'amount'	=>	$this->moving_units[$unit_id]
								);
							unset($this->moving_units[$unit_id]);
							query (
								// t_apearance auf 0, damit CopmutedTribesData
								// die nicht nochmal einlesen kann
								'UPDATE tribes_on_map SET t_movement_started=0 '.
								'WHERE id='.$this->id
							);
//							$t->_set_unit_has_been_stopped($sector_id);
						}
					}
				}
			}
		}
	}

	public function get_appearances($sector_id) {
		return isset($this->appearances[$sector_id])
			? $this->appearances[$sector_id]
			: null;
	}

	public function add_appearances($sector_id, $unit_id, $time_placed=null) {
		if (!$time_placed)
			$time_placed = Time::get_t_now();
		$o = new stdClass();
		$o->amount = $this->moving_units[$unit_id];
		$o->id = $unit_id;
		$o->type = MapCompute__ComputedTribesData::T_TRIBE;
		$this->appearances[$sector_id][$time_placed][] = $o;
	}



	/**
	 * Gibt ein Array zurück, das pro Sektor im Pfad die Ankuftszeit zurückgibt,
	 * aufsteigend sortiert.
	 *
	 * @param uint[] $path
	 * @param uint $start_time
	 * @param uint $arrival_time
	 * @return uint[]
	 */
	private static function get_position_t_stamps(array $path, $start_time,
		$arrival_time)
	{

		ValueChecker::t_stamp($start_time, 'start_time');
		ValueChecker::t_stamp($arrival_time, 'arrival_time');
		ValueChecker::int($start_time, 'start_time', 0, $arrival_time);

		$travel_duration = $arrival_time - $start_time;
		$count_of_path = count($path);
		$duration_per_sector = ($count_of_path > 1)
			? $travel_duration / ($count_of_path - 1)
			: 0;
		$t = $start_time;
		$result = array();
		for ($i = 0; $i < $count_of_path; $i++) {
			$result[$i] = (int)$t;
			$t += $duration_per_sector;
		}
		return $result;
	}

//	/**
//	 * Gibt alle reisende Tribes im Sektor zum Berechnungszyklus $cycle zurück,
//	 * und zwar als "$array[Tribe-ID] = Anzahl".
//	 *
//	 * @param [0,1,...99] $sector_id
//	 * @param uint $cycle
//	 * @return uint[]
//	 */
//	public function get_tribes_in_sector($sector_id, $cycle) {
//		if ($sector_id < 0 || $sector_id > 99)
//			throw new IllegalArgumentException("[0,1,...99]", $sector_id);
//
//		$result = array();
//		if ($this->movement_data[$cycle][$sector_id]) {
//			foreach ($this->movement_data[$cycle][$sector_id] as $tid => $jnk) {
//				$amount = $this->get_amount($tid, $sector_id, $cycle);
//				if ($amount)
//					$result[$tid] += $amount;
//			}
//		}
//	}

//	/**
//	 * Gibt die Anzahl der Übergebenen Einheit zurück, die sich gerade in der
//	 * Bewegung im Sektor zum Berechnungszyklus $cycle befindet.
//	 *
//	 * @param uint $tribe_id
//	 * @param [0,1,...99] $sector_id
//	 * @param $cycle
//	 * @return ufloat
//	 */
//	public function get_amount($tribe_id, $sector_id, $cycle) {
//		if ($sector_id < 0 || $sector_id > 99)
//			throw new IllegalArgumentException("[0,1,...99]", $sector_id);
//
//		if (!get_tribe_is_moving($tribe_id))
//			return 0;
//		return (isset($this->movement_data[$cycle][$sector_id][$tribe_id]))
//			? $this->movement_data[$cycle][$sector_id][$tribe_id]['amount']
//			: 0;
//	}

//	/**
//	 * Gibt die Anzahl Einheiten zurück, die insgesamt zu einem gewissen
//	 * Berechnungszyklus durch den Sektor laufen. Wirf kein Zyklus angegeben,
//	 * wird die Summe über alle Zyklen zurückgegeben.
//	 *
//	 * @param [01,1,..99] $sector_id
//	 * @param uint $cycle
//	 * @return ufloat
//	 */
//	public function get_total_amount($sector_id, $cycle=false) {
//		$result = 0;
//		if ($cycle == false) {
//			foreach ($this->movement_data as $data_per_cycle) {
//				if ($data_per_cycle[$sector_id]) {
//					foreach ($data_per_cycle[$sector_id] as $object_arr) {
//						if (get_tribe_is_moving($object_arr['tribe_id']))
//							$result += $object_arr['amount'];
//					}
//				}
//			}
//		}
//		else {
//			if ($this->movement_data[$cycle][$sector_id]) {
//				foreach ($this->movement_data[$cycle][$sector_id]
//					as $object_arr)
//				{
//					if (get_tribe_is_moving($object_arr['tribe_id']))
//						$result += $object_arr['amount'];
//				}
//			}
//		}
//		return $result;
//	}

	/**
	 * Gibt die sich gerade bewegenden Einheiten als Array
	 * "$a[Tribe-ID] = Anzahl" zurück.
	 *
	 * @return uint[]
	 */
	public function get_all_moving_units() {
		return $this->moving_units ? $this->moving_units : array();
	}

	/**
	 * Fügt einen Tribe zu den bewegenden hinzu.
	 *
	 * @param TribeOnMap $tribe
	 */
	public function add_moving_tribe(TribeOnMap $tribe) {
		$this->moving_units[$tribe->get_id()] = $tribe->get_amount();
	}

	public function remove_moving_tribe(TribeOnMap $tribe) {
		unset($this->moving_units[$tribe->get_id()]);
	}


	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
	}
}

?>