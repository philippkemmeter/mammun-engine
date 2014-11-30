<?php


require_once(dirname(__FILE__).'/host/HostControl.inc');
require_once(dirname(__FILE__).'/../shared/classes/ScienceConstants.inc');

/**
 * Repräsentiert den Technologiebaum eines Users. Enthält alle Forschungen,
 * die der Nutzer bereits erforscht hat und alle Forschungen, die er als nächstes
 * zur Auswahl hat.
 */
class Techtree {
	const SC_CAT_GENERAL = 1;
	const SC_CAT_RACES = 2;
	const SC_CAT_ABILITIES = 3;
	const SC_CAT_BUILDINGS = 4;
	const SC_CAT_ARTIFACT = 5;

	private $uid;
	private $researchable_scs;			// erforschbare Forschungen als Liste, gruppiert
	private $researchable_scs_assoc;	// erforschbare Forschungen, aber in der Form $x[SCID]=(bool)
	private $science_field;				// $science_field[sc_id][sc_level] = array(Werte)
	private $sciences_researched;		// wieivele Forschungslevel er bereits erreicht hat
	private $cur_researched_scs;		// Redundante Speicherung der Forschungsdaten
										// der Forschungen, die er aktuell hat mit auch dem
										// Level = cur_level gruppiert nach Kategorie
	private $max_ever_scs;

	private static $INSTANCES;

	private function __construct($uid, $forced=false) {
		$this->uid = $uid;
		$this->generate_techtree($forced);
	}


	/**
	 * @return Techtree
	 */
	public static function get_instance($uid, $forced=false) {
		if (!isset(self::$INSTANCES[$uid]) || $forced)
			self::$INSTANCES[$uid] = new Techtree($uid, $forced);
		return self::$INSTANCES[$uid];
	}

	/**
	 * Gibt das maximal erreichbare Level einer Forschung zurück.
	 * Prinzipiell ist das in ScienceConstants gespeichert, aber falls man
	 * die mal löscht, holt er sich die Daten aus der DB.
	 */
	public static function get_science_max_level($sc_selected) {
		if ($sc_selected>ITEM_SCIENCE_ID)
			return 1;
		else if (!ScienceConstants::$MAX_LEVELS)
			return(mysql_result(query('SELECT MAX(level) FROM sciences WHERE sc_id='.$sc_selected),0));
		else
			return ScienceConstants::$MAX_LEVELS[$sc_selected];
	}

