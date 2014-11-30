<?php
require_once(dirname(__FILE__).'/../../extensions/PHPUnitMammunTestCase.inc');
require_once(CONF_GAME_ROOT .
	'/include/classes/Tribes.inc');

class TribesTest extends PHPUnitMammunTestCase {

	const UID_TO_TEST = 3760;

	public function test_morph_slot_limit() {
		/**
		 * Initialisierung: Tribes in die DB schreiben, die er morphen soll
		 * und danach die IDs auslesen.
		 */
		query('LOCK TABLES tribes WRITE');
		query(
			'DELETE FROM tribes WHERE uid='.self::UID_TO_TEST.' AND race!='
				. SC_ID_SCIENTISTS . ' AND race!=' . SC_ID_CLUB
		);
		query(
			'INSERT INTO tribes (race, name, uid, cost, att, def) VALUES '.
				'('.SC_ID_JAVELIN.',' . "'',".self::UID_TO_TEST.',0,1,1),'.
				'('.SC_ID_ADV_JAVELIN.',' . "'',".self::UID_TO_TEST.',0,1,1),'.
				'('.SC_ID_BOW.',' . "'',".self::UID_TO_TEST.',0,1,1),'.
				'('.SC_ID_ADV_BOW.',' . "'',".self::UID_TO_TEST.',0,1,1),'.
				'('.SC_ID_BLADE.',' . "'',".self::UID_TO_TEST.',0,1,1),'.
				'('.SC_ID_AXE_BEATEN.',' . "'',".self::UID_TO_TEST.',0,1,1)'
		);
		$res = query(
			'SELECT id FROM tribes WHERE race IN ('
				. SC_ID_JAVELIN . ',' . SC_ID_ADV_JAVELIN . ','
				. SC_ID_BOW . ',' . SC_ID_ADV_BOW . ','
				. SC_ID_BLADE . ',' . SC_ID_AXE_BEATEN . ') ' .
				'AND uid='.self::UID_TO_TEST.' AND cost=0'
		);
		query('UNLOCK TABLES');

		$this->assertEquals(6, mysql_num_rows($res));
		$error = 0;
		$ids = array();


		/**
		 * Der Name ist der Tribe-Name.
		 */
		$name = uniqid(false, true);

		/**
		 * Jetzt kommt der eigentliche Test. Alle Fehler verzögern wir,
		 * damit wir noch hinter uns aufräumen können.
		 */
		for ($i = 0; $row = mysql_fetch_row($res); $i++) {
			$id = $row[0];
			$ids[] = $id;	// Alle IDs mitschreiben, damit man nachher löschen
							// kann

			if (!$error) {
				$tribes = Tribes::get_instance(self::UID_TO_TEST);
				$tribe = $tribes->get_tribe_by_id($id);
				try {
					$tribe->morph(1000, $name, new UnitClothes(0));
					if ($i >= Tribes::TRIBE_MORPH_SIMUL_MAX)
						$error = 1;	// Wird später zu fail
				}
				catch (Exception $e) {
					if ($i < Tribes::TRIBE_MORPH_SIMUL_MAX)
						$error = $e; // Wird später geworfen
				}
			}
		}

		/**
		 * Clean-Up.
		 * Hierfür muss man die Tribesklasse durchforsten und schauen, was die
		 * Schritte alles in der Db hinterlassen haben.
		 * Es wird nun entsprechend alles gelöscht.
		 */
		query(
			'DELETE FROM tribes WHERE id IN (' . implode(',', $ids) . ')'
		);
		query(
			'DELETE FROM tribes_on_map WHERE tribe_id IN ('
				. implode(',', $ids) . ')'
		);

		/**
		 * Jetzt werden die Fehler geworfen, die vorher aufgetreten sind. Db
		 * sollte als clean bleiben.
		 */

		if ($error instanceof Exception)
			throw $error;

		if ($error) {
			$this->fail('Zu viele Tribes gleichzeitig!');
		}

	}

}
?>