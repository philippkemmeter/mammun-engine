<?php

/**
 * Diese Klasse steht für ein die abstrakten Datenmodelle eines ebenso noch
 * abstrakten Objektes auf der Karte. Es liefert die gemeinsamen Schnittstellen
 * für die Kartenberechnung ansich, dass zwischen Gebäuden und Einheiten in
 * Bestimmten Fällen nicht wirklich unterschieden werden muss.
 */
abstract class MapCompute__ComputedMapObjectsData {
	const T_BUILDING = 1;
	const T_TRIBE = 2;
	/**
	 * Enthält alle Daten, die aus Ergebnissen vorheriger Berechnungen hervor-
	 * gingen und nun in der Datenbank stehen, sowie die Daten, die komplett
	 * neu sind, und eine erste Berechnung erfahren müssen. Kurz: Alle Daten,
	 * die für die nächste Berechnung berücksichtigt werden müssen (was diese
	 * Objekte angeht) sind hier gespeichert.
	 * Die Daten sind nach Sektor und Berechnungszyklus angeordnet, da
	 * sektorweise pro Zyklus neue Daten berücksichtigt werden müssen.
	 *
	 * @var stdClass[][][]
	 */
	protected $data_to_compute;

	/**
	 * Enthält die Ergebnisse der letzten Berechnung. Im Unterschied zu
	 * $data_to_compute werden hier die Ergebnisse abgelegt, nachdem ein
	 * die Berechnung basierend auf den Daten von $data_to_compute
	 * abgespeichert. Hier liegen also die aktuellsten Daten sektorweise vor,
	 * und zwar nicht pro Berechnungszyklus, sondern eben als ganz klares
	 * Endergebnis.
	 *
	 * @var stdClass[][]
	 */
	protected $computed_results;

	/**
	 * Diese Variable enthält genau, zu welchem Zeitpunkt wieviele Objekte
	 * welchen Typs neu dazugekommen sind, um sie in die Berechnungen
	 * miteinzubeziehen. Im Gegensatz zu $data_to_compute werden hier nur
	 * Anzahlen gespeichert und auch nicht pro Zyklus, sondern pro genauem
	 * Zeitpunkt. Während $data_to_compute hauptsächlich zur Berechnung
	 * umkämpfter Gebiete benötigt wird, wird $appearances hauptsächlich für
	 * die Berechnung _nicht_ umkämpfter Gebiete eingesetzt.
	 * Negative Einträge in dieser Variablen sind möglich und stehen für
	 * das Verschwinden entsprechend vieler Instanzen des entsprechenden
	 * Objektes.
	 *
	 * @var stdClass[]
	 */
	protected $appearances;

	/**
	 * Da wir eine nachgeahmte Nested-Class haben, und einen Pointer auf die
	 * Vaterklasse benötigen, gibt es diese Hilfsvariable. Diese zeigt auf
	 * die Vaterinstanz der MapCompute-Klasse.
	 *
	 * @var MapCompute
	 */
	protected $outer_class;

	/**
	 * Die für die Karte eindeutige ID der letzten Berechnung, auf der die neue
	 * Berechnung aufbaut. Anhand dieser ID wird das meiste von $data_to_compute
	 * initialisiert.
	 *
	 * @var uint
	 */
	protected $compute_id;

	/**
	 * Die innerhalb der Welt eindeutige ID der Karte, welche berechnet werden
	 * soll.
	 *
	 * @var uint
	 */
	protected $map_id;

	/**
	 * Die ID der Welt in der die Karte sich befindet, die berechnet werden
	 * soll.
	 *
	 * @var uint
	 */
	protected $world_id;

	/**
	 * Speichert pro Sektor, welche User (UID) darin Objektinstanzen haben.
	 * Dadurch kann sehr effizient entschieden werden, ob der Sektor umkämpft
	 * ist, oder nicht, und welche Spieler gerade ggf. gegeneinander kämpfen.
	 *
	 * @var uint[]
	 */
	protected $uids_in_sector;

	/**
	 * Diese Funktion speichert die von MapCompute berechneten Ergebnisse, die
	 * diesem Objekt dann zugeführt wurden, in der Datenbank unter der
	 * übergebenen Compute-ID ab.
	 *
	 * @param uint $compute_id
	 */
	abstract public function save($compute_id);

	/**
	 * Gibt zurück, wie viele Instanzen eines bestimmten Objekttyps sich in dem
	 * übergebenen Sektor befinden oder FALSE, falls sich in dem Sektor auf
	 * der durch die Outer-Class definierten Karte keine solche Instanz befindet.
	 *
	 * Ist ferner $sector_id === false gesetzt, wird die Summe über alle
	 * Sektoren zurückgegeben.
	 *
	 * @param uint $object_id
	 * @param [0,1,...99] $sector_id
	 * @param uint $cycle
	 * @return ufloat/FALSE
	 */
	public function get_amount($object_id, $sector_id=false, $cycle=1) {
		if ($sector_id === false) {
			$result = 0;
			for ($i = 0; $i < 100; $i++) {
				if (!isset($this->data_to_compute[$cycle][$i][$object_id]))
					continue;
				$result += $this->data_to_compute[$cycle][$i][$object_id]->amount;
			}
			return $result;
		}
		$sector_id_old = $sector_id;
		$sector_id = (int)$sector_id;
		if ($sector_id < 0 || $sector_id > 99)
			throw new IllegalArgumentException("[0,1...,99]", $sector_id);

		return (isset($this->data_to_compute[$cycle][$sector_id][$object_id]))
			? $this->data_to_compute[$cycle][$sector_id][$object_id]->amount
			: false;
	}

