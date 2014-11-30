<?php
require_once(dirname(__FILE__).'/../shared/classes/IScienceChooser.inc');

/**
 * Verwaltet das Auswählen von Forschungen nach Kartenende - also welche
 * Forschungsfortschritte man auf die nächste Karte mitnimmt.
 */

class ScienceChooser implements IScienceChooser {
	private $uid;
	private $sciences_known;	// Forschungen, die der Spieler kennt
	private $sciences_selected;	// Welche Forschungen aktuell gewählt hat
	private $sciences_selected_amount;	// Wieviele (also count(sciences_selected))
	private $sciences_kept_before;// Welche Forschungen er bereits auf alten Karten
								// gewählt hatte

	private static $INSTANCES;

	public static function get_instance($uid, $force=false) {
		$state = User::get_instance_by_id($uid)->get_state();
		if (($state != User::STATE_CHOOSING_SCIENCES_WON_LAST_MAP) &&
			($state != User::STATE_CHOOSING_SCIENCES_LOST_LAST_MAP))
		{
			throw new ExceptionWithErrorlog("Cannot use this functionality now.",
				0, "State of User is $state. Should be either ".
				User::STATE_CHOOSING_SCIENCES_WON_LAST_MAP. " or ".
				User::STATE_CHOOSING_SCIENCES_LOST_LAST_MAP. ".");
		}
		if (!self::$INSTANCES[$uid] || $force)
			self::$INSTANCES[$uid] = new ScienceChooser($uid, $force);
		return self::$INSTANCES[$uid];
	}

	private function __construct($uid, $force) {
		$this->uid = (int)$uid;
		$this->init($force);
	}

	public function __destruct() {
		// Die unsets sind dafür da, die Session so klein wie möglich zu halten.
		// alles, was man nicht braucht, wird gelöscht - eine große Session
		// bremst das System ungemein
		$all_unset = true;
		if ($this->sciences_known) {
			$_SESSION['ScienceChooser'][$this->uid]['sciences_known'] = $this->sciences_known;
			$all_unset = false;
		}
		else
			unset($_SESSION['ScienceChooser'][$this->uid]['sciences_known']);

		if ($this->sciences_kept_before) {
			$_SESSION['ScienceChooser'][$this->uid]['sciences_kept_before'] = $this->sciences_kept_before;
			$all_unset = false;
		}
		else
			unset($_SESSION['ScienceChooser'][$this->uid]['sciences_kept_before']);

		if ($this->sciences_selected) {
			$_SESSION['ScienceChooser'][$this->uid]['sciences_selected'] = $this->sciences_selected;
			$all_unset = false;
		}
		else
			unset($_SESSION['ScienceChooser'][$this->uid]['sciences_selected']);

		if ($all_unset)
			unset($_SESSION['ScienceChooser'][$this->uid]);
	}

	private function init($force) {
		if ($force) {
			unset($_SESSION['ScienceChooser'][$this->uid]);
		}
		$this->sciences_selected = isset($_SESSION['ScienceChooser'][$this->uid]['sciences_selected'])
			? $_SESSION['ScienceChooser'][$this->uid]['sciences_selected']
			: array();
		@($this->sciences_known =
				$_SESSION['ScienceChooser'][$this->uid]['sciences_known']);
		@($this->sciences_kept_before =
				$_SESSION['ScienceChooser'][$this->uid]['sciences_kept_before']);

		$this->sciences_selected_amount = count($this->sciences_selected);


		if (!$this->sciences_selected_amount) {
			$this->sciences_known=array();
			$had_been_keeping = mysql_result(
				query(
					'SELECT COUNT(*) FROM science_keeping WHERE uid='.$this->uid
				),0
			);
			if (!$had_been_keeping)	// Alle erforschten Forschungen in die science_keeping kopieren
				query(
					'INSERT INTO science_keeping(uid,sc_id,cur_level) '.
						'SELECT uid,sc_id,cur_level FROM science '.
						'WHERE uid='.$this->uid
				);

			$result = query(
				'SELECT sc_id, cur_level FROM science_keeping '.
				'WHERE uid='.$this->uid
			);
			while($row=mysql_fetch_assoc($result)) {
				$this->sciences_known[$row['sc_id']]=$row['cur_level'];
			}
			// Auf Kartenstart zurücksetzen. Das ist der Ausgangspunkt von dem
			// aus jetzt weiter gewählt werden kann, was mitgenommen wird.
			// Die Forschungen vom Kartenstart werden in jedem Fall mitgenommen.
			Science::get_instance($this->uid)->clear_current_map();
		}
		if (!$this->sciences_kept_before) {
			$result = query(
				'SELECT sc_id, cur_level FROM science_kept '.
				'WHERE uid='.$this->uid
			);
			while ($row=mysql_fetch_assoc($result)) {
				// Abwärtskompatibilität. Früher wurde cur_level benutzt. Heute wird
				// pro level ein Eintrag gemacht, um einen Zeitlichen Ablauf der Wahl
				// zu haben, damit Handycap möglich wird.
				$this->sciences_kept_before[$row['sc_id']]
						+= ($row['cur_level'] > 0) ? $row['cur_level'] : 1;
			}
		}
	}

