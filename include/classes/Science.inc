<?

require_once(dirname(__FILE__).'/score/GameScore.inc');
require_once(dirname(__FILE__).'/map/Mapsettings.inc');
require_once(dirname(__FILE__).'/Tribes.inc');
require_once(dirname(__FILE__).'/Techtree.inc');
require_once(dirname(__FILE__).'/EventLog.inc');
require_once(dirname(__FILE__).'/../shared/classes/helper/IConvertablePHPStdClass.inc');
require_once(dirname(__FILE__).'/exceptions/HackattackException.inc');
require_once(dirname(__FILE__).'/Accesscontrol.inc');
require_once(dirname(__FILE__).'/medals/Medals.inc');
require_once(dirname(__FILE__).'/medals/UserMedals.inc');
/**
 * Verwaltet alle Forschungen.
 */

/**
 * Hilfklasse ScienceProject repräsentiert ein Forschungsprojekt, das gerade
 * läuft oder sich im Pausezustand befindet.
 *
 * Alle Funktionen, die mit einem "_" beginnen, sind private außer für die
 * Friend-Class Science. Friends werden von PHP nicht unterstützt, daher müssen
 * die Funktionen public markiert werden.
 */
class ScienceProject implements IConvertablePHPStdClass {
	private $uid;
	private $cur_level;
	private $finish_time;
	private $last_stamp;
	private $sc_id;
	private $paused;
	private $progress;			// fortschritt als Verhältnis forschzeit/gesamtzeit
	private $sc_count;			// wie viele Wissenschaftler zugeteilt sind
	private $grow;				// wie hoch das Wachstum der Forschung ansich ist
	private $need_write_back;	// ob es nötig ist, die DB zu aktualisieren

	const PROJECT_PAUSED_FINISH_TIME = 3000000000;
	const E_UNSET_THIS = 1;	// errorcode, wenn construct fehlschlug

	private static $INSTANCES;	// singleton: eine Instanz pro uid-sc_id-Paar

	/**
	 * Konstruktor ist private. Statische Funktion _get_instance() benutzen.
	 * @param int $uid		Die ID des Users, der das Projekt forscht.
	 * @param int $sc_id	Die ID der Forschung, welche durch das Projekt
	 * 						erforscht werden soll
	 * @throws Exception	Wenn Projekt nicht (mehr) existiert und gelöscht
	 * 						werden soll
	 * 						[ein unset($this) ist ab PHP5 nicht mehr möglich]
	 */
	private function __construct($uid, $sc_id) {
		$this->uid = $uid;
		$this->sc_id = $sc_id;
		$res = query("SELECT * FROM science WHERE uid=$uid AND sc_id=$sc_id");
		$scs_data = Techtree::get_instance($uid)->get_science($sc_id, "+1");
		$this->cur_level = $scs_data['level']-1;
		$this->grow = $scs_data['grow'];
		if (mysql_num_rows($res) > 0) {
			$sc_data = mysql_fetch_assoc($res);
			$this->progress = $sc_data['l_progress'];
			$this->finish_time = $sc_data['finish_time'];
			$this->last_stamp = $sc_data['l_stamp'];
			$this->sc_count = $sc_data['count'];
			$this->paused  = ($this->finish_time ==
									self::PROJECT_PAUSED_FINISH_TIME);
			if (!$this->update()) {	// Wird nicht mehr weitergeforscht, ...
										// ... muss Objekt gekillt werden.

				$this->__destruct();	// Das muss explizit aufgerufen werden!
										// (Exception killt Objekt hart)
				throw new Exception("", self::E_UNSET_THIS);// Exception werfen
															// (verhindert Obj-
															// Erzeugung)
			}
		}
		else {
			$this->progress = $this->finish_time =
				$this->last_stamp = $this->sc_count = 0;
			$this->paused = false;	// es ist nicht pausiert, es existiert nicht
		}
		$this->need_write_back = false;
	}
	/**
	 * Sobald das Objekt gekillt wird, soll alles in die DB geschrieben werde
	 */
	function __destruct() {
		if ($this->need_write_back);
			$this->write_back();
	}

	/**
	 * Nur eine Instanz pro UID-SC_ID-Paar
	 * !!NUR von friend Science zu aufzurufen.
	 */
	public static function _get_instance($uid, $sc_id) {
		if (!self::$INSTANCES || !self::$INSTANCES[$uid][$sc_id])
			self::$INSTANCES[$uid][$sc_id] = new ScienceProject($uid, $sc_id);
		return self::$INSTANCES[$uid][$sc_id];
	}


