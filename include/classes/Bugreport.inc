<?php
require_once(dirname(__FILE__).'/exceptions/HackattackException.inc');
require_once(dirname(__FILE__).'/../shared/classes/exceptions/IllegalArgumentException.inc');
require_once(dirname(__FILE__).'/../shared/classes/IBugreports.inc');
/**
 * File created by Philipp Kemmeter on 07.02.2009 at 13:24:35
 */

/**
 * Behandelt Bug-Reports (fügt neue ein, zeigt alle an, markiert erledigte etc)
 *
 * @author Philipp Kemmeter
 */
class Bugreport implements IBugreports {
	static private $db = null;

	/**
	 * @return Db
	 */
	static private function get_db() {
		if (!self::$db)
			self::$db = new Db(Db::BUG_HOST, Db::BUG_NAME, Db::BUG_PW,
				Db::BUG_SELECT_DB);
		return self::$db;
	}

	private function __construct(){}

	/**
	 * Fügt einen neuen Fehlerbericht in die Datenbank ein und sendet den
	 * Verantwortlichen eine E-Mail
	 *
	 * @param uint $user_id
	 * @param string $browser
	 * @param string $os
	 * @param string $resolution
	 * @param string $window_size
	 * @param uint $bug_source
	 * @param string $repro_desc
	 * @param string $bug_comment
	 * @return uint 	Die ID des Bugs in der DB
	 */
	static public function insert_new_report($user_id, $browser, $os, $resolution,
		$window_size, $bug_source, $repro_desc, $bug_comment)
	{
		$browser = string_decode($browser);
		$os = string_decode($os);
		$repro_desc = string_decode($repro_desc);
		$bug_comment = string_decode($bug_comment);

		self::get_db()->query(
			"INSERT INTO bugreports SET ".
				"user_ID=$user_id, browser='$browser', os='$os', ".
				"resolution='$resolution', window_size='$window_size', ".
				"bug_source='$bug_source', repro_desc='$repro_desc', ".
				"comment='$bug_comment', datum=NOW()"
		);
		$bug_id = mysql_insert_id();
		if (strlen($bug_comment) > 100)
			$bug_comment = substr($bug_comment, 0, 98).'...';

		$mailtext = <<<REPORT
A new bug has been reported. The description by the user is the following:

$bug_comment


All details available here:
http://{$_SERVER['HTTP_HOST']}/game/bug.php?bug_id=$bug_id
REPORT;


		if (defined('CONF_SEND_BUG_REPORTS_TO') && CONF_SEND_BUG_REPORTS_TO)
			pv_mail(CONF_SEND_BUG_REPORTS_TO, "New bug reported", $mailtext);

		return $bug_id;
	}

	/**
	 * Gibt alle Bugs aus der Datenbank als ein Array von stdClass-Objekten
	 * aufsteigend sortiert nach Datum zurück
	 * (also ein direktes Abbild der bugreports-DB)
	 * Der Parameter $limit bezeichnet den LIMIT-Parameter in Mysql (Beispiel:
	 * '4' oder '10, 20')
	 * Wenn $open_bugs_only gesetzt wird, werden nur die Bugs zurückgegeben,
	 * die weder gefixt, noch gerade in Bearbeitung sind.
	 *
	 * @param string $limit
	 * @param bool $open_bugs_only
	 * @return stdClass[]
	 */
	static public function get_reports($limit=false, $open_bugs_only=false) {
		// TODO $limit auf Gültigkeit prüfen und Exception werden, wenn Blödsinn
		$result = array();
		$sql =
			"SELECT b.*, u.nick, u.email ".
				"FROM bugreports AS b ".
				"LEFT JOIN ".Db::GAME_SELECT_DB.".users AS u ON b.user_ID=u.id";
		if ($open_bugs_only)
			$sql .= " AND b.status<2";
		$sql .=	" ORDER BY b.datum DESC ";
		/*if ($limit)
			$sql .= "LIMIT $limit";*/
		$res = self::get_db()->query($sql);

		// 1. Schritt: Alle Bugs unter dem key der ID abspeichern
		while ($bug = mysql_fetch_object($res)) {
			$result[$bug->id] = $bug;
			$result[$bug->id]->joint_bugs = array();
			$a = self::get_db()->query(
				"SELECT COUNT(*) FROM bug_attach WHERE bug_id={$bug->id}"
			);
			$result[$bug->id]->attach_amount = @mysql_result($a,0);
		}

//		// 2. Schritt: Joint berücksichtigen, um gleiche Bugs zu gruppieren
//		$joint_bugs = array();
//		foreach ($result as $bug_id => $bug) {
//			$result[$bug_id]->joint_bugs =
//				self::fetch_all_joints($result[$bug_id]->joint_id, $result);
//			$joint_bugs[] = $result[$bug_id]->joint_bugs;
//		}

		// 2. Schritt: Parent_id berücksichtigen zum Gruppieren
		foreach ($result as $bug_id => $bug) {
			if (($bug->joint_id <= 0) || ($bug->joint_id == $bug_id))
				continue;
			if (isset($result[$bug->joint_id])) {
				$result[$bug->joint_id]->joint_bugs[] = $bug;
				unset($result[$bug->id]);
			}
		}

//		// 3. Schritt: Bugs, die in den Joints vorkommen, aus Ebene 0 der
//		// 			Results löschen, dass die Joint-Bugs nicht 2x übermittelt
//		//			werden.
//		$n = count($joint_bugs);
//		for ($i = 0; $i < $n; $i++) {
//			$m = count($joint_bugs[$i]);
//			for ($j = 0; $j < $m; $j++) {
//				unset($result[$joint_bugs[$i][$j]->id]);
//			}
//		}
		return $result;
	}

