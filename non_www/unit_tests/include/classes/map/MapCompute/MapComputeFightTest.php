<?php
require_once(dirname(__FILE__)
	.'/../../../../extensions/PHPUnitMammunTestCase.inc');
require_once(CONF_GAME_ROOT .
	'/include/classes/map/MapCompute/MapCompute.inc');

/**
 * Testet die Kampffunktion
 *
 * @author Philipp Kemmeter
 */
class MapComputeFightTest extends PHPUnitMammunTestCase {
	private $tribes_on_map_id = 0;
	private $war_sector_id = 0;
	private $enemy_tribes_on_map_id = 0;
	private $enemy_uid = 0;
	private $my_amount = 1000;
	private $enemy_amount = 600;


	public function setUp() {
		$this->install_db_dump('pvgame__phil_andreas_on_map');

		/**
		 * Pick a tribe
		 */
		$tribe = mysql_fetch_row(query(
			"SELECT tmap.id, tmap.tribe_id, tmap.path ".
				"FROM tribes_on_map AS tmap ".
				"INNER JOIN tribes ON tmap.tribe_id=tribes.id ".
			"WHERE uid=3760 LIMIT 1"
		));

		$this->assertArrayHasKey(0, $tribe, "No tribe found in map.");
		$this->assertArrayHasKey(1, $tribe, "No tribe found on mf.");

		$this->tribes_on_map_id = $tribe[0];

		$this->assertGreaterThan(
			0, $this->tribes_on_map_id, 'No tribe found in map.'
		);

		/**
		 * Sektor, wo gekämpft werden kann
		 * (Da jeder bei dem Dump nur Einheiten in seinem HS hat, können wir
		 * einfach irgendeinen Tribe nehmen, der in einem anderen Sektor steht)
		 */
		$tribe = mysql_fetch_row(query(
			"SELECT tmap.id, t.uid, tmap.path FROM tribes_on_map AS tmap, ".
				"tribes AS t ".
			"WHERE tmap.path!='{$tribe[2]}' AND t.id=tmap.tribe_id LIMIT 1"
		));
		$this->assertArrayHasKey(0, $tribe, "No enemy tribe found in map.");
		$this->assertArrayHasKey(1, $tribe, "No enemy tribe found on mf.");

		$this->enemy_tribes_on_map_id = $tribe[0];
		$this->enemy_uid = $tribe[1];
		$this->war_sector_id = $tribe[2];

		$this->assertGreaterThan(
			0, $this->war_sector_id, 'No hostile tribe found on map'
		);
		$this->assertGreaterThan(
			0, $this->enemy_uid, 'No hostile tribe found in mf'
		);
		$this->assertGreaterThan(
			0, $this->enemy_tribes_on_map_id, 'No hostile tribe found on map'
		);

		/**
		 * Daten in der DB auf die, die wir testen wollen.
		 * Ausgangs-HP seien 1000 vs 600
		 */
		query(
			"UPDATE tribes_on_map SET amount={$this->my_amount} ".
			"WHERE id={$this->tribes_on_map_id}"
		);
		query(
			"UPDATE tribes_on_map SET amount={$this->enemy_amount} " .
			"WHERE id={$this->enemy_tribes_on_map_id}"
		);

		/**
		 * Time-Stamps erstmal alle auf jetzt, damit die alle gerade erst
		 * angekommen sind.
		 */
		query(
			"UPDATE tribes_on_map SET t_appearance=" .
				(Time::get_t_now()-24*3600)
		);
	}

	public function tearDown() {
		$this->install_db_dump('pvgame__10users');
		MapCompute::clear_of_all_users();	// Kill caches and data
		TribeOnMap::clear_of_all_users();	// Kill caches and data
	}