	/**
	 * Generiert den Techtree des Users, aber nicht komplett, sondern nur für
	 * das aktuelle Level, das aktuelle Level+1 (also das nächste), und für das
	 * Max-Level, das er jemals erforscht hatte.
	 * Dabei wird $this->science_field, welches diese Informationen enthält,
	 * initialisiert, sowie $this->sciences_researched und $this->cur_researched_scs.
	 * Prinzipiell wird diese Berechnung nur einmal pro Session durchgeführt.
	 * In Ausnahmefällen kann mit Setzen des Parameters $forced=true eine
	 * Neugenerierung erzwungen werden. Dann werden alle Daten aus der DB geholt
	 * und die Sessiondaten anschließend überschrieben.
	 *
	 * @param bool $forced=false	Auf true setzen für erzwungene Neugenerierung
	 */
	private function generate_techtree($forced=false) {
		if (User::get_current_user() &&
			(User::get_current_user()->get_id() == $this->uid))
		{
			// Daten aus der Session holen, wenn alle Daten bereit
			if (isset($_SESSION['Techtree']['science_field'])
				&& isset($_SESSION['Techtree']['sciences_researched'])
				&& isset($_SESSION['Techtree']['cur_researched_scs'])
				&& !$forced)	// hier kommt $forced-Parameter ins Spiel
			{
				$this->science_field = $_SESSION['Techtree']['science_field'];
				$this->sciences_researched = $_SESSION['Techtree']['sciences_researched'];
				$this->cur_researched_scs = $_SESSION['Techtree']['cur_researched_scs'];
				return;
			}
			// Sessiondaten killen und alles neu generieren
			else {
				unset($_SESSION['Techtree']);
			}
		}
		// Für den Host nichts auf Vorrat generieren. Der Host hat alle Forschungen
		// sowieso auf Max, so dass wir kein cur_level, max_level etc brauchen
		else if ($this->uid == HostControl::USER_HOST_ID)
			return;

		$this->science_field = array();
		$query_max_level =
			'SELECT ss.*, '.
			'	sq.need_i1, sq.need_sc1, sq.need_sc1_lev, sq.sc1_2_op, '.
			'	sq.need_sc2, sq.need_sc2_lev, sq.sc2_3_op, sq.need_sc3, '.
			'	sq.need_sc3_lev '.
			'FROM sciences AS ss, techtree AS t '.
			'LEFT JOIN sciences_reqs AS sq '.
			'ON sq.level=(t.max_level) AND sq.sc_id=t.sc_id '.
			'WHERE  ss.sc_id=t.sc_id '.
			'	AND ss.level=(t.max_level) '.
			'	AND t.uid='.$this->uid.' '.
			'ORDER BY ss.sc_id';

		$res = query($query_max_level);
		while ($row = mysql_fetch_assoc($res)) {
			$this->science_field[$row['sc_id']][$row['level']] = $row;
			$this->science_field[$row['sc_id']]['max_level_ever'] = $row['level'];
		}


		$query_cur_level =
			'SELECT ss.*, '.
			'	sq.need_i1, sq.need_sc1, sq.need_sc1_lev, sq.sc1_2_op, '.
			'	sq.need_sc2, sq.need_sc2_lev, sq.sc2_3_op, sq.need_sc3, '.
			'	sq.need_sc3_lev '.
			'FROM sciences AS ss, science AS s '.
			'LEFT JOIN sciences_reqs AS sq '.
			'ON sq.level=s.cur_level AND sq.sc_id=s.sc_id '.
			'WHERE  ss.sc_id=s.sc_id '.
			'	AND ss.level=s.cur_level '.
			'	AND s.uid='.$this->uid.' '.
			'ORDER BY ss.sc_id';

		$res = query($query_cur_level);
		while ($row = mysql_fetch_assoc($res)) {
			$this->science_field[$row['sc_id']][$row['level']] = $row;
			$this->science_field[$row['sc_id']]['cur_level'] = $row['level'];
			$this->sciences_researched += $row['level'];
			$this->cur_researched_scs[self::get_category_by_sc_id($row['sc_id'])][$row['sc_id']] = $row;
		}

		$query_cur_level_plus_1 =
			'SELECT ss.*, '.
			'	sq.need_i1, sq.need_sc1, sq.need_sc1_lev, sq.sc1_2_op, '.
			'	sq.need_sc2, sq.need_sc2_lev, sq.sc2_3_op, sq.need_sc3, '.
			'	sq.need_sc3_lev '.
			'FROM sciences AS ss, science AS s '.
			'LEFT JOIN sciences_reqs AS sq '.
			'ON sq.level=(s.cur_level+1) AND sq.sc_id=s.sc_id '.
			'WHERE  ss.sc_id=s.sc_id '.
			'	AND ss.level=(s.cur_level+1) '.
			'	AND s.uid='.$this->uid.' '.
			'ORDER BY ss.sc_id';

		$res = query($query_cur_level_plus_1);
		while ($row = mysql_fetch_assoc($res)) {
			$this->science_field[$row['sc_id']][$row['level']] = $row;
		}
		if (User::get_current_user() &&
			(User::get_current_user()->get_id() == $this->uid))
		{
			//Daten in die Session schreiben
			$_SESSION['Techtree']['science_field'] = $this->science_field;
			$_SESSION['Techtree']['sciences_researched'] = $this->sciences_researched;
			$_SESSION['Techtree']['cur_researched_scs'] = $this->cur_researched_scs;
		}
	}
	private function generate_science_field_level($sc_id, $level) {
		if (isset($this->science_field[$sc_id][$level]))	// existiert schon
			return;
		if ($level <= 0)	// Wäre eine leere Query
			return;


		$query_level =
			'SELECT ss.*, '.
			'	sq.need_i1, sq.need_sc1, sq.need_sc1_lev, sq.sc1_2_op, '.
			'	sq.need_sc2, sq.need_sc2_lev, sq.sc2_3_op, sq.need_sc3, '.
			'	sq.need_sc3_lev '.
			'FROM sciences AS ss '.
			'LEFT JOIN sciences_reqs AS sq '.
			'ON sq.level='.$level.' AND sq.sc_id=ss.sc_id '.
			'WHERE  ss.sc_id='.$sc_id.' '.
			'	AND ss.level='.$level.' '.
			'ORDER BY ss.sc_id';
		$res = query($query_level);

		while ($row = mysql_fetch_assoc($res)) {
			// Mehr als max_level ist nicht bekannt
			$this->science_field[$row['sc_id']][$row['level']] = $row;
		}
		if (User::get_current_user() &&
			(User::get_current_user()->get_id() == $this->uid))
		{
			//Daten in die Session schreiben
			$_SESSION['Techtree']['science_field'] = $this->science_field;
		}
	}

