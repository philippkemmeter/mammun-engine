<?
require_once(dirname(__FILE__).'/../shared/classes/IEventMessages.inc');
require_once(dirname(__FILE__)
				.'/../shared/classes/exceptions/IllegalArgumentException.inc');

/**
 * Diese Klasse verwaltet ereignisgesteuerter Systemnachrichten an den User. Im
 * Gegensatz zu EventLog, welches nur Ereignisse protokolliert, ist hier der
 * Zweck, dass die Nachrichten einmal aufpoppen, dann als gelesen markiert
 * und somit gelöscht werden, dass sie nicht mehr aufpoppen.
 *
 * @author Philipp Kemmeter
 */
class EventMessages implements IEventMessages {
	private $uid;
	private $messages = array();
	private $messages_init_from_db = false;


	private static $INSTANCES;

	/**
	 * @param uint $uid
	 * @return EventMessages
	 */
	public static function get_instance($uid) {
		$uid = (int)$uid;
		if (!isset(self::$INSTANCES[$uid]))
			self::$INSTANCES[$uid] = new self($uid);
		return self::$INSTANCES[$uid];
	}

	/**
	 * @param uint $uid
	 * @return EventMessages
	 */
	private function __construct($uid) {
		$this->uid = $uid;
		// Verzögert!$this->init_from_db();
	}

	private function init_from_db() {
		/**
		 * Holt alle Nachrichten vom Server unter Berücksichtigung aller
		 * gesetzten Filter.
		 *
		 * Das ist eine zusätzliche Sicherheit, da eigentlich ja gar keine
		 * Nachrichten in die DB eingefügt werden, die rauszufiltern sind.
		 * ABER: Diese Klasse ist nicht für alle Nachrichtentypen zum Schreiben,
		 * aber zum Lesen verantwortlich, also muss sie hier nochmal checken!
		 */
		$e1 = self::TYPE_GAME_ENGINE_MIN;
		$e2 = self::TYPE_GAME_ENGINE_MAX;
		$p1 = self::TYPE_PAYMENT_MIN;
		$p2 = self::TYPE_PAYMENT_MAX;
		$s1 = self::TYPE_SPECIAL_MIN;
		$s2 = self::TYPE_SPECIAL_MAX;
		$t1 = self::TYPE_SPECIAL_POPUPS_MIN;
		$t2 = self::TYPE_SPECIAL_POPUPS_MAX;
		$res = query(
			"SELECT num, type, t_stamp, extra, extra_str, score, flags, ".
				"medal_id, ".
				"IF(".
					"f.uid IS NULL OR ".
					"((m.type < $e1) OR (m.type > $e2) ".
						"OR (f.filter_engine != ".
							"(f.filter_engine | (1 << m.type - $e1)))) ".
					"AND ((m.type < $p1) OR (m.type > $p2) ".
						"OR (f.filter_payment != ".
							"(f.filter_payment | (1 << (m.type - $p1))))) ".
					"AND ((m.type < $s1) OR (m.type > $s2) ".
						"OR (f.filter_special != ".
							"(f.filter_special | (1 << (m.type - $s1))))) ".
					"AND ((m.type < $t1) OR (m.type > $t2) ".
						"OR (f.filter_special_popups != ".
							"(f.filter_special_popups | (1 << (m.type - $t1))))) ".
				", 0, 1) AS add_score_and_kill ".
			"FROM m_event AS m ".
			"LEFT JOIN m_event_filter AS f ".
				"ON m.uid=f.uid ".
			"WHERE m.uid={$this->uid} AND t_stamp<=".Time::get_t_now()." ".
			"ORDER BY t_stamp DESC"
		);
		if (mysql_num_rows($res) == 0) {
			return;
		}

		while ($row = mysql_fetch_object($res)) {
			/**
			 * Die rausgefilterten Nachrichten, die aber Punkte bringen bei
			 * Erhalt, sollen jetzt schon die Punkte bringen und dann gelöscht
			 * werden. Die anderen bringen erst beim Löschen der Nachricht von
			 * Hand punkte.
			 */

			if ($row->add_score_and_kill) {
				$this->add_score($row->type, $row->score);
				$this->delete_message($row->num);
				continue;
			}
			unset($row->add_score_and_kill);
			$this->messages[] = $row;
		}
	}

