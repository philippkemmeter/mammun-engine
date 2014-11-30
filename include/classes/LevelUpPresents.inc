<?
require_once(dirname(__FILE__)
	.'/../shared/classes/datastructures/IIndividualization.inc');
	require_once(dirname(__FILE__)
	.'/../shared/classes/ILevelUpPresents.inc');

/**
 * Wenn ein Spieler ein LevelUp erfährt, dann vergibt diese Klasse das
 * entsprechende Geschenk.
 *
 * @author Philipp Kemmeter
 *
 */
class LevelUpPresents implements ILevelUpPresents {
	private static $INSTANCES;
	private $uid;

	public static function get_instance($uid) {
		if (!isset(self::$INSTANCES[$uid]))
			self::$INSTANCES[$uid] = new self($uid);
		return self::$INSTANCES[$uid];
	}

	private function __construct($uid) {
		$this->uid = $uid;
	}

	/**
	 * Vergibt dem Spieler der Instanz das angegebene Geschenk.
	 *
	 * @param uint $level
	 */
	public function give_present($level) {
		$res = query(
			"SELECT type, id, blueprint FROM level_up_presents WHERE level=$level"
		);
		if (mysql_num_rows($res)) {
			list($type, $id, $blueprint) = mysql_fetch_row($res);

			if (!$blueprint) {
				/**
				 * Item geben
				 */
				Server::get_instance(
					CONF_PAYMENT_SERVER_URL, CONF_MAMMUN_AUTH_SECRET
				)->put_items_into_chest(
					$this->uid,	array($type => $id)
				);
			}
			else {
				/**
				 * Nichts geben ;-)
				 */
			}
			/**
			 * Nutzer informieren
			 *
			 * (DEAKTIVIERT.
			 * Der User bekommt das Level-UP immer live mit, weil er
			 * klicken muss, um das Level-UP zu erhalten, also braucht man
			 * hier nix vermerken)
			 */
			EventMessages::get_instance($this->uid)->add_entry(
				IEventMessages::TYPE_LEVELUP_PRESENT_GAINED,
				($blueprint) ? -$type : $type,
				$id
			);
		}
	}

	/**
	 * Gibt zurück, welches Geschenk der Spieler beim angegebenen Level
	 * erhalten würde.
	 *
	 * Der Rückgabewert ist ein Standart-Objekt mit folgenden Werten:
	 * <code>
	 * $result->type		Individualisierungs-Typ
	 * $result->id			Individualisierungs-ID
	 * $result->blueprint	Ob nur Blueprint oder tatsächlich das Gebäude
	 * </code>
	 *
	 * @param uint $level
	 * @return stdClass
	 */
	public static function get_present($level) {
		$result = new stdClass();
		$res = query(
			"SELECT type, id, blueprint FROM level_up_presents ".
				"WHERE level=$level"
		);
		if (mysql_num_rows($res)) {
			list($type, $id, $blueprint) = mysql_fetch_row($res);
			$result->type = $type;
			$result->id = $id;
			$result->blueprint = $blueprint;
		}
		return $result;
	}
}
?>