	/**
	 * Gibt eine von vier möglichen Kategorien als String zurück, in die die
	 * übergebene Science-ID gehört.
	 *
	 * @param int $sc_id	Die ID der Science die zugeordnet werden soll
	 * @param bool $return_string	Ob String zurückgegeben weredn soll oder ID
	 *
	 * @return string		Die Kategorie ['general', 'races', 'buildings',
	 * 							'abilities']
	 * @return uint			eine der self::SC_CAT-Konstanten
	 */
	public static function get_category_by_sc_id($sc_id, $return_string=true) {
		// Prinzipiell alle Forschungen sind generell, wenn die ID kleiner als
		// 1000 ist. ID 2 ist die ID der Forscher und wird daher zu den Rassen
		// geschoben und hier übersprungen.
		if ($sc_id == 2)
			return ($return_string) ? 'races' : self::SC_CAT_RACES;
		else if ($sc_id < 1000)
			return ($return_string) ?  'general' : self::SC_CAT_GENERAL;
		// Alle Gebäudefoschungen haben IDs zwischen im Bereich von 1000 bis
		// 1999; alle Forschungen mit diesen IDs sind Gebäudeforschungen.
		else if ($sc_id < 2000)
			return ($return_string) ? 'buildings' : self::SC_CAT_BUILDINGS;
		//Rassen sind zwischen 3000 bis 3999
		else if ($sc_id < 4000)
			return ($return_string) ? 'races' : self::SC_CAT_RACES;
		// Ability-Forschungen haben IDs zwischen im Bereich von 4000 bis 4999;
		else if ($sc_id < 5000)
			return ($return_string) ? 'abilities' : self::SC_CAT_ABILITIES;
		else
			return ($return_string) ?  'general' : self::SC_CAT_GENERAL;
	}

	/**
	 * Gibt die Stringrepräsentation eines Kategorie-Codes zurück
	 *
	 * Ein Kategorie-Code ist eine der self::SC_CAT-Konstanten.
	 *
	 * @param uint $cat_code
	 * @return $string
	 */
	public static function category_code_to_string($cat_code) {
		switch ($cat_code) {
			case self::SC_CAT_GENERAL:
				return 'general';
			case self::SC_CAT_RACES:
				return 'races';
			case self::SC_CAT_BUILDINGS:
				return 'buildings';
			case self::SC_CAT_ABILITIES:
				return 'abilities';
			case self::SC_CAT_ARTIFACT:
				return 'artifacts';
		}
		return 'general';
	}