	/**
	 * Schreibt die Daten in die DB.
	 */
	private function write_back() {
		$sql =
			"UPDATE science SET count={$this->sc_count}, ".
			"	l_stamp={$this->last_stamp}, l_progress={$this->progress}, ".
			"	finish_time={$this->finish_time}, cur_level={$this->cur_level} ".
			"WHERE uid={$this->uid} AND sc_id={$this->sc_id}";
		query($sql);
	}

	/**
	 * Pausiert das aktuelle Projekt
	 * !!NUR von friend Science zu aufzurufen.
	 * @throws ExceptionWithErrormail bei potentiellen Designfehlern ;-)
	 */
	public function _stop() {
		if (!$this->sc_count) {
			throw new ExceptionWithErrormail("Stopping impossible",
				$this->uid, 'All', "Stopping of an not running ScProject",
				"In ScienceProject::_stop()...");
		}
		$cur_progress = $this->get_progress();
		if ($cur_progress > 1)
			$cur_progress = 0.99; // nur zur Sicherheit. Kam bei PV vor.
		$this->paused = true;
		$this->finish_time = self::PROJECT_PAUSED_FINISH_TIME;
		$this->sc_count = 0;
		$this->last_stamp = 0;
		$this->progress = $cur_progress;
		$this->need_write_back = true;
	}

	/**
	 * Startet das Projekt (z.B. weil pausiert oder weil neu erstellt),
	 * oder setzt die Wissenschaftleranzahl entsprechend auf $count, wenn
	 * bereits gestartet wurde.
	 * !!NUR von friend Science zu aufzurufen.
	 * @param int $count	Wie viele Wissenschaftler dem Projekt zugeteilt
	 * 						werden sollen
	 */
	public function _start($count) {
		// Fall 1: er ist bereits am Forschen - man muss nur updaten
		if ($this->sc_count) {
			/* es muss nun die finish_time angepasst werden, da die Forscherzahl
			  sich ändert*/
			// wie lange bräuchte die Forschung insgesamt, wenn schon immer mit
			// der neuen Forscherzahl geforscht würde
			$time_needed = Science::get_instance($this->uid)->
					science_time_needed($this->grow, $count);
			// diese Zeit anteilig (noch ausstehender Progress) anrechnen, um
			// neue Endzeit zu bekommen
			$cur_progress = $this->get_progress();
			$finish_time = round(Time::get_t_now() + (1 - $cur_progress) *
															$time_needed);
			$this->finish_time = $finish_time;
			$this->progress = $cur_progress;
			$this->sc_count = $count;
			$this->last_stamp = Time::get_t_now();
			$this->need_write_back = true;
		}
		// Fall 2: er forscht im Moment nicht daran (ggf. auch nur pausiert)
		else  {
			// wie lange wird er für die Forschung brauchen
			$time_needed = Science::get_instance($this->uid)->
					science_time_needed($this->grow, $count);

			// Fall 2a: pausiert
			if ($this->paused) {
				$cur_progress = $this->get_progress();

				$finish_time = round(Time::get_t_now() + (1 - $cur_progress)
																* $time_needed);
				$this->need_write_back = true;
			}
			// Fall 2b: noch nie geforscht: Manueller Insert in die DB nötig
			else {
				$finish_time = round(Time::get_t_now() + $time_needed);
				$sql =
					"INSERT INTO science SET uid={$this->uid}, ".
						"sc_id={$this->sc_id}, count=$count, ".
						"finish_time=$finish_time, l_stamp=".Time::get_t_now();
				query($sql);
				$this->need_write_back = false;
			}

			$this->finish_time = $finish_time;
			// $this->progress bleibt unberücksichtigt
			$this->sc_count = $count;
			$this->last_stamp = Time::get_t_now();
		}
	}

