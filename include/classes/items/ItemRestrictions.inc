<?php
/**
 * Diese Klasse behandelt die Beschränkungen für die Verfügbarkeit von Items
 * einzelner User.
 */

class ItemRestrictions {
	private $uid;

	private static $INSTANCES;


	/**
	 * Jede DB-Query wird hier gecachet, da die Daten in der DB statisch sind.
	 * @var assoc
	 */
	private static $request_cache;

	/**
	 * Die benutzerspezifischen Konsequenzen der einzelnen Items wird hier
	 * gecacht.
	 *
	 * Das Objekt ist wie folgt aufgebaut:
	 * <code>
	 * $user_cache[$type][$id]->may_buy_from_shop 	ob der Spieler das Item
	 * 													im Shop kaufen darf
	 * </code>
	 * @var assoc
	 */
	private $user_cache;

	/**
	 * Gibt die ItemRestriction-Instanz des angegebenen Users zurück.
	 *
	 * @param uint $uid
	 * @return ItemRestriction
	 */
	public static function get_instance($uid) {
		if (!isset(self::$INSTANCES[$uid]))
			self::$INSTANCES[$uid] = new self($uid);
		return self::$INSTANCES[$uid];
	}

	/**
	 * Erzeugt eine neue ItemRestriction-Instanz für den angegebenen User.
	 *
	 * @return ItemRestrictions
	 */
	private function __construct($uid) {
		$this->uid = $uid;
	}

	/**
	 * Liest die Daten aus der DB aus und erstellt den entsprechenden Cache-
	 * Eintrag für das angegebene Item für den User der Instanz.
	 *
	 * @param uint $type
	 * @param uint $id
	 */
	private function create_cache($type, $id=0) {
		if (!isset($this->user_cache[$type])) {
			if (!self::$request_cache[$type]) {
				$res = query(
					"SELECT brezeln, usr_level, id FROM item_restrictions ".
					"WHERE type=$type"	// Wir holen gleich alle vom Typ
				);
				if (mysql_num_rows($res) > 0) {
					while ($row = mysql_fetch_object($res)) {
						$row_id = $row->id;
						unset($row->id);
						self::$request_cache[$type][$row_id] = $row;
					}
				}
				else {
					// Markierung, dass für den Typ wirklich nix existiert.
					self::$request_cache[$type][0] = true;
				}
			}
			$user = User::get_instance_by_id($this->uid);
			foreach (self::$request_cache[$type] as $id => $row) {
				if ($id <= 0)
					continue;
				/**
				 * Es gibt Einschränkungen, schauen, welche dieser User erfüllt
				 */
				// Alle vom Typ aufbauen
				$this->user_cache[$type][$id]->may_buy_from_shop =
								(($user->get_brezeln() >= $row->brezeln)
								&& ($user->get_cur_level() >= $row->usr_level));
			}
		}

		if (($id > 0) && !isset($this->user_cache[$type][$id])) {
			/**
			 * Es gibt keine Einschränkungen, jeder darf alles, also alles TRUE
			 */
			$this->user_cache[$type][$id]->may_buy_from_shop = true;
		}
		if (!isset($this->user_cache[$type]))
			$this->user_cache[$type] = array();
	}

	/**
	 * Gibt zurück, ob der User das angegebene Item vom Shop kaufen darf.
	 *
	 * @param uint $type
	 * @param uint $id
	 * @return bool
	 */
	public function may_buy_item_from_shop($type, $id) {
		if (!isset($this->user_cache[$type][$id]))
			$this->create_cache($type, $id);

		return $this->user_cache[$type][$id]->may_buy_from_shop;
	}

	/**
	 * Gibt alle Restriktionen zurück.
	 *
	 * Wird <code>$id</code> weggelassen oder <code>0</code> gesetzt, werden
	 * alle Restriktionen aller Items des Typs zurückgegeben.
	 *
	 * @param uint $type
	 * @param uint $id
	 * @return assoc/stdClass
	 */
	public function get_restrictions($type, $id=0) {
		if ($id > 0) {
			if (!isset($this->user_cache[$type][$id]))
				$this->create_cache($type, $id);
			$result = new stdClass();
			foreach ($this->user_cache[$type][$id] as $key => $value)
				$result->$key = $value;
			foreach (self::$request_cache[$type][$id] as $key => $value)
				$result->$key = $value;
		}
		else {
			if (!isset($this->user_cache[$type]))
				$this->create_cache($type);
			$result = null;
			foreach ($this->user_cache[$type] as $id => $data) {
				if (!is_array($data) && !is_object($data))
					continue;
				$result[$id] = new stdClass();
				foreach ($data as $key => $value) {
					$result[$id]->$key = $value;
				}
			}
			foreach (self::$request_cache[$type] as $id => $data) {
				if (!is_array($data) && !is_object($data))
					continue;
				if (!isset($result[$id]))
					$result[$id] = new stdClass();
				foreach ($data as $key => $value)
					$result[$id]->$key = $value;
			}
		}

		return $result;
	}
}
?>