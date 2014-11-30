<?php

require_once(dirname(__FILE__).'/../../shared/classes/exceptions/IHackattackException.inc');

/**
 * Repräsentiert den Fehlerfall, wenn durch eine wahrscheinlich gezielte
 * Manipulation von Input, eine illegale Aktion erzielt werden soll.
 * (Beispiel: Versuch, Wissenschaftler droppen)
 * 
 * @author Philipp Kemmeter
 */

class HackattackException extends Exception implements IHackattackException {
	private $uid;
	
	/**
	 * Einziger Konstruktor der Klasse. $count gibt den Hack-Count an, den
	 * der User aufgrund der Ausnahme bekommen soll. Mit erzeugen dieser
	 * Exception wird automatisch ein Hackattack für den gewählten User
	 * durchgeführt.
	 *
	 * @param string $message 	Nachricht, welche dem User zurückgegeben werden 
	 * 							soll (Wenn explizit "" übergeben wird, dann
	 * 							bekommt der User KEINE Rückmeldung)
	 * @param int $uid 		Die ID des Hackers
	 * @param int $code		Welche Art des Hackens lag vor
	 */
	public function __construct($message, $uid, $code) {
		$this->uid = $uid;
		parent::__construct($message, $code);
		$this->report_hack();
	}
	
	/**
	 * Gibt die UID des Hackers zurück
	 * 
	 * @return int UID des Hackers
	 */
	public function getUid() {	// Naming-Convention von Exception übernommen
		return $this->uid;
	}

	/**
	 * Zählt den Hackversuch des Users
	 */
	private function report_hack() {
		query (
			"INSERT INTO hacks SET uid={$this->getUid()}, ".
				"attack_type={$this->getCode()}, attack_count=1 ".
			"ON DUPLICATE KEY UPDATE attack_count=attack_count+1"
		);
		error_log(
			"Hackattack by {$this->getUid()} [Code: {$this->getCode()}]: ".
				$this->getMessage()
		);
	}
}


?>