	/**
	 * Generiert die Forschungen, die der User erforschen kann. Versucht über
	 * die Session zu arbeiten. Wird $force angegeben, wird in jedem Fall
	 * neugeneriert
	 *
	 * @param bool $forced	Neugenerierung erzwungen (kein Session-Cache)
	 */
	private function generate_researchable_sciences_arrays($forced=false) {
		if (User::get_current_user() &&
			(User::get_current_user()->get_id() == $this->uid) && !$forced)
		{
			// versuch aus der Session zu holen
			if (isset($_SESSION['Techtree']['researchable_scs'])
				&& isset($_SESSION['Techtree']['researchable_scs_assoc'])
				&& !$forced)
			{
				$this->researchable_scs
					= $_SESSION['Techtree']['researchable_scs'];
			 	$this->researchable_scs_assoc
			 		= $_SESSION['Techtree']['researchable_scs_assoc'];
			 	return;
			}
		}

		$next_level_reqs = $this->science_field;

		$this->researchable_scs = array();
		$this->researchable_scs_assoc = array();
		foreach (ScienceConstants::$ALL_SCIENCES as $sc_id)
			/* erforschbare herausfiltern und speichern */
		{
			$cur_level = isset($this->science_field[$sc_id]['cur_level'])
				? $this->science_field[$sc_id]['cur_level'] : 0;
			if ((($sc_id < 2000)||($sc_id == 2900)||($sc_id > 2999))
				&&($this->may_research($sc_id, $cur_level+1))
				&&($this->has_artifact($sc_id, $cur_level)>0))
			{

				$this->researchable_scs[self::get_category_by_sc_id($sc_id)][]
					= $sc_id;
				$this->researchable_scs_assoc[$sc_id] = true;
			}
		}
		if (User::get_current_user() &&
			(User::get_current_user()->get_id() == $this->uid))
		{
			$_SESSION['Techtree']['researchable_scs'] = $this->researchable_scs;
			$_SESSION['Techtree']['researchable_scs_assoc']
				= $this->researchable_scs_assoc;
		}
	}

	private function has_artifact($sc_id, $sc_level) {
		$level = $sc_level+1;
		if ($level < Techtree::get_science_max_level($sc_id))
			return 1;
		$artifact = $this->science_field[$sc_id][$level]['need_i1'];
		if (!$artifact)	// kein Item benötigt
			return 1;
		$query =
			'SELECT DISTINCT sector_id '.
			'FROM artifacts '.
			'WHERE state=0 '.
			'	AND identified=1 '.
			'	AND artifact_id='.$artifact.' '.
			'	AND user_id='.$this->uid;
		$result = query($query);
		if (mysql_num_rows($result))
			return $artifact;
		return ($artifact*(-1));
	}