	/**
	 * Gibt zurück, wie viele Instanzen des Objekts insgesamt maximal im
	 * Sektor vorhanden sein dürfen.
	 * (Da amount den Hitpoints entsprechen, bedeutet das übertragen: "wie
	 * viele Hitpoints des Objekts sind 100% 'Gesundheit'")
	 *
	 * @param uint $object_id
	 * @param [0,1,...99] $sector_id
	 * @return ufloat/FALSE
	 */
	public function get_max_amount($object_id, $sector_id) {
		$sector_id_old = $sector_id;
		$sector_id = (int)$sector_id;
		if ($sector_id < 0 || $sector_id > 99)
			throw new IllegalArgumentException("\$sector_id expected to be in ".
				"[0,1...,99]", $sector_id);
		return (isset($this->computed_results[$sector_id][$object_id]))
			? $this->computed_results[$sector_id][$object_id]->max_amount
			: false;
	}

	/**
	 * Gibt zurück, wie viele Instanzen sich insgesamt im angegeben Sektor
	 * zu einem gewissen Zyklus befinden. Wird kein Zyklus angegeben, wird
	 * die Summe aller Zyklen zurückgegeben. ACHTUNG: Dies entspricht NICHT
	 * dem Endergebnis der Berechnung!
	 *
	 * @param [0,1,...99] $sector_id	Die ID des Sektors
	 * @param uint $cycle				Berechnungszyklus
	 * @return ufloat
	 */
	public function get_total_amount($sector_id, $cycle=false) {
		$result = 0;
		if ($cycle === false) {
			foreach ($this->data_to_compute as $data_per_cycle) {
				if (isset($data_per_cycle[$sector_id])) {
					foreach ($data_per_cycle[$sector_id] as $object_arr) {
						$result += $object_arr->amount;
					}
				}
			}
		}
		else {
			if ($this->data_to_compute[$cycle][$sector_id]) {
				foreach ($this->data_to_compute[$cycle][$sector_id]
					as $object_arr)
				{
					$result += $object_arr->amount;
				}
			}
		}
		return $result;
	}

	/**
	 * Gibt zurück, wie viele Gegner ein bestimmtes Objekt in dem
	 * übergebenen Sektor getöten hat oder FALSE, wenn sich in dem Sektor auf
	 * der durch Outer-Class definierten Karte keine solche Instanz befindet.
	 *
	 * @param uint $object_id
	 * @param [0,1,..99] $sector_id
	 * @return ufloat/FALSE
	 */
	public function get_killed($object_id, $sector_id) {
		$in_sector_id %= 100;
		return (isset($this->computed_results[$sector_id][$object_id]))
			? $this->computed_results[$sector_id][$object_id]->killed
			: false;
	}

	/**
	 * Gibt zurück, wie viele Instanzen eines bestimmten Objekttyps in dem
	 * übergebenen Sektor gestorben sind oder FALSE, wenn sich in dem Sektor auf
	 * der durch Outer-Class definierten Karte keine solche Instanz befindet.
	 * ACHTUNG: Aktuell ist damit INTERN zwar ein Sterben gemeint, EXTERN sieht
	 * es aber nach Hitpoint-/SP-Verlust aus, da eine gestorbene Instanz als
	 * ein verlorener HP gezählt wird - nicht verwirrt sein hier...
	 *
	 * @param uint $object_id
	 * @param [0,1,..99] $sector_id
	 * @return ufloat/FALSE
	 */
	public function get_died($object_id, $sector_id) {
		$in_sector_id %= 100;
		return (isset($this->computed_results[$sector_id][$object_id]))
			? $this->computed_results[$sector_id][$object_id]->died
			: false;
	}

	/**
	 * Gibt zurück, von wann die Daten der Objektinstanz im Sektor stammen
	 * oder FALSE, wenn sich in dem Sektor auf der durch Outer-Class
	 * definierten Karte keine solche Instanz befindet.
	 *
	 * @param uint $object_id
	 * @param [0,1,..99] $sector_id
	 * @param uint $cycle
	 * @return uint/FALSE
	 */
	public function get_time_placed($object_id, $sector_id, $cycle=1) {
		$in_sector_id %= 100;
		return (isset($this->data_to_compute[$cycle][$sector_id][$object_id]))
			? $this->data_to_compute[$cycle][$sector_id][$object_id]->time_placed
			: false;
	}

