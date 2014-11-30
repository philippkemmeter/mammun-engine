<?php
/**
 * Diese Klasse ist für statistische Daten und deren Pufferung von Items
 * zuständig.
 */
class ItemStatistics {
	const TIMEOUT=Time::ONE_DAY;

	private static $cache = null;

	private function __construct() {}	// rein statisch

	/**
	 * Berechnet den Seltenheitsfaktor eines Items.
	 *
	 * Dieser Wert basiert auf der Anzahl der Iteminstanzen eines Items (also
	 * wie oft ein genau gleiches Item verfügbar ist) <code>$item_amt</code>
	 * und der Anzahl der User, auf dessen Basis <code>$item_amt</code>
	 * berechnet wurde <code>$user_amt</code>.
	 *
	 * Beispielsweise kann <code>$item_amt</code> die Anzahl aller Roten Hüte
	 * aller aktiven Spieler sein, dann müsste <code>$item_amt</code> die
	 * Anzahl aller aktiven Spieler sein.
	 *
	 * Der sich ergebene Seltenheitsfaktor basiert auf dem Verhältnis der
	 * beiden Werte. Es wird aber üblicherweise ein Wert zwischen <code>0</code>
	 * und <code>10</code> zurückgegeben, wobei ein größerer Wert eine höhere
	 * Seltenheit darstellt.
	 *
	 * <code>0</code> bedeutet: So viele Items wie User oder mehr.
	 * Die restlichen Werte werden auf Basis des 10er Logarithmus des Nenners
	 * eines Bruches, der das Verhältnis als 1:X darstellt errechnet.
	 *
	 * @param uint $item_amt
	 * @param uint $user_amt
	 * @return ufloat
	 */
	private static function calculate_rareness_factor($item_amt, $user_amt) {
		/**
		 * Es sind mehr Items als User drin => Überhaupt nicht selten => 0
		 */
		if ($item_amt > $user_amt)
			return 0;
		if ($item_amt <= 0)
			return 0;

		return round(log10($user_amt / $item_amt)*10);
	}

	/**
	 * Erzeugt den persistenten lokalen DB-Cache, der alle Statistikdaten aller
	 * Items hält.
	 */
	private static function create_db_cache() {
		/**
		 * Die Daten sind veraltet, wir holen komplett alle neu vom
		 * Payment-Server
		 */
		$active_ids = User::get_user_ids_active();
		$new_data = Server::get_instance(CONF_PAYMENT_SERVER_URL,
			CONF_MAMMUN_AUTH_SECRET)
				->statistics_get_all_user_items_amount($active_ids);
		$sql =
			"INSERT INTO item_statistics_cache (type, id, rareness_factor) ".
			"VALUES ";
		$first = true;
		$active_user_amt = count($active_ids);
		foreach ($new_data as $type => $data) {
			foreach ($data as $id => $amount) {
				if ($first) {
					$first = false;
				}
				else {
					$sql .= ',';
				}

				$factor = self::calculate_rareness_factor(
					$amount, $active_user_amt);

				$sql .=
					"($type, $id, $factor)";
			}
		}
		if (!$first) {
			query("DELETE FROM item_statistics_cache");
			query($sql);
			query(
				"INSERT INTO item_statistics_cache_1 SET cache_id=0, ".
					"last_update=".Time::get_t_now(). " ".
				"ON DUPLICATE KEY UPDATE last_update=".Time::get_t_now()
			);
		}
	}

	/**
	 * Erzeugt den Cache im PHP-Speicher, der die Statistikdaten kürzlich
	 * erfragter Items hält.
	 *
	 * @param uint $type
	 * @param uint $id
	 * @param bool $force	Alle Caches neu aufbauen.
	 */
	private static function create_my_cache($type, $id=0, $force=false) {
		if (!isset(self::$cache[$type])) {
			/**
			 * Zuerst befragen wir unsere lokale Cache-Tabelle.
			 * Dabei ist die erste Frage, ob sie veraltet ist, wenn ja, müssen
			 * wir neue Daten vom Payment-Server anfordern und sie neu aufbauen.
			 *
			 * Danach holen wir gleich alle Einträge eines Typs aus unsere (ggf.
			 * neu aufgebauten) Cache-Tabelle, um gleich den gesamten Block in
			 * unseren Cache im PHP-Speicher (self::$cache) zu schreiben.
			 */
			// Check 1: Daten veraltet?
			query(	// Wir müssen hier Locks beachten, weil ggf. der DB-Cache
					// im Moment neu aufgebaut wird.
				"LOCK TABLES item_statistics_cache_1 WRITE, ".
					"item_statistics_cache WRITE, users READ"
			);
			if ($force) {
				self::create_db_cache();
			}
			else {
				$last_update = (int)@mysql_result(
					query(
						"SELECT last_update FROM item_statistics_cache_1 ".
						"WHERE cache_id=0"
					),0
				);
				if ((Time::get_t_now() - $last_update) >= Time::ONE_DAY) {
					self::create_db_cache();
				}
			}
			query("UNLOCK TABLES");

			$res = query(
				"SELECT rareness_factor, id ".
					"FROM item_statistics_cache ".
				"WHERE type=$type"	// Wir holen gleich alle vom Typ
			);
			if (mysql_num_rows($res) > 0) {
				while ($row = mysql_fetch_object($res)) {
					$row_id = $row->id;
					unset($row->id);
					self::$cache[$type][$row_id] = $row;
				}
			}
		}
		/**
		 * Das Item existiert nicht, oder im Cache sind (noch) keine Daten.
		 * Wir markieren das Feld deswegen mit NULL, der anfragende kann dann
		 * entscheiden, ob er den Cache neu generieren lassen will, oder sich
		 * mit der Info zufrieden gibt, dass es (noch) keine Daten gibt.
		 */
		if (($id > 0) && !isset(self::$cache[$type][$id])) {
			self::$cache[$type][$id] = null;
		}
		if (!isset(self::$cache[$type]))
			self::$cache[$type] = array();
	}

	/**
	 * Gibt die Statistikdaten des angegebenen Items zurück.
	 *
	 * Wird <code>$id</code> weggelassen, werden alle des Typs zurückgegeben.
	 *
	 * Der Rückgabewert hat dabei folgende Struktur:
	 * <code>
	 * $o->rareness_factor		Seltenheitsfaktor zwischen 0 (häufig) und 10
	 * 							(unfassbar selten)
	 * </code>
	 *
	 * @param uint $type
	 * @param uint $id
	 */
	public static function get_statistic_data($type, $id=0) {
		if ($id > 0) {
			if (!isset(self::$cache[$type][$id]))
				self::create_my_cache($type, $id);
			return self::$cache[$type][$id];
		}
		else {
			if (!isset(self::$cache[$type]))
				self::create_my_cache($type);
			$result = null;
			foreach (self::$cache[$type] as $id => $foo)
				$result[$id] = self::get_statistic_data($type, $id);
			return $result;
		}
	}
}
?>