	/**
	 * Prüft, ob bereits Level-Up passiert sein muss (ggf. mehrere) und setzt
	 * die eigenen Parameter auf den neusten Stand
	 * @throws ExceptionWithErrormail 	bei Designfehlern
	 * @return bool		ob am Projekt nach Update noch weitergeforscht wird
	 */
	private function update() {
		$further_research_possible = true;

		/**
		 * Die Finish-Time wird gleich geändert, wir brauchen sie aber noch
		 * später, also zwischenspeichern.
		 */
		$finish_time = $this->finish_time;
		// Projekt ist fertig
		while (Time::get_t_now() > $this->finish_time) {
			/**
			 * Schleife, weil mehrere Level-Ups möglich sein können
			 */

			$techtree = Techtree::get_instance($this->uid);

			$this->cur_level++;	// Level erhöhen - auch Techtree informieren
			$techtree->increase_cur_level($this->sc_id);

			// ist die gerade fertig gewordene Forschung mit dem neuen Level
			// erforschbar? Dann weiter daran forschen.
			$researchable_scs = $techtree->get_researchable_scs_assoc();
			if (@$researchable_scs[$this->sc_id]){// er darf daran weiterforschen
				$scs_data = $techtree->get_science($this->sc_id, "+1");
				if ($this->cur_level != $scs_data['level']-1)
					throw new ExceptionWithErrormail(
						"Techtree noch sync with ScienceProject!",
						$this->uid, 'All', 'Techtree not sync w/ ScProj',
						"\$this->cur_level=={$this->cur_level}, but ".
						"\$scs_data['level']-1==".($scs_data['level']-1));

				$this->grow = $scs_data['grow'];

				$this->last_stamp = $this->finish_time;
				$this->progress = 0;	// bedeutet: Zum Zeitpunkt finish_time
										// war die neue Forschung bei 0%
				// wie lange braucht er, um die neue Stufe zu erforschen
				$time_needed = Science::get_instance($this->uid)->
					science_time_needed($this->grow, $this->sc_count);
				$this->finish_time = $this->last_stamp + $time_needed;
			}
			else {
				$this->paused = true;
				$this->finish_time = self::PROJECT_PAUSED_FINISH_TIME;
				$this->sc_count = 0;
				$this->last_stamp = 0;
				$this->progress = 0;
				$further_research_possible=false;	// Projekt beendet
			}

			$user = User::get_instance_by_id($this->uid);

			/**
			 * Forschungsereignis in Log eintragen
			 */
			EventLog::get_instance($this->uid, $user->get_world_id(),
				$user->get_map_id())
				->add_entry(
					EventLog::TYPE_SC_FINISHED,
					$this->sc_id,
					'',
					$finish_time
				);

			/**
			 * Ggf. Medallie vergeben
			 */
			$sc_medals_cat = Medals::get_medals(
				IMedals::TYPE_SCIENCE_RESEARCHED_CATEGORY
			);
			$sc_medals_cat = $sc_medals_cat[
									IMedals::TYPE_SCIENCE_RESEARCHED_CATEGORY];
			$sc_medals_scid = Medals::get_medals(
				IMedals::TYPE_SCIENCE_RESEARCHED_LEVEL
			);
			$sc_medals_scid = $sc_medals_scid[
									IMedals::TYPE_SCIENCE_RESEARCHED_LEVEL];

			$c1 = Techtree::get_instance($this->uid)
					->get_researched_sciences_count();
			$cat = Techtree::get_category_by_sc_id($this->sc_id, false);
			$c2 = Techtree::get_instance($this->uid)
					->get_researched_sciences_count($cat);

			$my_medals_cat = UserMedals::get_instance($this->uid)->get_medals(
				IMedals::TYPE_SCIENCE_RESEARCHED_CATEGORY,
				0,
				UserMedals::SIMPLE_ARRAY
			);
			$my_medals_scid = UserMedals::get_instance($this->uid)->get_medals(
				IMedals::TYPE_SCIENCE_RESEARCHED_LEVEL,
				0,
				UserMedals::SIMPLE_ARRAY
			);

			$medal_id = 0;
			if ((isset($sc_medals_scid[$this->sc_id][1]))
				&& !in_array($sc_medals_scid[$this->sc_id][1], $my_medals_scid))
			{
				$medal_id = $sc_medals_scid[$this->sc_id][1];
			}
			else if ((isset($sc_medals_cat[$cat][$c2]))
				&& !in_array($sc_medals_cat[$cat][$c2], $my_medals_cat))
			{
				$medal_id = $sc_medals_cat[$cat][$c2];
			}
			else if ((isset($sc_medals_cat[0][$c1]))
				&& !in_array($sc_medals_cat[0][$c1], $my_medals_cat))
			{
				$medal_id = $sc_medals_cat[0][$c1];
			}

			if ($medal_id) {
				UserMedals::get_instance($this->uid)->add_medal(
					$medal_id, $finish_time
				);
			}
			/**
			 * Punkte durch Eventnachricht gutschreiben und Medallie vermerken
			 */
			EventMessages::get_instance($this->uid)->add_entry(
				EventMessages::TYPE_SC_FINISHED,
				$this->sc_id,
				'',
				$finish_time,
				GameScore::calc_score($this->grow,
					GameScore::TYPE_SCIENCE_RESULT),
				$medal_id
			);


			/**
			 * Datenbank ist asynch
			 */
			$this->need_write_back = true;
		}
		return $further_research_possible;	// Projekt noch am laufen
	}