	/**
	 * Das ist ein Wrapper, der dem Spieler die Punkte korrekt hinzufügt.
	 *
	 * @param uint $type
	 * @param uint $score
	 */
	private function add_score($type, $score) {
		if ($score > 0) {
			$user = User::get_instance_by_id($this->uid);
			switch ($type) {
				case self::TYPE_SC_FINISHED:
					$type = GameScore::TYPE_SCIENCE_RESULT;
					break;
				case self::TYPE_TRAINING_FINISHED:
				case self::TYPE_CONSTRUCTION_COMPLETED:
					$type = GameScore::TYPE_RESSOURCE_USE;
					break;
				default:
					$type = GameScore::TYPE_UNKNOWN;
			}
			GameScore::add_score(
				$user->get_world_id(), $user->get_map_id(), $this->uid,
				$score, $type, true
			);
		}
	}

	/**
	 * Gibt den Event-Log zurück. Kann leicht veraltet sein. Wenn die Aktualität
	 * sichergestellt werden soll, muss $force auf TRUE gesetzt werden.
	 *
	 * @param bool $force [=false]
	 * @return stdClass[]
	 */
	public function get_messages($force=false) {
		if (!$this->messages_init_from_db || $force)
			$this->init_from_db();
		return $this->messages;
	}

	/**
	 * Filtert für die Zukunft den gewählten Nachrichtentyp raus.
	 *
	 * Der Nutzer bekommt dann künftig keine Nachrichten des angegebenen Typs
	 * mehr.
	 *
	 * @param uint $type_to_filter
	 */
	public function add_filter($type) {
		if (($type >= self::TYPE_GAME_ENGINE_MIN)
			&& ($type <= self::TYPE_GAME_ENGINE_MAX))
		{
			$field = "filter_engine";
			$type -= self::TYPE_GAME_ENGINE_MIN;
		}
		else if (($type >= self::TYPE_PAYMENT_MIN)
			&& ($type <= self::TYPE_PAYMENT_MAX))
		{
			$field = "filter_payment";
			$type -= self::TYPE_PAYMENT_MIN;
		}
		else if (($type >= self::TYPE_SPECIAL_MIN)
			&& ($type <= self::TYPE_SPECIAL_MAX))
		{
			$field = "filter_special";
			$type -= self::TYPE_SPECIAL_MIN;
		}
		else if (($type >= self::TYPE_SPECIAL_POPUPS_MIN)
			&& ($type <= self::TYPE_SPECIAL_POPUPS_MAX))
		{
			$field = "filter_special_popups";
			$type -= self::TYPE_SPECIAL_POPUPS_MIN;
		}
		query(
			"INSERT INTO m_event_filter SET uid={$this->uid}, $field=$type ".
			"ON DUPLICATE KEY UPDATE $field=($field|(1 << $type))"
		);
	}

