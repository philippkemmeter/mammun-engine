<?php
require_once(dirname(__FILE__).'/../../shared/classes/score/IScoreTypes.inc');
require_once(dirname(__FILE__).'/../LevelUpPresents.inc');

/**
 * Verwaltet die Punkte eines Users einer Liga.
 *
 * Jeder Spieler kann innerhalb einer Liga Punkte sammeln. Dies sind die Punkte
 * aller Karten, die der Spieler in dieser Liga gespielt hat. Diese Punkte
 * können hiermit verwaltet werden, inklusive Ranking usw.
 *
 * @author Philipp Kemmeter
 */
class LeagueScore implements IScore, IScoreTypes {
	private $league_id;
	private static $db;

	private static $INSTANCES;

	/**
	 * Gibt die aktuelle Instanz der Liga zurück.
	 *
	 * @param uint $league_id
	 * @return LeagueScore
	 * @throws Exception
	 */
	public static function get_instance($league_id) {
		$league_id = (int)$league_id;

		if (!isset(self::$INSTANCES[$league_id]))
			self::$INSTANCES[$league_id] = new self($league_id);
		return self::$INSTANCES[$league_id];
	}

	/**
	 * Erzeugt ein neues LeagueScore-Objekt für die Liga mit der ID $league_id.
	 *
	 * @param uint $league_id
	 * @return LeagueScore
	 */
	private function __construct($league_id) {
		$this->league_id= $league_id;
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
			"INSERT INTO league_score_log SET uid=$uid, score=$amount, ".
				"type=$type, t_stamp=".Time::get_t_now().", ".
				"league_id={$this->league_id} ".
			"ON DUPLICATE KEY UPDATE score=score+$amount"
		);


		/**
		 * Bei Level-Up das entsprechende XP-Gebäude vergeben
		 */
		self::get_db()->query("START TRANSACTION");

		$old_score = $this->get_score($uid);
		$old_lvl = GameScoreCalculation::calc_level_by_score($old_score);
		$new_score = $old_score + $amount;
		$new_lvl = GameScoreCalculation::calc_level_by_score($new_score);

		self::get_db()->query(
			"INSERT INTO league_score SET uid=$uid, ".
				"league_id={$this->league_id}, score=$amount, ".
				"type=$type, creation_t_stamp=".Time::get_t_now()." ".
			"ON DUPLICATE KEY UPDATE score=score+$amount"
		);
		self::get_db()->query("COMMIT");

		if ($new_lvl > $old_lvl) {
			LevelUpPresents::get_instance($uid)->give_present($new_lvl);
		}
	}

	/**
	 * Gibt die Rangfolge geordnet nach $ordered_by zurück.
	 *
	 * @param uint $ordered_by
	 * @return stdClass[]
	 * @see include/classes/score/IScore#get_ranking()
	 */
	public function get_ranking($order_by=self::ORDER_SCORE_DESC) {
		$order_by_str = ($oder_by == self::ORDER_SCORE_ASC) ?
			"ORDER BY score ASC" :
			"ORDER BY score DESC";

		$res = self::get_db()->query(
			"SELECT uid,SUM(score) AS score FROM league_score ".
			"WHERE league_id={$this->league_id} GROUP BY uid $order_by_str"
		);

		$result = array();
		while ($o = mysql_fetch_object($res))
			$result[] = $o;
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
			"SELECT SUM(score) FROM league_score WHERE uid=$uid AND ".
			"league_id={$this->league_id}"
		),0);
		return $result ? $result : 0;
	}

	/**
	 * Löscht alle Punkte des Users der Liga irreversibel.
	 * @param uint $uid
	 */
	public function clear($uid) {
		self::get_db()->query(
			"DELETE FROM league_score WHERE uid=$uid ".
				"AND league_id={$this->league_id}"
		);
	}

	public static function clear_of_all_users() {
		self::get_db()->query(
			"DELETE FROM league_score"
		);
		self::get_db()->query(
			"DELETE FROM league_score_log"
		);
	}
}
?>