	/**
	 * Setzt einen Bug unterhalb von einem anderen an (Join). Dies wird
	 * i.d.R. dann gemacht, wenn $sub_bug das gleiche beschreibt wie $main_bug.
	 * Der Bug mit der ID $sub_bug_id wird in diesem Fall dem anderen
	 * untergeornet.
	 *
	 * @param uint $main_bug
	 * @param uint $sub_bug
	 */
	public static function join_bugs($main_bug_id, $sub_bug_id) {
		$main_bug_id = (int)$main_bug_id;
		$sub_bug_id = (int)$sub_bug_id;
		if ($main_bug_id <= 0)
			throw new IllegalArgumentException("main_bug_id", "{1,2,..}", $main_bug_id);
		if ($main_bug_id <= 0)
			throw new IllegalArgumentException("sub_bug_id", "{1,2,..}", $sub_bug_id);
//		$res = query (
//			"SELECT joint_id FROM bugreports.bugreports WHERE id=$main_bug_id"
//		);
//		$joint_id = mysql_result($res, 0);
//		if ($joint_id > 0) {
//			self::join_bugs($joint_id, $sub_bug_id);
//		}
//		else {
			self::get_db()->query (
				"UPDATE bugreports SET joint_id=$main_bug_id ".
				"WHERE id=$sub_bug_id"
			);
//		}
	}

	/**
	 * Holt rekursiv alle Joints eines Bugs (also Bugs, die als dasselbe
	 * gekennzeichnet wurden) heraus.
	 * @param uint $id
	 * @param stdClass[] $all_bugs
	 * @param stdClass[] $result
	 * @return stdClass[]
	 */
	static private function fetch_all_joints($id, array $all_bugs, $result=array()) {
		if ($id <= 0 || !isset($all_bugs[$id]))
			return $result;

		$new_bug = clone $all_bugs[$id];
		if (isset($new_bug->joint_bugs))
			unset($new_bug->joint_bugs);
		$result[] = $new_bug;
		return self::fetch_all_joints($all_bugs[$id]->joint_id,
			$all_bugs, $result);
	}

	/**
	 * Holt rekursiv alle Joints eines Bugs aus der Datenbank.
	 * @param uint $id
	 * @param stdClass[] $result
	 * @return stdClass[]
	 */
	static private function fetch_all_joints_db($id, $result=array()) {
		if ($id <= 0)
			return $result;
		$res = self::get_db()->query (
			"SELECT b.*, u.nick, u.email, ".
				"FROM bugreports AS b ".
			"LEFT JOIN ".Db::GAME_SELECT_DB.".users AS u ON b.user_ID=u.id ".
			"WHERE b.id=$id"
		);
		if (mysql_num_rows($res) < 1)
			return $result;	// Kette unterbrochen - kann man nicht weitermachen

		$new_bug = mysql_fetch_object($res);
		$result[] = $new_bug;
		return self::fetch_all_joints_db($new_bug->joint_id, $result);
	}

