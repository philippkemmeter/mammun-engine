<?php
require_once(dirname(__FILE__).'/../shared/classes/IEventLog.inc');

/**
 * Diese Klasse ist verantwortlich für einen Log, der pro Spieler pro Karte die
 * Ereignisse auf der Karte protokolliert (Kämpfe gestartet / beendet,
 * Forschungen beendet usw).
 *
 * @author Philipp Kemmeter
 */
class EventLog implements IEventLog {
	private $uid;
	private $map_id;
	private $world_id;
	private $log = array();
	private $log_init_from_db = false;
	private $data_to_insert = null;
	private $log_count = null;

	private static $INSTANCES;
	private static $db;

	/**
	 * @param uint $uid
	 * @param uint $map_id
	 * @return EventLog
	 */
	public static function get_instance($uid, $world_id, $map_id) {
		if ($uid <= 0)
			throw new IllegalArgumentException("uid", "N1", $uid);
		$uid = (int)$uid;
		if ($world_id < 0)
			throw new IllegalArgumentException("world_id", "N0", $world_id);
		$world_id = (int)$world_id;
		if ($map_id < 0)
			throw new IllegalArgumentException("map_id", "N0", $map_id);
		$map_id = (int)$map_id;

		if (!isset(self::$INSTANCES[$uid][$world_id][$map_id]))
			self::$INSTANCES[$uid][$world_id][$map_id]
					= new self($uid, $world_id, $map_id);
		return self::$INSTANCES[$uid][$world_id][$map_id];
	}

	/**
	 * Gibt die aktuelle Datenbank zurück, in welcher die Punkte gespeichert
	 * werden/sind.
	 *
	 * @return Db
	 */
	private static function get_db() {
		if (!self::$db)
			self::$db = new Db(Db::LOGS_HOST, Db::LOGS_NAME,
								Db::LOGS_PW, Db::LOGS_SELECT_DB);
		return self::$db;
	}

	private function __construct($uid, $world_id, $map_id) {
		$this->uid = $uid;
		$this->world_id = $world_id;
		$this->map_id = $map_id;
		// Verzögert!$this->init_from_db();
	}

	private function init_from_db() {
		// Sicherstellen, dass die Forschung aktuell ist.
		// TODO: Das ist ein Workaround!
		Science::get_instance($this->uid)->get_science_projects(true);

		$res = self::get_db()->query(
			"SELECT type, t_stamp, extra, extra_str, num FROM ".
				"game_event_log WHERE uid={$this->uid} ".
				"AND world_id={$this->world_id} ".
				"AND map_id={$this->map_id} AND t_stamp<=".Time::get_t_now()." ".
			"UNION SELECT type, t_stamp, extra, extra_str, num FROM ".
				"game_event_log_no_map WHERE uid={$this->uid} ".
				"AND t_stamp<=".Time::get_t_now()." ".
			"ORDER BY t_stamp DESC"
		);
		if (mysql_num_rows($res) == 0) {
			return;
		}



		while ($row = mysql_fetch_object($res)) {
			$this->log[] = $row;
		}
	}

	public function __destruct() {
		if ($this->data_to_insert) {
			/**
			 * Alle Daten einfügen. Idee der Query: Wenn ein Datum extakt
			 * dasselbe mal auftritt, dann soll es keine neue Zeile in der
			 * Tabelle beanspruchen, sondern es soll ein Counter erhöht werden
			 * "num". So bleibt die Ausgabe übersichtlich, da es sich
			 * wahrscheinlich um einen Fehler handelt. Der Fehler kann aber
			 * so getrackt werden (im Gegensatz zu INSERT IGNORE...).
			 */
			$n = count($this->data_to_insert);
			/**
			 * Je nachdem, ob Kartenbezogene Meldung oder allgemeine Meldung in
			 * andere Tabellen schreiben.
			 */
			if ($this->world_id) {
				$sql =
					"INSERT INTO game_event_log ".
						"(uid, world_id, map_id, type, extra, extra_str, ".
							"t_stamp, num) ".
					"VALUES ";
				$first = true;
				for ($i = 0; $i < $n; $i++) {
					if ($first)
						$first = false;
					else
						$sql .= ',';

					$sql .=
						"({$this->uid}, ".
						"{$this->world_id}, ".
						"{$this->map_id}, ".
						"{$this->data_to_insert[$i]->type}, ".
						"{$this->data_to_insert[$i]->extra}, ".
						"'{$this->data_to_insert[$i]->extra_str}', ".
						"{$this->data_to_insert[$i]->t_stamp}, ".
						"1)";

				}
			}
			else {
				$n = count($this->data_to_insert);
				$sql =
					"INSERT INTO game_event_log_no_map ".
						"(uid, type, extra, extra_str, t_stamp, num) ".
					"VALUES ";
				$first = true;
				for ($i = 0; $i < $n; $i++) {
					if ($first)
						$first = false;
					else
						$sql .= ',';

					$sql .=
						"({$this->uid}, ".
						"{$this->data_to_insert[$i]->type}, ".
						"{$this->data_to_insert[$i]->extra}, ".
						"'{$this->data_to_insert[$i]->extra_str}', ".
						"{$this->data_to_insert[$i]->t_stamp}, ".
						"1)";

				}
			}
			$sql .=
				" ON DUPLICATE KEY UPDATE num=num+1";
			self::get_db()->query($sql);
		}
	}

