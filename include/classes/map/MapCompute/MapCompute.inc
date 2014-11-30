<?php

require_once (dirname(__FILE__).'/../MapDescription.inc');
require_once (dirname(__FILE__).'/../../../shared/classes/Time.inc');
require_once (dirname(__FILE__).'/../MapPlayers.inc');
require_once (dirname(__FILE__).'/../Mapsettings.inc');
require_once (dirname(__FILE__).'/../../TribeOnMap.inc');
require_once (dirname(__FILE__).'/../../../shared/classes/exceptions/IllegalArgumentException.inc');
require_once (dirname(__FILE__).'/../../Resource.inc');

require_once (dirname(__FILE__).'/ComputedMapData.inc');
require_once (dirname(__FILE__).'/ComputedSectorData.inc');
require_once (dirname(__FILE__).'/ComputedTribesData.inc');
require_once (dirname(__FILE__).'/ComputedBuildingData.inc');
require_once (dirname(__FILE__).'/MovingTribesData.inc');
require_once (dirname(__FILE__).'/../../../shared/classes/Direction.inc');
require_once (dirname(__FILE__).'/../../EventLog.inc');
require_once (dirname(__FILE__).'/../../EventMessages.inc');


//Es gibt ein paar Nested Classes (leider wird das von PHP nicht unterstützt)
//die gewisse Datensätze in der DB verwalten und repräsentieren.



/**
 * Das hier ist die neue Kern-Engine. Natürlich benötigt sie weitere Klassen,
 * aber hier werden die Hauptberechnungen der Karte durchgeführt, d.h. alle
 * Berechnungen, die nötig sind, um die Karte von einem alten in der DB
 * gespeicherten Stand zum Zeitpunkt X auf den Stand von Zeitpunkt Y zu bringen.
 *
 * @author Philipp Kemmeter
 */
class MapCompute {
	const ERR_MAP_NOT_STARTED = 1;

	/**
	 * Tribes werden als einzelne Kampfeinheit umdefiniert, Units als
	 * Lebenspunkte (also wie bei Gebäuden, wo die Units die Strukturpunkte
	 * darstellen). Ein Tribe, der MAX_UNITS_PER_TRIBE Units innehält, ist
	 * somit eine vollständig geheilte/gesunde Kampfeinheit auf dem Schlachtfeld
	 */
	const MAX_UNITS_PER_TRIBE = 1000;	// NIEMALS ÄNDERN!
	const NORMAL_COMP_TIMEOUT = 60;
	/**
	 * Das Computation Delay ist eine Sicherheitsverzögerung, die es verhindert,
	 * dass gewisse Operationen doppelt gemacht werden könnnen. Der Wert von 3s
	 * wurde sehr überlegt gewählt, also hier nichts dran drehen!
	 */
	const COMPUTATION_DELAY = 3;
	const NORMAL_FIGHT_SPEED = 1;
	const SLOWER1_FIGHT_SPEED = 0.8;
	const FASTER1_FIGHT_SPEED = 1.2;

	const ATTDEF_DOM_GROUP_NOBODY = 1;
	const ATTDEF_DOM_GROUP_USER = 2;
	const ATTDEF_DOM_GROUP_HOST = 3;

	const DEBUG_VERBOSE = 0;

	private /*uint*/ $world_id;
	private /*uint*/ $id;

	/**
	 * @var MapDescription
	 */
	private $map_desc;
	protected /*int*/ $timeout;
	protected /*uint*/ $last_compute_time;
	protected /*uint*/ $last_compute_id;
	protected /*uint*/ $cycles_to_compute;
	protected /*uint*/ $cycle_duration;

	/**
	 * Enthält die kartenspezifischen Berechnungsdaten
	 *
	 * @var MapCompute__ComputedMapData
	 */
	private $computed_map_data;

	/**
	 * Enthält die sektorspezifischen Berechnungsdaten
	 *
	 * @var MapCompute__ComputedSectorData
	 */
	private $computed_sector_data;

	/**
	 * Enthält die gebäudespezifischen Berechnungsdaten
	 *
	 * @var MapCompute__ComputedBuildingsData
	 */
	private $computed_buildings_data;

	/**
	 *  Enthält die einheitenspezifischen Berechnungsdaten
	 *
	 * @var MapCompute__ComputedTribesData
	 */
	private $computed_tribes_data;

	/**
	 * Enthält die bewegungsspezifischen Berechnungsdaten
	 *
	 * @var MapCompute__MovingTribesData
	 */
	private $moving_tribes_data;

	/**
	 * Enthält Infos zu allen Spielern auf der Karte
	 *
	 * @var MapPlayers
	 */
	private $map_players;

	/**
	 * Gibt an, ob die Karte neu berechnet werden muss. Nur dann wird die Karte
	 * auch erneut in die DB gespeichert, weil sonst keine Änderungen seit dem
	 * letzten Speichern existieren.
	 *
	 * @var bool
	 */
	protected $need_recompute;

	/**
	 * Sektoren, die als Dirty markiert sind, werden außer der Reihe in jedem
	 * Fall bei der nächsten Berechnung mitberechnet (danach wird das Dirty-Flag
	 * entfernt).
	 *
	 * @var bool[100]
	 */
	private $dirty_sectors;

	protected /*ufloat*/ $fightspeed;
		// Ein Faktor, der die Kampfgeschwindigkeit beeinflusst

	protected static $INSTANCES = array();

	/**
	 * Gibt die entsprechende MapCompute-Instanz zurück.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param uint $timeout[optional]
	 * @return MapCompute
	 * @throws Exception
	 */
	public static function get_instance($world_id, $map_id,
		$timeout=self::NORMAL_COMP_TIMEOUT)
	{
		if (!isset(self::$INSTANCES[$world_id][$map_id]))
			self::$INSTANCES[$world_id][$map_id] = new self($world_id, $map_id);
		self::$INSTANCES[$world_id][$map_id]->init($timeout);
		return self::$INSTANCES[$world_id][$map_id];
	}

	/**
	 * Erzeugt ein neues MapCompute-Objekt. Die ID der zu berechnenden Karte
	 * muss übergeben werden.
	 *
	 * @param uint $world_id
	 * @param int $map_id	Die ID der zu berechnenden Karte
	 * @throws Exception
	 * @return MapCompute
	 */
	protected function __construct($world_id, $map_id) {
//		error_log("MapCompute::__construct($map_id). Memory: ".number_format(memory_get_usage()));
		$this->id = $map_id;
		$this->world_id = $world_id;
		$this->map_desc = MapDescription::get_instance($world_id, $map_id);
		if ($this->map_desc->get_state() == MapDescription::STATE_MAP_IDLE) {
			throw new Exception("MAP IS NOT STARTED, YET!",
				self::ERR_MAP_NOT_STARTED);
		}
		$this->fightspeed = self::SLOWER1_FIGHT_SPEED;
		$this->last_compute_time = 0;
		$this->last_compute_id = 0;
		$this->timeout = 0;
		$this->dirty_sectors = array();
	}

	public function __destruct() {
	}

	/**
	 * Wird von get_instance aufgerufen. Initialisiert alle Daten (erneut), wenn
	 * der übergebene Timeout überschritten ist. Sprich: Wird z.B. 30 übergeben
	 * als Timeout, so bekommt man im schlimmsten Fall eine 30s veraltete
	 * Berechnung zurück. So wird viel Rechenzeit eingespart.
	 *
	 * @param unsigned int $timeout in Sekunden
	 */
	protected function init($timeout=5) {
		if ($this->world_id == 0)
			return;
		// MUTEX: Keine zwei Computes derselben Karte ineinander
		$sem_id = sem_get(MUTEX_MAP_COMPUTE_OFFSET+$this->world_id*10000+
			$this->id, 1, 0600, -1);
		sem_acquire($sem_id);

		try {
			$this->timeout = ($timeout < 0) ? INF : $timeout;
			$recompute_only_dirty_sectors = false;

			if (($this->last_compute_time + $this->timeout) <= Time::get_t_now())
			{
				$this->last_compute_time = 0;	// wird von get_last_compute_from_db
				$this->last_compute_id = 		// gesetzt
					$this->get_last_compute_from_db($this->last_compute_time);

				if (!$this->last_compute_id) {
					/*
					 * Wenn jetzt keine Zeit gefunden werden konnte, dann gibt es
					 * absolut nichts zu tun - die Karte ist komplett leer, ohne
					 * jegliche Einheit und ohne jegliches Gebäude.
					 */
					if ($this->last_compute_time <= 0) {
						error_log('MapCompute::init: last_compute_id AND ' .
							'last_compute_time are empty! Nothing to '.
							'calculate, the map is empty! Please check this, '.
							'this shouldn\'t happen. '.
							"world_id={$this->world_id}, map_id={$this->id}"
						);
						/*if ($timeout > 0) {
							// MUTEX
							sem_release($sem_id);
							$this->init(0);	// retry
						}*/
						sem_release($sem_id);
						return;
					}
				}

				if (($this->last_compute_time + $this->timeout)
					< Time::get_t_now())
				{
					if (self::DEBUG_VERBOSE > 0)
						error_log(
							"\$this->last_compute_time + \$this->timeout [="
							.($this->last_compute_time)." + "
													.($this->timeout)."] < ".
							"Time::get_t_now() [=".Time::get_t_now()."]"
						);
					$this->need_recompute = true;
				}
			}
			/**
			 * Eigentlich müsste kein Recompute erfolgen, weil der Timeout es
			 * verbietet, aber wir haben dirty sectors, welche berechnet werden
			 * müssen.
			 */
			else if (!empty($this->dirty_sectors)) {
				$this->need_recompute = true;
				$recompute_only_dirty_sectors = true;
				if (self::DEBUG_VERBOSE > 1)
					error_log("dirty sectors, recompute necessary: ".
						"{$this->last_compute_time}");
			}
			/**
			 * Wir haben auch keine dirty sectors => kein recompute nötig, alles
			 * wie gehabt.
			 */
			else {
				$this->need_recompute = false;
				if (self::DEBUG_VERBOSE > 4)
					error_log("no recompute: {$this->last_compute_time}");
			}

			/**
			 * Wenn es keine last_compute_time gibt, dann können wir auch nichts
			 * berechnen. Das ist immer dann der Fall, wenn der init-Schritt
			 * explizit übersprungen werden soll, um z.B. nur ein dirty-Flag zu
			 * setzen.
			 */
			if (!$this->last_compute_time || $timeout == INF) {
				if (self::DEBUG_VERBOSE > 1)
					error_log(
						"Timeout is INF or last_compute_time is null: Skipping!"
					);

				sem_release($sem_id);
				return;
			}

			// Zyklendauer und -Anzahl ermitteln
			// muss als aller erstes geschehen
			$this->cycle_duration =
				self::compute_cycle_duration(
					$this->last_compute_time,
					Time::get_t_now(), $this->cycles_to_compute
				);
			if (self::DEBUG_VERBOSE > 4) {
				error_log("Cylcle duration: {$this->cycle_duration}");
			}

			// Alle Daten einlesen (lassen)
			if (self::DEBUG_VERBOSE > 4) {
				error_log("Fetching computed_map_data from Db...");
			}
			$t0 = microtime(true);
			$this->computed_map_data =
				MapCompute__ComputedMapData::get_instance(
					$this, $this->need_recompute
				);
			$t1 = microtime(true);

			if (self::DEBUG_VERBOSE > 4) {
				error_log("Fetching computed_sector_data from Db...");
			}
			$this->computed_sector_data =
				MapCompute__ComputedSectorData::get_instance(
					$this, $this->need_recompute
				);
			$t2 = microtime(true);

			if (self::DEBUG_VERBOSE > 4) {
				error_log("Fetching computed_building_data from Db...");
			}
			$this->computed_buildings_data =
				MapCompute__ComputedBuildingData::get_instance(
					$this, $this->need_recompute
				);
			$t3 = microtime(true);

			if (self::DEBUG_VERBOSE > 4) {
				error_log("Fetching moving_tribes_data from Db...");
			}
			$this->moving_tribes_data =
				MapCompute__MovingTribesData::get_instance(
					$this, $this->need_recompute
				);
			$t4 = microtime(true);

			if (self::DEBUG_VERBOSE > 4) {
				error_log("Fetching tribes_data from Db...");
			}
			$this->computed_tribes_data =
				MapCompute__ComputedTribesData::get_instance(
					$this, $this->need_recompute
				);
			$t5 = microtime(true);

			if (self::DEBUG_VERBOSE > 4) {
				error_log("Loading map_players...");
			}
			$this->map_players =
				MapPlayers::get_instance($this->world_id, $this->id,
					$this->need_recompute);

			$t6 = microtime(true);


			if ($this->need_recompute) {
				$this->compute_map($recompute_only_dirty_sectors);
				$t7 = microtime(true);
				if (self::DEBUG_VERBOSE > 0)
					error_log("computing... ({$this->last_compute_id}) - ".
						($t7-$t0)."s needed");

				$this->save();
				$this->check_for_defeated_players();
			}
		}
		catch (Exception $e) {
			// MUTEX
			sem_release($sem_id);
			throw $e;
		}
		catch (HackattackException $e) {
			// MUTEX
			sem_release($sem_id);
			throw $e;
		}
		catch (IllegalArgumentException $e) {
			// MUTEX
			sem_release($sem_id);
			throw $e;
		}
		catch (ExceptionWithErrorlog $e) {
			// MUTEX
			sem_release($sem_id);
			throw $e;
		}
		catch (ExceptionWithErrormail $e) {
			// MUTEX
			sem_release($sem_id);
			throw $e;
		}


		// MUTEX
		sem_release($sem_id);
	}