	/**
	 * Assert, ob im Sektor der Kampf-Status korrekt gesetzt ist.
	 *
	 * @param assoc $sector_data_db
	 * @param MapCompute $mc
	 * @param uint $sector_id
	 * @param uint $t_stamp
	 */
	private function assertFighting($sector_data_db, MapCompute $mc,
		$sector_id, $t_stamp)
	{
		// Check for status change
		$this->assertEquals(
			MapCompute__ComputedSectorData::SECTOR_STATUS_FIGHT,
			$sector_data_db['status'],
			'Wrong sector status in Db'
		);

		// Check for owner still the enemy
		$this->assertEquals(
			$this->enemy_uid, $sector_data_db['owner'], 'Wrong owner in Db'
		);

		// Check for t_stamp_start has been set correctly
		$this->assertEquals(
			$t_stamp,
			$sector_data_db['t_stamp_start'],
			'Wrong start tstamp in Db'
		);

		// Now check the same in the MapCompute object
		$this->assertTrue($mc->get_fight_in_sector($sector_id));
		$this->assertFalse($mc->get_explore_in_sector($sector_id));
		$this->assertEquals($this->enemy_uid, $mc->get_dominator($sector_id));
	}

	/**
	 * Wendet einen Assert auf den Tribe amount an.
	 * Beispiel:
	 * <code>
	 * assertTribeAmountX(1000, 123, 'assertEquals');
	 * </code>
	 *
	 * @param float $expected
	 * @param uint $tribe_id
	 * @param string $assert_method
	 */
	private function assertTribeAmountX($expected, $tribe_id, $assert_method) {
		ValueChecker::id($tribe_id, 'tribe_id');
		ValueChecker::float($expected, 'expected');

		/**
		 * Check, ob in der Datenbank der berechnete Wert kleiner ist, als der
		 * Ausgangswert.
		 */
		$amount = mysql_result(query(
			'SELECT amount FROM computed_tribes '.
			'WHERE world_id=7 AND map_id=0 '.
				"AND tribe_id=$tribe_id AND compute_id="
				.'(SELECT MAX(compute_id) FROM computed_maps '.
					'WHERE world_id=7 AND map_id=0)'
		), 0);
		$this->$assert_method($expected, $amount);
		$tribe = TribeOnMap::get_instance($tribe_id);
		$this->$assert_method($expected, $tribe->get_amount());
	}

	/**
	 * Assert, ob ein Tribe weniger ist als angegeben.
	 *
	 * @param float $expected
	 * @param uint $tribe_id
	 */
	private function assertTribeAmountLessThan($expected, $tribe_id) {
		$this->assertTribeAmountX($expected, $tribe_id, 'assertLessThan');
	}

	/**
	 * Assert, ob ein Tribe mehr ist als angegeben.
	 *
	 * @param float $expected
	 * @param uint $tribe_id
	 */
	private function assertTribeAmountGreaterThan($expected, $tribe_id) {
		$this->assertTribeAmountX($expected, $tribe_id, 'assertGreaterThan');
	}

	/**
	 * Assert, ob ein Tribe gleich ist wie angegeben.
	 *
	 * @param float $expected
	 * @param uint $tribe_id
	 */
	private function assertTribeAmountEquals($expected, $tribe_id) {
		$this->assertTribeAmountX($expected, $tribe_id, 'assertEquals');
	}


	/**
	 * Testet, ob eine Einheit, die in einen feindlichen Sektor geht, einen
	 * Kampf auslöst.
	 */
	public function test_fight_starts_correctly() {
		/**
		 * Tribe ab ins Kampfgebiet schicken (vor einer Sekunde angekommen)
		 */
		query(
			'UPDATE tribes_on_map SET '.
				't_movement_started=' . (Time::get_t_now()-10) . ','.
				't_appearance=' . (Time::get_t_now()-1) . ','.
				"moved_by=3760, path='40{$this->war_sector_id}' ".
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
			'WHERE world_id=7 AND map_id=0 '.
				"AND sector_id={$this->war_sector_id} AND compute_id="
				.'(SELECT MAX(compute_id) FROM computed_maps '.
					'WHERE world_id=7 AND map_id=0)'
		));

