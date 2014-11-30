<?php
/**
 * Schnittstelle für die Verwaltung von Punktelisten.
 * 
 * @author Philipp Kemmeter
 */
interface IScore {
	const ORDER_SCORE_DESC = 0;
	const ORDER_SCORE_ASC = 1;
	
	/**
	 * Fügt dem User mit der angegebenen ID die übergebene Anzahl von Punkten 
	 * hinzu. Wird Typ angegeben wird zusätzlich registriert, wofür es Punkte 
	 * gab. Die entsprechenden Typ-Konstanten sind in den einzelnen Klassen
	 * zu verwalten.
	 * Wird ein negativer Wert für $amount angegeben, werden die Punkte 
	 * abgezogen.
	 * 
	 * @param uint $uid
	 * @param uint $amount
	 * @param uint $type
	 */
	public function add_score($uid, $amount, $type=false);
	
	/**
	 * Gibt die aktuelle Rangliste zurück. Durch die Angabe von $ordered_by kann
	 * explizit bestimmt werden, wonach diese sortiert sein soll. Standard ist
	 * nach der Anzahl der Punkte absteigend.
	 * 
	 * Zurückgegeben wird ein Array von Standardobjekten, wobei jedes Objekt
	 * mindestens die Felder score und uid enthalten muss.
	 *  
	 * @param uint $ordered_by
	 * @return stdClass[]
	 */
	public function get_ranking($order_by=self::ORDER_SCORE_DESC);
	
	/**
	 * Gibt die aktuelle Punktzahl des Users mit der angegebenen ID zurück.
	 * 
	 * @param uint $uid
	 * @return float
	 */
	public function get_score($uid);
}
?>