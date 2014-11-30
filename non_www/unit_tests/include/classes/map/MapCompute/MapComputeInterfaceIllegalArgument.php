<?php
require_once(dirname(__FILE__)
	.'/../../../../extensions/PHPUnitMammunTestCase.inc');
require_once(CONF_GAME_ROOT .
	'/include/classes/map/MapCompute/MapCompute.inc');

class MapComputeInterfaceIllegalException extends PHPUnitMammunTestCase {

	private static $mc;

	public static function setUpBeforeClass() {
		self::install_db_dump('pvgame__phil_andreas_on_map');
		self::$mc = MapCompute::get_instance(7, 0, 0);
	}

	public static function tearDownAfterClass() {
		self::install_db_dump('pvgame__10users');
	}

	public function provider_get_moving_tribes_excetions() {
		return array(
			array(-1, Direction::ALL),
			array(false, Direction::MIN-1),
			array(false, Direction::MAX+1),
			array(100, Direction::ALL),
			array('muh', Direction::ALL),
			array(10, 'muh')
		);
	}

	/**
	 * @dataProvider provider_get_moving_tribes_excetions
	 * @expectedException IllegalArgumentException
	 */
	public function test_get_moving_tribes_excetions($sector_id, $direction)
	{
		self::$mc->get_moving_units($sector_id, $direction);
	}

	public function provider_get_tribes_in_sector() {
		return array(
			array(-1, 3760),
			array(40, 0),
			array(40, -1),
			array(100, 3760),
			array('muh', 3760),
			array(40, 'muh')
		);
	}

	/**
	 * @dataProvider provider_get_tribes_in_sector
	 * @expectedException IllegalArgumentException
	 */
	public function test_get_tribes_in_sector($sector_id, $uid)
	{
		self::$mc->get_tribes_in_sector($sector_id, $uid);
	}
}

?>