		$this->assertFighting(
			$sector_data_db, $mc, $this->war_sector_id, Time::get_t_now()-1
		);
	}

	public function provider_computes() {
		$result = array();
		for ($i = 1; $i < 3; $i++) {
			$result[] = array($i);
		}
		return $result;
	}

	/**
	 * Testet, ob ein Tribe auch im Kampf was auf die Mütze bekommt.
	 */
	public function test_tribe_looses_hp_while_fighting() {
		/**
		 * Tribe ab ins Kampfgebiet schicken (vor einer Stunde angekommen)
		 */
		$fighting_time = 3600;
		query(
			'UPDATE tribes_on_map SET '.
				't_movement_started='
					. (Time::get_t_now()-$fighting_time-100) . ','.
				't_appearance=' . (Time::get_t_now()-$fighting_time) . ','.
				"moved_by=3760, path='40{$this->war_sector_id}' ".
			"WHERE id={$this->tribes_on_map_id}"
		);
		/**
		 * Wir nehmen weiterhin an, dass die Jungs jeweils zwischen 50 und 150
		 * HP verlieren.
		 * Da das ganze ein hochdynamisches Modell ist und noch niemand die DGL
		 * gelöst hat, müssen wir das so abschätzen :)
		 */
		$min = 20;
		$max = 100;

		/**
		 * Compute the map
		 */
		$mc = MapCompute::get_instance(7, 0, 0);

		/**
		 * Check, ob die Werte in den erwarteten Grenzen liegen
		 */
		$this->assertTribeAmountLessThan(
			$this->my_amount-$min, $this->tribes_on_map_id
		);
		$this->assertGreaterThan(
			$this->my_amount-$max, $this->tribes_on_map_id
		);

		$this->assertTribeAmountLessThan(
			$this->enemy_amount-$min, $this->enemy_tribes_on_map_id
		);
		$this->assertGreaterThan(
			$this->enemy_amount-$max, $this->enemy_tribes_on_map_id
		);

	}

	/**
	 * Testet, ob nach einer geschätzten langen Zeit der Kampf vorbei ist, und
	 * wie dann die Lage ist.
	 *
	 * @dataProvider provider_computes
	 */
	public function test_fight_ends_correctly($computes) {
		/**
		 * Tribe ab ins Kampfgebiet schicken
		 */
		$fighting_time = 20*3600;
		query(
			'UPDATE tribes_on_map SET '.
				't_movement_started='
					. (Time::get_t_now()-$fighting_time-100) . ','.
				't_appearance=' . (Time::get_t_now()-$fighting_time) . ','.
				"moved_by=3760, path='40{$this->war_sector_id}' ".
			"WHERE id={$this->tribes_on_map_id}"
		);


		for ($i = 0; $i < $computes; $i++) {
			// Bezieht sich auf den Engine-Bug, wo nach dem Kampf alle
			// Einheiten, die überlebt hatten, wieder resettet wurden.
			// Hier wird also geprüft, ob auch korrekt aus der DB gelesen wird.
			MapCompute__ComputedTribesData::clear_cache();
			/**
			 * Compute the map
			 */
			$mc = MapCompute::get_instance(7, 0, 0);
		}


		/**
		 * Check, ob wir Schaden bekommen haben, aber noch leben
		 */
		$this->assertTribeAmountGreaterThan(0, $this->tribes_on_map_id);
		$this->assertTribeAmountLessThan(
			$this->my_amount-100, $this->tribes_on_map_id
		);

		/**
		 * Check, ob der Gegner ausgelöscht wurde
		 */
		$this->assertEquals(
			0,
			mysql_num_rows(query(
				'SELECT amount FROM computed_tribes '.
				'WHERE world_id=7 AND map_id=0 '.
					"AND tribe_id={$this->enemy_tribes_on_map_id} ".
					"AND compute_id="
					.'(SELECT MAX(compute_id) FROM computed_maps '.
						'WHERE world_id=7 AND map_id=0)'
			))
		);
		try {
			$tribe = TribeOnMap::get_instance($this->enemy_tribes_on_map_id);
			$this->fail('Tribe should not exist anymore.');
		}
		catch(Exception $e) {
			// Expected result
		}
	}

}
?>