<?php
require_once (dirname(__FILE__).'/../../shared/classes/IMedals.inc');
/**
 * Diese Klasse verwaltet alle Medallientypen, die es gibt.
 *
 * Dabei sind alle Medallien wie folgt strukturiert:
 *
 * Zunächst gibt es einen Typ, der aussagt, für welche Grundsätzliche Aktion
 * der Orden verliehen wird (z.B. für das Bauen). Der Untertyp gibt dann an,
 * was genau dort gemacht werden soll (z.B. für das Bauen eines Unterstands).
 * Der Wert Amount gibt an, wie oft die durch Typ und Untertyp spezifizierte
 * Aktion durchgeführt werden muss, um den Orden zu bekommen.
 *
 * Der Untertyp wird durch die Klassen, die die durch Typ spezifizierten
 * Aktionen definiert.
 *
 * Beispielmedallien:
 * Typ: Medals::TYPE_SCIENCE_RESEARCHED, Untertyp: SC_TYPE_ABILITY, amount=1
 * ** Sobald man die erste Fähigkeit erforscht hat, gibt es eine Medallie.
 *
 * Typ: Medals::TYPE_UNITS_TRAINED, Untertyp: SC_ID_JAVELIN, amount=20
 * ** Nachdem der 20. Speerwerfer trainiert wurde, gibt es diese Medallie
 *
 * Der Untertyp ist optional:
 *
 * Typ: Medals::TYPE_SCIENCE_RESEARCHED, Untertyp: 0, amount=100
 * ** Bei 100 Forschungen gleich welcher Art bekommt man diese Medallie
 *
 *
 *
 * @author Philipp Kemmeter
 */

class Medals implements IMedals {	/** Statisch */
	private function __construct(){}

	/**
	 * Gibt die Medallien zurück, die dem entsprechenden Typ und Untertyp
	 * entsprechen.
	 *
	 * Sowohl Typ als auch Untertyp müssen nicht angegeben werden, wenn dieser
	 * Filter nicht gewünscht ist.
	 *
	 * @param uint $type
	 * @param uint $subtype
	 * @return uint[][][]
	 */
	public static function get_medals($type=0, $subtype=0) {
		$sql = "SELECT id, type, subtype, amount FROM medals ";
		if ($type > 0) {
			$type = (int)$type;
			$sql .= "WHERE type=$type";
			if ($subtype > 0) {
				$subtype = (int)$subtype;
				$sql .= " AND subtype=$subtype";
			}
		}
		$res = query($sql);
		$result = array();
		while ($row = mysql_fetch_object($res)) {
			$result[$row->type][$row->subtype][$row->amount] = $row->id;
		}
		return $result;
	}
}
?>