	//// GETTER ////
	/**
	 * Gibt zurück, wie weit das Projekt gerade geforscht ist, wobei 1 fertig
	 * und 0-1 entsprechend das Verhältnis angibt (prozent).
	 * @return float aktueller Forschungsfortschritt
	 */
	public function get_progress() {
		if ($this->paused)	// Dann ist der Progress aktuell in DB und Objekt
			return $this->progress;
		$this->update();
		return $this->progress + (Time::get_t_now() -  $this->last_stamp) /
												$this->get_time_needed();
	}

	/**
	 * Gibt zurück, wie viele Scientists am Projekt derzeit arbeiten
	 * @return int Scientists, die gerade am Projekt arbeiten
	 */
	public function get_sc_count() {
		return $this->sc_count;
	}

	/**
	 * Gibt die ID der Science zurück, die gerade durch dieses Projekt erforscht
	 * wird.
	 * @return int die sc_id
	 */
	public function get_sc_id() {
		return $this->sc_id;
	}

	/**
	 * Gibt den Zeitpunkt zurück, wann die Forschung abgeschlossen sein wird.
	 * @return t_stamp Forschungsendzeitpunkt
	 */
	public function get_finish_time() {
		$this->update();
		return $this->finish_time;
	}

	/**
	 * Gibt die Zeit zurück, die die Forschung bei aktuell zugewiesenen Forschern
	 * bis zum entgültigen Erforschen andauern würde (konjunktiv, da ja noch in
	 * der Zukunft permanent Forscher hin und her gezogen werden können UND da
	 * dies in der Vergangenheit ggf. bereits getan wurde, was NICHT
	 * berücksichtigt wird!)
	 *
	 * @return int 		Aktuelle Insgesamtdauer bei gleichbleibender aktueller
	 * 					Forscherzahl
	 */
	public function get_time_needed() {
		$this->update();
		return Science::get_instance($this->uid)->
				science_time_needed($this->grow, $this->sc_count);
	}

	/**
	 * Gibt den Namen der Forschung zurück
	 * @return string Name der Forschung
	 */
	public function get_name() {
		return ScienceConstants::$SC_NAME[$this->sc_id];
	}

	/**
	 * Gibt das Level der Projektes, das gerade angestrebt wird. Hat ein User
	 * eine Forschung Level 1 und erforscht jetzt Level 2, dann wird 2 zurück-
	 * gegeben.
	 *
	 * @return unsigned int	aktuelles Level
	 */
	public function get_level() {
		$this->update();
		return $this->cur_level+1;
	}

	/**
	 * Gibt das Objekt als stdClass reduziert zurück.
	 *
	 * @return stdClass
	 */
	public function to_php_std_class() {
		$this->update();	// be surely up to date
		$x = new stdClass();
		$x->uid = $this->uid;
		$x->cur_level = $this->cur_level;
		$x->finish_time = $this->finish_time;
		$x->last_stamp = $this->last_stamp;
		$x->sc_id = $this->sc_id;
		$x->paused = $this->paused;
		$x->progress = $this->progress;
		$x->sc_count = $this->sc_count;
		$x->grow = $this->grow;
		return $x;
	}
}


define ('MAGIC_SCIENCE_NUMBER', 		395042400);//131680800; //das ist 3600*10000*7.3156 zum normieren


/**
 * Die eigentliche Science-Klasse. Verwaltet alle Forschungen eines Users.
 */
class Science {
	private $uid;
	private /*ScienceProject[]*/ $science_projects;	// Alle Forschungsprojekte des Users
													// assoziativ als 'sc_id' => $project
	private /*Techtree*/ $techtree;			// Der aktuelle Techtree des Users
	private /*Tribe*/ $scientists;			// Die Scientists des Users
	private /*int*/ $max_projects_parallel;	// Wie viele Projekte der User glz. forschen darf
	private /*Mapsettings*/ $mapsettings;	// enthält u.a. den aktuellen Zeitfaktor
	private $science_pow;					// Die aktuelle Fähigkeit der Wissenschaftler des akt. Users
	private $world_id;
	private $map_id;						// ID der Karte auf dem der User ist
	private /*array*/ $restricted_sciences;	// Welche Forschungen für den User verboten sind