	/**
	 * Wählt alle Forschungen für die nächste Karte automatisch.
	 * Gibt zurück, welche Forschungen er behalten hat.
	 *
	 * @return uint[]
	 * @throws Exception Bei allen Fehlern
	 * @see ScienceChooser#select_science
	 */
	public function select_all_sciences_automatically() {
		while ($this->sciences_selected_amount
			< $this->get_sel_scs_amount_total())
		{
			$possibilities = array();
			$techtree = Techtree::get_instance($this->uid);
			$researchable_scs = $techtree->get_researchable_scs();
			$known_scs = $this->get_known_sciences();
			foreach ($researchable_scs as $type => $scs_ids) {
				$n = count($scs_ids);
				for ($i = 0; $i < $n; $i++) {
					$sc_id = $scs_ids[$i];
					if ($techtree->get_science_cur_level($sc_id)
						< $known_scs[$sc_id])
					{
						/**
						 * Wenn die Forschung total wichtig ist, dann soll sie
						 * auch unbedingt gewählt werden
						 */
						if (in_array($sc_id,
							ScienceConstants::$VERY_IMPORTANT_SCS))
						{
							$possibilities = array($sc_id);
							break 2;
						}
						/**
						 * Sonst zum Array der Möglichkeiten hinzufügen
						 * (wird dann nachher zufällig eins gewählt)
						 */
						else
							$possibilities[] = $sc_id;
					}
				}
			}
			if (empty($possibilities))
				break;
			$this->select_science(random_value($possibilities));
		}
		$result = $this->sciences_selected;
		$this->finish();
		return $result;
	}

	/**
	 * Hierüber wird eine Forschung vom User zum behalten gewählt.
	 *
	 * @param int $sc_id	Die ID der gewählten Forschung
	 * @return int 			Übrige Forschungen
	 */
	public function select_science($sc_id) {
		$sc_id = (int)$sc_id;
		if ($this->sciences_selected_amount >= $this->get_sel_scs_amount_total())
			throw new Exception("Already all selected", self::ERR_MAX_SELECTABLE_SCIENCES_REACHED);
		$techtree = Techtree::get_instance($this->uid);
		$cur_level = $techtree->get_science_cur_level($sc_id);
		if ($this->sciences_known[$sc_id] <= $cur_level)
			throw new Exception("Science never known", self::ERR_SCIENCE_NEVER_KNOWN);
		if (!$techtree->may_research($sc_id))
			throw new Exception("May not research", self::ERR_MAY_NOT_RESEARCH_SCIENCE);

		if (!$cur_level)
			query("INSERT IGNORE INTO science SET uid={$this->uid}, sc_id=$sc_id, cur_level=1");
		else
			query("UPDATE science SET cur_level=cur_level+1 WHERE sc_id=$sc_id AND uid={$this->uid}");
		$techtree->increase_cur_level($sc_id);	// Erhöhung dem Techtree melden

		$this->sciences_selected[] = $sc_id;
		$this->sciences_selected_amount++;
		return $this->get_sel_scs_amount_total() - $this->sciences_selected_amount;
	}

	/**
	 * Gibt die Anzahl der Forschungen zurück, die der Spieler maximal mitnehmen
	 * kann. Diese Anzahl impliziert die Forschungsaufsparungen, die er von der
	 * letzten Karte ggf. hatte, weil er nicht genug mitnehmen durfte.
	 * Dies liefer dasselbe Ergebnis wie
	 * get_sel_scs_amount_from_older_maps()+get_sel_scs_amount_from_last_map()
	 *
	 * @return int Maximale Forschungsanzahl
	 */
	public function get_sel_scs_amount_total() {
		$amount = $this->get_sel_scs_amount_from_last_map()
			+ $this->get_sel_scs_amount_from_older_maps();
		return $amount > 0 ? $amount : 0;
	}