	/**
	 * Überprüft, ob das Level $level der Forschung mit der ID $sc_id erforscht
	 * werden darf, oder nicht. Ist $level nicht gegeben, dann wird das nächste
	 * Level angenommen (also cur_level+1);
	 * WICHTIG: Das Script funktioniert nur dann einwandfrei, wenn die Datenbank
	 * gescheit gefüllt ist. Regeln:
	 * - gibt es nur 1 need_sc, so hat sie in need_sc1 zu stehn, bei 2en in
	 * 		need_sc1 und need_sc2
	 * - gibt es nur 1 need_sc, so haben sc1_2_op und sc2_3_op leer zu sein,
	 * 		bei 2en hat sc2_3_op leer zu sein
	 * @param int $sc_id	Die ID der Forschung, welche geprüft werden soll
	 * @param int $level	Das Level der Forschung, welche geprüft werden soll.
	 */
	public function may_research($sc_id, $level=-1) {
		/*if ($this->is_restricted($sc_id,$level))
			return false;*/
		$cur_level = isset($this->science_field[$sc_id]['cur_level'])
			? $this->science_field[$sc_id]['cur_level']
			: 0;
		if ($level == -1)
			$level = $cur_level+1;
		if ($level > Techtree::get_science_max_level($sc_id))
			return false;

			// TODO: DAs ist QUATSCH. SP_MAPs müssen noch gemacht werden
//		if (($_SESSION['map_type']==SP_MAP) &&
//				(User::get_current_user()->get_last_score()>0))
//		{
//			if ($sc_id > ITEM_SCIENCE_ID)	{
//				/**
//				 * Wenn Artefakte zum erforschen eingeführt werden, dann muss
//				 * das hier angepasst werden.
//				 *
//				$result=query('SELECT DISTINCT sector_id FROM artifacts WHERE state=0 AND artifact_id='.$sc_id.' AND user_id='.User::get_current_user()->get_id());
//				if (mysql_num_rows($result)) {
//					while ($row=mysql_fetch_assoc($result))  {
//
//						THIS IS A GENOME HACK - NUM_HOSTS DOES NOT EXIST ANYMORE
//
//			  			if ($row['sector_id']>NUM_HOSTS*100) {
//			  				if ($row['sector_id']%(NUM_HOSTS*100)
//			  					== User::get_current_user()->get_map_id())
//		  					{
//				  				return true;
//		  					}
//			  			}
//			  			else if (floor($row['sector_id']/100)
//			  				== User::get_current_user()->get_map_id())
//		  				{
//			  				return true;
//		  				}
//					}
//				}
//				else*/
//					return false;
//			}
//		}

		if ($sc_id == SC_ID_MORPHING || $sc_id == SC_ID_SCIENTISTS)
			return false;

		// TODO:
		/*if ($sc_id>ITEM_SCIENCE_ID)	{
			$result=query('SELECT DISTINCT sector_id FROM artifacts WHERE state=0 AND artifact_id='.$sc_id.' AND user_id='.$this->uid);
			return (mysql_num_rows($result) > 0);
		}*/

		$myscience = $this->get_science($sc_id, $level);

		for ($j = 0; $j < 3; $j++)          // init $reqArray
			$reqArray[$j] = 1;

		/* Die folgende for-Schleife ist etwas tricky. Sie geht alle möglichen
		 * need_scs durch, also von need_sc1 bis need_sc3, schaut ob sie erfüllt
		 * sind, speichert die erkenntnis in $reqArray[1] bis $reqArray[3] */

		for ($j = 1; $j < 4; $j++) {
			if ($myscience['need_sc'.$j]) {
				$need_sc_id = $myscience['need_sc'.$j];
				$ro['cur_level']
					= isset($this->science_field[$need_sc_id]['cur_level'])
						? $this->science_field[$need_sc_id]['cur_level']
						: 0;
				if ($myscience['need_sc'.$j.'_lev'] > $ro['cur_level'])
					$reqArray[$j-1] = 0;          /* entsprechende Variable 0 setzen */
			}
			else
				break;
		}
		if ($j == 1)              /* wenn es keine Requirements gibt */
			return 1;             /* sind sie erfüllt */

        /* wenn es requirements gibt */
		$bool1 = ($myscience['sc1_2_op'] == 2)
			? ($reqArray[0] || $reqArray[1])
			: ($reqArray[0] && $reqArray[1]);
		$bool2 = ($myscience['sc2_3_op'] == 2)
			? ($bool1 || $reqArray[2])
			: ($bool1 && $reqArray[2]);
		/* Es wird als wie folgt verknüpft: ($req1Met AND/OR $req2Met) AND/OR $req3Met */
		return ($bool1 && $bool2);    // nur wenn alles erfüllt ist, ist alles erfüllt ;)
	}


	/**
	 * Erhöht das aktuelle Level der durch $sc_id identifizierten Forschung um eins.
	 * Diese Funktion wird gebraucht bei Level ups während der Laufzeit der Session,
	 * damit nicht unnötigerweise aus der DB alles neu generiert werden muss.
	 *
	 * @param int $sc_id	Die ID der Science, dessen Level gesetzt werden soll
	 */
	public function increase_cur_level($sc_id) {
		/**
		 * Die Forschung ist neu erforscht worden und wird deswegen jetzt dem
		 * Techtree hinzugefügt.
		 */
		if (empty($this->science_field[$sc_id]['cur_level'])) {
			$cur_level = 1;
			$this->generate_science_field_level($sc_id, $cur_level);
			$this->science_field[$sc_id]['cur_level'] = $cur_level;
			$max_level = @mysql_result(query(
				"SELECT max_level FROM techtree WHERE sc_id=$sc_id ".
					"AND uid=".$this->uid), 0
			);
			if (!$max_level)
				$max_level=0;
			$this->science_field[$sc_id]['max_level_ever']
				= ($max_level > $cur_level) ? $max_level : $cur_level;
			$this->cur_researched_scs[self::get_category_by_sc_id($sc_id)][$sc_id]
				= $this->science_field[$sc_id][$cur_level];

			if (User::get_current_user() &&
				(User::get_current_user()->get_id() == $this->uid))
			{
				$_SESSION['Techtree']['cur_researched_scs']
					= $this->cur_researched_scs;
				$_SESSION['Techtree']['science_field']
					= $this->science_field;
			}
			$this->sciences_researched++;
		}
		else {
			$this->science_field[$sc_id]['cur_level']++;
		}
		if (User::get_current_user() &&
			(User::get_current_user()->get_id() == $this->uid))
		{
			//Daten in die Session schreiben
			$_SESSION['Techtree']['science_field'] = $this->science_field;
		}
		// erzwungene Neugenerierung der erforschbaren Forschungen
		$this->generate_researchable_sciences_arrays(true);

		// Wenn es eine Rasse oder Fähigkeit war, müssen die Tribes aktualisiert werden
		$cat = $this->get_category_by_sc_id($sc_id);
		if ($cat === 'races')
			Tribes::get_instance($this->uid)->update_all_tribes_with_race($sc_id);
		else if ($cat == 'abilities')
			Tribes::get_instance($this->uid)->update_all_tribes_with_ability($sc_id);

		// Das science_field muss nicht upgedatet werden. Das Update
		// geschieht dann, wenn er per get_science darauf zugreifen möchte und
		// das Level noch nicht generiert wurde.
	}