	/**
	 * Gibt den letzten compute zurück. Im Referenzparameter $last_compute_time
	 * wird die zugehörige Zeit gespeichert.
	 *
	 * @param int &$last_compute_time	Zeit wird hierüber zurückgegeben.
	 * @return int last_compute_id
	 */
	private function get_last_compute_from_db(&$last_compute_time) {
		$map_id = $this->map_desc->get_map_id();
		$world_id = $this->map_desc->get_world_id();
		$res = query(
			'SELECT * FROM computed_maps '.
			"WHERE world_id=$world_id AND map_id=$map_id ".
				"AND insert_finished=1 ".
			'ORDER BY time_computed DESC'
		);
		$num_computes = mysql_num_rows($res);
		$last_compute_id = 0;
		$last_compute_time = 0;
		// Es wurde schonmal eine Berechnung der Tribes durchgeführt.
		if ($num_computes > 0) {
//			/**
//			 * Nur die Computedrows nehmen, die älter als
//			 * self::COMPUTATION_DELAY Sekunden sind. Jüngere Datensätze sind
//			 * von vorherigen Berechnungen, die denselben Zeitpunkt
//			 * widerspiegeln. Würden wir COMPUTATION_DELAY ignorieren, würden
//			 * Einheiten verdoppelt. */
//			do {
				$last_compute_row = mysql_fetch_assoc($res);
//			} while ($last_compute_row['time_computed'] >
//				(Time::get_t_now() - self::COMPUTATION_DELAY));
			$last_compute_id = $last_compute_row['compute_id'];
			$last_compute_time = $last_compute_row['time_computed'];
		}
		if ($last_compute_id > 0) {
			/**
			 * Wurde die Karte bereits öfter als 3x berechnet, werden die
			 * älteren Einträge gelöscht, so dass nur die drei neusten Einträge
			 * behalten werden. (Dass das 3 sind ist relativ willkürlich. Hier
			 * sollte man mal mathematische Beweise führen!)
			 * [Anm.: Die obige do-while-Schleife verzerrt natürlich, was
			 * behalten wird. Es werden also 3 behalten, älter als
			 * COMPUTATION_DELAY sind, und zusätzlich noch die, die jünger waren.
			 * Erfahrungsgemäß weicht es dadurch um höchstens 1 ab. Zusätzlich
			 * werden alle behaklten, die nicht älter als 10s sind. Dadurch
			 * weicht es erfahrungsgemäß erneut um bis zu 1 ab. Wir behalten
			 * also ca. 3-5 Datensätze und schmeißen den Rest weg.
			 * Das Wegwerfen dient nur dazu, die DB klein zu halten.]
			 **/
			if ($num_computes > 3) {
				//mindestens eins wurde sowieso schon behalten (s. do-while)
				mysql_fetch_assoc($res);	// behalten
				mysql_fetch_assoc($res);	// behalten
				// alle anderen löschen, aber nur, wenn sie älter als 10s sind
				while ($row = mysql_fetch_assoc($res)) {
					if ($row['time_computed'] < Time::get_t_now() -10) {
						query(
							'DELETE FROM computed_map_data '.
							"WHERE world_id=$this->world_id ".
								'AND map_id='.$this->id.' AND compute_id='.
								$row['compute_id']
						);
						query(
							'DELETE FROM computed_sector_data '.
							"WHERE world_id=$this->world_id ".
								'AND map_id='.$this->id.' AND compute_id='.
								$row['compute_id']
						);
						query(
							'DELETE FROM computed_tribes '.
							"WHERE world_id=$this->world_id ".
								'AND map_id='.$this->id.' AND compute_id='.
								$row['compute_id']
						);
						query(
							'DELETE FROM computed_buildings '.
							"WHERE world_id=$this->world_id ".
								'AND map_id='.$this->id.' AND compute_id='.
								$row['compute_id']
						);
						query(
							'DELETE FROM computed_maps '.
							"WHERE world_id=$this->world_id ".
								'AND map_id='.$this->id.' AND compute_id='.
								$row['compute_id']
						);
						query(
							'DELETE FROM computed_extra '.
							"WHERE world_id=$this->world_id ".
								'AND map_id='.$this->id.' AND compute_id='.
								$row['compute_id']
						);
					}
				}
			}
		}
		else {
			/**
			 * Es gibt noch keine Berechnungen, last_compute_time ist hier
			 * zu dem Zeitpunkt dann auch 0. Es muss nun ein sinnvoller
			 * Startzeitpunkt festgelegt werden. Dieser ist nicht t_now,
			 * sondern zum Zeitpunkt der ersten Einheit / des ersten
			 * Gebäudes auf der Karte.
			 */
			$min_time_tribes = @mysql_result(
				query(
					'SELECT MIN(t_appearance) FROM tribes_on_map '.
					"WHERE world_id=$world_id AND map_id=$map_id ".
					"	AND t_appearance<=".(Time::get_t_now()-1)
				), 0
			);
			$min_time_buildings = @mysql_result(
				query(
					'SELECT MIN(time_build_started) '.
					'FROM building '.
					"WHERE world_id=$world_id AND map_id=$map_id ".
					"	AND time_build_started<=".(Time::get_t_now()-1)
				), 0
			);
			if (!$min_time_tribes)
				$last_compute_time = $min_time_buildings;
			else if (!$min_time_buildings)
				$last_compute_time = $min_time_tribes;
			else
				$last_compute_time = min($min_time_tribes, $min_time_buildings);
			// Wenn die Karte komplett leer ist, ist die Zeit jetzt immernoch 0
		}
		/**
		 * Zum aktuellen Zeitpunkt haben wir den jüngsten Compute gefunden, der
		 * in Frage kommt, falls es einen gibt. Zudem wurde die DB etwas
		 * aufgeräumt.
		 * Nun wird last_compute_id und last_compute_time zurückgegeben.
		 */
		return $last_compute_id ? $last_compute_id : 0;
	}

	/**
	 * Speichert den aktuellen Stand in die Datenbank (also die letzte
	 * Berechnung)
	 */
	private function save() {
		// Nur speichern, wenn nötig, also, wenn sich was geändert hat, also
		// wenn recompute gemacht wurde
		if ($this->need_recompute) {
			query(
				"INSERT INTO computed_maps SET world_id={$this->world_id}, ".
					"map_id={$this->id}, ".
					"time_computed=".Time::get_t_now().", ".
					"insert_finished=0"
			);
			$this->last_compute_id =
				mysql_result(query("SELECT LAST_INSERT_ID()"),0);
			$this->last_compute_time = Time::get_t_now();

			$this->computed_map_data->save($this->last_compute_id);
			$this->computed_buildings_data->save($this->last_compute_id);
			$this->computed_tribes_data->save($this->last_compute_id);
			$this->computed_sector_data->save($this->last_compute_id);
			$this->moving_tribes_data->save();

			// TODO: ROLLBACK ON FAILURE

			query(
				'UPDATE computed_maps SET insert_finished=1 '.
				'WHERE world_id='.$this->world_id.' AND map_id='.$this->id.' '.
					'AND compute_id='.$this->last_compute_id
			);
		}
	}


	/**
	 * Fügt dem User mit der übergebenen ID die Ressourcen hinzu, die er in dem
	 * übergebenen Sektor für den übergebnenen Zeitraum gewonnen hat.
	 *
	 * @param int $sector 	Sektor, in dem die Resourcen produziert werden
	 * @param int $uid		User, der die Resourcen bekommen soll
	 * @param float $duration_in_hours		für welchen Zeitraum
	 */
	private function add_map_bio($sector_id, $uid, $duration_in_hours) {
		/**
		 * Nur den Spielern Ressourcen geben, die auch leben, also noch auf
		 * der Karte sind!
		 */
		if (MapPlayers::get_instance($this->world_id, $this->id)
				->get_user_state_on_map($uid) != MapPlayers::STATE_STILL_ALIVE)
		{
			return;
		}

		for ($r = 0; $r < Resource::NUM_RESOURCES; $r++) {
			$sector_resource_to_add =
				Resource::get_instance($uid)->get_resource_gain($r, $sector_id,
					false, true)
					* $duration_in_hours;

			$this->computed_sector_data->add_resource($uid, $sector_id, $r,
				$sector_resource_to_add);
		}
	}

	/**
	 * Berechnet die gesamte Karte. Es gibt für jeden Sektor hierbei zwei
	 * Möglichkeiten: Entweder zyklenweise (nötig, wenn Kampf) oder direkte
	 * Berechnung (nur möglich, wenn kein Kampf).
	 *
	 * @param bool $recompute_only_dirty_sectors	nur die Dirtys berechnen
	 */
	private function compute_map($recompute_only_dirty_sectors) {
		/**
		 * 1. Fall:
		 * Nur die dirty sectors berechnen
		 */
		if ($recompute_only_dirty_sectors) {
			for ($sector_id = 0; $sector_id < 100; $sector_id++) {
				if (!empty($this->dirty_sectors[$sector_id]))
					$this->compute_sector($sector_id);
			}
		}
		/**
		 * 2. Fall:
		 * Alle berechnen
		 */
		else {
		// alle möglichen Sektoren durchgehen
			for ($sector_id = 0; $sector_id < 100; $sector_id++) {
				$this->compute_sector($sector_id);
			}
		}
		/**
		 * In jedem Fall wird davon ausgegangen, dass jetzt die Karte aktuell
		 * ist und die nächste Berechnung erst nach Timeout oder Dirty erfolgen
		 * muss.
		 */
		// Compute der Karte ist fertig - timestamp setzen
		$this->last_compute_time = Time::get_t_now();
		// Es gibt keine dirty Sectors mehr, da alles berechnet:
		$this->dirty_sectors = array();
	}

