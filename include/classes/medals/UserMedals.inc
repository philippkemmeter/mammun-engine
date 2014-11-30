<?php
require_once(dirname(__FILE__).'/../../shared/classes/exceptions/'.
	'IllegalArgumentException.inc');
/**
 * Diese Klasse verwaltet die Medallien einzelner User.
 *
 * @author Philipp Kemmeter
 */

class UserMedals {
	const SIMPLE_ARRAY = 0;
	const OBJECT_ARRAY = 1;

	private $uid;

	private static $INSTANCES;

	/**
	 * Gibt den Medallienzugriff eines Users zurück
	 *
	 * @param uint $uid
	 * @return UserMedals
	 */
	public static function get_instance($uid) {
		if ($uid < 0) {
			throw new IllegalArgumentException("uid", "N+", "$uid");
		}
		$uid = (int)$uid;
		if (!isset(self::$INSTANCES[$uid]))
			self::$INSTANCES[$uid] = new self($uid);
		return self::$INSTANCES[$uid];
	}

	/**
	 *
	 * @param uint $uid
	 * @return UserMedals
	 */
	private function __construct($uid) {
		$this->uid = $uid;
	}

	/**
	 * Gibt alle Medallien des Spielers zurück.
	 *
	 * Jede Medallie ist ein Objekt folgender Struktur:
	 *
	 * $o->id		ID der Medallie
	 * $o->t_stamp	Wann die Medallie verliehen wurde
	 *
	 * @param uint $type
	 * @param uint $subtype
	 * @param {0;1} $return_type
	 * @return stdClass[]
	 */
	public function get_medals($type=0, $subtype=0,
		$return_type=self::OBJECT_ARRAY)
	{
		$sql =
			"SELECT u.medal_id AS id, u.t_stamp, m.type, m.subtype, m.amount ".
				"FROM users_medals AS u, medals AS m ".
			"WHERE uid={$this->uid} AND u.medal_id=m.id ";
		if ($type) {
			$type = (int)$type;
			$sql .=	"AND m.type=$type ";
			if ($subtype) {
				$subtype = (int)$subtype;
				$sql .= "AND m.subtype=$subtype ";
			}
		}

		$res = query($sql);

		$result = array();
		while ($row = mysql_fetch_object($res)) {
			if ($return_type == self::OBJECT_ARRAY)
				$result[] = $row;
			else
				$result[] = $row->id;
		}
		return $result;
	}

	/**
	 * Verleiht dem Spieler die angegebene Medallie.
	 *
	 * Der Zeitpunkt kann frei gewählt werden, ist standartmäßig aber auf JETZT
	 * gesetzt.
	 * Hat der Spieler diese Medallie bereits bekommen, passiert nichts.
	 *
	 * @param uint $id
	 * @param uint $t_stamp
	 * @return bool		Ob die Medallie neu hinzugefügt wurde. Sonst hatte der
	 * 					User sie wahrscheinlich schon.
	 */
	public function add_medal($id, $t_stamp=0) {
		$id = (int)$id;
		if (!$t_stamp)
			$t_stamp = Time::get_t_now();
		else
			$t_stamp = (int)$t_stamp;

		query(
			"INSERT IGNORE INTO users_medals ".
			"SET uid={$this->uid}, medal_id=$id, t_stamp=$t_stamp"
		);
		return (bool)mysql_affected_rows();
	}

	/**
	 * !!!!
	 * Löscht alle Medallien aller User.
	 * !!!!
	 */
	public static function clear_of_all_users() {
		query(
			"DELETE FROM users_medals"
		);
	}
}
?>