	///// GETTER ////

	/**
	 * Gibt die Forschung mit der ID $sc_id und gewähltem Level zurück. Ist zudem
	 * $only_if_researched gesetzt, wird geprüft, ob der aktuelle User diese
	 * Forschung überhaupt erforscht hat (default: keine Prüfung)
	 * @param int $sc_id		Forschungs-ID
	 * @param int/string $level	Das Level der Forschung. Wird + oder - vorgestellt
	 * 							(also z.B. "+1") wird relativ zum cur_level verfahren
	 * @param bool $only_if_researched=false	Wenn true, entsprechender Test
	 * @return array	Die Forschung als asso-array oder false im Fehlerfall
	 */
	public function get_science($sc_id, $level='+0', $only_if_researched=false) {
		if ($only_if_researched && !$this->get_science_cur_level($sc_id))
			return false;	// dann hat er's nicht erforscht

		if (($level{0} === '+') || ($level{0} === '-'))	{// relativ zum cur_level
			$cur_level = $this->get_science_cur_level($sc_id);
			if ($level{0} === '+') {
				$cur_level += (int)(substr($level, 1));
			}
			else {
				$cur_level -= (int)(substr($level, 1));
			}
			$level = $cur_level;
		}
		if (!isset($this->science_field[$sc_id][$level]))
			$this->generate_science_field_level($sc_id, $level);

		return $this->science_field[$sc_id][$level];
	}


	public function get_science_cur_level($sc_id) {
		if ($this->uid == HostControl::USER_HOST_ID) // Host hat immer Maximum
			return Techtree::get_science_max_level($sc_id);

		return isset($this->science_field[$sc_id]['cur_level'])
			? $this->science_field[$sc_id]['cur_level']
			: 0;
	}

	/**
	 * Gibt zurück, welches Level der User dieser Forschung als höchstes jemals
	 * erforscht hat.
	 *
	 * @param uint $sc_id
	 * @return uint
	 */
	public function get_science_max_level_ever($sc_id) {
		if ($this->uid == USER_HOST_ID)	// Host hat immer Maximum
			return Techtree::get_science_max_level($sc_id);

		return $this->science_field[$sc_id]['max_level_ever']
			? $this->science_field[$sc_id]['max_level_ever']
			: 0;
	}

	/**
	 * Gibt die erforschbaren Sciences unguppiert als assoziatives Array zurück.
	 * Und zwar in der Form $array[$sc_id] = true, wenn erforschbar. Ansonsten
	 * undefiniert. Zum direkten abfragen, ob eine gewisse Forschung erforschbar
	 * ist, ist diese Darstellung also die erste Wahl, da keine Arraysuche erfolgen
	 * muss.
	 *
	 * @return array($sc_id => true)
	 */
	public function get_researchable_scs_assoc() {
		if (!$this->researchable_scs_assoc)
			$this->generate_researchable_sciences_arrays();
		return $this->researchable_scs_assoc;
	}