	/**
	 * Berechnet einen Sektor. Dabei gibt es zwei Möglichkeiten, entweder
	 * zyklenweise (aufwendig, nötig, wenn Kampf), oder direkte Berechnung
	 * (unaufwendig, nur möglich, wenn kein Kampf)
	 *
	 * @param [0,..99] $sector_id
	 */
	private function compute_sector($sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		if ($this->computed_tribes_data->is_fighting($sector_id))
			$this->compute_fight_in_sector($sector_id);
		else {
			 // Markieren, dass der Kampf beendet ist
			$this->computed_sector_data->end_fight($sector_id);

			if (($this->computed_tribes_data->get_total_amount($sector_id) > 0)
				|| ($this->computed_buildings_data
						->get_total_amount($sector_id) > 0))
			{
				$this->compute_no_fight_in_sector($sector_id);
			}
			else {
				$this->no_compute_sector($sector_id);
			}
		}
	}

	/**
	 * Berechnet den angegebenen Sektor nicht, sondern schaut nur noch nach
	 * Dominanzen. Das ist nur deswegen wichtig, weil ganz leere Sektoren
	 * sonst nicht mehr Berechnet würden und die Dominanz ewig beim letzten
	 * Eroberer läge. Wir wollen aber, dass man zum Halten mindestens ein
	 * Haus oder eine Einheit im Sektor hat.
	 *
	 * @param [0,..99] $sector_id
	 */
	private function no_compute_sector($sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		$this->set_dominator($sector_id, 0,	Time::get_t_now(), false);
	}

	/**
	 * Berechnet die Kleinigkeiten, die anfallen, wenn in einem Sektor kein
	 * Kampf stattgefunden hat.
	 *
	 * @param [0,1,...99] $sector_id
	 */
	private function compute_no_fight_in_sector($sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		if (self::DEBUG_VERBOSE > 2) {
			error_log("compute_no_fight_in_sector($sector_id)");
		}
		$appearances =
			my_array_merge_recursive(
				$this->computed_tribes_data->get_appearances($sector_id),
				$this->computed_buildings_data->get_appearances($sector_id)
			);

		/**
		 * In appearances stehen nun nach Timestamps sortiert, wann welche
		 * Einheiten in den Sektor dazukamen. Es wird immer etappenweise das
		 * Bio verteilt, und zwar an den, der die Etappe dominierte. Da die
		 * letzte Etappe bis t_now nicht berücksichtigt würde (es gibt in
		 * appearances keinen Eintrag, da keine Veränderung), wird einfach ein
		 * leerer Eintrag in appearances unter dem Zeitstempel von t_now
		 * angelegt. Das bedeutet dann "bis zu dem Zeitpunkt keine Veränderung".
		 * Somit funktioniert die Biozuteilung mithilfe der folgenden Schleife
		 * auf natürliche Weise :)
		 */
		if (!isset($appearances[Time::get_t_now()]))
			$appearances[Time::get_t_now()] = array();


		$prev_time_placed = 0;
		$tribes_per_user = array();
		$buildings_per_user = array();
		$dominator_history = array();
		$joined_amount_per_tribe = array();
		$joined_amount_per_building = array();
		$cur_dominator = $this->computed_sector_data->get_dominator($sector_id);
		$uids_in_sector = array();
		$dom_change_time = Time::get_t_now();

		$sector_c = new WorldCoordinates(
			$this->get_world_id(), $this->get_map_id(), $sector_id
		);

		// Alle Zeitpunkte durchgehen, zu denen sich was geändert hat
		foreach ($appearances as $cur_time_placed => $object_data_array) {
			// Alle Tribes durchgehen, die zu dem Zeitpunkt (in dem Sektor)
			// dazukamen oder auch abgezogen wurden
			foreach ($object_data_array as $object) {
				if (!is_object($object)) {
					error_log("NOT OBJECT!: " . print_r($object,true));
				}
				if ($object->type ==
					MapCompute__ComputedMapObjectsData::T_TRIBE)
				{	// Es ist ein Tribe
					$tid = $object->id;
					$amount = $object->amount;
					try {
						$tribe = TribeOnMap::get_instance($tid,
							$sector_id,
							$this->get_map_id(),
							$this->get_world_id()
						);
					}
					catch (Exception $e) {
						/**
						 * Tribe existiert nicht: überspringen
						 */
						$joined_amount_per_tribe[$tid] = 0;
						continue;
					}

					/**
					 * Wenn sich die Einheit bewegt, kann sie nicht mehr im
					 * Sektor sein - überspringen.
					 * Das kommt regulär dann vor, wenn zwischen dem letzten
					 * Compute und jetzt die Bewegung initiiert wurde.
					 */
					if ($tribe->is_moving()) {
						$joined_amount_per_tribe[$tid] = 0;
						continue;
					}

					if (!isset($joined_amount_per_tribe[$tid]))
						$joined_amount_per_tribe[$tid] = 0;

					$uid = $tribe->get_uid();
					if (!in_array($uid, $uids_in_sector))
						$uids_in_sector[] = $uid;
					// Einheit dem User zuschreiben (wichtig für Dominanz)
					if (empty($tribes_per_user[$uid]) ||
						!in_array($tribe, $tribes_per_user[$uid]))
					{
						$tribes_per_user[$uid][] = $tribe;
					}

					// Achtung: Der Dominator kann nicht trivial direkt bestimmt
					// werden, da amount auch negativ sein kann. Daher wird das
					// außerhalb dieser Schleife erledigt.

					// Gesamt-/Endsumme pro Tribe aktualisieren

					$amount_plus_per_healing =
						((Time::get_t_now() - $cur_time_placed) / 3600)
							* $tribe->get_healing_speed();

					$joined_amount_per_tribe[$tid] += $amount;
					$joined_amount_per_tribe[$tid] += $amount_plus_per_healing;
					if (self::DEBUG_VERBOSE > 4) {
						error_log("Tribe $tid has healed ".
												"$amount_plus_per_healing HP");
					}
					if ($joined_amount_per_tribe[$tid] >
						self::MAX_UNITS_PER_TRIBE)
					{
						$joined_amount_per_tribe[$tid]
							= self::MAX_UNITS_PER_TRIBE;
					}
				}
				else if ($object->type ==
					MapCompute__ComputedMapObjectsData::T_BUILDING)
				{	// Es ist ein Gebäude
					$bid = $object->id;
					$amount = $object->amount;
					if ($amount <= 0) {
//						pv_mail("p.kemmeter@gausus.com",
//							"Gebäude-Anzahl: 0",
//							"MapCompute: Zeile: ".__LINE__.":\n".
//							var_export($object, true)
//						);
					}
					if (!isset($joined_amount_per_building[$bid]))
						$joined_amount_per_building[$bid] = 0;

					$ulc_x = floor($bid/1000);
					$ulc_y = $bid % 1000;
					try {
						$building = BuildingOnMap::get_instance(
							$sector_c,
							$ulc_x,
							$ulc_y
						);
						$uid = $building->get_uid();

						if (!in_array($uid, $uids_in_sector))
							$uids_in_sector[] = $uid;
						// Gebäude dem User zuschreiben
						if (empty($buildings_per_user[$uid]) ||
							!in_array($building, $buildings_per_user[$uid]))
						{
							$buildings_per_user[$uid][] = $building;
						}

						// Gesamt-/Endsumme der SP pro Gebäude aktualisieren
						$joined_amount_per_building[$bid] += $amount;
					}
					catch (Exception $e) {
						/**
						 * Gebäude existiert nicht (mehr)
						 */
						$this->computed_buildings_data->set_computed_amount(
							$bid, $sector_id, 0
						);
					}
				}
			}

			// dominator zum Zeitpunkt $time_placed bestimmen

			$new_dominator =
				self::calc_dominator($tribes_per_user, $buildings_per_user);

			$expl_just_finished = false;
			if ($this->exploration_just_finished($sector_id, $new_dominator,
				$cur_time_placed))
			{
				$this->computed_sector_data->end_exploration(
					$sector_id, $cur_time_placed
				);
				$expl_just_finished = true;
			}

			if ($expl_just_finished || ($cur_dominator == $new_dominator)) {
				$dominator_history[$cur_time_placed] = $new_dominator;
				$dom_change_time = $cur_time_placed;

				// Dem Dominator für die Zeit der Dominanz Bio gutschreiben
				if ($new_dominator && $prev_time_placed) {
					$time_dom_in_h = ($cur_time_placed - $prev_time_placed)/3600;
					$this->add_map_bio($sector_id, $uid, $time_dom_in_h);
				}

				$cur_dominator = $new_dominator;

				$this->set_dominator(
					$sector_id, $cur_dominator,	$dom_change_time, false
				);
			}
			elseif (!$this->computed_sector_data
				->get_explore_in_sector($sector_id))
			{
				$this->computed_sector_data->start_exploration(
					$sector_id, $cur_time_placed
				);
			}

			$prev_time_placed = $cur_time_placed;
		}
		foreach ($joined_amount_per_tribe as $tid => $amount) {
			try {
				$tribe = TribeOnMap::get_instance($tid, $sector_id,
					$this->get_map_id(), $this->get_world_id());
			}
			catch (Exception $e) {
				/**
				 * Tribe existiert nicht: überspringen
				 */
				unset($joined_amount_per_tribe[$tid]);
				$this->computed_tribes_data->set_computed_amount($tid,
					$sector_id, 0);
				continue;
			}
			$this->computed_tribes_data->set_computed_amount(
				$tid, $sector_id, $amount
			);
			$this->computed_tribes_data->set_tribe_is_camouflaged(
				$tid, $sector_id, $tribe->can_camouflage()
			);
		}
		foreach ($joined_amount_per_building as $bid => $amount) {
			if ($amount <= 0) {
//				pv_mail("p.kemmeter@gausus.com",
//					"Gebäude-Anzahl <= 0",
//					"MapCompute: Zeile: ".__LINE__.":\n".
//					var_export($joined_amount_per_building, true)
//				);
			}
			$this->computed_buildings_data->set_computed_amount(
				$bid, $sector_id, $amount
			);
		}

		/**
		 * Wenn mehrere UIDs im Sektor vertreten sind, dann sollen alle Gebäude
		 * dem User übertragen werden, der Dominiert.
		 * Wenn nur eine UID im Sektor ist, heißt das, das nur Gebäude und
		 * Einheiten eines Users da sind => Gebäude müssen dann nicht übertragen
		 * werden.
		 */
		if (count($uids_in_sector) > 1) {
			Buildings::assign_buildings_in_sector_to_user($sector_c,
				$cur_dominator);
		}

	}


	/**
	 * Prüft, ob eine Sektorerkundung gerade in dem Sektor läuft, obwohl sie
	 * beendet werden muss, weil sie schon abgelaufen ist.
	 *
	 * @param uint $sector_id
	 * @param uint $t_stamp
	 * @return bool
	 */
	private function exploration_just_finished($sector_id, $exploring_user,
		$t_stamp=0)
	{
		ValueChecker::sector_id($sector_id, 'sector_id');
		ValueChecker::t_stamp($t_stamp, 't_stamp');

		if ($this->computed_sector_data->get_explore_in_sector($sector_id)) {
			/**
			 * Der Host braucht keine Erkundungszeit
			 */
			if ($exploring_user == HostControl::USER_HOST_ID)
				return true;

			/**
			 * Der eigene Homesektor ist immer erkundet
			 */
			if ($exploring_user > 0) {
				$hs = User::get_instance_by_id($exploring_user)
					->get_homesector() % 100;

				if ($hs == $sector_id)
					return true;
			}

			/**
			 * Jetzt kommt der normale check, ob die Erkundungszeit abgelaufen
			 * ist
			 */
			if (!$t_stamp)
				$t_stamp = Time::get_t_now();

			$time_end
				= $this->computed_sector_data
					->get_last_state_change_t_stamp($sector_id)
							+ CIBal::EXPLORATION_TIME;

			return ($time_end <= $t_stamp);
		}
		return false;
	}

