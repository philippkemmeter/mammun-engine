<?php
require_once(dirname(__FILE__)
	.'/../../../../extensions/PHPUnitMammunTestCase.inc');
require_once(CONF_GAME_ROOT .
	'/include/classes/map/MapCompute/MapCompute.inc');

class MapComputeMovingEndDetectionTest extends PHPUnitMammunTestCase {
	private $tribe_id = 0;
	private $tribes_on_map_id = 0;

	public function setUp() {
		$this->install_db_dump('pvgame__phil_andreas_on_map');

		/**
		 * Pic a tribe
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
	}

	public function provider_arrived_tribes_detection() {
		return array(
			array(Time::get_t_now()-100),
			array(Time::get_t_now()-1),
		);
	}

	/**
	 * @dataProvider provider_arrived_tribes_detection
	 */
	public function test_arrived_tribes_detection($t_appearance) {
		/**
		 * Move the tribe
		 */
		query(
			'UPDATE tribes_on_map SET '.
				't_movement_started=' . ($t_appearance-100) . ','.
				"t_appearance=$t_appearance, ".
				'amount=300, '.
				'moved_by=3760, path="4041" '.
			"WHERE id={$this->tribes_on_map_id}"
		);

		/**
		 * Compute the map
		 */
		$this->mc = MapCompute::get_instance(7, 0, 0);
		$moving = $this->mc->get_moving_units(41);

		/**
		 * Check, that there is no tribe moving.
		 */
		$this->assertEmpty($moving, 'Tribe is detected as moving!');

		/**
		 * Check, if tribe has been arrived.
		 */
		$tribes = $this->mc->get_tribes_in_sector(41, 3760);

		$this->assertEquals(1, count($tribes), 'Tribe has not been found.');
		$this->assertEquals(3760, $tribes[0]->get_uid());
		$this->assertEquals($this->tribes_on_map_id, $tribes[0]->get_id());
		// Durch Heilung ist der Wert mehr als die 300, die wir oben festgelegt
		// haben, aber nur wenig mehr.
		$this->assertLessThanOrEqual(301, $tribes[0]->get_amount());
		$this->assertGreaterThan(300, $tribes[0]->get_amount());

		/**
		 * Check, that the tribe's state "movement" has been reset
		 */
		$t_movement_started = @mysql_result(query(
			"SELECT t_movement_started FROM tribes_on_map ".
			"WHERE id={$this->tribes_on_map_id}"
		),0);

		$this->assertEquals(0, $t_movement_started);
	}


}
?>