	/**
	 * Filtert für die Zukunft den gewählten Nachrichtentyp NICHT MEHR raus.
	 *
	 * Der Nutzer bekommt dann künftig WIEDER Nachrichten des angegebenen Typs.
	 *
	 * @param uint $type_to_filter
	 */
	public function remove_filter($type) {
		if (($type >= self::TYPE_GAME_ENGINE_MIN)
			&& ($type <= self::TYPE_GAME_ENGINE_MAX))
		{
			$field = "filter_engine";
			$type -= self::TYPE_GAME_ENGINE_MIN;
		}
		else if (($type >= self::TYPE_PAYMENT_MIN)
			&& ($type <= self::TYPE_PAYMENT_MAX))
		{
			$field = "filter_payment";
			$type -= self::TYPE_PAYMENT_MIN;
		}
		else if (($type >= self::TYPE_SPECIAL_MIN)
			&& ($type <= self::TYPE_SPECIAL_MAX))
		{
			$field = "filter_special";
			$type -= self::TYPE_SPECIAL_MIN;
		}
		else if (($type >= self::TYPE_SPECIAL_POPUPS_MIN)
			&& ($type <= self::TYPE_SPECIAL_POPUPS_MAX))
		{
			$field = "filter_special_popups";
			$type -= self::TYPE_SPECIAL_POPUPS_MIN;
		}
		query(
			"UPDATE m_event_filter SET $field=($field&~(1 << $type)) ".
				"WHERE uid={$this->uid}"
		);
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
	 * @param uint $score
	 * @param uint $medal_id
	 */
	public function add_entry($type, $extra=0, $extra_str='', $t_stamp=0,
		$score=0, $medal_id=0)
	{
		if (($type < 0)	|| ($type > 255))
		{
			throw new IllegalArgumentException("type",
				'{0, 1, ..., 255}',	$type);
		}
		$type = (int)$type;

		if ($medal_id) {
			$score += GameScoreCalculation::calc_score(
				1, IScoreTypes::TYPE_MEDAL);
		}

		/**
		 * Wir checken mal schnell, ob das nicht vielleicht rausgefiltert
		 * werden soll.
		 */
		$res = query("SELECT * FROM m_event_filter WHERE uid={$this->uid}");
		if (mysql_num_rows($res) > 0) {
			$row = mysql_fetch_object($res);
			if ((($type >= self::TYPE_GAME_ENGINE_MIN)
				&& ($type <= self::TYPE_GAME_ENGINE_MAX)
				&& ($row->filter_engine == ($row->filter_engine
								| (1 << ($type - self::TYPE_GAME_ENGINE_MIN)))))

				||

				((($type >= self::TYPE_SPECIAL_MIN)
				&& ($type <= self::TYPE_SPECIAL_MAX)
				&& ($row->filter_engine == ($row->filter_engine
								| (1 << ($type - self::TYPE_SPECIAL_MIN)))))) )
			{
				/**
				 * Filter passt, also müssen wir die ggf. mit der Nachricht
				 * verbundenen Punkte gutschreiben (da der Spieler die Nachricht
				 * ja nie erhält uns somit nicht aktiv löschen kann), fügen
				 * die Nachricht aber gar nicht erst in die DB ein, sondern
				 * brechen hier ab.
				 */
				$this->add_score($type, $score);
				return;
			}

		}
		if (!$t_stamp)
			$t_stamp = Time::get_t_now();
		else
			$t_stamp = (int)$t_stamp;
		$extra = (int)$extra;
		$score = (int)$score;
		$medal_id = (int)$medal_id;
		/**
		 * Einzufügendes Datum erzeugen.
		 */
		$row = new stdClass();
		$row->t_stamp = $t_stamp;
		$row->type = $type;
		$row->extra = $extra;
		$row->score = $score;
		$row->medal_id = $medal_id;


		/**
		 * Transaktionssichere DB: Autoincrement geht nicht so, braucht man
		 * auch nicht, kann man selbst machen :)
		 */
		query("START TRANSACTION");
		$row->num = @mysql_result(
			query("SELECT MAX(num) FROM m_event WHERE uid={$this->uid}"), 0
		) + 1;

		/**
		 * Jetzt die Nachricht direkt in die DB schreiben
		 */
		query(
			"INSERT INTO m_event SET uid={$this->uid}, num={$row->num}, ".
				"type=$type, t_stamp=$t_stamp, extra=$extra, ".
				"extra_str='$extra_str', flags=".self::FLAG_NEW_MESSAGE.", ".
				"score=$score, medal_id=$medal_id"
		);
		query("COMMIT");

		/**
		 * Aktuelle Nachrichtenliste aktuallisieren.
		 */
		$this->messages[] = $row;
	}

	/**
	 * Markiert die angegebene Nachricht als gelesen.
	 *
	 * @param uint $id
	 */
	public function mark_message_as_read($id) {
		$id = (int)$id;
		query(
			"UPDATE m_event SET	flags=(flags&~".self::FLAG_NEW_MESSAGE.") ".
			"WHERE uid={$this->uid} AND num=$id"
		);
		$n = count($this->messages);
		for ($i = 0; $i < $n; $i++){
			if ($this->messages[$i]->num == $id) {
				$this->messages[$i]->flags &= ~self::FLAG_NEW_MESSAGE;
				break;
			}
		}
	}

	/**
	 * Löscht die angegebene Nachricht.
	 *
	 * @param uint $id
	 */
	public function delete_message($id) {
		$id = (int)$id;
		query("START TRANSACTION");
		$res = query(
			"SELECT type, score FROM m_event WHERE uid={$this->uid} AND num=$id"
		);
		if (mysql_num_rows($res) > 0) {
			$row = mysql_fetch_object($res);
			query(
				"DELETE FROM m_event WHERE uid={$this->uid} AND num=$id"
			);
		}
		query("COMMIT");

		$this->add_score($row->type, $row->score);

		$this->messages_init_from_db = false;	// out of sync
		for ($i = 0, $n = count($this->messages); $i < $n; ++$i) {
			if ($this->messages[$i]->num == $id) {
				array_splice($this->messages, $i, 1);
				break;
			}
		}
	}

	/**
	 * !!!!! LÖSCHT ALLE !!!!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = null;
		query("DELETE FROM m_event");
	}
}

?>