	/**
	 * Berechnet anhand der übergebenen Gebäude und Stämme, wem der Sektor
	 * aktuell gehört.
	 *
	 * @param array $tribes_per_user
	 * @param array $buildings_per_user
	 * @param array $unit_amount_per_user	Wie viele Einheiten insgesamt im Sektor sind
	 * @return uint
	 */
	private static function calc_dominator(array $tribes_per_user,
		array $buildings_per_user, array $unit_amount_per_user=array())
	{
		$dominator = 0;
		$max = 0;
		foreach ($tribes_per_user as $uid => $data) {
			if (count($data) > $max) {
				$dominator = $uid;
				$max = count($data);
			}
			else if (count($data) == $max) {
				$dominator = 0;
			}
		}

		// Hat das auch zum Gleichstand geführt, dann die HP vergleichen
		if (!$dominator && count($tribes_per_user) > 0) {
			$max = 0;
			foreach ($unit_amount_per_user as $uid => $amount) {
				if ($amount > $max) {
					$max = $amount;
					$dominator = $uid;
				}
			}
		}

		if (!$dominator) {
			// Sind keine Truppen im Sektor,
			// wird die Häuseranzahl verglichen
			$buildings_per_uid = array();
			$max = 0;
			foreach ($buildings_per_user as $uid => $data) {
				if (count($data) > $max) {
					$max = count($data);
					$dominator = $uid;
				}
			}
		}
		return $dominator;
	}


	/**
	 * Setzt den Dominator in dem angegeben Sektor.
	 *
	 * Ggf. kann hierdurch ein Dominatorwechsel entstehen, der protokolliert
	 * wird und Ereignisse auslösen kann. Deswegen muss $t_stamp angegeben
	 * werden, um solche Ereignisse dem richtigen Zeitpunkt zuordnen zu können.
	 *
	 * $fight gibt dabei an, weil wichtig für Ereignisse, ob das ganze im oder
	 * direkt nach einem Kampf passiert ist, oder nicht.
	 *
	 * Es wird zurückgegeben, ob ein Dominatorwechsel stattgefunden hat.
	 *
	 * @param uint $sector_id
	 * @param uint $uid
	 * @param uint $t_stamp
	 * @param bool $fight
	 * @return bool
	 */
	private function set_dominator($sector_id, $uid, $t_stamp, $fight) {
		ValueChecker::sector_id($sector_id, 'sector_id');
		ValueChecker::id($uid, 'uid', true);
		ValueChecker::t_stamp($t_stamp, 't_stamp');
		ValueChecker::bool($fight, 'fight');

		if (self::DEBUG_VERBOSE > 4)
			error_log("set_dominator($sector_id, $uid, $t_stamp, $fight)");
		$old_dominator_id = $this->computed_sector_data
			->get_dominator($sector_id);

		/**
		 * Wenn ein Dominatorwechsel stattgefunden hat, dann dieses
		 * Ereignis protokollieren.
		 *
		 * $uid > 0 checkt dabei, ob ein Spieler oder der Host einen Sektor
		 * eingenommen hat. Bei <= 0 wäre der Sektor neutral geworden.
		 */
		if (($uid > 0) && ($uid != $old_dominator_id)) {
			/**
			 * GGf. Medallie verleihen (1)
			 */
			$medals = Medals::get_medals(
				IMedals::TYPE_ATTACK_SUCCESSFUL);
			$medals = $medals[IMedals::TYPE_ATTACK_SUCCESSFUL];

			if (!$fight && ($uid != HostControl::USER_HOST_ID)) {
				$ev_log = EventLog::get_instance($uid, $this->get_world_id(),
					$this->get_map_id()
				);
				/**
				 * Der Sektor war vorher unbesetzt: Es gab keinen Kampf
				 */

				$ev_log->add_entry(
						EventLog::TYPE_SECTOR_WON_NO_BATTLE,
						$sector_id,
						'',
						$t_stamp
					);
				/**
				 * GGf. Medallie verleihen (2)
				 */
				$c = $ev_log->get_total_event_amount(
					EventLog::TYPE_SECTOR_WON_NO_BATTLE);


				if (isset($medals[self::ATTDEF_DOM_GROUP_NOBODY][$c])) {
					$medal_id = $medals[self::ATTDEF_DOM_GROUP_NOBODY][$c];

					UserMedals::get_instance($uid)->add_medal(
						$medal_id, $t_stamp
					);

					/**
					 * User Über Medalliengewinn informieren
					 */
					EventMessages::get_instance($uid)->add_entry(
						EventLog::TYPE_SECTOR_WON_NO_BATTLE,
						$sector_id,
						'',
						$t_stamp,
						0,
						$medal_id
					);
				}
			}
			else if ($uid && $old_dominator_id) {
				if ($uid != HostControl::USER_HOST_ID) {
					EventLog::get_instance($uid, $this->get_world_id(),
						$this->get_map_id()
					)->add_entry
					(
						EventLog::TYPE_SECTOR_BATTLE_WON,
						$sector_id,
						User::get_instance_by_id($old_dominator_id)
							->get_nick(),
						$t_stamp
					);
				}

				if ($old_dominator_id != HostControl::USER_HOST_ID) {
					if ($uid != HostControl::USER_HOST_ID) {
						/**
						 * GGf. Medallie verleihen (2)
						 */
						$ev_log = EventLog::get_instance($uid,
							$this->get_world_id(), $this->get_map_id()
						);
						$c = $ev_log->get_total_event_amount(
							EventLog::TYPE_SECTOR_BATTLE_WON, 0,
							User::get_host_user()->get_nick()
						);
						$c_all = $ev_log->get_total_event_amount(
							EventLog::TYPE_SECTOR_BATTLE_WON);
						$c = $c_all - $c;

						if (isset($medals[self::ATTDEF_DOM_GROUP_USER][$c])) {
							$medal_id = $medals[self::ATTDEF_DOM_GROUP_USER][$c];

							UserMedals::get_instance($uid)->add_medal(
								$medal_id, $t_stamp
							);

							/**
							 * User Über Medalliengewinn informieren
							 */
							EventMessages::get_instance($uid)->add_entry(
								EventLog::TYPE_SECTOR_BATTLE_WON,
								$sector_id,
								User::get_instance_by_id($old_dominator_id)
									->get_nick(),
								$t_stamp,
								0,
								$medal_id
							);
						}
					}
					EventLog::get_instance($old_dominator_id,
						$this->get_world_id(), $this->get_map_id())->add_entry
					(
						EventLog::TYPE_SECTOR_BATTLE_LOST,
						$sector_id,
						User::get_instance_by_id($uid)
							->get_nick(),
						$t_stamp
					);
				}
				else {
					if (is_easter()) {
						EventMessages::get_instance($uid)->add_entry(
							EventMessages::TYPE_FOUND_EASTER_EGG_IN_DARK,
							0,
							'',
							$t_stamp
						);
					}
					/**
					 * GGf. Medallie verleihen (2)
					 */
					$ev_log = EventLog::get_instance($uid,
						$this->get_world_id(),
						$this->get_map_id()
					);
					$c = $ev_log->get_total_event_amount(
						EventLog::TYPE_SECTOR_BATTLE_WON, 0,
						User::get_host_user()->get_nick());

					if (isset($medals[self::ATTDEF_DOM_GROUP_HOST][$c])) {
						$medal_id = $medals[self::ATTDEF_DOM_GROUP_HOST][$c];

						UserMedals::get_instance($uid)->add_medal(
							$medal_id, $t_stamp
						);

						/**
						 * User Über Medalliengewinn informieren
						 */
						EventMessages::get_instance($uid)->add_entry(
							EventLog::TYPE_SECTOR_BATTLE_WON,
							$sector_id,
							User::get_instance_by_id($old_dominator_id)
								->get_nick(),
							$t_stamp,
							0,
							$medal_id
						);
					}
				}
			}
		}
		$this->computed_sector_data->set_dominator($sector_id, $uid);

		return ($uid != $old_dominator_id);
	}

