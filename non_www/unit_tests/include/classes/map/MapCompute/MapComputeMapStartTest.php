<?php
require_once(dirname(__FILE__)
	.'/../../../../extensions/PHPUnitMammunTestCase.inc');
require_once(CONF_GAME_ROOT .
	'/include/classes/map/MapCompute/MapCompute.inc');

/**
 * Testet, ob der Zustand einer neuen Karte stabil ist nach der ersten
 * Berechnung.
 *
 * @author Philipp Kemmeter
 */
class MapComputeMapStartTest extends PHPUnitMammunTestCase {
	public function setUp() {
		$this->install_db_dump('pvgame__phil_andreas_on_map');

		$time = Time::get_t_now()-1;
		/**
		 * Time-Stamps erstmal alle auf jetzt, damit die alle gerade erst
		 * angekommen sind.
		 */
		query(
			"UPDATE tribes_on_map SET t_appearance=$time"
		);
		query(
			"UPDATE maps SET inserted=$time, started=$time"
		);
		query(
			"UPDATE building SET finish_time=$time, time_built=$time, ".
				"time_build_started=$time"
		);

		/**
		 * Anzahl auf 800, weil das ist unter 1000 (max) und man kann schauen,
		 * ob die nicht spontan beim Start schon auf 1000 gesetzt werden durch
		 * ultraschnelle Heilung.
		 */
		query(
			"UPDATE tribes_on_map SET amount=800"
		);
	}

	public function tearDown() {
		$this->install_db_dump('pvgame__10users');
		MapCompute::clear_of_all_users();	// Kill caches and data
	}

	/**
	 * Testet, ob alle Einheiten von der Engine gefunden werden, obwohl sie
	 * gerade erst platziert wurden.
	 */
	public function test_all_units_present() {
		$res = query(
			"SELECT id FROM tribes_on_map"
		);

		while ($row = mysql_fetch_row($res)) {
			$this->assertGreaterThanOrEqual(
				800, TribeOnMap::get_instance($row[0])->get_amount()
			);
			$this->assertLessThanOrEqual(
				802, TribeOnMap::get_instance($row[0])->get_amount()
			);
		}
	}

	/**
	 * Testet, ob alle Gebäude von der Engine gefunden werden, obwohl sie
	 * gerade erst platziert wurden.
	 */
	public function test_all_buildings_present() {
		$res = query(
			"SELECT sector_id, ulc_x AS x, ulc_y AS y FROM building"
		);

		while ($row = mysql_fetch_assoc($res)) {
			$this->assertNotNull(BuildingOnMap::get_instance(
				new WorldCoordinates(7, 0, $row['sector_id']),
				$row['x'], $row['y']
			));
		}
	}

	/**
	 * Testet, ob alle Sektoren, in denen eine Einheit steht, dem
	 * entsprechenden Besitzer zugeordnet wurden, obwohl die Karte gerade
	 * erst gestartet ist.
	 */
	public function test_dominators_tribes() {
		$res = query(
			"SELECT t.uid, tmap.path FROM tribes_on_map AS tmap, tribes AS t ".
			"WHERE t.id=tmap.tribe_id"
		);

		while ($row = mysql_fetch_assoc($res)) {
			$this->assertEquals(
				$row['uid'],
				MapCompute::get_instance(7, 0, 0)->get_dominator($row['path']),
				"Dominator test failed at sector {$row['path']}."
			);
		}
	}

}

?>