	/**
	 * Gibt den Bugreport zurück, den die ID $id angibt.
	 *
	 * @param uint $id
	 * @return stdClass
	 */
	static public function get_report($id) {
		if (!$id)
			return new stdClass();
		$res = self::get_db()->query (
			"SELECT b.*, u.nick, u.email, FROM bugreports AS b ".
			"LEFT JOIN ".Db::GAME_SELECT_DB.".users AS u ON b.user_ID=u.id ".
			"LEFT JOIN bug_attach AS a ON b.id=a.bug_id ".
			"WHERE b.id=".((int)$id)
		);
		$bug = mysql_fetch_object($res);
		$a = self::get_db()->query(
			"SELECT COUNT(*) FROM bug_attach WHERE bug_id={$bug->id}"
		);
		$bug->attach_amount = @mysql_result($a,0);
		$bug->joint_bugs = array();
		$res = self::get_db()->query (
			"SELECT b.*, u.nick, u.email, FROM bugreports AS b ".
			"LEFT JOIN ".Db::GAME_SELECT_DB.".users AS u ON b.user_ID=u.id ".
			"WHERE b.joint_id=".((int)$id)
		);
		while ($sub_bug = mysql_fetch_object($res)) {
			$a = self::get_db()->query(
				"SELECT COUNT(*) FROM bug_attach WHERE bug_id={$sub_bug->id}"
			);
			$sub_bug->attach_amount = @mysql_result($a,0);
			$bug->joint_bugs[] = $sub_bug;
		}
//		$bug->joint_bugs = self::fetch_all_joints_db($bug->joint_id);
		return $bug;
	}

	/**
	 * Setzt den Bug mit der angegebenen ID auf den angegebenen Status. Es muss
	 * ein Kommentar dazu geschrieben werden, der beschreibt, wie der Bug gefixt
	 * wurde o.ä.
	 *
	 * Bugs können nur bestimmte User fixen. Wenn anderer User das
	 * versucht => HackattackException
	 *
	 * @param uint $id
	 * @param string $comment
	 * @throws HackattackException wenn User das nicht darf
	 */
	static public function set_state($id, $state, $comment) {
		$id = (int)$id;
		$state = (int)$state;
		//TODO Check, dass der User das auch darf
		self::get_db()->query (
			"UPDATE bugreports ".
			"SET fixcomment='$comment', status=$state, fixdate=NOW() ".
			"WHERE id=$id"
		);
	}

	/**
	 * Setzt die Priorität des Bugs mit der angegebenen ID entsprechend.
	 *
	 * @param uint $id
	 * @param ubyte $priority
	 * @throws HackattackException wenn User das nicht darf
	 */
	static public function set_priority($id, $priority) {
		$id = (int)$id;
		$priority = (int)$priority;
		//TODO Check, dass der User das auch darf
		self::get_db()->query (
			"UPDATE bugreports ".
			"SET priority=$priority WHERE id=$id"
		);
	}

	/**
	 * Setzt die Kategorie-ID, der der Bug mit der angegebenen ID angehören
	 * soll.
	 *
	 * @param uint $id
	 * @param ubyte $cat_id
	 * @throws HackattackException wenn User das nicht darf
	 */
	static public function set_cat_id($id, $cat_id) {
		$id = (int)$id;
		$cat_id = (int)$cat_id;
		//TODO Check, dass der User das auch darf
		self::get_db()->query (
			"UPDATE bugreports SET cat_id=$cat_id ".
			"WHERE id=$id"
		);
	}

	/**
	 * Fügt ein Attachment dem Bug mit der angegebenen ID hinzu.
	 *
	 * @param uint $id
	 * @param string $data
	 */
	static public function add_attachment($id, $data, $filename) {
		$id = (int)$id;
		self::get_db()->query (
			"INSERT INTO bug_attach SET bug_id='$id', data='$data', ".
				"name='$filename'"
		);
	}

	/**
	 * Gibt die angefügten Dateien des Bugs mit der angegebenen ID zurück.
	 * Dabei ist jede Datei ein Objekt, welches in data die Binärdaten und
	 * in name den Dateinamen enthält.
	 * Wird $attach_id angegeben, wird nur das eine Attachment zurückgegeben,
	 * sonst alle.
	 *
	 * @param $bug_id
	 * @param $attach_id
	 * @return stdClass / stdClass[]
	 */
	public static function get_attachments($bug_id, $attach_id=false) {
		$bug_id = (int)$bug_id;
		$attach_id = (int)$attach_id;
		if ($attach_id !== false) {
			$res = self::get_db()->query(
				"SELECT * FROM bug_attach WHERE id=$attach_id AND ".
					"bug_id=$bug_id"
			);
			$result = mysql_fetch_object($res);
			$result = $result->data;
		}
		else {
			$res = self::get_db()->query(
				"SELECT data,name FROM bug_attach WHERE bug_id=$bug_id"
			);
			$result = array();
			while($adb = mysql_fetch_object($res)) {
				$result[] = $adb;
			}
		}
		return $result;
	}
}
?>