	/**
	 * Berechnet zyklenweise den Kampf innerhalb eines Sektors.
	 *
	 * @param [0,1,99] $sector_id
	 *
	 * @see MapCompute::compute_map()
	 */
	private function compute_fight_in_sector($sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		/**
		 * Summe aller Tribes (HP) mit gleicher ID.
		 * @var uint[]
		 */
		$joint_tribes = array();
		/**
		 * Summe aller Tribes (HP) eines Users
		 * @var uint[]
		 */
		$unit_amount_per_user = array();
		/**
		 * Alle Tribe-Objekte eines Users
		 * @var TribeOnMap[]
		 */
		$tribes_per_user = array();
		/**
		 * Summe aller Gebäude (SP) mit gleicher ID
		 * @var uint[]
		 */
		$joint_building = array();
		/**
		 * Anzahl aller Einheiten (HP/SP) im Sektor
		 * @var uint
		 */
		/**
		 * Alle Buildings-Objekte eines Users
		 * @var BuildingOnMap[]
		 */
		$buildings_per_user = array();
		$total_unit_amount = 0;
		/**
		 * Welcher Spieler welche Einheiten bekämpft
		 */
		$enemies = array();
		$fighting = false;
		$sector_status_tstamp = 0;
		$cur_dominator = $this->computed_sector_data->get_dominator($sector_id);

		$sector_c = new WorldCoordinates(
			$this->get_world_id(), $this->get_map_id(), $sector_id
		);
		if (self::DEBUG_VERBOSE > 0)
			error_log("Cycles to compute: ".$this->cycles_to_compute);
		for ($cycle = 1; $cycle <= $this->cycles_to_compute; $cycle++) {
			$tribes = $this->computed_tribes_data->get_all_in_sector(
								$sector_id, $cycle);
			if (self::DEBUG_VERBOSE > 0)
				error_log("Kampf? Cycle $cycle, Sektor $sector_id");
			/**
			 * in $tribes sind jetzt also in einem Array alle Tribes, die in
			 * diesem Sektor in diesem Zyklus dazugekommen sind.
			 */

			/**
			 * Es wird zunächst davon ausgegangen, dass die Parteien noch
			 * dieselben sind, wie im Zyklus davor. Bei Änderung, müssen Gegner
			 * neu bestimmt werden
			 */
			$party_constellation_changed = false;
			// 1. Joinen - wie viele sind aktuell insgesamt da
			foreach ($tribes as $tid => $amount_to_add) {
				try {
					$tribe = TribeOnMap::get_instance($tid,
						$sector_id, $this->get_map_id(),
						$this->get_world_id()
					);
				}
				catch (Exception $e) {
					/**
					 * Tribe existiert nicht: überspringen
					 */
					unset($joint_tribes[$tid]);
					$this->computed_tribes_data->set_computed_amount(
							$tid, $sector_id, 0
					);
					continue;
				}
				if ($tribe->is_moving()) {
					/**
					 * Wenn sich die Einheit bewegt, kann sie nicht mehr im
					 * Sektor sein - überspringen.
					 * Das kommt regulär dann vor, wenn zwischen dem letzten
					 * Compute und jetzt die Bewegung initiiert wurde.
					 */
					unset($joint_tribes[$tid]);
					$this->computed_tribes_data->set_computed_amount(
							$tid, $sector_id, 0
					);
					continue;
				}

				/**
				 * Initialisierungen
				 */
				if (!isset($joint_tribes[$tid]))
					$joint_tribes[$tid] = 0;

				$uid = $tribe->get_uid();

				$old_amount = $joint_tribes[$tid];
				if (($old_amount == 0) && ($amount_to_add > 0))
				{ // Wenn bislang dieser Tribe noch nicht im Sektor gewesen ist
					if (!isset($unit_amount_per_user[$uid]))
					{ // Der User ist bislang noch nicht im Sektor gewesen
						if (count($unit_amount_per_user) > 0)
						{ // Es sind nun mehrere User im Sektor => Kampf
							$fighting = true;
							$sector_status_tstamp =
								$this->cycle_to_time(
									$cycle, $this->last_compute_time
								);
						}
					}
					// Ein Spieler ist dazugekommen => Andere Parteien
					$party_constellation_changed = true;
				}

				if (!isset($unit_amount_per_user[$uid]))
					$unit_amount_per_user[$uid] = 0;

				if (!isset($tribes_per_user[$uid]))
					$tribes_per_user[$uid] = array();

				if (($joint_tribes[$tid] + $amount_to_add) <= 0)
				{ // Alle Einheiten des Typs abgezogen/geflohen
					if (self::DEBUG_VERBOSE > 0)
						error_log("TribeOnMap mit ID $tid geflohen?");
					$amount_to_add = $joint_tribes[$tid] * (-1);
					unset($joint_tribes[$tid]);	// tribe nicht mehr da

					// Geflohene Einheit aus dem Gegnerarray raushauen
					foreach ($unit_amount_per_user as $uid => $jnk) {
						if (is_array($enemies[$uid])) {
							$key = array_search($tid, $enemies[$uid]);
							if ($key !== false)
								unset ($enemies[$my_tribe->get_uid()][$key]);
						}
					}
				}
				$total_unit_amount += $amount_to_add;
				$joint_tribes[$tid] += $amount_to_add;
				$unit_amount_per_user[$uid] += $amount_to_add;
				if (!in_array($tribe, $tribes_per_user[$uid]))
				{
					$tribes_per_user[$uid][] = $tribe;
				}

				if ($unit_amount_per_user[$uid] <= 0) {
					// Ein Spieler ist weggegangen => Andere Parteien
					$party_constellation_changed = true;
					unset($unit_amount_per_user[$uid]);	// Spieler hat nix mehr
					if (count($unit_amount_per_user) < 2) {
						$fighting = false;// kein Kampf mehr, nur noch eine Partei
						$sector_status_tstamp =
							$this->cycle_to_time(
								$cycle, $this->last_compute_time
							);
					}
				}
			}

			/**
			 * Nun das Joinen für die Gebäude
			 */
			$buildings = $this->computed_buildings_data->get_all_in_sector(
				$sector_id, $cycle
			);
			foreach ($buildings as $bid => $amount_to_add) {
				$ulc_x = floor($bid/1000);
				$ulc_y = $bid % 1000;
				try {
					$building = BuildingOnMap::get_instance(
						$sector_c,
						$ulc_x,
						$ulc_y
					);
					$uid = $building->get_uid();
					// Gebäude dem User zuschreiben
					if (empty($buildings_per_user[$uid]) ||
						!in_array($building, $buildings_per_user[$uid]))
					{
						$buildings_per_user[$uid][] = $building;
					}
					$joint_building[$bid] += $amount_to_add;
				}
				catch (Exception $e) {
					/**
					 * Gebäude existiert nicht (mehr)
					 */
					$this->computed_buildings_data->set_computed_amount(
						$bid, $sector_id, 0
					);
				}
			}

			/**
			 * $joint_tribes enthält jetzt einen Eintrag für jeden
			 * Tribe, der aktuell (also seit Zyklus 1 bis jetzt) im Sektor ist,
			 * und zu wievielt.
			 * $tribes dagegen nur einen Eintrag, für die Tribes, die gerade
			 * in diesem Zyklus dazugekommen sind. $tribes ist also aktuell
			 * uninteressant und wir verwenden nun $joint_tribes für alle
			 * weiteren Berechnungen.
			 * [Anm. Natürlich wäre es denkbar, $tribes mit $joint_tribes zu
			 * überschreiben, aber ich denke, es schade der Übersicht.]
			 */

			// 2. Gegner bestimmen: Welcher User bekämpft konkret welche Einheiten
			if ($fighting) {
				if (self::DEBUG_VERBOSE > 0)
					error_log("Fighting...");
				if ($party_constellation_changed)
				{ // Wird nur gemacht, wenn Kampf und andere Parteien als im Vorzyklus
					$enemies = array();
					foreach ($unit_amount_per_user as $uid => $jnk) {
						foreach ($joint_tribes as $enemy_tid => $jnk2) {
							try {
								$enemy = TribeOnMap::get_instance($enemy_tid,
									$sector_id,	$this->get_map_id(),
									$this->get_world_id()
								);
							}
							catch (Exception $e) {
								/**
								 * Tribe existiert nicht: überspringen
								 */
								unset($joint_tribes[$enemy_tid]);
								$this->computed_tribes_data->set_computed_amount(
									$enemy_tid, $sector_id, 0
								);
								continue;
							}

							// Wenn beide Tribes demselben gehören => keine Gegner
							if ($uid == $enemy->get_uid())
								continue;
							$enemies[$uid][] = $enemy_tid;
						}
					}
				}

				// 3. Kampf :)
				/**
				 * Im Kampf wird zunächst jeder Stamm durchgegangen und dann dessen
				 * Gegner. Es kämpft also jeder Tribe gegen jeden Gegner.
				 * Um die Variablenbenennungen übersichtlicher zu halten, wird
				 * alles so dargestellt, als ob ICH der Tribe bin, der in der
				 * äußeren schleife ist, und MEIN Gegner in der inneren. Dadurch
				 * beginnen die meisten Variablen mit "my_" oder "enemy_" :)
				 */
				$dead_enemies = array();//Pro Gegner und pro Killer ein Eintrag
				foreach ($joint_tribes as $my_tid => $my_amount) {
					if (!$my_amount) {
						unset($joint_tribes[$my_tid]);
						continue;	// sollte nicht vorkommen, vorher geprüft
					}
					try {
						$my_tribe = TribeOnMap::get_instance($my_tid,
							$sector_id, $this->get_map_id(),
							$this->get_world_id()
						);
					}
					catch (Exception $e) {
						/**
						 * Tribe existiert nicht: überspringen
						 * sollte nicht vorkommen.
						 */
						unset($joint_tribes[$my_tid]);
						$this->computed_tribes_data->set_computed_amount(
							$my_tid, $sector_id, 0
						);
						continue;
					}

					$total_enemy_amount = 0;	// Wie viele Gegner hat mein Stamm?
					foreach ($unit_amount_per_user as $uid => $amount)
						if ($my_tribe->get_uid() != $uid)
							$total_enemy_amount += $amount;

					$units_fighting = 0;
					if (!is_array($enemies[$my_tribe->get_uid()]))
						$enemies[$my_tribe->get_uid()] = array();

					foreach ($enemies[$my_tribe->get_uid()] as $enemy_tid) {
						if (!isset($dead_enemies[$enemy_tid]))
							$dead_enemies[$enemy_tid] = array();	// init
						$my_t_uid = $my_tribe->get_uid();
						if (!isset($dead_enemies[$enemy_tid][$my_t_uid]))
							$dead_enemies[$enemy_tid][$my_t_uid] = 0;
						unset($my_t_uid);

						try {
							$enemy_tribe = TribeOnMap::get_instance($enemy_tid,
								$sector_id, $this->get_map_id(),
								$this->get_world_id()
							);
						}
						catch (Exception $e) {
							/**
							 * Tribe existiert nicht: überspringen
							 * sollte nicht vorkommen.
							 */
							$this->computed_tribes_data->set_computed_amount(
								$enemy_tid, $sector_id, 0
							);
							continue;
						}

						$enemy_amount = $joint_tribes[$enemy_tid];
						if (!$enemy_amount)
							continue;	// sollte auch nicht vorkommen


						// Wie viele Gegner hat der Gegner?
						$total_enemys_enemy_amount = 0;
						foreach ($unit_amount_per_user as $uid => $amount)
							if ($enemy_tribe->get_uid() != $uid)
								$total_enemys_enemy_amount += $amount;

						$enemy_proportion = $my_amount / $total_enemys_enemy_amount;
						$my_proportion = $enemy_amount / $total_enemy_amount;
						$my_fighting_amount = $my_amount * $my_proportion;
						$enemy_fighting_amount = $enemy_amount * $enemy_proportion;

						/**
						 * Nun wird anhand der Gesamtgegner und Gesamtfreunde, sowie
						 * anhand des eigenen Angriffs- und des gegnerischen
						 * Verteidigungswerts ein Kampfwert namens "Unit Power"
						 * errechnet. Dieser ist ausschlaggebend für den Schaden,
						 * der jetzt von unserer Einheit angerichtet wird.
						 */
						if ($my_tribe->get_att() == 0)
						{	// Fall 1: Trivial: Kein Att, keine Power
							$unit_power = 0;
						}
						else
						{	// Fall 2: (a * n) / (d * m)
							$my_att = $my_tribe->get_att();
							$enemy_def = $enemy_tribe->get_def();
							if (!$enemy_def)	// DB-Fehler? Sollte nicht vorkommen
								$enemy_def = 1;
							if ($enemy_fighting_amount == 0)
							{	// Sollte nicht vorkommen! Besser den Gegner nicht
								// angreifen und ne Errormail schicken
								$unit_power = 0;
								errormail(
									"In MapCompute::compute_fight_in_sector($sector_id): ".
									"Line ".__LINE__.": \$enemy_fighting_amount==0!"
								);
							}
							else {
								$unit_power = $my_att * $my_fighting_amount /
											($enemy_def * $enemy_fighting_amount);
							}
							// Unit Power mit Grenzwert versehen - sonst wird's
							// zu krass...
							$unit_power = pow($unit_power, 0.75);
						}
						/**
						 * Jetzt wird zugehauen. Die HP des Gegners entscheiden,
						 * wie viele davon nun abdanken dürfen.
						 */
						$enemy_hp = $enemy_tribe->get_hp();
						if (!$enemy_hp)
							$enemy_hp = 1;

						$dead_guys = $unit_power * $enemy_fighting_amount
							* $this->cycle_duration / ($enemy_hp * 500);
						$dead_guys *= $this->fightspeed;
						/**
						 * Nachdem nun so und so viele sterben sollen, werden noch
						 * ein paar Multiplikatoren miteinbezogen, die diese Anzahl
						 * erhöhen oder verringern können.
						 */
						$dead_guys *= self::get_stampede_multiplier($enemy_amount,
							$total_enemys_enemy_amount);
						$dead_guys *= self::get_total_unit_amount_multiplier(
							$total_unit_amount);
						$dead_guys *= self::get_ssp_multiplier($my_tribe,
							$enemy_tribe);
						$dead_guys *= self::get_counter_abil_multiplier($my_tribe,
							$enemy_tribe);
						/**
						 * $dead_guys sind die Leute, die durch diesen einen Schlag
						 * von "meinem" Tribe gestorben sind. Die Gesamtsumme
						 * für den Zyklus lässt sich über $dead_enemies errechnen.
						 * Damit nicht in einem Zyklus mehr sterben können, als da
						 * sind, wird nun folgendes geprüft:
						 */
						$amount_of_enemy_tribe_dead
							 = array_sum($dead_enemies[$enemy_tid]);
						$amout_of_enemy_tribe_still_alive =
							$enemy_amount - $amount_of_enemy_tribe_dead;
						if ($dead_guys > $amout_of_enemy_tribe_still_alive) {
							$dead_guys = $amout_of_enemy_tribe_still_alive;
						}
						$dead_enemies[$enemy_tid][$my_tribe->get_uid()]
							+= $dead_guys;

						/**
						 * Statistische Daten erheben
						 */
						$this->computed_tribes_data->add_killed($my_tid,
							$sector_id, $dead_guys);
						$this->computed_map_data->add_enemy_units_killed(
							$my_tribe->get_uid(), $dead_guys);

					} // end of "foreach ($enemies[$my_tribe->get_uid()] as $enemy_tid)"
				} // end of "foreach ($joint_tribes as $tid => $my_amount)"

				// 4. Alle gestorbenen durchgehen
				foreach ($dead_enemies as $enemy_tid => $killed_array) {
					$dead_guys_total = array_sum($killed_array);
					$enemy_amount = $joint_tribes[$enemy_tid];

					if (self::DEBUG_VERBOSE > 2) {
						error_log ("Dead enemy TID: $enemy_tid, ".
							"dead amount: $dead_guys_total, total amount: ".
							$enemy_amount);
					}
					try {
						$enemy_tribe = TribeOnMap::get_instance($enemy_tid,
							$sector_id, $this->get_map_id(),
							$this->get_world_id()
						);
					}
					catch (Exception $e) {
						/**
						 * Tribe exisitert nicht: überspringen
						 * sollte nicht vorkommen.
						 */
						unset($dead_enemies[$enemy_tid]);
						$this->computed_tribes_data->set_computed_amount(
							$enemy_tid, $sector_id, 0
						);
						continue;
					}
					if ($dead_guys_total >= $enemy_amount) {
						if (self::DEBUG_VERBOSE > 1)
							error_log("All units of Tribe $enemy_tid are dead!");
						$dead_guys_total = $enemy_amount;
						/**
						 * Um herauszufinden, wer ihn getötet hat, gibt es
						 * mehrere Möglichkeiten. Eine wäre z.B. zu sagen, dass
						 * derjenige ihn getötet hat, der den letzten Schlag tat
						 * innerhalb des Zyklus' tat. Das macht aber wenig Sinn,
						 * weil die Reihenfolge, in der die Jungs hauen dürfen,
						 * nicht vorhersagbar ist.
						 * Daher bekommt derjenige den Kill, der in dem Zyklus
						 * am meisten Schaden angerichtet hat.
						 */
						arsort($killed_array);	// Geht das eleganter/schneller?
						reset($killed_array);
						$killer = each($killed_array);
						$this->kill_tribe(
							$enemy_tribe,
							$this->cycle_to_time($cycle, $this->last_compute_time),
							$sector_id,
							$killer['key']
						);

						$joint_tribes[$enemy_tid] = 0;

						// Gestorbene Einheit aus allen Arrays raushauen
						foreach ($enemies as $uid => $jnk) {
							if (is_array($enemies[$uid])) {
								$key = array_search($enemy_tid, $enemies[$uid]);
								if ($key !== false)
									unset ($enemies[$uid][$key]);
							}
						}

						if (is_array($tribes_per_user[$enemy_tribe->get_uid()])) {
							$key = array_search($enemy_tribe,
								$tribes_per_user[$enemy_tribe->get_uid()]);
							if ($key !== false)
								unset ($tribes_per_user[$enemy_tribe->get_uid()][$key]);
						}
					}
					else {
						$joint_tribes[$enemy_tid] -= $dead_guys_total;
					}
					$this->computed_tribes_data->add_died($enemy_tid,
							$sector_id, $dead_guys_total);

					$total_unit_amount -= $dead_guys_total;
					$unit_amount_per_user[$enemy_tribe->get_uid()]
						 -=	$dead_guys_total;

					if (self::DEBUG_VERBOSE > 1)
						error_log("User {$enemy_tribe->get_uid()} hat noch ".
							$unit_amount_per_user[$enemy_tribe->get_uid()].
							" Units im Sektor.");
					// Wenn der gegn. User keine Einheiten mehr im Sektor hat
					if ($unit_amount_per_user[$enemy_tribe->get_uid()] <= 0) {
						unset($unit_amount_per_user[$enemy_tribe->get_uid()]);
						$party_constellation_changed = true;
						if (count($unit_amount_per_user) < 2) {
							$fighting = false;// kein Kampf mehr, nur noch eine Partei
							if (self::DEBUG_VERBOSE > 2)
								error_log("Setting \$fighting=false");
							$sector_status_tstamp =
								$this->cycle_to_time(
									$cycle, $this->last_compute_time
								);
						}
					}
				}
			}	// end of "if ($fighting)"

			/**
			 * Während des Kampfes gibt es keine Rohstoffe und der Dominator
			 * wechselt auch nicht - erst wenn der Kampf vorrüber ist.
			 * Also: "Wenn nicht gekämpft wird, Dominator neu
			 * bestimmen und Bio hinzufügen"
			 *
			 * ACHTUNG: Else wäre hier falsch (war alter Bug), weil $fighting
			 * sich im if-case oben auf false gesetzt werden kann und das hier
			 * dann noch ausgeführt werden muss.
			 */
			if (!$fighting) {
				/**
				 * TODO Auch hier heilen
				 */
				if (self::DEBUG_VERBOSE > 0)
					error_log("no fight in cycle $cycle in sector $sector_id");
				// 5. Dominator neu bestimmen
				$new_dominator = self::calc_dominator($tribes_per_user,
					$buildings_per_user, $unit_amount_per_user);

				// 6. Gebäude übernehmen lassen
				Buildings::assign_buildings_in_sector_to_user(
					$sector_c, $new_dominator
				);

				// 7. Bio für den Zyklus hinzufügen
				$duration_in_hours = $this->cycle_duration/3600;
				if ($new_dominator) {
					$this->add_map_bio($sector_id, $new_dominator,
						$duration_in_hours);
				}

				$this->set_dominator(
					$sector_id,	$new_dominator,
					$this->cycle_to_time($cycle, $this->last_compute_time),
					true
				);
				$cur_dominator = $new_dominator;
			}
		} // end of "foreach ($cycle)"

		if (self::DEBUG_VERBOSE > 2)
			error_log("All cycles computed. Fight computation done.");
		/**
		 * Jetzt, am Ende der Kampfberechnung, wird der Dominator zur Anzeige
		 * festgelegt. Sprich: Wem gehört der Sektor jetzt?
		 *
		 * NEIN! Das ist nun deaktiviert, da der Sektor im Falle eines Kampfes
		 * immer vom Besitzer dominiert bleibt (neue Regel)
		 */
		// Dominator neu bestimmen
		/** DEAKTIVIERT
		if (!$cur_dominator)
			$cur_dominator =
				self::calc_dominator($tribes_per_user, $buildings_per_user,
				$unit_amount_per_user);
		$this->computed_sector_data->set_dominator($sector_id, $cur_dominator);
		*/

		foreach ($joint_tribes as $tid => $amount) {
			if (self::DEBUG_VERBOSE > 2)
				error_log("Computed tribes: $tid => $amount in $sector_id");
			$this->computed_tribes_data->set_computed_amount(
				$tid, $sector_id, $amount
			);
			$this->computed_tribes_data->set_tribe_is_camouflaged(
				$tid, $sector_id, ($tribe->can_camouflage() && !$fighting)
			);
		}
		foreach ($joint_building as $bid => $amount) {
			if ($amount <= 0) {
//				pv_mail("p.kemmeter@gausus.com",
//					"Gebäude-Anzahl <= 0",
//					"MapCompute: Zeile: ".__LINE__.":\n".
//					var_export($joined_amount_per_building, true)
//				);
			}
			$this->computed_buildings_data->set_computed_amount(
				$bid, $sector_id, $amount
			);
		}

		if ($fighting) {
			$this->computed_sector_data->start_fight(
				$sector_id,
				$sector_status_tstamp
			);
		}
		else {
			// TODO weitere Checks für Explore
			$this->computed_sector_data->end_fight(
				$sector_id,
				$sector_status_tstamp
			);
		}
	}

