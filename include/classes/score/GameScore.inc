<?php
require_once(dirname(__FILE__).'/MapScore.inc');
require_once(dirname(__FILE__).'/LeagueScore.inc');
require_once(dirname(__FILE__).'/../../shared/classes/score/GameScoreCalculation.inc');
require_once(dirname(__FILE__).'/../../shared/classes/score/IScoreTypes.inc');

/**
 * Wrapper zum hinzufügen der Punkte im Spiel, da diese sowohl der Liga, als
 * auch der Karte hinzugefügt werden muss. Zudem wird mithilfe von
 * ScoreCalculation noch die entsprechende Punktzahl berechnet, die hinzugefügt
 * werden muss, so dass man zum Hinzufügen von Punkten sich um nicht kümmern
 * muss.
 *
 * @author Philipp Kemmeter
 */
class GameScore extends GameScoreCalculation implements IScoreTypes {
	/**
	 * Konstruktor ist private, da GameScore rein statisch (namespace)
	 */
	private function __construct() {

	}

	/**
	 * Fügt dem User mit der angegebenen ID die übergebene Anzahl von Punkten
	 * hinzu. Wird Typ angegeben wird zusätzlich registriert, wofür es Punkte
	 * gab.
	 * Wird ein negativer Wert für $amount angegeben, werden die Punkte
	 * abgezogen.
	 * Die Karten-ID gibt an, für welche Karte der Spieler die Punkte erhalten
	 * soll. Dabei wird nicht geprüft, ob die Karte existiert oder der Spieler
	 * jemals auf dieser Karte gespielt hat.
	 *
	 * Gibt zurück, wie viele Punkte hinzugefügt wurden.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param uint $uid
	 * @param uint $amount
	 * @param uint $type
	 * @param uint $no_calc	Punkte sollen wörtlich übernommen werden!
	 * @return ufloat 		Wie viele Punkte hinzugefügt wurden
	 */
	public static function add_score($world_id, $map_id, $uid, $amount,
		$type=self::TYPE_UNKNOWN, $no_calc=false)
	{
		if (!$no_calc)
			$amount = self::calc_score($amount, $type);
		if ($amount <= 0)
			return 0;

		MapScore::get_instance($world_id, $map_id)
			->add_score($uid, $amount, $type);
		LeagueScore::get_instance(0)->add_score($uid, $amount, $type);

		return $amount;
	}
}
?>