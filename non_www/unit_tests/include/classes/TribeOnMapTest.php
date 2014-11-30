<?php
require_once(dirname(__FILE__)
	.'/../../extensions/PHPUnitMammunTestCase.inc');
require_once(CONF_GAME_ROOT . '/include/classes/TribeOnMap.inc');

/**
 * Testet die Funktionen von TribeOnMap
 *
 * @author Philipp Kemmeter
 */
class TribeOnMapTest extends PHPUnitMammunTestCase {
	private $tribes_on_map_id = 0;

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
	}

	public function tearDown() {
		$this->install_db_dump('pvgame__10users');
		MapCompute::clear_of_all_users();	// Kill caches and data
		TribeOnMap::clear_of_all_users();	// Kill caches and data
	}

	/**
	 * Testet, ob die Methode get_amount den richtigen Wert zurückgibt, auch
	 * wenn noch kein Compute gewesen ist (frisch gesiedelt).
	 */
	public function test_get_amount_no_compute() {
		$my_tribe = TribeOnMap::get_instance($this->tribes_on_map_id);
		$this->assertEquals(1000, $my_tribe->get_amount());
	}

	/**
	 * Testet, ob die Methode get_amount den richtigen Wert zurückgibt, nachdem
	 * ein Compute stattfand.
	 */
	public function test_get_amount_after_compute() {
		$my_tribe = TribeOnMap::get_instance($this->tribes_on_map_id);
		MapCompute::get_instance(7, 0, 0);	// compute
		$this->assertEquals(1000, $my_tribe->get_amount());
	}

}