	/**
	 * Wenn nur noch wenige Einheiten im Sektor sind, werden diese panisch und
	 * sterben deswegen schneller. Der Multiplikator hängt von der Anzahl der
	 * vielleicht panisch werdenden Einheiten und der Gesamtanzahl deren
	 * Gegner ab (Nach dem Motto: wenige gegen viele => Panik)
	 *
	 * @param ufloat $unit_amount			Anzahl der vielleicht panisch
	 * 										werdenden
	 * @param ufloat $total_enemy_amount	Gesamtanzahl aller Gegner
	 * @return ufloat
	 */
	private static function get_stampede_multiplier($unit_amount,
		$total_enemy_amount)
	{
		ValueChecker::float($unit_amount, 'unit_amount', 0);
		ValueChecker::float($total_enemy_amount, 'total_enemy_amount', 0);

		// Wenn genug eigene oder keien Gegner, wird man auch nicht panisch
		if (($unit_amount >= 100) || ($total_enemy_amount <= 0))
			return 1;

		// Sonst wird der Grad berechnet:
		$ratio = $unit_amount / $total_enemy_amount;
		if ($ratio < 0.0001)
			return 50;
		if ($ratio < 0.001)
			return 30;
		if ($ratio < 0.005)
			return 20;
		if ($ratio < 0.01)
			return 10;
		return 1;
	}

	/**
	 * Sind insgesamt nur wenige Einheiten in einem Sektor, wird die Zeit
	 * gerafft. Das Kampfsystem würde sonst nicht genug Schaden anrichten und
	 * die Kämpfe mit wenigen Einheiten würden sonst unerwartet lange dauern.
	 *
	 * @param ufloat $total_unit_amount	Gesamtanzahl der Einheiten
	 * @return ufloat
	 */
	private static function get_total_unit_amount_multiplier($total_unit_amount)
	{
		ValueChecker::float($total_unit_amount, 'total_unit_amount', 0);

		if ($total_unit_amount < 10)
			return 10;
		if ($total_unit_amount < 100)
			return 5;
		if ($total_unit_amount < 500)
			return 2;
		if ($total_unit_amount < 10000)
			return 1;
		if ($total_unit_amount < 20000)
			return 0.7;
		if ($total_unit_amount < 40000)
			return 0.6;

		return 0.5;
	}

	/**
	 * Gibt den Multiplikator zurück, der durch ggf. existierenden SSP-Vorteil
	 * entsteht. Dabei wird ausgegangen, dass $attacking_tribe der Tribe ist,
	 * der $defending_tribe angreift und deswegen für diesen Angriff einen
	 * SSP-Vorteil für sich entscheiden könnte.
	 *
	 * @param TribeOnMap $attacking_tribe
	 * @param TribeOnMap $defending_tribe
	 * @return ufloat
	 */
	private static function get_ssp_multiplier(TribeOnMap $attacking_tribe,
		TribeOnMap $defending_tribe)
	{
		if ($attacking_tribe->has_ssp_advantage_against($defending_tribe))
			return 1.3;

		return 1;
	}

	/**
	 * Gibt den Multiplikator zurück, der durch Counter-Abilities entsteht.
	 *
	 * @param TribeOnMap $attacking_tribe
	 * @param TribeOnMap $defending_tribe
	 * @return ufloat
	 */
	private static function get_counter_abil_multiplier(
		TribeOnMap $attacking_tribe, TribeOnMap $defending_tribe)
	{
		return 1;	// coming soon
	}