	/**
	 * Gibt den Event-Log zurück. Kann leicht veraltet sein. Wenn die Aktualität
	 * sichergestellt werden soll, muss $force auf TRUE gesetzt werden.
	 *
	 * @param bool $force [=false]
	 * @return stdClass[]
	 */
	public function get_log($force=false) {
		if (!$this->log_init_from_db || $force)
			$this->init_from_db();
		return $this->log;
	}


	/**
	 * Fügt einen Eintrag in den Log ein. Wird $t_stamp nicht gesetzt, wird
	 * JETZT als Zeitpunkt gewählt. Ansonsten können frei Zeitpunkte (auch
	 * vergangene oder zukünftige) gewählt werden.
	 *
	 * @param uint $type
	 * @param uint $extra
	 * @param string $extra_str
	 * @param uint $t_stamp
	 */
	public function add_entry($type, $extra, $extra_str, $t_stamp=0) {
		if (($type < self::TYPE_GAME_ENGINE_MIN)
			|| ($type > self::TYPE_GAME_ENGINE_MAX))
		{
			throw new IllegalArgumentException("type",
				'['.self::TYPE_GAME_ENGINE_MIN.';'.self::TYPE_GAME_ENGINE_MAX.']',
				$type);
		}
		$type = (int)$type;
		if (!$t_stamp)
			$t_stamp = Time::get_t_now();
		else
			$t_stamp = (int)$t_stamp;

		/**
		 * Einzufügendes Datum erzeugen.
		 */
		$row = new stdClass();
		$row->extra = (int)$extra;
		$row->extra_str = $extra_str;
		$row->t_stamp = $t_stamp;
		$row->type = $type;

		/**
		 * Aktuellen Log aktuallisieren.
		 * Dies kann zu temporär Dopplungen führen, die beim Speichern
		 * aber wieder aufgehoben werden, durch den Primary-Key der DB.
		 */
		$this->log[] = $row;
		$this->log[count($this->log)-1]->num = 1;

		/**
		 * Zähler anpassen
		 */
		@$this->log_count[$row->type][$row->extra][$row->extra_str]++;
		@$this->log_count[$row->type][0][$row->extra_str]++;

		/**
		 * Das Datum vermerken, um es bei Laufzeitende in die DB zu schreiben.
		 */
		$this->data_to_insert[] = $row;
	}


	/**
	 * Diese Funktion gibt mit direktem DB-Zugriff zurück, wie oft das
	 * angegebene Ereignis für den angegebenen User eintrat.
	 *
	 * @param uint $type
	 * @param uint $extra
	 * @param string $extra_str
	 * @return uint
	 */
	public function get_total_event_amount($type, $extra=0,	$extra_str='') {
		ValueChecker::int($type, 'type');
		ValueChecker::int($extra, 'extra');
		ValueChecker::string($extra_str, 'extra_str', true);

		/**
		 * Zuerst holen wir die Daten aus der Datenbank (das könnte man cachen)
		 */
		$sql = '';
		if ($extra <= 0)
			$extra = 0;
		else {
			$sql .= " AND extra=$extra";
		}
		if ($extra_str)
			$sql .= " AND extra_str='$extra_str'";

		$res = self::get_db()->query(
			"SELECT SUM(num) FROM ".
				"game_event_log WHERE uid={$this->uid} ".
				"AND type=$type{$sql} AND t_stamp<=".Time::get_t_now()." ".
			"UNION SELECT SUM(num) FROM ".
				"game_event_log_no_map WHERE uid={$this->uid} ".
				"AND type=$type{$sql} AND t_stamp<=".Time::get_t_now()
		);

		$result = 0;
		while ($row = mysql_fetch_row($res))
			$result += $row[0];

		/**
		 * Jetzt kommt der Offset dazu, der durch das gepufferte Einfügen
		 * entstanden ist
		 */
		if (empty($extra_str) && isset($this->log_count[$type][$extra])
			&& is_array($this->log_count[$type][$extra]))
		{
			foreach ($this->log_count[$type][$extra] as $str_amt)
				$result += $str_amt;
		}
		else if (isset($this->log_count[$type][$extra][$extra_str]))
			$result += $this->log_count[$type][$extra][$extra_str];


		return $result;
	}
}

?>