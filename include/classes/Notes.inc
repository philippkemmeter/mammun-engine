<?php
/**
 * File created by Philipp Kemmeter on 10.02.2009 at 19:08:31
 */
 
/**
 * Verwaltet die Notizen eines Spielers
 * 
 * @author Philipp Kemmeter
 */
class Notes implements IConvertablePHPStdClass {
	private $uid;
	private $note;
	private $t_stamp;
	private $db_need_update = false;
	
	private static $INSTANCES = array();
	
	/** 
	 * Gibt die Instanz zurück, die der UID zugeordnet ist (singleton)
	 * 
	 * @param uint $uid
	 * @return Notes
	 */
	public static function get_instance($uid) {
		if (!isset(self::$INSTANCES[$uid])) {
			self::$INSTANCES[$uid] = new Notes($uid);
		}
		return self::$INSTANCES[$uid];
	}
	
	private function __construct($uid) {
		$this->uid = $uid;
		if (isset($_SESSION['Notes'][$uid])) {
			$this->note = $_SESSION['Notes'][$uid]['note'];
			$this->t_stamp = $_SESSION['Notes'][$uid]['t_stamp'];
		}
		else {
			$this->init_from_db();
		}
	}
	
	public function __destruct() {
		if ($this->db_need_update) {
			query (
				"INSERT INTO notes SET note='{$this->note}', ".
					"t_stamp={$this->t_stamp}, uid={$this->uid} ".
				"ON DUPLICATE KEY UPDATE note='{$this->note}', ".
					"t_stamp={$this->t_stamp}"
			);
		}
		$_SESSION['Notes'][$this->uid]['note'] = $this->note;
		$_SESSION['Notes'][$this->uid]['t_stamp'] = $this->t_stamp;
	}
	
	private function init_from_db() {
		$res = query (
			"SELECT note, t_stamp FROM notes WHERE uid={$this->uid}"
		);
		if (mysql_num_rows($res) == 0) {
			$this->note = '';
			$this->t_stamp = 0;
		}
		else {
			$tmp = mysql_fetch_assoc($res);
			$this->note = $tmp['note'];
			$this->t_stamp = $tmp['t_stamp'];
		}
	}
	
	/**
	 * Setzt die Notiz des Spielers auf den entsprechenden Text
	 * 
	 * @param string $note
	 */
	public function set_note($note) {
		$this->note = $note;
		$this->t_stamp = Time::get_t_now();
		$this->db_need_update = true;
	}
	
	/**
	 * Gibt die Notiz des Spielers zurück
	 * 
	 * @return string
	 */
	public function get_note() {
		return $this->note;
	}
	
	/**
	 * Gibt zurück, wann die Notiz das letzte mal geändert wurde.
	 * 
	 * @return uint
	 */
	public function get_last_modified_time() {
		return $this->t_stamp;
	}
	
	public function to_php_std_class() {
		$obj = new stdClass();
		$obj->note = $this->note;
		$obj->t_stamp = $this->t_stamp;
		return $obj;
	}
}
?>