	/**
	 * Löscht einen Tribe aus einem Sektor (z.B. weil er im Kampf gestorben ist,
	 * aber auch, wenn er abgezogen wurde oder floh).
	 *
	 * @param TribeOnMap	$tribe	Der Tribe, der gelöscht/getötet werden soll
	 * @param uint $t_stamp			Der Zeitpunkt, als das Ereignis geschah
	 * @param uint $killer_uid		Ggf derjenige, der getötet hat
	 */
	private function kill_tribe(TribeOnMap $tribe, $t_stamp, $sector_id,
		$uid=0)
	{
		ValueChecker::t_stamp($t_stamp, 't_stamp');
		ValueChecker::sector_id($sector_id, 'sector_id');
		ValueChecker::id($uid, 'uid', true);

		if ($tribe->get_uid() != HostControl::USER_HOST_ID) {
			$ev_log = EventLog::get_instance($tribe->get_uid(),
				$this->get_world_id(), $this->get_map_id());
			$ev_log->add_entry(
				EventLog::TYPE_TRIBE_DIED,
				$sector_id,
				$tribe->get_name(),
				$t_stamp
			);

			if ($uid > 0) {
				$ev_log = EventLog::get_instance($uid,
					$this->get_world_id(), $this->get_map_id());
				$ev_log->add_entry(
					EventLog::TYPE_TRIBE_KILLED,
					$sector_id,
					$tribe->get_name(),
					$t_stamp
				);
			}

			/**
			 * Kleidung, die er trug, wieder in die Schatzkiste legen.
			 */
			try {
				$clothes_array = $tribe->get_clothes()->to_php_array();

				$amount_of_clothes = 0;
				foreach ($clothes_array as $type => $id) {
					if (isset($id)) {
						if ($id > 0)
							$amount_of_clothes++;
						else
							unset($clothes_array[$type]);
					}
				}
				if ($amount_of_clothes > 0) {
					/*error_log("Trying to put clothes of Tribe ".
									"{$tribe->get_id()} back to chest");*/
					Server::get_instance(CONF_PAYMENT_SERVER_URL,
						CONF_MAMMUN_AUTH_SECRET)
							->put_items_into_chest(
								$tribe->get_uid(),
								$clothes_array
							);
					//error_log("done.");

					$ev_log = EventLog::get_instance($tribe->get_uid(),
						$this->get_world_id(), $this->get_map_id());
					$ev_log->add_entry(
						EventLog::TYPE_CLOTHES_OF_UNIT_GAINED,
						$amount_of_clothes,
						$tribe->get_name(),
						$t_stamp
					);
				}
			}
			catch (Exception $e) {
				error_log($e->getMessage());
			}
			catch (IllegalArgumentException $e) {
				error_log($e->getMessage());
			}
		}

		if (self::DEBUG_VERBOSE > 0)
			error_log("killing tribe with ID ".
				$tribe->get_id().
			 	"    at $t_stamp"
		);
		$tribe->remove_from_db();
	}

	/**
	 * Diese Funktion prüft alle Spieler auf Verlustbedingung und entfernt alle
	 * Spieler, die verloren haben von der Insel.
	 */
	private function check_for_defeated_players() {
		$uid_has_units = null;
		$uid_domination = null;
		for ($sector_id = 0; $sector_id < 100; $sector_id++) {
			$tribes = $this->get_tribes_in_sector($sector_id);
			foreach ($tribes as $tribe) {
				$uid_has_units[$tribe->get_uid()] = true;
			}
			$uid_domination[
				$this->computed_sector_data->get_dominator($sector_id)] = true;
		}
		$map_players = MapPlayers::get_instance($this->world_id, $this->id);
		$uids = $map_players->get_uids_on_map(true);
		foreach ($uids as $uid) {
			if (!isset($uid_domination[$uid]) && !isset($uid_has_units[$uid])) {
				/**
				 * Ein Spieler hat eine Schonfrist auf der Karte.
				 * Dadurch kann sich die Engine erst stabilisieren, bevor sie
				 * den User vielleicht versehentlich von der Karte wirft.
				 * Ein paar Minuten sollten hier aus technischer Sicht reichen.
				 */
				if ((Time::get_t_now() - $map_players->get_start_time($uid))
					> Time::TWO_DAYS)
				{
					try {
						$map_players->leave_map($uid);
					}
					catch(Exception $e) {
						error_log("ERROR!!! ".$e->getCode.', '.$e->getMessage());
					}
					error_log("User with ID $uid has to leave the island ".
						"(Map ID {$this->get_id()}. ".
						"He has no forces and no territory...");
				}
			}
		}
	}

	/**
	 * Berechnet, wie lange ein Zyklus dauert, wenn von $compute_time1 bis
	 * $compute_time2 berechnet werden soll (also
	 * |$compute_time2-$compute_time1|+1 lang).
	 *
	 * @param uint $compute_time1	Startzeitpunkt der Berechnung
	 * @param uint $compute_time2	Endzeitpunkt der Berechnung
	 * @param uint &$cycles_to_compute	Wie viele Zyklen von der Dauer
	 * 									die Gesamtdauer ergeben
	 * @return uint Zyklusdauer in Sekunden
	 */
	public static function compute_cycle_duration($compute_time1,
		$compute_time2, &$cycles_to_compute)
	{
		ValueChecker::t_stamp($compute_time1, 'compute_time1');
		ValueChecker::t_stamp($compute_time2, 'compute_time2');


		$time_to_compute = abs($compute_time2 - $compute_time1) + 1;
		$cycle_duration = 0;
		if ($time_to_compute < 30)
			$cycle_duration = $time_to_compute;
		else if ($time_to_compute < 300)
			$cycle_duration = 30;
		else	// Chris' magische Formel:
			$cycle_duration = ($time_to_compute/($time_to_compute/
				($time_to_compute/3000 + 100) + 20)) * 3;

		$cycles_to_compute = floor($time_to_compute / $cycle_duration);
		if ($cycles_to_compute > 5000)
			$cycles_to_compute = 5000;
		$cycle_duration = $time_to_compute / $cycles_to_compute;
		$cycle_duration *= Mapsettings::get_instance()->get_timefactor();

		return $cycle_duration;
	}

	/**
	 * Errechnet den aktuellen Dominator im übergebenen Sektor neu. Muss dann
	 * aufgerufen werden, wenn nach der Kartenberechnung noch Einheiten
	 * verschwinden oder hinzukommen (Hinzukommen wäre z.B ein Gebäudeupgrade,
	 * verschwinden wäre z.B. Movement).
	 *
	 * @param [0,1,...99] $sector_id
	 */
	private function recalculate_current_dominator($sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		/**
		 * Wenn noch gekämpft wird, dann kein recalc machen, weil:
		 * Neue Dominatorregeln: Während des Kampfes ändert sich die Dominanz
		 * nie, erst, wenn der Kampf vorrüber ist.
		 */
		if ($this->computed_sector_data->get_fight_in_sector($sector_id))
			return;

		$tribes_in_sector =
			$this->computed_tribes_data->get_computed_tribes_in_sector(
				$sector_id);

		$buildings_in_sector =
			$this->computed_buildings_data->get_computed_buildings_in_sector(
				$sector_id);

		$buildings_per_user = array();
		foreach ($buildings_in_sector as $building) {
			$buildings_per_user[$building->get_uid()][] = $building;
		}
		$tribes_per_user = array();
		foreach ($tribes_in_sector as $tribe) {
			$tribes_per_user[$tribe->get_uid()][] = $tribe;
		}
		$this->set_dominator(
			$sector_id,
			self::calc_dominator($tribes_per_user, $buildings_per_user),
			Time::get_t_now(),
			false
		);
	}

	/**
	 * Ordnet einem Zeitpunkt den Zyklus zu, im dem die Berechnung durchgeführt
	 * wird. Gibt die Zyklusnummer zurück (beginnend bei 1).
	 *
	 * @param uint $cur_t_stamp		Der Zeitpunkt, dem ein Zyklus zugeordnet
	 * 								werden soll
	 * @param uint $cycle1_t_stamp	Der Zeitpunkt, als Zyklus 1 angefangen hat
	 * @return uint	Nummer des Zyklus (beginnend bei 1)
	 *
	 * @throws IllegalArgumentException
	 */
	public function time_to_cycle($cur_t_stamp, $cycle1_t_stamp) {
		//+1 weil cycles vonm 1 an zählen
		ValueChecker::t_stamp($cur_t_stamp, 'cur_t_stamp');
		ValueChecker::t_stamp($cycle1_t_stamp, '$cycle1_t_stamp');

		$cycle = floor((($cur_t_stamp - $cycle1_t_stamp) + 1)
							/ $this->cycle_duration) + 1;
		return ($cycle > 0) ? $cycle : 1;
	}

	/**
	 * Ordnet einem Berechnungszyklus einen Zeitpunkt zu, als dieser Zyklus
	 * begann. Gibt diesen Zeitpunkt zurück.
	 *
	 *
	 */
	public function cycle_to_time($cycle, $cycle1_t_stamp) {
		ValueChecker::int($cycle, 'cycle', 1);
		ValueChecker::t_stamp($cycle1_t_stamp, '$cycle1_t_stamp');

		$result = round ($cycle * $this->cycle_duration + $cycle1_t_stamp);
		if ($result > Time::get_t_now()-1)	// niemals in die Zukunft gehn
			$result = Time::get_t_now()-1;
		return $result;
	}

	/**
	 * Gibt den Zeitstempel zurück, wann die letzte Berechnung stattfand.
	 *
	 * @return uint
	 */
	public function get_last_compute_time() {
		return $this->last_compute_time;
	}

	/**
	 * Gibt den Identifier der letzten Berechnung der Karte zurück. Dieser
	 * getter ist für interne Benutzung gedacht für die nested Classes
	 * (bei PHP gibt es aber keine Friends).
	 *
	 * @return uint
	 */
	public function get_last_compute_id() {
		return $this->last_compute_id;
	}

	/**
	 * Gibt die ID der Klasse zurück, die gleichzeitig auf die ID der Karte ist,
	 * die die Klasse berechnen soll (durch Singleton ist Sichergestellt, dass
	 * es pro Karte nur eine Instanz der Klasse gibt)
	 *
	 * @return uint
	 */
	public function get_id() {
		return $this->id;
	}

	/**
	 * Gibt die Welt zurück, zu der die Karte gehört, die berechnet wird.
	 */
	public function get_world_id() {
		return $this->world_id;
	}

	/**
	 * Synonym für get_id()
	 * @return uint
	 */
	public function get_map_id() {
		return $this->get_id();
	}


	/**
	 * Gibt an, wie viele SP das Gebäude mit der übergebenen ID aktuell hat.
	 * Existiert das Gebäude nicht auf der Karte, oder wurde es noch nicht
	 * berechnet, da gerade erst gebaut, wird FALSE zurückgegeben.
	 *
	 * @param Point2D $ulc
	 * @param [0,1,...99] $sector_id
	 * @return float/FALSE
	 */
	public function get_sp_of_building(Point2D $ulc, $sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		if (!isset($this->computed_buildings_data))
			return false;

		$bid = MapCompute__ComputedBuildingData::get_bid_by_ulc($ulc);
		return $this->computed_buildings_data->get_computed_amount($bid,
			$sector_id);
	}

	/**
	 * @param Point2D $ulc
	 * @param [0,1,...99] $sector_id
	 * @return float/FALSE
	 */
	public function get_att_boon_of_building(Point2D $ulc, $sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		return 0;
	}
	/**
	 * @param Point2D $ulc
	 * @param [0,1,...99] $sector_id
	 * @return float/FALSE
	 */
	public function get_def_boon_of_building(Point2D $ulc, $sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		return 0;
	}