	private static $INSTANCES;	// singleton: eine Instanz pro User

	const ERR_TOO_MANY_PROJ_PARALLEL = 1;
	const ERR_NO_PROJ_POSSIBLE_AT_ALL = 2;
	const ERR_ALL_SCIENTISTS_AT_WORK = 3;


	private function __construct($uid) {
		$this->uid = $uid;
		$t = Tribes::get_instance($uid);
		$tribes = $t->get_tribes_by_race(SC_ID_SCIENTISTS);
		$this->scientists = $tribes[0];	// es gibt eh nur einen Scientist-Tribe
		$this->mapsettings = Mapsettings::get_instance();
		$this->techtree = false;	// verzögert
		$this->world_id = User::get_instance_by_id($uid)->get_world_id();
		$this->map_id = User::get_instance_by_id($uid)->get_map_id();
		$this->max_projects_parallel = false;	// verzögert
		$this->science_projects = false;	// verzögert
	}

	/**
	 * @return Science
	 */
	public static function get_instance($uid) {
		if (!self::$INSTANCES || !self::$INSTANCES[$uid])
			self::$INSTANCES[$uid] = new Science($uid);
		return self::$INSTANCES[$uid];
	}

	/**
	 * Fügt Forscher einem bestehenden Projekt hinzu bzw zieht sie ab, wenn
	 * negative Anzahl übergeben wurde. Ist das Projekt nicht gestartet, wird
	 * es versucht zu starten
	 *
	 * @param int $sc_id			ID der zu forschenden Forschung
	 * @param int $scientist_count	Anzahl der Forscher, die zugeteilt werden sollen
	 * @throws HackattackException	Wenn unerlaubte aktionen ausgeführt werden
	 * @throws Exception			Bei allen anderen Fehlern (TC-level zu klein etc)
	 */
	public function add_scientists_to_project($sc_id, $scientist_count) {
		if ($this->science_projects[$sc_id])
			$scientist_count = $this->science_projects[$sc_id]->get_sc_count() + $scientist_count;
		$this->start_research($sc_id, $scientist_count);
	}
	/**
	 * Startet die Forschung der gewählten $sc_id oder nimmt die Forschung wieder
	 * auf, wenn pausiert. Sind bereits Forscher dem Projekt zugeteilt, wird die
	 * aktuelle Anzahl auf die übergebene korrigiert.
	 *
	 * @param int $sc_id			ID der zu forschenden Forschung
	 * @param int $scientist_count	Anzahl der Forscher, die zugeteilt werden sollen
	 * @throws HackattackException	Wenn unerlaubte aktionen ausgeführt werden
	 * @throws Exception			Bei allen anderen Fehlern (TC-level zu klein etc)
	 */
	public function start_research($sc_id, $scientist_count) {
		if (!$this->get_techtree()->may_research($sc_id))
			throw new HackattackException("Action not allowed", $this->uid,
				HackattackException::CODE_MAY_NOT_RESEARCH);

		if (!$this->get_max_projects_parallel())
			throw new Exception("", self::ERR_NO_PROJ_POSSIBLE_AT_ALL);

		$science_projects = $this->get_science_projects();
		$sc_count = $this->scientists->get_count();
		$sc_at_work = $this->get_scientists_at_work();

		// Range-Test
		if ($scientist_count > $sc_count)
			$scientist_count = $sc_count;
		else if ($scientist_count < 0)
			$scientist_count = 0;

		if (!@$science_projects[$sc_id]) {
			/** neues Projekt. Test nötig, ob er ein neues starten darf */
			// 1. Test: Hat er noch Forscher zu Zuteilen:
			if ($scientist_count <= 0)	// keine Forscher (oder negative) zugeteilt?
				return;					// einfach ignorieren
			else if ($sc_at_work >= $sc_count)	// wenn bereits alle Forscher arbeiten
				throw new Exception("", self::ERR_ALL_SCIENTISTS_AT_WORK);	// Fehler
			else if (($sc_count-$sc_at_work) < $scientist_count)//wenn zu viele ausgewählt,
//				throw new Exception("", self::ERR_ALL_SCIENTISTS_AT_WORK);	// Fehler
				$scientist_count = $sc_count-$sc_at_work;		//maximal verfügbare wählen


			// 2. Test: Darf er überhaupt noch Projekte starten
			$project_count = ($science_projects)
				? count($science_projects)
				: 0;
			if ($this->get_max_projects_parallel() <= $project_count)
				throw new Exception("", self::ERR_TOO_MANY_PROJ_PARALLEL);

			// wenn alle Tests ja, dann erstellen wir eins :)
			$science_projects[$sc_id] =
				ScienceProject::_get_instance($this->uid, $sc_id);
			$this->science_projects = $science_projects;
		}
		else {
			/** Altes Projekt. Auch Tests nötig */
			if ($scientist_count <= 0) {
				// => Projekt muss gestoppt werden, da keine Forscher mehr daran tätig
				$this->stop_research($sc_id);
				return;
			}

			$max_sc_for_this_project = $sc_count -
				($sc_at_work - $science_projects[$sc_id]->get_sc_count());

			// Wenn er welche hinzufügen will, schauen, dass er nicht zuviele addiert
			if ($scientist_count > $science_projects[$sc_id]->get_sc_count()) {
				if ($sc_at_work >= $sc_count)	// wenn bereits alle Forscher arbeiten
					throw new Exception("", self::ERR_ALL_SCIENTISTS_AT_WORK);	// Fehler
				else if ($scientist_count > $max_sc_for_this_project) {
					$scientist_count = $max_sc_for_this_project;
				}
			}
		}
		$science_projects[$sc_id]->_start($scientist_count);
	}