	/**
	 * Gibt ein Array zurück, sortiert nach den Ankuftszeiten, welches pro
	 * Ankunftszeit pro Objekt-ID eine Anzahl enthält, also:
	 * RESULT[$t_stamp][$tribe_id] == $amount_of_units_of_this_object_in_sector
	 *
	 * @param [0,1,..99] $sector_id		Die ID des Sektors
	 * @return stdClass[][]
	 */
	public function get_appearances($sector_id) {
		return isset($this->appearances[$sector_id])
			? $this->appearances[$sector_id] : null;
	}

	/**
	 * Gibt die berechnete Anzahl der Instanzen des angegebenen Objekts im
	 * angegebenen Sektor zurück.
	 *
	 * @param uint $object_id
	 * @param [0,1,....99]
	 * @return uint
	 *
	 * @see MapCompute::compute_fight_in_sector()
	 * @see self::add_computed_amount()
	 * @see self::set_computed_amount()
	 */
	public function get_computed_amount($object_id, $sector_id) {
		return (isset($this->computed_results[$sector_id][$object_id])) ?
			$this->computed_results[$sector_id][$object_id]->amount
			: 0;
	}

	/**
	 * Setzt die Anzahl der Instanzen des angegebenen Objekts im angegebenen
	 * Sektor als Ergebnis der Berechnung auf $amount.
	 *
	 * @param uint $object_id
	 * @param [0,1,...99] $sector_id	Sektor-ID
	 * @param float $amount				Ergebnisanzahl
	 *
	 * @see MapCompute::compute_fight_in_sector()
	 * @see self::add_computed_amount()
	 */
	public function set_computed_amount($object_id, $sector_id, $amount) {
		$this->computed_results[$sector_id][$object_id]->amount = $amount;
	}

	/**
	 * Fügt die angegebene Anzahl dem Objekt im Sektor hinzu. Diese Funktion
	 * wird benutzt, um während der Berechnung die Zwischenergebnisse
	 * entsprechend zu speichern.
	 *
	 * @param uint $object_id
	 * @param [0,1,...99] $sector_id
	 * @param float $amount				Hinzuzufügende Anzahl
	 *
	 * @see MapCompute::compute_fight_in_sector()
	 */
	public function add_computed_amount($object_id, $sector_id, $amount) {
		$this->computed_results[$sector_id][$object_id]->amount += $amount;
	}

	/**
	 * Setzt die Anzahl, die das übergebene Objekt im angegebenen Sektor
	 * getötet hat.
	 *
	 * @param uint $object_id
	 * @param [0,1,...99] $sector_id
	 * @param float $amount
	 *
	 * @see MapCompute::compute_fight_in_sector()
	 */
	public function set_killed($object_id, $sector_id, $amount) {
		$this->computed_results[$sector_id][$object_id]->killed = $amount;
	}

	/**
	 * Erhöht die Anzahl der Objekte, die das übergebene Objekt im Sektor
	 * getötet hat um die angegebene Zahl.
	 *
	 * @param uint $object_id
	 * @param [0,1,...99] $sector_id	Sektor-ID
	 * @param float $amount				Hinzuzufügende Anzahl
	 *
	 * @see MapCompute::compute_fight_in_sector()
	 */
	public function add_killed($object_id, $sector_id, $amount) {
		$this->computed_results[$sector_id][$object_id]->killed += $amount;
	}


	/**
	 * Setzt die Anzahl der gestorbenen Instanzen des übergebenen Objekts im
	 * Sektor.
	 *
	 * @param uint $object_id
	 * @param [0,1,...99] $sector_id	Sektor-ID
	 * @param float $amount				Ergebnisanzahl
	 *
	 * @see MapCompute::compute_fight_in_sector()
	 */
	public function set_died($object_id, $sector_id, $amount) {
		$this->computed_results[$sector_id][$object_id]->died = $amount;
	}

	/**
	 * Erhöht die Anzahl der gestorbenen Instanzen des übergebenen Objekts im
	 * Sektor.
	 *
	 * @param uint $object_id
	 * @param [0,1,...99] $sector_id	Sektor-ID
	 * @param float $amount				Hinzuzufügende Anzahl
	 * @see MapCompute::compute_fight_in_sector()
	 */
	public function add_died($object_id, $sector_id, $amount) {
		$this->computed_results[$sector_id][$object_id]->died += $amount;
	}

	/**
	 * Gibt alle Objekte im Sektor zurück, die im übergebenen Cycle dazukamen
	 * oder Weggingen (dann negative Anzahl), und zwar als
	 * "$array[Object-ID] = Anzahl"
	 *
	 * @param [0,1,...99] $sector_id
	 * @param uint $cycle
	 */
	public function get_all_in_sector($sector_id, $cycle) {
		$result = array();
		if (isset($this->data_to_compute[$cycle][$sector_id])) {
			foreach ($this->data_to_compute[$cycle][$sector_id] as $oid => $jnk)
			{
				if (!isset($result[$oid]))
					$result[$oid] = 0;
				$result[$oid] += $this->get_amount($oid, $sector_id, $cycle);
			}
		}
		return $result;
	}
}

?>