	public function get_att_boon_of_tribe($tid, $sector_id) {
		ValueChecker::id($tid, 'tid');
		ValueChecker::sector_id($sector_id, 'sector_id');

		return 0;
	}
	public function get_def_boon_of_tribe($tid, $sector_id) {
		ValueChecker::id($tid, 'tid');
		ValueChecker::sector_id($sector_id, 'sector_id');

		return 0;
	}
	public function get_hp_boon_of_tribe($tid, $sector_id) {
		ValueChecker::id($tid, 'tid');
		ValueChecker::sector_id($sector_id, 'sector_id');

		return 0;
	}
	public function get_speed_boon_of_tribe($tid, $sector_id) {
		ValueChecker::id($tid, 'tid');
		ValueChecker::sector_id($sector_id, 'sector_id');

		return 0;
	}
	public function get_amount_of_tribe($tid, $sector_id) {
		ValueChecker::id($tid, 'tid');
		ValueChecker::sector_id($sector_id, 'sector_id');

		if (!$this->computed_tribes_data)
			return false;
		return $this->computed_tribes_data->get_computed_amount($tid,
			$sector_id);
	}
	public function get_tribe_is_camouflaged($tid, $sector_id) {
		ValueChecker::id($tid, 'tid');
		ValueChecker::sector_id($sector_id, 'sector_id');

		if (!$this->computed_tribes_data)
			return false;
		return $this->computed_tribes_data
			->get_tribe_is_camouflaged($tid, $sector_id);
	}

	/**
	 * Gibt die ID des Users zurück, der gerade den Sektor dominiert. Wird hier
	 * 0 zurückgegeben, bedeutet das, dass niemand den Sektor dominiert.
	 *
	 * @param [0,1,99] $sector_id	ID des Sektors
	 * @return uint
	 */
	public function get_dominator($sector_id) {
		ValueChecker::sector_id($sector_id, 'sector_id');

		if (!$this->computed_sector_data)
			return 0;
		return $this->computed_sector_data->get_dominator($sector_id);
	}

	/**
	 * Gibt alle Sektoren als IDs zurück, die gerade vom User mit der
	 * angegebenen ID dominiert werden.
	 *
	 * @param uint $uid
	 * @return {0,1,...,99}[]
	 */
	public function get_all_sectors_dominated_by($uid) {
		ValueChecker::id($uid, 'uid');

		if (!$this->computed_sector_data)
			return array();
		return $this->computed_sector_data->get_all_sectors_dominated_by($uid);
	}

	/**
	 * Gibt zurück, wie viel Resource der User in Sektor $sector zum Zeitpunkt
	 * der letzten Berechnung besessen hat.
	 *
	 * @param [0,1,..99] $sector_id	ID des Sektors
	 * @param uint $which			welche Resource
	 * @param uint $uid				ID des Users
	 * @return ufloat
	 */
	public function get_sector_resource($sector_id, $which, $uid) {
		ValueChecker::sector_id($sector_id, 'sector_id');
		ValueChecker::int($which, 'which');
		ValueChecker::id($uid, 'uid');

		if (!$this->computed_sector_data)
			return 0;
		return $this->computed_sector_data->get_resource($uid, $sector_id,
			$which);
	}

	/**
	 * Gibt die Einheiten im Sektor zurück. Die Rückgabe kann nach User-ID
	 * gefiltert werden, so dass nur diejenigen Einheiten zurückgegeben werden,
	 * die dem User mit der übergebenen ID gehören.
	 *
	 * @param [0,1,...99] $sector_id	ID des Sektors
	 * @param uint $uid					Die ID des Besitzers
	 * @return TribeOnMap[]
	 */
	public function get_tribes_in_sector($sector_id, $uid=false) {
		if (!$this->computed_tribes_data)
			return array();

		ValueChecker::sector_id($sector_id, 'sector_id');

		$tribes
			= $this->computed_tribes_data->get_computed_tribes_in_sector($sector_id);

		if ($uid === false)
			return $tribes;

		ValueChecker::id($uid, 'uid');

		$result = array();
		foreach ($tribes as $t) {
			if ($t->get_uid() == $uid)
				$result[] = $t;
		}
		return $result;
	}

	/**
	 * Gibt die Gesamtanzahl der Einheiten im Sektor zurück. Kann nach User-ID
	 * gefiltert werden, so dass nur diejenigen Einheiten zurückgegeben werden,
	 * die dem User mit der übergebenen ID gehören.
	 *
	 * @param [0,1,...99] $sector_id
	 * @param uint $uid
	 * @return uint
	 */
	public function get_total_amount_in_sector($sector_id, $uid=false) {
		if (!$this->computed_sector_data)
			return 0;

		ValueChecker::sector_id($sector_id, 'sector_id');
		if ($uid !== false) {
			ValueChecker::id($uid, 'uid');
		}

		$tribes = $this->get_computed_tribes_in_sector($sector_id);
		$result = 0;
		foreach ($tribes as $t) {
			if (($uid === false) || ($t->get_uid() == $uid))
				$result += $t->get_amount();
		}
		return $result;
	}


	/**
	 * Gibt die Einheiten zurück, die sich auf der Karte bewegen. Das Ergebnis
	 * kann durch Angabe eines bestimmten Sektors und/oder durch Angabe einer
	 * Menge von erlaubten Bewegungsrichtungen gefiltert werden.
	 * @example get_moving_units(false, Direction::LEFT | Direction::RIGHT) gibt
	 * 			alle Einheiten zurück, die sich irgendwo auf der Karte nach rechts
	 * 			oder links bewegen
	 * @example get_moving_units(21) gibt alle bewegenden Einheiten in Sektor 21
	 * 			zurück.
	 *
	 * @param [0,1,...99] $sector
	 * @param Direction $direction
	 * @return TribeOnMap[]
	 */
	public function get_moving_units($sector_id=false,
		$direction=Direction::ALL)
	{
		if ($sector_id !== false) {
			ValueChecker::sector_id($sector_id, 'sector_id');
		}
		ValueChecker::int($direction, 'direction',
							Direction::MIN, Direction::MAX);

		if (!$this->moving_tribes_data)
			return array();


		$moving = $this->moving_tribes_data->get_all_moving_units();
		$result = array();
		foreach ($moving as $unit_id => $amount) {
			try {
				$t = TribeOnMap::get_instance($unit_id);
			}
			catch (Exception $e) {
				/**
				 * Tribe existiert nicht: überspringen
				 */
				continue;
			}
			// Test 1: Ist die Einheit im richtigen Sektor
			if (($sector_id === false)
				|| ($t->get_current_position() == $sector_id))
			{
				// Test 2: Bewegt sich die Einheit in die richtige Richtung
				if (($direction | $t->get_heading()) == $direction) {
					$result[] = $t;
				}
			}
		}
		return $result;
	}

	/**
	 * Wenn ein TribeOnMap bewegt wurde, muss das hier registriert werden,
	 * damit nicht bis zum nächsten Datenbankabgleich alles hinterherlagt.
	 * Wird hier ein Tribe übergeben, der sich nicht bewegt, wird FALSE
	 * zurückgegeben, ansonsten true.
	 *
	 * @param TribeOnMap $tribe
	 * @return bool
	 */
	public function register_movement(TribeOnMap $tribe) {
		if (!$this->moving_tribes_data)
			return false;

		if (!$tribe->is_moving())
			return false;

		// Bewegende Einheit als solche hinzufügen
		$this->moving_tribes_data->add_moving_tribe($tribe);
		// Und als stationäre löschen
		$sector_id = $tribe->get_current_position();
		$this->computed_tribes_data->set_computed_amount(
			$tribe->get_id(),
			$sector_id,
			0
		);
		// Dominator neu berechnen
		$this->recalculate_current_dominator($sector_id);

		// Änderungen speichern. Ganz wichtig, da im Destruktor nicht
		// gespeichert wird!
		$this->save();
		return true;
	}

	/**
	 * Registriert, dass die angegebene Einheit, die sich eben noch in der
	 * Bewegung befand, gerade gestoppt wurde.
	 *
	 * @param TribeOnMap $tribe
	 */
	public function register_stop(TribeOnMap $tribe) {
		if (!$this->moving_tribes_data)
			return false;
		if ($tribe->is_moving())
			return false;

		// Bewegende Einheit löschen und als stationäre hinzufügen
		$this->moving_tribes_data->remove_moving_tribe($tribe);
		$sector_id = $tribe->get_current_position();
		$this->moving_tribes_data->add_appearances(
			$sector_id,
			$tribe->get_id()
			/**
			 * Die Einheit ist hier im Compute mit Anzahl 0 verzeichnet, ein
			 * $tribe->get_amount() würde diese berechnete Anzahl zurückliefern,
			 * wir wollen aber die, die er in der Bewegung hatte, um sie
			 * jetzt zu übertragen.
			 *
			$tribe->get_amount_last_moving()*/
		);

		// Dominator neu berechnen
		$this->recalculate_current_dominator($sector_id);

		// Änderungen speichern. Ganz wichtig, da im Destruktor nicht
		// gespeichert wird!
		$this->save();
		return true;
	}

	/**
	 * Markiert den angegebenen Sektor als dirty, so dass dieser beim nächsten
	 * Aufruf von MapCompute::get_instance auf jeden Fall neu berechnet wird
	 * (auch wenn der Timeout noch nicht erfolgt ist).
	 *
	 * @param [0,1,...99] $sector_id
	 */
	public function mark_sector_as_dirty($sector_id) {
		if (self::DEBUG_VERBOSE > 1)
			error_log("Marking sector $sector_id as dirty");
		$sector_id = (int)ValueChecker::sector_id($sector_id, 'sector_id');
		$this->dirty_sectors[$sector_id] = true;
	}

	/**
	 * Berechnet dirty Sektoren JETZT.
	 */
	public function compute_dirty_sectors() {
		if (self::DEBUG_VERBOSE > 1)
			error_log("Explicit recompute of dirty sectors: "
				.print_r($this->dirty_sectors,true));
		if (!empty($this->dirty_sectors))
			$this->init();
	}

	/**
	 * Gibt zurück, ob im Sektor nach Berechnung noch gekämpft wird
	 *
	 * @param [0,1,...,99] $sector_id
	 * @return bool
	 * @throws IllegalArgumentException
	 */
	public function get_fight_in_sector($sector_id) {
		if (!$this->computed_sector_data)
			return false;
		ValueChecker::sector_id($sector_id, 'sector_id');

		return $this->computed_sector_data->get_fight_in_sector($sector_id);
	}

	/**
	 * Gibt zurück, ob im Sektor nach Berechnung noch eine Erkundung stattfindet
	 *
	 * @param [0,1,...,99] $sector_id
	 * @return bool
	 * @throws IllegalArgumentException
	 */
	public function get_explore_in_sector($sector_id) {
		if (!$this->computed_sector_data)
			return false;
		ValueChecker::sector_id($sector_id, 'sector_id');

		return $this->computed_sector_data->get_explore_in_sector($sector_id);
	}

	/**
	 * Diese Funktion ist private und nur von Friends-Klassen aufzurufen.
	 *
	 * @access private
	 * @return MapCompute__MovingTribesData
	 */
	public function _get_moving_tribes_data() {
		return $this->moving_tribes_data;
	}

	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		MapCompute__ComputedMapData::clear_of_all_users();
		MapCompute__ComputedBuildingData::clear_of_all_users();
		MapCompute__ComputedSectorData::clear_of_all_users();
		MapCompute__ComputedTribesData::clear_of_all_users();
		MapCompute__MovingTribesData::clear_of_all_users();
		self::$INSTANCES = array();
		query("DELETE FROM computed_maps");
	}
}

?>