	/**
	 * Stoppt das Forschungsprojekt mit der übergebenen sc_id
	 *
	 * @param int $sc_id	Welche Forschung. Wenn -1 (default), dann ALLE
	 * @throws ExceptionWithErrormail	bei Designfehlern :)
	 *
	 */
	public function stop_research($sc_id=-1) {
		if ($sc_id == -1) {
			foreach ($this->science_projects as $sc_id => $sc_pro) {
				$sc_pro->_stop();
				unset($this->science_projects[$sc_id]);
			}
		}
		else {
			if (!$this->science_projects[$sc_id])
				throw new ExceptionWithErrormail("Not running Science tried to stop.",
					User::get_current_user()->get_id(), 'All',
					'Error: Science::stop', 'in Science::stop_research($sc_id)');
			$this->science_projects[$sc_id]->_stop();
			unset($this->science_projects[$sc_id]);
		}
	}

	public function set_scientists_of_project($sc_id, $amount) {
		$this->start_research($sc_id, $amount);
	}

	private function set_restricted_sciences() {
		if (!$this->restricted_sciences) {
			if ((User::get_current_user()->get_id() == $uid)
					&& $_SESSION['Science']['restricted_sciences']) {
				$this->restricted_sciences = $_SESSION['Science']['restricted_sciences'];
			}
			else {
				$template_id = @mysql_result(query('SELECT template_id FROM maps WHERE world_id='.$this->world_id.' AND map_id='.$this->map_id),0);
				$result=query('SELECT level FROM template_restricted_sciences WHERE template_id='.$template_id.' AND science_id < 0');
				if (mysql_num_rows($result))
					$this->restricted_sciences['all'] = mysql_result($result, 0);
				else
					$this->restricted_sciences['all'] = -1;

				$result = query('SELECT science_id,level FROM template_restricted_sciences WHERE template_id='.$template_id.' AND science_id>0');
			    while($row = mysql_fetch_assoc($result))
					$this->restricted_sciences[$row['science_id']]['maxlev'] = $row['level'];
				if (User::get_current_user()->get_id() == $uid)
					$_SESSION['Science']['restricted_sciences'] = $this->restricted_sciences;
			}
		}
		return $this->restricted_sciences;
	}

	/// GETTER
	public function get_science_projects($force=false) {
		if (($this->science_projects === false) || $force) {
			$sql =	// holt alle laufenden projekte des Users aus der DB
				'SELECT sc_id FROM science '.
				'WHERE uid='.$this->uid.' AND count > 0';
			$res = query($sql);
			$this->science_projects = array();
			while ($row = mysql_fetch_assoc($res)) {
				try {
					$this->science_projects[$row['sc_id']] =
						ScienceProject::_get_instance($this->uid, $row['sc_id']);
				}
				catch(Exception $e) {
/*					if ($this->science_projects[$row['sc_id']]) // ist immer false, oder?
						unset($this->science_projects[$row['sc_id']]);*/
				}
			}
		}
		return $this->science_projects;
	}
	public function get_science_projects_data() {
		$science_projects = $this->get_science_projects();
		$result = array();
		foreach ($science_projects as $sc_id => $project)
			$result[$sc_id] = $project->to_php_std_class();
		return $result;
	}
	public function get_techtree() {
		if (!$this->techtree)
			$this->techtree = Techtree::get_instance($this->uid);
		return $this->techtree;
	}

