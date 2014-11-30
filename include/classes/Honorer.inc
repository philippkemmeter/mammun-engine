<?php

/*
 * Created on Apr 5, 2007 by Philipp Kemmeter
 */
require_once(dirname(__FILE__).'/Accounter.inc');
require_once(dirname(__FILE__).'/../gen_funcs.inc');
require_once(dirname(__FILE__).'/Db.inc');

 class Honorer {
 	private $uid;					// id des Users
 	private $honor;					// Ruhm des Users
 	private $play_time;				// Ruhm pro Spielzeit
 	private $honor_recruits;		// Ruhm aller User, die er geworben hat
 	private $max_honor_ever;		// maximalen Ruhmpunkte des Users
 	private $honor_campaign;		// Ruhmpunnkte der aktuellen / letzten Kampagne
 	private $is_honor_campaign;		// ist aktuell eine Kampagne am Start :)

 	static private $INSTANCE = array();	// instanzüberwachung. Nur eine Instanz pro UID innerhalb von PV
 										// Konstruktor ist private. Objekt statisch über get_instance($uid)
 										// anfordern! (s.u.)

 	const CAMPAIGN_POINTS_TO_REACH = 5000;	// die Punkte, die man zum Gewinnen der Kampagne braucht
 	/*const PTS_SUICIDE = 100;
 	const PTS_ALLY_BREAK = 1000;
 	const PTS_ALLY_CANCEL = 100;
 	const PTS_NAP_BREAK = 100;
 	const PTS_ALLY_STAY = 100;
 	const PTS_NAP_STAY = 10;*/
 	const PTS_MAP_PLAYED = 100;
 	const PTS_MAP_WON = 50;

 	const DEBUG = 0;

 	/* Diese Statische Funtion als Konstruktor benutzen !*/
 	public static function get_instance($uid) {
 		if (!self::$INSTANCE || !self::$INSTANCE[$uid]) {
 			self::$INSTANCE[$uid] = new Honorer($uid);
 		}
 		if (self::DEBUG)
 			echo "HONORER: User id@get_instance: $uid<br>\n";
 		return self::$INSTANCE[$uid];
 	}

 	private function __construct($uid) {
 		if (self::DEBUG)
 			echo "HONORER: Constructor called<br>\n";

 		$this->uid = $uid;
 		$honor_array = mysql_fetch_array(query("SELECT honor, max_honor_ever, honor_campaign FROM users WHERE id_ffa=$uid OR id_clanmode=$uid"));
 		$this->honor = $honor_array['honor'];
 		$this->max_honor_ever = $honor_array['max_honor_ever'];
 		$this->honor_campaign = $honor_array['honor_campaign'];
 		$this->play_time = /** FUNKTION EXISITERT NICHT*/get_play_time($uid);
 		$this->honor_recruits = 0;	// Ruhmpunkte all seiner Rekruten aufaddieren
 		$honor_recruits_res = query("SELECT honor FROM users WHERE recruited_by=$uid");
 		while ($honor_recruit = mysql_fetch_array($honor_recruits_res))
 			if ($honor_recruit['honor'] > 0)	// negativen Ruhm nicht einbeziehn
 				$this->honor_recruits += $honor_recruit['honor']/2;	// durch zwei teilen, weil User immer doppelt rekrutiert

 		// Wenn bei einem beliebigen User der Ruhm ungleich -1 ist, dann läuft die Kampagne
		$test_honor_campaign = mysql_result(query("SELECT honor_campaign FROM users LIMIT 1"),0);
		$this->is_honor_campaign = ($test_honor_campaign != -1);
 	}

 	public function __destruct() {
 		if (self::DEBUG)
 			echo "HONORER: Destructor called.<br>\n";
 		$this->save_all();
 	}

 	/**
 	 * Ruhmpunkte hinzufügen.
 	 * Je nachdem, ob er dadurch eine neue Ruhmpunktestufe überschritten hat, bekommt er dafür als Lohn
 	 * Premium.
 	 * Da man auch Ruhm verlieren kann (durch unehrenhaftes Verhalten, wie Vertragsbrüche etc),
 	 * wird auch der maximal erreichte Ruhm gespeichert, dass man nicht bescheißen kann und mehrfach
 	 * pro Level Premium geiern kann :)
 	 */
 	public function add_honor($count) {
 		$old_honor_level = $this->get_honor_level();
 		$uid = $this->uid;
 		if (self::DEBUG)
 			echo "HONORER: add_honor($count) called. \$uid = $uid.<br>\n";
 		$this->set_honor($this->honor + $count);
 		if ($this->is_honor_campaign)
 			$this->add_honor_campaign($count);
 		if ($this->honor > $this->max_honor_ever) {	// er hat jetzt mehr Ruhm als jemals zuvor
 			$this->set_max_honor_ever($this->honor);

 			$new_honor_level = $this->get_honor_level();
 			if ($old_honor_level < $new_honor_level) {	// wenn er zudem eine neue "Stufe des Ruhms" erreicht hat ;-)
 				$this->give_gift($new_honor_level);	// dann dem User das entsprechende Geschenk zuweisen
 			}
 		}
 	}

 	public function remove_honor($count) {
 		$this->set_honor($this->honor - $count);
 		$this->set_honor_campaign($this->honor_campaign - $count);
 	}

 	public function get_honor($including_recruits=true) {
 		if ($including_recruits)
 			return $this->honor + $this->get_honor_due_to_others();
 		else
 			return $this->honor;
 	}

 	public function get_honor_rating($including_recruits=true) {
 		return (!$this->play_time) ? '-' : $this->get_honor($including_recruits) / $this->play_time * 1000;
 	}

 	/**
 	 * Gibt die Ehre durch andere zurück. Diese kann eigentlich nicht unter 0 sinken, da bei jedem einzelnen Spieler
 	 * bereits gecheckt wird, ob die Ehre positiv ist, oder nicht
 	 */
 	public function get_honor_due_to_others() {
 		return ($this->honor_recruits > 0) ? $this->honor_recruits/10 : 0;
 	}

 	public function get_honor_campaign() {
 		return $this->honor_campaign;
 	}

	public function is_honor_campaign() {
		return $this->is_honor_campaign;
	}
	/**
	 * Beendet einfach die Kampagne. Kann man auch vorzeitig machen. Mit der Funktion wird KEIN sieger ermittelt!
	 * Eine Beendigung der Kampagne löscht alle gesammelten Kampagnenpunkte (aber nicht von der allgemeinen Ehre)
	 * Also am besten nicht von außen aufrufen, es sei denn, man ist sich sehr sicher, dass man das wirklich will
	 */
	public function end_campaign() {
		 //Idee: Allen users kampagnen Punkte auf -1 setzen als Zeichen, dass keine Kampagne läuft
		 query("UPDATE users SET honor_campaign=-1");
		 $this->honor_campaign = -1;
		 $this->is_honor_campaign = false;
	}

	/**
	 * Startet eine Kampagne. Alle Kampagnen-Punkte werden auf 0 zurückgesetzt, was ungleich -1 ist und anzeigt,
	 * dass die Kampagne läuft :)
	 */
	public static function start_campaign() {
		query("UPDATE users SET honor_campaign=0");
	}

	/* PRIVATE */

 	private function add_honor_campaign($count) {
 		$this->set_honor_campaign($this->honor_campaign + $count);
 		if ($this->honor_campaign >= self::CAMPAIGN_POINTS_TO_REACH) {	// User hat die Kampagne gewonnen
			$this->give_campaign_gift_and_end_campaign();
 		}
 	}

	/* Die Funktion speichert alles vom RAM in die DB um. Wird vom Destruktor aufgerufen und könnte
	 * (aus paranoia-gründen) eventuell auch wann anders aufgerufen werden.
	 * Sofern der Destruktor sicher funktioniert, wird dieses Konzept des Speicherns beim Beenden
	 * viele DB-Zugriffe vermeiden :)
	 */
	private function save_all() {
 		if (self::DEBUG)
		  echo "UPDATE users SET honor={$this->honor}, max_honor_ever={$this->max_honor_ever}, honor_campaign={$this->honor_campaign} WHERE id_ffa={$this->uid} OR id_clanmode={$this->uid}";
		query("UPDATE users SET honor={$this->honor}, max_honor_ever={$this->max_honor_ever}, honor_campaign={$this->honor_campaign} WHERE id_ffa={$this->uid} OR id_clanmode={$this->uid}");

	}

	private function set_honor($honor) {
		$this->honor = $honor;
		//query("UPDATE users SET honor=$honor WHERE id_ffa={$this->uid} OR id_clanmode={$this->uid}");
	}

	private function set_max_honor_ever($max_honor) {
		$this->max_honor_ever = $max_honor;
		//query("UPDATE users SET max_honor_ever=$max_honor WHERE id_ffa={$this->uid} OR id_clanmode={$this->uid}");
	}

	private function set_honor_campaign($honor) {
		$this->honor_campaign = $honor;
		//query("UPDATE users SET honor_campaign=$honor WHERE id_ffa={$this->uid} OR id_clanmode={$this->uid}");
	}

 	private function get_honor_level() {
 		return floor($this->honor / 10000);
 	}

 	private function give_gift($for_level) {
 		$A = new Accounter($this->uid);
 		$A->change_account_type(ACCOUNT_PREMIUM, 3);
 		unset($A);
 	}

 	private function give_campaign_gift_and_end_campaign() {
 		$A = new Accounter($this->uid);
 		$A->change_account_type(ACCOUNT_PREMIUM, 6);	// ein halbes Jahr Premium geben für erfolgreich gewonnene Kampagne
 		unset($A);

 		$this->end_campaign();
 	}
}

?>