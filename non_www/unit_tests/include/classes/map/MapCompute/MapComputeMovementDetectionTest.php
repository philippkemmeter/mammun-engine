<?php
require_once(dirname(__FILE__)
	.'/../../../../extensions/PHPUnitMammunTestCase.inc');
require_once(CONF_GAME_ROOT .
	'/include/classes/map/MapCompute/MapCompute.inc');

class MapComputeMovementDetectionTest extends PHPUnitMammunTestCase {
	private $tribe_id = 0;
	private $tribes_on_map_id = 0;
	private $mc = null;

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


		/**
		 * Move the tribe
		 */
		query(
			'UPDATE tribes_on_map SET '.
				't_movement_started=' . (Time::get_t_now()-10) . ','.
				't_appearance=' . (Time::get_t_now()+100) . ','.
				'moved_by=3760, path="4041" '.
			"WHERE id={$tribes_on_map_id}"
		);

		/**
		 * Compute the map
		 */
		$this->mc = MapCompute::get_instance(7, 0, 0);
		$this->tribe_id = $tribe_id;
		$this->tribes_on_map_id = $tribes_on_map_id;
	}

	public function tearDown() {
		$this->install_db_dump('pvgame__10users');
	}

	public function provider_moving_units_detection() {
		return array(
			array(false, Direction::ALL, true),
			array(40, Direction::ALL, true),
			array(40, Direction::RIGHT, true),
			array(false, Direction::RIGHT, true),
			array(40, Direction::RIGHT | Direction::LEFT, true),
			array(false, Direction::RIGHT | Direction::LEFT, true),
			array(false, Direction::RIGHT | Direction::UP, true),
			array(false, Direction::RIGHT | Direction::DOWN, true),
			array(false, Direction::RIGHT | Direction::TELEPORT, true),

			array(false, Direction::DOWN, false),
			array(40, Direction::UP, false),
			array(41, Direction::RIGHT, false),
			array(0, Direction::RIGHT, false)
		);
	}

	/**
	 * @dataProvider provider_moving_units_detection
	 */
	public function test_moving_units_detection_pos($sector_id, $direction,
		$shall_detect_tribe)
	{
		$moving = $this->mc->get_moving_units($sector_id, $direction);
		/**
		 * Check, that there is only one tribe Moving and that this tribe is
		 * ours.
		 */
		if ($shall_detect_tribe) {
			$this->assertEquals(1, count($moving));
			$this->assertEquals(3760, $moving[0]->get_uid());
			$this->assertEquals($this->tribes_on_map_id, $moving[0]->get_id());
		}
		else {
			$this->assertEmpty($moving);
		}
	}

}
?>