	/**
	 * Gibt die Forschungsgeschwindigkeit zurück als standardisierten Wert.
	 *
	 * @return float
	 */
	public function get_science_pow() {
		if (!$this->science_pow) {
			/**
			 * Nur solange puffern, wie sich der Zeitfaktor der Karte nicht
			 * ändert
			 */
			if (empty($_SESSION['Science'][$this->uid]['science_pow'])
				|| ($_SESSION['Science'][$this->uid]['science_pow_time_fac']
					!= Mapsettings::get_instance()->get_timefactor() )
				)
			{
				$data = Techtree::get_instance($this->uid)->get_science(
					SC_ID_SCIENTISTS);

				$time_fac = Mapsettings::get_instance()->get_timefactor();

				$_SESSION['Science'][$this->uid]['science_pow']
					= $data['f_extra1'] * $time_fac;

				$_SESSION['Science'][$this->uid]['science_pow_time_fac']
					= $time_fac;

			}
			$this->science_pow
				= $_SESSION['Science'][$this->uid]['science_pow'];
		}
		return $this->science_pow;
	}

	public function get_max_projects_parallel() {
		if ($this->max_projects_parallel === false) {
			$this->max_projects_parallel = 10000;
		}
		return $this->max_projects_parallel;
	}

	/**
	 * Gibt zurück, wie viele Forscher insgesamt gerade arbeiten.
	 * @return int Arbeitende-Forscher-Anzahl
	 */
	public function get_scientists_at_work() {
		$this->science_projects = $this->get_science_projects(); // ggf noch nicht initialisiert
		if (!$this->science_projects)
			return 0;
		$result = 0;
		foreach ($this->science_projects as $project) {
			$result += $project->get_sc_count();
		}
		return $result;
	}

	/**
	 * Berechnet anhand von der Science-Power, wie lange ein Forschungsprojekt
	 * benötigt, wenn es einen gewissen Wachstumswert aufweist und bestimmt viele
	 * Forscher zugeteilt sind.
	 * @param int $grow		Wachstumswert - spiegelt die Forschungsgeschwindigkeit
	 * 						abstrakt wider
	 * @param int $count	Wieviele Forscher der Forschung zugeteilt sind
	 */
	public function science_time_needed($grow, $count) {
		/*if ($count == 0)
			Accesscontrol::kill_session(); // acc sharing*/
		return (MAGIC_SCIENCE_NUMBER * $grow /
				($count * $this->get_science_pow() *
					(-atan($count / 3000 - 5.5) + 6)
				));
	}

	/**
	 * !!!Löscht alle erforschten Forschungen und setzt somit den Forschungsstand
	 * des Users auf Spielbeginn zurück. Irreversibel! Mit Vorsicht zu genießen!
	 */
	public function clear() {
		query("DELETE FROM science WHERE uid={$this->uid}");
		query("INSERT INTO science SET uid={$this->uid}, sc_id=".SC_ID_SCIENTISTS.", cur_level='1'");
		query("INSERT INTO science SET uid={$this->uid}, sc_id=".SC_ID_MORPHING.", cur_level='1'");
		query("INSERT INTO science SET uid={$this->uid}, sc_id=".SC_ID_CLUB.", cur_level='1'");
		query("INSERT INTO science SET uid={$this->uid}, sc_id=".SC_ID_SHED.", cur_level='1'");
		query("DELETE FROM science_kept WHERE uid={$this->uid}");
		query("INSERT INTO science_kept SET uid={$this->uid}, sc_id=".SC_ID_SCIENTISTS);
		query("INSERT INTO science_kept SET uid={$this->uid}, sc_id=".SC_ID_MORPHING);
		query("INSERT INTO science_kept SET uid={$this->uid}, sc_id=".SC_ID_CLUB);
		query("INSERT INTO science_kept SET uid={$this->uid}, sc_id=".SC_ID_SHED);
		$this->science_projects = false;
//		Techtree::get_instance($this->uid)->clear();
	}