	/**
	 * Gibt den Anteil der maximal auswählbaren Forschungen zurück, die der User
	 * aufgrund gezwungener Aufsparungen jetzt einlösen darf.
	 * @return int
	 */
	public function get_sel_scs_amount_from_older_maps() {
		$user = User::get_instance_by_id($this->uid);
		return $user->get_add_keep_sciences();
	}
	/**
	 * Gibt den Anteil der maximal auswählbaren Forschungen zurück, die der User
	 * sich auf der letzten Karte verdient hat
	 * @return int
	 */
	public function get_sel_scs_amount_from_last_map() {
		require_once(dirname(__FILE__).
			'/../shared/classes/score/GameScoreCalculation.inc');
		$user = User::get_instance_by_id($this->uid);
		$last_score = $user->get_last_score();
		$new_score = $user->get_cur_score();
		$amount = GameScoreCalculation::calc_level_by_score($new_score)
			- GameScoreCalculation::calc_level_by_score($last_score);
		return $amount > 0 ? $amount : 0;
	}

	/**
	 * Gibt alle bisher ausgewählten Forschungen als Array von Science-IDs zurück.
	 * @return int[]	gewählte Forschungen
	 */
	public function get_selected_sciences() {
		return $this->sciences_selected;
	}

	/**
	 * Gibt die Anzahl aller bisher ausgewählten Forschungen zurück
	 * @return int
	 */
	public function get_selected_sciences_amount() {
		return $this->sciences_selected_amount;
	}

	/**
	 * Gibt zurück, welche Forschungen ihm bekannt sind
	 * @return int[]
	 */
	public function get_known_sciences() {
		return $this->sciences_known;
	}

	/**
	 * Gibt zurück, wieviel der Spieler auf der letzten Map geforscht hat.
	 * @return int
	 */
	public function get_last_map_research_amount() {
		$sc_levels_before_last_map = array_sum($this->sciences_kept_before);
		$sc_levels_after_last_map = array_sum($this->sciences_known);
		return $sc_levels_after_last_map - $sc_levels_before_last_map;
	}

	/**
	 * Setzt die bisher gewählten Forschungen zurück, als hätte der Spieler
	 * bisher noch nichts gewählt.
	 * @return int	wieviele Forschungen er wählen darf
	 */
	public function reset() {
		unset($_SESSION['ScienceChooser'][$this->uid]);
		$this->init();
		return $this->get_sel_scs_amount_total() - $this->sciences_selected_amount;
	}

	/**
	 * Beendet das Wählen. Es werden die gewählten Forschungen dem Nutzer
	 * entsprechend dauerhaft und unwiderruflich gutgeschrieben.
	 */
	public function finish() {
		query('DELETE FROM science_keeping WHERE uid='.$this->uid);
		$scs_left = $this->get_sel_scs_amount_total()
						- $this->sciences_selected_amount;
		$user = User::get_instance_by_id($this->uid);
		$user->set_add_keep_sciences($scs_left);

		foreach ($this->sciences_selected as $sc_id) {
			query("INSERT IGNORE INTO science_kept SET uid={$this->uid}, sc_id=$sc_id");
		}
		$this->sciences_kept_before = null;
		$this->sciences_known = null;
		$this->sciences_selected = null;
		$this->sciences_selected_amount = 0;

		/**
		 * Nach der Wahl direkt alles resetten, um Spieler wieder auf Level 0
		 * zurückzusetzen.
		 */
//		$user->clear();
//		Science::get_instance($this->uid)->clear();
//		Techtree::get_instance($this->uid)->clear();
//		LeagueScore::get_instance(0)->clear($user->get_id());

		// Nächster Schritt: Karte wählen
		$user->set_state(User::STATE_CHOOSING_MAPS);
		return User::STATE_CHOOSING_MAPS;
	}


	/**
	 * !!!ACHTUNG: Setzt alle Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		unset($_SESSION['ScienceChooser']);
		query("DELETE FROM science_keeping");
	}

}


?>