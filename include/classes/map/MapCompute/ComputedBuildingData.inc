<?php

require_once (dirname(__FILE__).'/ComputedMapObjectsData.inc');
require_once (dirname(__FILE__)
					.'/../../../shared/classes/datastructures/Point2D.inc');
/**
 * Diese Klasse repräsentiert die Daten der Gebäude, die aus Map-Berechnungen
 * heraus entstanden sind, oder in solche Berechnungen miteinbezogen werden
 * müssen. Konkret heißt das z.B., dass hierüber der aktuelle Zustand des
 * Gebäudes (ob angeschlagen, wie sehr etc), aber auch alle Attribute erlangt
 * werden müssen, da auf alle Attribute etc etc andere Objekte auf der Karte
 * Einfluss haben können.
 */
class MapCompute__ComputedBuildingData
	extends MapCompute__ComputedMapObjectsData
{
	private static $INSTANCES = array();
	/**
	 * Gibt die Instanz zurück. Das ist eine Nested-Class (fake) und somit
	 * muss eine Referenz auf die Outer-Class mitübergeben werden.
	 *
	 * @param MapCompute $outer
	 * @param bool $force
	 * @return MapCompute__ComputedBuildingData
	 */
	public static function get_instance(MapCompute $outer, $force = false)
	{
		if (!$outer) {
			throw new Exception("Outer Class must not be NULL");
		}
		$map_id = $outer->get_id();
		$world_id = $outer->get_world_id();
		if (!isset(self::$INSTANCES[$world_id][$map_id])) {
			self::$INSTANCES[$world_id][$map_id] =
				new MapCompute__ComputedBuildingData($outer);
		}
		else if ($force)
			self::$INSTANCES[$world_id][$map_id]->init_from_db();


		return self::$INSTANCES[$world_id][$map_id];
	}


	private function __construct(MapCompute $outer) {
		$this->compute_id = $outer->get_last_compute_id();
		$this->world_id = $outer->get_world_id();
		$this->map_id = $outer->get_id();
		$this->outer_class = $outer;

		$this->init_from_db();
	}

	public function save($compute_id) {
		$this->compute_id = $compute_id;

		$sql =
			"INSERT INTO computed_buildings ".
			"(world_id, map_id, sector_id, ulc_x, ulc_y, compute_id, ".
				"hitpoints,	killed, died) ".
			"VALUES ";

		/**
		 * Alle Sektoren durchgehen und darin alle Gebäude. Diese Werte dann
		 * speichern.
		 */
		$first_value = true;
		foreach ($this->computed_results as $sector_id => $b_datas) {
			foreach ($b_datas as $bid => $b_data) {
				if ($b_data->amount > 0) {
					if (!$b_data->killed)
						$b_data->killed = 0;
					if (!$b_data->died)
						$b_data->died = 0;

					if ($first_value) {
						$first_value = false;
					}
					else {
						$sql .= ',';
					}
					$ulc_x = floor($bid/1000);
					$ulc_y = $bid % 1000;
					$sql .=
						"({$this->world_id}, ".
						"{$this->map_id}, ".
						"$sector_id, ".
						"$ulc_x, ".
						"$ulc_y, ".
						"{$compute_id}, ".
						"{$b_data->amount},".
						"{$b_data->killed},".
						"{$b_data->died})";
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
		$this->data_to_compute = array();
		$compute_time = $this->outer_class->get_last_compute_time();
		$world_id = $this->world_id;
		$map_id = $this->map_id;
		$this->arrivals = array();
		$this->computed_results = array();
		if ($this->compute_id) {
			/**
			 * Zuerst die bereits berechnenten (also die alten) Gebäude holen.
			 * Gibt es keine $this->compute_id, ist klar, dass es noch keine
			 * Berechnung gab und wir sparen uns diese Query.
			 */
			$res = query (
				'SELECT b.uid, b.sector_id, b.ulc_x, b.ulc_y, '.
					'c.hitpoints AS amount, c.killed, c.died, '.
					'bs.hitpoints AS max_amount '.
				"FROM computed_buildings AS c, ".
					"building AS b, buildings AS bs ".
				'WHERE bs.type=b.type AND (b.type<'.		/** KEINE BÄUME */
						Buildings::NEUTRAL_BUILDINGS_BEGIN.' OR b.type > '.
						Buildings::NEUTRAL_BUILDINGS_END.') '.
					'AND bs.level=b.cur_level AND b.world_id=c.world_id AND '.
					'b.map_id=c.map_id AND b.sector_id=c.sector_id AND '.
					'b.ulc_x=c.ulc_x AND b.ulc_y=c.ulc_y AND '.
					'c.compute_id='.$this->compute_id.' AND '.
					'c.world_id='.$this->world_id.' AND '.
					'c.map_id='.$this->map_id
			);
			while ($row = mysql_fetch_assoc($res)) {
				// bei den alten tun wir so, als wären sie zur Computezeit
				// platziert worden
				$row['time_placed'] = $compute_time;
				$this->insert_db_row_in_data_arrays($row, 1);
			}
		}

		/**
		 * Jetzt kommen die neuen Gebäude. Diese sind nach der
		 * computezeit oder zur computezeit entstanden. An das Entstehen neuer
		 * Gebäude können Ereignisse geknüpft sein, deswegen muss das hier
		 * unterschieden werden.
		 * Diese Gebäude sind nicht zwangsweise fertiggestellt.
		 */
		$res = query (
			'SELECT b.uid, b.sector_id, b.ulc_x, b.ulc_y, '.
				'bs.hitpoints AS amount, bs.hitpoints AS max_amount, '.
				'time_build_started AS time_placed '.
			"FROM buildings AS bs, building AS b ".
			/**
			 * Das LEFT-JOIN prüft in Kombination mit dem IS NULL in der
			 * WHERE-Clause, dass das Gebäude noch nicht in der computed_spalte
			 * vorkommt.
			 * Das scheint robuster zu sein, als die Zeitfenster. Mal schaun.
			 */
			"LEFT JOIN computed_buildings AS c ON c.sector_id=b.sector_id AND ".
				'c.world_id=b.world_id AND c.map_id=b.map_id AND '.
				'c.ulc_x=b.ulc_x AND c.ulc_y=b.ulc_y '.
				'AND c.compute_id='.$this->compute_id.' '.
			'WHERE bs.type=b.type AND bs.level=b.cur_level '.
				'AND (b.type<'.		/** KEINE BÄUME */
						Buildings::NEUTRAL_BUILDINGS_BEGIN.' OR b.type > '.
						Buildings::NEUTRAL_BUILDINGS_END.') '.
				'AND b.world_id='.$world_id.' AND b.map_id='.$map_id.' '.
				'AND b.time_build_started<='.Time::get_t_now().' '.
				'AND c.ulc_x IS NULL'	// <= IS NULL :)
//			'	AND b.time_build_started>='.$compute_time
		);
		while ($row = mysql_fetch_assoc($res)) {
			$row['killed'] = $row['died'] = 0;
			$cycle = $this->outer_class->time_to_cycle(
				$row['time_placed'], $compute_time);
			$this->insert_db_row_in_data_arrays($row, $cycle);
			// Triggers? Momentan noch keine eingebaut
		}
		/**
		 * Und zu guter letzt kommen die Upgrades. Sie sind etwas anders
		 * zu behandeln, da in der DB zu jedem Upgrade nur die Gesamtsumme
		 * der SP steht, nicht aber das, was durch das Upgrade hinzukommt.
		 * Das muss hier noch explizit errechnet werden, um das Dazukommen
		 * von "Einheiten" ermöglichen zu können.
		 * Zusätzlich können an ein Upgrade-Event eigene Triggers gekoppelt
		 * sein.
		 * ACHTUNG: Ein Upgrade von Level 0 auf Level 1 ist eine
		 * Neukonstruktion. Da ein Gebäude Level 0 bereits alle SP besitzt,
		 * wird der Fall der Neukonstruktion nochmal anders behandelt, nämlich
		 * so, dass KEINE SP dazukommen, sondern nur Events getriggert werden
		 * (z.B. Punkte vergabe oder Fertigstellung in der DB markieren)
		 */

		$res = query (
			'SELECT b.uid, b.sector_id, b.ulc_x, b.ulc_y, '.
				'b.finish_time AS time_placed, bs1.hitpoints AS max_amount, '.
				'bs2.hitpoints AS prev_max_amount, b.cur_level AS level, '.
				'b.uid '.
			"FROM building AS b, buildings AS bs1 ".
			'LEFT JOIN buildings AS bs2 ON bs2.level=bs1.level-1 '.
			'	AND bs1.type=bs2.type '.
			'WHERE bs1.type=b.type AND bs1.level=b.cur_level '.
				'AND b.type<'.Buildings::NORMAL_TREE.' '.	/** KEINE BÄUME */
				'AND b.world_id='.$world_id.' '.
				'AND b.map_id='.$map_id.' '.
				'AND b.finish_time<='.(Time::get_t_now()-1).' '.
				'AND b.to_build!=0'
		);
		while ($row = mysql_fetch_assoc($res)) {
			/**
			 * Gebäudeupgrade oder Neukonstruktion fertig, DB updaten,
			 * Punkte vergeben. Durch die WHERE-Clause wird sichergestellt,
			 * dass das UPDATE nur einmal passiert (auch wenn mehrere
			 * Threads gerade dasselbe Berechnen.
			 * Liefert dann mysql_affected_rows() 0 zurück, ist klar, dass
			 * das Update bereits gemacht wurde und somit alle damit
			 * verbundenen Trigger schon ausgeführt wurden. Somit wird
			 * elegant die Notwendigkeit einer Transaktionsengine umgangen
			 * und Geschwindigkeit gewonnen.
			 */
			query (
				'UPDATE building SET cur_level='.($row['level']+1).', '.
					'to_build=0, progress=0, t_stamp=0 '.
				"WHERE world_id=$world_id AND map_id=$map_id AND ".
					" sector_id={$row['sector_id']} ".
					" AND ulc_x={$row['ulc_x']} AND ulc_y={$row['ulc_y']} ".
					" AND to_build!=0"
			);

			if (mysql_affected_rows() == 1) {
				/**
				 * Wenn $row['level'] == 0, dann Neukonstruktion.
				 * Keine SP hinzufügen, aber ggf Triggers
				 */
				if ($row['level'] == 0) {
				}
				/**
				 * Update-Triggers
				 */
				else {
				}
				/**
				 * Nur im Upgradefall neue SP hinzufügen!
				 */
				if ($row['level'] > 0) {
					$row['amount'] = $row['max_amount'] - $row['prev_max_amount'];
					$row['killed'] = $row['died'] = 0;
					$cycle = $this->outer_class->time_to_cycle(
						$row['time_placed'], $compute_time);

					$this->insert_db_row_in_data_arrays($row, $cycle);
				}
			}
		}
	}

	private function insert_db_row_in_data_arrays($row, $cycle) {
		if ($row['amount'] <= 0) {
//			pv_mail("p.kemmeter@gausus.com", "Inserting 0-Building",
//				var_export($row, true)
//			);
		}
		$sector_id = $row['sector_id'];
		$building_id = self::get_bid_by_ulc(
			new Point2D($row['ulc_x'], $row['ulc_y'])
		);

		$b1 = new stdClass();
		$b1->amount = $row['amount'];
		$b1->time_placed = $row['time_placed'];
		$this->data_to_compute[$cycle][$sector_id][$building_id] = $b1;

		$b2 = new stdClass();
		$b2->id = $building_id;
		$b2->amount = $row['amount'];
		$b2->type = self::T_BUILDING;	// Markierung nötig, wegen Joins in MapCompute
		$b2->ulc_x = $row['ulc_x'];
		$b2->ulc_y = $row['ulc_y'];
		$this->appearances[$sector_id][$row['time_placed']][] = $b2;

		$b3 = new stdClass();
		$b3->amount = $row['amount']; // Wird von MapCompute überschrieben
		$b3->max_amount = $row['max_amount'];
		$b3->died = $row['died'];
		$b3->killed = $row['killed'];
		$b3->ulc_x = $row['ulc_x'];
		$b3->ulc_y = $row['ulc_y'];

		$this->computed_results[$sector_id][$building_id] = $b3;

		if (!isset($this->uids_in_sector[$sector_id])
			|| !in_array($row['uid'], $this->uids_in_sector[$sector_id]))
		{
			$this->uids_in_sector[$sector_id][] = $row['uid'];
		}
	}

	/**
	 * Um die Gebäude im MapCompute entsprechend eindeutig ablegen zu können,
	 * braucht man eine eindeutige, eindimensionale ID.
	 * Diese kann aus der ULC heraus berechnet werden.
	 *
	 * @param Point2D $ulc_x
	 */
	public static function get_bid_by_ulc(Point2D $ulc) {
		return $ulc->x * 1000 + $ulc->y;
	}

	/**
	 * Gibt alle berechneten Gebäude im Sektor als Array zurück
	 *
	 * @param [0,1,..99] $sector_id		Die ID des Sektors
	 * @return BuildingOnMap[]
	 */
	public function get_computed_buildings_in_sector($sector_id) {
		$result = array();
		if (isset($this->computed_results[$sector_id])) {
			foreach ($this->computed_results[$sector_id] as $bid => $data) {
				if ($data->amount > 0) {
					$result[] = BuildingOnMap::get_instance(
						new WorldCoordinates(
							$this->world_id, $this->map_id,	$sector_id
						),
						$data->ulc_x,
						$data->ulc_y
					);
				}
			}
		}
		return $result;
	}


	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		query ("DELETE FROM computed_buildings");
	}
}

?>