	/**
	 * ACHTUNG: Löscht alle erforschten Forschungen ALLER USER.
	 * Irreversibel
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		query("DELETE FROM science");
		query("DELETE FROM science_kept");
		//TODO: Das muss mit einer Query gehn, oder?
		$ids = query("SELECT id FROM users");
		while ($id = mysql_fetch_array($ids)) {
			$uid = $id['id'];
			query("INSERT INTO science SET uid=$uid, sc_id=".SC_ID_SCIENTISTS.", cur_level='1'");
			query("INSERT INTO science SET uid=$uid, sc_id=".SC_ID_MORPHING.", cur_level='1'");
			query("INSERT INTO science SET uid=$uid, sc_id=".SC_ID_CLUB.", cur_level='1'");
			query("INSERT INTO science SET uid=$uid, sc_id=".SC_ID_SHED.", cur_level='1'");
			query("INSERT INTO science_kept SET uid=$uid, sc_id=".SC_ID_SCIENTISTS);
			query("INSERT INTO science_kept SET uid=$uid, sc_id=".SC_ID_MORPHING);
			query("INSERT INTO science_kept SET uid=$uid, sc_id=".SC_ID_CLUB);
			query("INSERT INTO science_kept SET uid=$uid, sc_id=".SC_ID_SHED);
		}
	}

	/**
	 * Setzt den Forschungsstand des Spielers auf den, den er beim Antritt der
	 * letzten Karte hatte.  Es werden also alle Forschungen verworfen, die er
	 * auf der aktuellen bzw. gerade gespielten Karte gemacht hat.
	 *
	 * Hat er eine Karte beendet und seine Forschungen
	 * gewählt, aber noch keine neue Karte gewählt, dann werden hierüber die
	 * Forschungen für die neue Karte initialisiert.
	 *
	 * Wird zudem $handicap_amount angegeben, werden Forschungen die angegebene
	 * Anzahl an Forschungen zusätzlich abgezogen. Somit kann der Spieler
	 * weiter zurückgesetzt werden, als beim Kartenstart, was zum Handicapping
	 * von stärkeren Spielern beitragen kann.
	 *
	 * Irreversibel! Mit Vorsicht zu genießen!
	 *
	 * @param uint $handicap_amount
	 */
	public function clear_current_map($handicap_amount=0) {
		query("DELETE FROM science WHERE uid={$this->uid}");
		if ($handicap_amount > 0) {
			/**
			 * Handicapping
			 */
			/**
			 * Startforschungen immer hinzufügen.
			 */
			query(
				"INSERT INTO science (uid, sc_id, cur_level) VALUES ".
					"({$this->uid}, ".SC_ID_SCIENTISTS.", 1), ".
					"({$this->uid}, ".SC_ID_MORPHING.", 1), ".
					"({$this->uid}, ".SC_ID_CLUB.", 1), ".
					"({$this->uid}, ".SC_ID_SHED.", 1) "
			);
			/**
			 * Alle Forschungen auslesen, die er gerne mitnehmen würde,
			 * abzüglich derer, die er ja eben eingetragen bekommen hat.
			 */
			$res = query(
				'SELECT sc_id FROM science_kept '.
				"WHERE uid={$this->uid} AND sc_id NOT IN (".
					SC_ID_SCIENTISTS . ', ' . SC_ID_MORPHING . ', '
					. SC_ID_CLUB . ', ' . SC_ID_SHED . ')'
			);
			/**
			 * Gibt an, wie viele Forschungen er noch behalten darf nach
			 * Handicapping.
			 * @var int
			 */
			$allow_num = mysql_num_rows($res) - $handicap_amount;
			if ($allow_num > 0) {
				$cnt = 0;
				$sciences = null;
				while (($row = mysql_fetch_assoc($res))&&($cnt++ < $allow_num))
					@$sciences[$row['sc_id']]++;

				$sql =
					'INSERT IGNORE INTO science (uid, sc_id, cur_level) '.
					'VALUES ';
				$first = true;
				foreach ($sciences as $sc_id => $level) {
					if (!$first)
						$sql .= ', ';
					else
						$first = false;

					$sql .= "({$this->uid}, $sc_id, $level)";
				}
				if (!$first)	// mindestens ein Eintrag muss gemacht werden
					query($sql);
			}
		}
		else {
			/**
			 * Kein Handicap: Einfach alles von science_kept übertragen
			 */
			query(
				'INSERT INTO science (uid,sc_id,cur_level) '.
					'SELECT uid,sc_id,count(*) AS cur_level FROM science_kept '.
					'WHERE uid='.$this->uid.' GROUP BY sc_id'
			);
		}
		$this->science_projects = false;
		$this->techtree = Techtree::get_instance($this->uid, true); // forced reload
	}
}


?>
