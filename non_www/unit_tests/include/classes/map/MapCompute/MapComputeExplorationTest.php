<?php
require_once(dirname(__FILE__)
	.'/../../../../extensions/PHPUnitMammunTestCase.inc');
require_once(CONF_GAME_ROOT .
	'/include/classes/map/MapCompute/MapCompute.inc');

class MapComputeExplorationTest extends PHPUnitMammunTestCase {

	private $tribe_id = 0;
	private $tribes_on_map_id = 0;

	public function setUp() {
		$this->install_db_dump('pvgame__phil_andreas_on_map');

		/**
		 * Pick a tribe
		 */
		$tribe = mysql_fetch_row(query(
			"SELECT tmap.id, tmap.tribe_id FROM tribes_on_map AS tmap ".
				"INNER JOIN tribes ON tmap.tribe_id=tribes.id ".
				"WHERE uid=3760 LIMIT 1"
		),0);

		$this->assertArrayHasKey(0, $tribe, "No tribe found in mf.");
		$this->assertArrayHasKey(1, $tribe, "No tribe found on map.");

		$tribes_on_map_id = $tribe[0];
		$tribe_id = $tribe[1];

		$this->assertGreaterThan(0, $tribe_id, "No tribe found in mf.");
		$this->assertGreaterThan(0, $tribes_on_map_id, "No tribe found on map.");

		$this->tribe_id = $tribe_id;
		$this->tribes_on_map_id = $tribes_on_map_id;
	}

	public function tearDown() {
		$this->install_db_dump('pvgame__10users');
		MapCompute::clear_of_all_users();	// Kill caches and data
		TribeOnMap::clear_of_all_users();	// Kill caches and data
	}


	/**
	 * Hilfsfunktion, die checkt, ob der angegebene Sektor im Explore-Zustand
	 * ist.
	 *
	 * @param assoc $sector_data_db
	 * @param MapCompute $mc
	 * @param uint $sector_id
	 * @param uint $t_stamp
	 */
	private function assertIsExploration($sector_data_db, MapCompute $mc,
		$sector_id, $t_stamp)
	{
		// Check for status change
		$this->assertEquals(
			MapCompute__ComputedSectorData::SECTOR_STATUS_EXPLORE,
			$sector_data_db['status'],
			'Wrong sector status in Db'
		);

		// Check for owner still "nobody"
		$this->assertEquals(
			0, $sector_data_db['owner'], 'Wrong owner in Db'
		);

		// Check for t_stamp_start has been set correctly
		$this->assertEquals(
			$t_stamp,
			$sector_data_db['t_stamp_start'],
			'Wrong start tstamp in Db'
		);

		// Now check the same in the MapCompute object
		$this->assertTrue($mc->get_explore_in_sector($sector_id));
		$this->assertFalse($mc->get_fight_in_sector($sector_id));
		$this->assertEquals(0, $mc->get_dominator($sector_id));
	}

	/**
	 * Hilfsfunktion, die checkt, dass der angegebene Sektor NICHT im
	 * Explore-Zustand ist.
	 *
	 * @param assoc $sector_data_db
	 * @param MapCompute $mc
	 * @param uint $sector_id
	 * @param uint $owner
	 */
	private function assertNotExploration($sector_data_db, MapCompute $mc,
		$sector_id, $owner)
	{
		// Check for status stay
		$this->assertEquals(
			MapCompute__ComputedSectorData::SECTOR_STATUS_NONE,
			$sector_data_db['status'],
			'Wrong sector status in Db'
		);

		// Check for owner
		$this->assertEquals(
			$owner, $sector_data_db['owner'], 'Wrong owner in Db'
		);

		// Now check the same in the MapCompute object
		$this->assertFalse($mc->get_explore_in_sector($sector_id));
		$this->assertFalse($mc->get_fight_in_sector($sector_id));
		$this->assertEquals($owner, $mc->get_dominator($sector_id));
	}

	/**
	 * Testet, ob eine Einheit, wenn sie in einen leeren Landfleck geht, eine
	 * Erkundung auslöst.
	 */
	public function test_start_exploration() {
		/**
		 * Move the tribe
		 */
		query(
			'UPDATE tribes_on_map SET '.
				't_movement_started=' . (Time::get_t_now()-10) . ','.
				't_appearance=' . (Time::get_t_now()-1) . ','.
				'moved_by=3760, path="4041" '.
			"WHERE id={$this->tribes_on_map_id}"
		);

		/**
		 * Compute the map
		 */
		$mc = MapCompute::get_instance(7, 0, 0);

		/**
		 * Datenbank prüfen, ob der Status passt usw.
		 */
		$sector_data_db = mysql_fetch_assoc(query(
			'SELECT status, t_stamp_start, owner FROM computed_sector_data '.
			'WHERE world_id=7 AND map_id=0 AND sector_id=41 AND compute_id='
				.'(SELECT MAX(compute_id) FROM computed_maps '.
					'WHERE world_id=7 AND map_id=0)'
		));

		/**
		 * Der nächste Test hängt von der Konfiguration ab. Wenn wir eine
		 * Erkundungsdauer haben, dann testen wir, ob die Erkundung stattfindet,
		 * ansonsten testen wir, dass sie nicht stattfindet. :)
		 */
		if (CIBal::EXPLORATION_TIME > 0) {
			$this->assertIsExploration($sector_data_db, $mc, 41,
				Time::get_t_now()-1);
		}
		else {
			$this->assertNotExploration($sector_data_db, $mc, 41, 3760);
		}

	}

	public function provider_end_exploration() {
		return array(
			array(CIBal::EXPLORATION_TIME-1, false),
			array(CIBal::EXPLORATION_TIME+1, true)
		);
	}

	/**
	 * Testet, ob die Erkundung korrekt beendet wird.
	 *
	 * @param uint $time_offset
	 * @param bool $may_end
	 * @dataProvider provider_end_exploration
	 */
	public function test_end_exploration($time_offset, $may_end) {
		if (CIBal::EXPLORATION_TIME > 0) {
			/**
			 * Move the tribe
			 */
			query(
				'UPDATE tribes_on_map SET '.
					't_movement_started=' .
						(Time::get_t_now()-$time_offset-10) . ','.
					't_appearance=' . (Time::get_t_now()-$time_offset) . ','.
					'moved_by=3760, path="4041" '.
				"WHERE id={$this->tribes_on_map_id}"
			);

			/**
			 * Compute the map
			 */
			$mc = MapCompute::get_instance(7, 0, 0);

			/**
			 * Datenbank prüfen, ob der Status passt usw.
			 */
			$sector_data_db = mysql_fetch_assoc(query(
				'SELECT status, t_stamp_start, owner FROM computed_sector_data '.
				'WHERE world_id=7 AND map_id=0 AND sector_id=41 AND compute_id='
					.'(SELECT MAX(compute_id) FROM computed_maps '.
						'WHERE world_id=7 AND map_id=0)'
			));

			if ($may_end) {
				$this->assertNotExploration($sector_data_db, $mc, 41, 3760);
			}
			else {
				$this->assertIsExploration($sector_data_db, $mc, 41,
					Time::get_t_now()-$time_offset);
			}

		}
		else {
			$this->markTestSkipped(
				"CIBal::EXPLORATION_TIME == 0. No exploration end test can ".
					"be done."
			);
		}
	}
}
?>