	/**
	 * Gibt die erforschbaren Sciences gruppiert nach Typ zurück. Aufgebaut ist
	 * das Array wie folgt:
	 * $array['general'] enthält als array alle sc_ids, der erforschbaren General-Scs
	 * $array['races'] enthält als array alle sc_ids, der erforschbaren Races
	 * $array['abilities'] enthält als array alle sc_ids, der erforschbaren Abilities
	 * $array['buildings'] enthält als array alle sc_ids, der erforschbaren Gebäude
	 * $array['artifacts'] enthält als array alle sc_ids, der erforschbaren Gebäude
	 * Diese Darstellung ist also die erste Wahl, wenn man alle erforschbaren
	 * Forschungen ausgeben will.
	 *
	 * @return array('general'=>int[], 'races'=>int[], 'abilities'=>int[],
	 * 									'buildings'=>int[], 'artifacts'=>int[])
	 */
	public function get_researchable_scs() {
		if (!$this->researchable_scs)
			$this->generate_researchable_sciences_arrays();
		return $this->researchable_scs;
	}


	/**
	 * Gibt die erforschbaren Forschungen in der Form der erforschten
	 * Forschungen zurück, d.h.: $result[category][sc_id] = $row
	 *
	 * @return array('general'=>stdClass[], 'races'=>stdClass[],
	 * 		'abilities'=>stdClass[], 'buildings'=>stdClass[],
	 * 		'artifacts'=>stdClass[])
	 */
	public function get_researchable_scs_values() {
		if (!$this->researchable_scs)
			$this->generate_researchable_sciences_arrays();
		$result = null;	// TODO: Das kann man cachen UND die Abfrage kann
						// man beschleunigen (aktuell pro Forschung eine Query
						// statt einer insgesamt!)
		foreach ($this->researchable_scs_assoc as $sc_id => $foo) {
			if ($foo) {
				$result[self::get_category_by_sc_id($sc_id)][$sc_id] =
					$this->get_science($sc_id, 1);	// führt zu je einer Query!
			}
		}
		return $result;
	}

	/**
	 * Gibt die Forschungen  zurück, die vom User auf der aktuellen
	 * Karte erforscht wurden und somit zur Verfügung stehen.
	 *
	 * @return array('general'=>stdClass[], 'races'=>stdClass[],
	 * 		'abilities'=>stdClass[], 'buildings'=>stdClass[],
	 * 		'artifacts'=>stdClass[])
	 */
	public function get_cur_researched_scs() {
		/*foreach ($this->science_field as $sc_id => $jnk) {
			if ($this->science_field[$sc_id]['cur_level']) {
				$this->cur_researched_scs[$sc_id] = $this->science_field[$sc_id][$this->science_field[$sc_id]['cur_level']];
			}
		}*/
		return $this->cur_researched_scs;
	}

	/**
	 * Gibt die Forschungen zurück, die vom User in seiner gesamten Spielzeit
	 * erspielt wurden. Das dient hauptsächlich zu Statistikzwecken, und dass
	 * der User einen stetig wachsenden Techtree besitzt.
	 *
	 * @return array('general'=>int[], 'races'=>int[], 'abilities'=>int[],
	 * 									'buildings'=>int[], 'artifacts'=>int[])
	 */
	public function get_max_ever_researched_scs() {
		foreach ($this->science_field as $sc_id => $jnk) {
			if ($this->science_field[$sc_id]['max_level_ever']) {
				$this->max_ever_scs[$sc_id] = $this->science_field[$sc_id]
					[$this->science_field[$sc_id]['max_level_lever']];
			}
		}
		return $this->max_ever_scs;
	}

	public function get_researched_sciences() {
		return $this->cur_researched_scs;
	}

	public function get_researched_sciences_count($category=false) {
		if (!$category)
			return $this->sciences_researched;
		else {
			if (is_anint($category)) {
				$category = self::category_code_to_string($category);
			}
			$cnt = 0;
			foreach ($this->cur_researched_scs[$category] as $foo)
				$cnt++;
			return $cnt;
		}
	}

