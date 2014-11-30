<?php

/*
 * Created on Dec 11, 2006 by Philipp Kemmeter
 */

/**
 * Gutscheinklasse erstellt und verwaltet Gutscheine jeder Art, indem es Gutschein-Codes erstellt und
 * bei Nutzung prüft, ob diese a) vorhanden und b) noch unbenutzt sind. Zudem wird in der DB protokolliert
 * wer den Gutschein gekauft und wer ihn genutzt hat, sowie wann das geschah.
 */
class Gutscheine {
	private $uid;	// Id des Users auf den sich die Aktion bezieht.
	private $t_now;

	function __construct($uid) {
		$this->uid = $uid;
		$this->t_now = Time::get_t_now();
	}

	public function create_code($type) {	// erstellt einen Code und speichert ihn in der DB, gekauft von $this->uid
		$code = substr(md5($this->t_now), 4, 6);
		query("INSERT INTO gutscheine SET bought_by={$this->uid}, type=$type, code='$code', t_stamp_bought={$this->t_now}");
		return $code;
	}

	public function get_gutschein_by_code($code) {
		$res = query("SELECT * FROM gutscheine WHERE code='$code'");
		if (mysql_num_rows($res) == 0)
			return -1;
		else {
			$gutschein = mysql_fetch_assoc($res);
			if ($gutschein['used_by'] != 0)
				return -2;
			else
				return $gutschein;
		}
	}

	public function use_code($code) {		// benutzt einen Code und speichert status in die DB, benutzt von $this->uid
		$res = query("SELECT * FROM gutscheine WHERE code='$code' AND used_by=0");
		if (mysql_num_rows($res) == 0)
			return false;
		else {
			query("UPDATE gutscheine SET t_stamp_used={$this->t_now}, used_by={$this->uid} WHERE code='$code' AND used_by=0");
			return true;
		}
	}

	public function set_uid($uid) {
		$this->uid = $uid;
	}
}

?>
