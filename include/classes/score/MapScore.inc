<?php
require_once(dirname(__FILE__).'/IScore.inc');
require_once(dirname(__FILE__).'/../Db.inc');
require_once(dirname(__FILE__).'/../../shared/classes/Time.inc');
require_once(dirname(__FILE__).'/../../shared/classes/score/IScoreTypes.inc');

/**
 * Klasse zur Verwaltung der Punkte der Spieler auf einer Karte.
 *
 * Genauer kann abgefragt werden, wie viele Punkte welcher Spieler auf welcher
 * Karte aktuell gesammelt hat (bzw. bei Karten, die vorbei sind, hatte).
 * Inklusive Ranglistenerstellung usw.
 *
 * @author Philipp Kemmeter
 */
class MapScore implements IScore, IScoreTypes {
	private $world_id;
	private $map_id;
	private static $db;

	private static $INSTANCES;

	/**
	 * Gibt die aktuelle Instanz der Karte zurück.
	 *
	 * @param uint $map_id
	 * @return MapScore
	 * @throws Exception
	 */
	public static function get_instance($world_id, $map_id) {
		$world_id = (int)$world_id;
		if ($world_id < 0)
			throw new Exception("Map does not exist!");
		$map_id = (int)$map_id;
		if ($map_id < 0)
			throw new Exception("Map does not exist!");

		if (!isset(self::$INSTANCES[$world_id][$map_id]))
			self::$INSTANCES[$world_id][$map_id] = new self($world_id, $map_id);
		return self::$INSTANCES[$world_id][$map_id];
	}

	/**
	 * Erzeugt ein neues MapScore-Objekt für die Map mit der ID $map_id.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @return MapScore
	 */
	private function __construct($world_id, $map_id) {
		$this->world_id = $world_id;
		$this->map_id = $map_id;
	}

	/**
	 * Gibt die aktuelle Datenbank zurück, in welcher die Punkte gespeichert
	 * werden/sind.
	 *
	 * @return Db
	 */
	private static function get_db() {
		if (!self::$db)
			self::$db = new Db(Db::SCORE_HOST, Db::SCORE_NAME,
								Db::SCORE_PW, Db::SCORE_SELECT_DB);
		return self::$db;
	}

	/**
	 * Fügt dem User Punkte hinzu.
	 *
	 * @param uint $uid
	 * @param uint $amount
	 * @param uint $type
	 * @see include/classes/score/IScore#add_score()
	 */
	public function add_score($uid, $amount, $type=false) {
		if (($type === false) || ($type < self::TYPE_UNKNOWN)
			|| ($type > self::TYPE_MAX))
		{
			$type = self::TYPE_UNKNOWN;
		}


		self::get_db()->query(
			"INSERT INTO map_score_log SET uid=$uid, score=$amount, ".
				"type=$type, t_stamp=".Time::get_t_now().", ".
				"world_id={$this->world_id}, ".
				"map_id={$this->map_id} ".
			"ON DUPLICATE KEY UPDATE score=score+$amount"
		);

		self::get_db()->query(
			"INSERT INTO map_score SET uid=$uid, score=$amount, ".
				"type=$type, creation_t_stamp=".Time::get_t_now().", ".
				"world_id={$this->world_id}, ".
				"map_id={$this->map_id} ".
			"ON DUPLICATE KEY UPDATE score=score+$amount"
		);
	}

	/**
	 * Gibt die Rangfolge geordnet nach $ordered_by zurück.
	 *
	 * @param uint $ordered_by
	 * @return stdClass[]
	 * @see include/classes/score/IScore#get_ranking()
	 */
	public function get_ranking($order_by=self::ORDER_SCORE_DESC) {
		$order_by_str = ($order_by == self::ORDER_SCORE_ASC) ?
			"ORDER BY score ASC" :
			"ORDER BY score DESC";

		$res = self::get_db()->query(
			"SELECT uid,SUM(score) AS score FROM map_score ".
			"WHERE world_id={$this->world_id} AND map_id={$this->map_id} ".
			"GROUP BY uid $order_by_str"
		);
		$uids =
			MapPlayers::get_instance($this->world_id, $this->map_id)
				->get_uids_on_map();

		$result = array();
		while ($o = mysql_fetch_object($res)) {
			$result[] = $o;
			if (($key = array_search($o->uid, $uids)) === false) {
				// Spieler nicht auf der Karte, Mapranking falsch?
				continue;
			}
			unset($uids[$key]);
		}

		// Alle User auf der Karte, die noch keine Punkte gemacht haben, werden
		// nun mit 0 Punkten ergänzt.
		foreach ($uids as $uid) {
			$o = new stdClass();
			$o->uid = $uid;
			$o->score = 0;
			$result[] = $o;
		}

		return $result;
	}

	/**
	 * Gibt die Punktzahl des Spielers zurück
	 *
	 * @param uint $uid
	 * @return float
	 * @see include/classes/score/IScore#get_score()
	 */
	public function get_score($uid) {
		$result = @mysql_result(self::get_db()->query(
			"SELECT SUM(score) FROM map_score WHERE uid=$uid AND ".
			"world_id={$this->world_id} AND ".
			"map_id={$this->map_id}"
		),0);
		return $result ? $result : 0;
	}

	public static function clear_of_all_users() {
		self::get_db()->query(
			"DELETE FROM map_score"
		);
		self::get_db()->query(
			"DELETE FROM map_score_log"
		);
	}
}
?>