	/**
	 * Gibt zurück, wie viel Ressource-Bonus der Nutzer bekommt, dadurch, dass
	 * er geforscht hat. Gibt ein Array zurück, wie die Ressource-Boni nach Typ
	 * sortiert erscheinen.
	 *
	 * @return float[]
	 */
	public function get_cur_resource_increases() {
		if (!is_array($this->cur_researched_scs['general'])) {
			// keine general-Sciences erforscht - kein Bonus ;-)
			return array();
		}

		$result = array();
		for ($i = 0; $i < Resource::NUM_RESOURCES; $i++)
			$result[$i] = 0;
		foreach ($this->cur_researched_scs['general'] as $data) {
			for ($i = 0; $i < Resource::NUM_RESOURCES; $i++) {
				$result[$i] += $data['resource'.($i+1)];
			}
		}

		return $result;
	}

	/**
	 * Gibt zurück, mit welchen Fähigkeitstypen der gewählte Einheitentyp
	 * ausgestattet werden darf.
	 * Gibt ein assotiatives Array der Form 'ABILITY_TYPE' => Anzahl zurück,
	 * das besagt, mit wie vielen Abilities der gewählte Einheitentyp von
	 * welcher Art ausgestattet werden kann.
	 *
	 * Also array(TYPE1 => k, TYPE2 => l ...).
	 *
	 * Ist die Anzahl 0, so ist es als beliebig viele zu deuten, ist kein
	 * Eintrag für einen Typ gesetzt, ist das als KEINE zu deuten.
	 *
	 * @param int $sc_id_of_race		Die Science-ID des gewählten Typs
	 * @return int[]	Welche Fähigkeitstypen wie oft
	 */
	public static function get_allowed_abilities_of_race($sc_id_of_race) {
		if (self::get_category_by_sc_id($sc_id_of_race) !== 'races')
			return false;	// keine Rasse => keine Abilities

		if (!isset($_SESSION['Techtree']['race_abilities'][$sc_id_of_race])) {
			$assoc = array();
			$res = query (
				"SELECT ability_type, amount FROM race_abilities ".
				"WHERE race_sc_id=$sc_id_of_race"
			);

			while ($row = mysql_fetch_assoc($res)) {
				$assoc[$row['ability_type']] = $row['amount'];
			}
			$_SESSION['Techtree']['race_abilities'][$sc_id_of_race] = $assoc;
		}
		else
			$assoc = $_SESSION['Techtree']['race_abilities'][$sc_id_of_race];

		if (!is_array($assoc))
			$assoc = Converter::stdClassToArray($assoc);

		return $assoc;
	}


	/**
	 * !!!ACHTUNG: Setzt die Daten des Users unwiederruflich zurück!
	 */
	public function clear() {
		query("DELETE FROM techtree WHERE uid={$this->uid}");
		query("INSERT INTO techtree SET uid={$this->uid}, sc_id=".SC_ID_SCIENTISTS.", max_level='1'");
		query("INSERT INTO techtree SET uid={$this->uid}, sc_id=".SC_ID_MORPHING.", max_level='1'");
		query("INSERT INTO techtree SET uid={$this->uid}, sc_id=".SC_ID_CLUB.", max_level='1'");
		query("INSERT INTO techtree SET uid={$this->uid}, sc_id=".SC_ID_SHED.", max_level='1'");
		$this->generate_researchable_sciences_arrays(true);
		$this->generate_techtree(true);
	}

	/**
	 * !!!ACHTUNG: Setzt alle Techtree-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		unset($_SESSION['Techtree']);
		query("DELETE FROM techtree");
		//TODO: Das muss mit einer Query gehn, oder?
		$ids = query("SELECT id FROM users");
		while ($id = mysql_fetch_array($ids)) {
			$uid = $id['id'];
			query("INSERT INTO techtree SET uid=$uid, sc_id=".SC_ID_SCIENTISTS.", max_level='1'");
			query("INSERT INTO techtree SET uid=$uid, sc_id=".SC_ID_MORPHING.", max_level='1'");
			query("INSERT INTO techtree SET uid=$uid, sc_id=".SC_ID_CLUB.", max_level='1'");
			query("INSERT INTO techtree SET uid=$uid, sc_id=".SC_ID_SHED.", max_level='1'");
		}
	}
}


?>