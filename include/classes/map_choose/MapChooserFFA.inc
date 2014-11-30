<?php


require_once(dirname(__FILE__).'/MapChooser.inc');
require_once(dirname(__FILE__).'/../World.inc');
require_once(dirname(__FILE__).'/../host/HostControl.inc');
require_once(dirname(__FILE__).'/../exceptions/HackattackException.inc');
require_once(dirname(__FILE__).'/../../shared/classes/exceptions/'.
											'IllegalArgumentException.inc');
/**
 * Die Klasse ist zuständig dafür, dass der User eine FFA-Karte wählen kann, 
 * dass diese sich füllt und, wenn voll, gestartet wird.
 */
class MapChooserFFA extends MapChooser {
	/**
	 * Wieviele Spieler auf einer Karte sein dürfen. Wenn z.B. 4 in 
	 * $ALLOWED_TYPES enthalten ist, dann heißt das, dass 4-Spieler-Karten 
	 * auswählbar sind.
	 * @var int[]
	 */
	protected static $ALLOWED_TYPES = array(4, 8, 6);
	const MAPS_PER_ALLOWED_TYPE = 2;	// wie viele Karten pro Typ
										// gleichzeitig angeboten werden sollen
	const DEBUG_VERBOSE = 0;

	private static $INSTANCES;

	protected $direct_start;	// Ob die Karte gleich starten soll oder
								// erst wenn voll.
	/**
	 * @param uint $uid
	 * @return MapChooserFFA
	 */
	public static function get_instance($uid, $world_id=0) {
		if ($uid <= 0) {
			throw new IllegalArgumentException("uid", "N1", $uid);
		}
		$uid = (int)$uid;
		if ($world_id < 0) {
			throw new IllegalArgumentException("world", "N0", $uid);
		}
		if (!isset(self::$INSTANCES[$uid][$world_id]))
			self::$INSTANCES[$uid][$world_id] = new self($uid, $world_id);

		return self::$INSTANCES[$uid][$world_id];
	}

	protected function __construct($uid, $world_id) {
		$user = User::get_instance_by_id($uid);
		if ($user->get_state() != User::STATE_CHOOSING_MAPS
			&& $user->get_state() != User::STATE_WAITING_ON_MAP
			&& $user->get_state() != User::STATE_WAITING_FOR_CLANMAP
			&& $user->get_state() != User::STATE_WAITING_ON_MAP_AND_READY
			&& $user->get_state() != User::STATE_NEW_PLAYER
			&& !$user->get_waiting_state())
		{
			throw new Exception("User state does not allow to choose a map now! ".
								"State is {$user->get_state()}");
		}
		$this->uid = (int)$uid;
		$this->direct_start = true;
		$this->world_id = $world_id;
		$this->init();
	}

	/**
	 * Hilfsfunktion, die zum Sortieren von $this->available_maps benutzt wird.
	 * Sie vergleicht $a['num_players'] mit $b['num_players'].
	 *
	 * @param array $a
	 * @param array $b
	 */
	static function compare_num_players($a, $b) {
		if ($a['num_players'] == $b['num_players'])
			return 0;
		return ($a['num_players'] < $b['num_players']) ? -1 : 1;
	}

	protected function init() {
		$user = User::get_instance_by_id($this->uid);
		$cur_level = $user->get_cur_level();
		$this->available_maps = array();

		/**
		 * Wenn keine Welt angegeben wurde, wird die gewählt, die am besten
		 * passt
		 */
		if (!$this->world_id)
			$this->world_id = World::get_instance_by_level($cur_level,
				World::MAP_TYPE_FFA)->get_id();

		/**
		 * Bei direktem Start werden alle Karten angeboten, die nicht voll sind,
		 * egal, ob sie bereits gestartet sind, oder nicht.
		 */
		if ($this->direct_start) {
			$sql =
				'SELECT t.num_players, m.world_id, m.map_id, t.template_id '.
				'FROM maps AS m, template_maps_1 AS t '.
				'WHERE m.world_id ='.$this->world_id.
				'	AND m.template_id=t.template_id '.
				'	AND (SELECT COUNT(*) FROM players_on_map WHERE '.
									'world_id=m.world_id AND map_id=m.map_id) '.
															'< t.num_players'.
				' 	AND (t.num_players='.self::$ALLOWED_TYPES[0];
		}
		/**
		 * Bei nicht direktem Start, wenn also Karten erst starten, wenn sie
		 * voll sind, werden nur die Karten angeboten, die noch nicht gestartet
		 * sind (state=1)
		 */
		else {
			$sql =
				'SELECT t.num_players, m.world_id, m.map_id, t.template_id '.
						/*,t.difficulty */
				'FROM maps AS m, template_maps_1 AS t '.
				'WHERE m.world_id ='.$this->world_id.' '.
				'	AND m.template_id=t.template_id '.
				'	AND m.state=1 AND (t.num_players='.self::$ALLOWED_TYPES[0];
		}
		$not_existing_maps = array();
		foreach (self::$ALLOWED_TYPES as $num_players) {
			$sql .= ' OR t.num_players=' . $num_players;
			/**
			 * Zunächst wird einmal davon ausgegangen, dass keine Karte
			 * in der DB exisitert
			 */
			$not_existing_maps[$num_players] = array();
			for ($i = 0; $i < self::MAPS_PER_ALLOWED_TYPE; $i++) {
				$not_existing_maps[$num_players][$i] = true;
			}
		}
		$sql .= ')';

		$result = query($sql);


		/**
		 * Bei direktem Start müssen die User aus der players_on_map-Tabelle
		 * geholt werden, da die Karte (wenn nicht leer) schon läuft.
		 */
		if ($this->direct_start) {
			while ($row = mysql_fetch_assoc($result)) {
				$res = query (
					'SELECT user_id,kill_time FROM players_on_map '.
					'WHERE world_id='.$row['world_id'].' AND '.
						'map_id='.$row['map_id']	// AND kill_time=0
				);
				$users = array();
				$dead_ones = array();
				if (mysql_num_rows($res) > 0) {
					while ($p_row = mysql_fetch_assoc($res)) {
						if ($p_row['kill_time'] <= 0)
							$users[] = $p_row['user_id'];
						else
							$dead_ones[] = $p_row['user_id'];
					}
				}

				$this->available_maps[] = array (
					'num_players'	=>	$row['num_players'],
					'users_on_map'	=>	$users,
					'dead_users'	=>	$dead_ones,
					'template_id'	=>	$row['template_id'],
					'world_id'		=>	$row['world_id'],
					'map_id'		=>	$row['map_id']
				);
				/**
				 * Eine $row['num_players']-Spieler-Karte existiert doch, also
				 * poppen wir :)
				 */
				array_pop($not_existing_maps[$row['num_players']]);
			}
		}
		/**
		 * Bei Start-Wenn-Karte-Voll, weiß das User-Objekt, welche User wo
		 * wartet.
		 */
		else {
			while ($row = mysql_fetch_assoc($result)) {
				$this->available_maps[] = array (
					'num_players'	=>	$row['num_players'],
					'users_on_map'	=>
						User::get_user_ids_waiting_on_map($row['world_id'],
							$row['map_id']),
					'dead_users'	=>	array(),		// Gibt natürlich keine
					'template_id'	=>	$row['template_id'],
					'world_id'		=>	$row['world_id'],
					'map_id'		=>	$row['map_id']
				);
				/**
				 * Eine $row['num_players']-Spieler-Karte existiert doch, also
				 * poppen wir :)
				 */
				array_pop($not_existing_maps[$row['num_players']]);
			}
		}
		/**
		 * Es kann vorkommen, dass nicht alle verfügbaren Karten in der DB
		 * eingetragen sind. Früher wurden z.B. die Karten nur dann eingetragen,
		 * wenn irgendwann einmal ein User darauf war. Heute ist es so, dass
		 * diese zwar direkt eingetragen werden, aber erst, wenn eine World
		 * zum erstenmal betreten wird. Werden die $ALLOWED_TYPES modifiziert,
		 * werden auch Lücken existieren.
		 * Daher werden hier nun alle nicht existenten Karten nachgetragen.
		 */
		foreach ($not_existing_maps as $num_players => $bool_array) {
			$sql =
				'SELECT template_id FROM template_maps_1 '.
				'WHERE state=2 '.
					'AND type=(type|'.MapTemplate::TYPE_SUITABLE_FOR_FFA.') '.
					"AND num_players=$num_players";
			$res = query($sql);
			$matching_template_ids = array();
			while ($row = mysql_fetch_assoc($res))
				$matching_template_ids[] = $row['template_id'];

			foreach ($bool_array as $jnk) {
				$pair = random_value_key_pair($matching_template_ids);
				unset($matching_template_ids[$pair['key']]);
				$template_id = $pair['value'];

				$map_id = World::get_instance($this->world_id)
					->insert_map($template_id, World::MAP_TYPE_FFA);
				$this->available_maps[] = array (
					'num_players'	=>	$num_players,
					'users_on_map'	=>	array(),
					'dead_users'	=>	array(),
					'template_id'	=>	$template_id,
					'world_id'		=>	$world_id,
					'map_id'		=>	$map_id
				);
			}
		}
		usort($this->available_maps, array('MapChooserFFA',
			'compare_num_players'));

		// Abschließend wird der chosen_map_index rekonstruiert.
		$chosen_map_id =
			User::get_instance_by_id($this->uid)->get_waiting_map_id() % 100;
		$chosen_world_id =
			floor(User::get_instance_by_id($this->uid)->get_waiting_map_id()/100);

		if ($chosen_world_id) {
			foreach ($this->available_maps as $index => $map) {
				if (($map['world_id'] == $chosen_world_id)
					&& ($map['map_id'] == $chosen_map_id))
				{
					$this->chosen_map_index = $index;
					break;
				}
			}
		}
	}

	/**
	 * Wechselt zu der angegebenen Welt.
	 *
	 * @param uint $world_id
	 */
	public function change_world($world_id) {
		parent::change_world($world_id);
		try {
			$world = World::get_instance($world_id);
		}
		catch (Exception $e) {
			throw new IllegalArgumentException("World does not exist: $world_id");
		}
		if ($world->get_minimum_level()
					> User::get_instance_by_id($this->uid)->get_cur_level())
		{
			throw new Exception("", self::ERR_WORLD_NEEDS_HIGHER_LEVEL);
		}

		$this->world_id = $world->get_id();
		$this->init();
	}

	/**
	 * Setzt den User auf die Map mit der angegebenen ID
	 *
	 * @param uint $map_index	Der User wird auf die
	 * 							Karte available_maps[$map_index] gesetzt
	 * @param uint $home		Wunsch-Homesector
	 * @param uint $color		Wunsch-Farbe
	 * @return int				Die komplette Sektor-ID des Heimatsektors, der
	 * 							dem User zugewiesen wurde.
	 */
	public function choose_map($map_index, $home=0, $color=0) {
		// MUTEX: Keine zwei Kartenzusagen gleichzeitig!!
		// (sonst würden Homesektoren doppelt vergeben)
		$sem_id = sem_get(MUTEX_MAP_CHOOSE, 1, 0600, -1);
		sem_acquire($sem_id);

		if (self::DEBUG_VERBOSE > 0) {
			error_log("MapChooserFFA::\$INSTANCES[{$this->uid}]->choose_map($map_index) called");
		}
		try {
			if ($map_index < 0)
				throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);
			$this->unchoose_map();

			$map = $this->available_maps[$map_index];
			if (empty($map) || empty($map['template_id'])
					|| empty($map['world_id']) || empty($map['map_id']))
				throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);

			if (in_array($this->uid, $map['users_on_map']) ||
				in_array($this->uid, $map['dead_users']))
			{
				throw new Exception("", self::ERR_USER_IS_OR_WAS_ON_MAP);
			}

			// Zuerst alle Heimatsektoren der Karte holen
			$sql =
				"SELECT sector_id FROM template_maps ".
				"WHERE template_id={$map['template_id']} AND home>0";
			$res = query($sql);
			if (mysql_num_rows($res) < 1)
				throw new Exception("", self::ERR_NO_HS_FOUND);
			$homes = array();
			// und in $homes speichern
			while ($row = mysql_fetch_assoc($res))
				$homes[$row['sector_id']] = true;

			/**
			 * Beim alten Modell, kein Direct-Start, werden die Homesectoren aus
			 * dem Feld "waiting_homesector" der Usertabelle geholt
			 */
			if (!$this->direct_start) {
				// Jetzt alle bereits belegten Heimatsektoren holen
				// TODO: Diese Query ist vom Typ ALL !
				$sql =
					"SELECT waiting_homesector FROM users ".
					"WHERE floor(waiting_homesector/100)={$map['map_id']}".
						" AND floor(waiting_homesector/10000)="
							.$map['world_id'];
				$res = query($sql);
				// und aus $homes wieder rauswerfen
				// FIXME: waiting_homesector is obsolete
				while ($row = mysql_fetch_assoc($res))
					unset($homes[$row['waiting_homesector']%100]);
			}
			/**
			 * Beim Direkten Start wird nicht gewartet, die homesectoren sind
			 * nicht in waiting_homesectors gespeichert, sondern in homesectors.
			 * Nun kann es aber sein, dass User auf der KArte bereits diese
			 * verlassen haben - dann wäre homesector nicht mehr aufzufinden und
			 * ein zwar aktuell nicht mehr vom Spieler beanspruchter, aber
			 * wahrscheinlich mit Einheiten vollgepumpter Sektor würde mehrfach
			 * vergeben. Das will niemand.
			 * Deswegen werden die Homesectoren nochmal in players_on_map
			 * gespeichert (was auch die Suchgeschwindigkeit erhöht). Die dort
			 * gespeicherten Werte meinen nicht, welchen Homesektor der User
			 * JETZT innehat, sondern, welcher ihm auf der Karte mit der ID
			 * $map_id für diese Karte zugewiesen worden ist. Und genau das
			 * wollen wir hier haben. Beim alten Modell ist der aktuelle HS
			 * immer derselbe, wie der, der ihm zugewiesen wurde, da man
			 * nachträglich nicht auf die Karte drauf konnte.
			 */
			else {
				$sql =
					"SELECT homesector FROM players_on_map ".
					"WHERE world_id={$map['world_id']} ".
						" AND map_id={$map['map_id']}";
				$res = query($sql);
				// und aus $homes wieder rauswerfen
				while ($row = mysql_fetch_assoc($res))
					unset($homes[$row['homesector']]);
			}

			if (count($homes) == 0) {
				throw new Exception("", self::ERR_MAP_ALREADY_FULL);
			}

			/**
			 * Wenn ein Wunsch-HS gewählt wurde, dann versuchen diesen zu
			 * wählen.
			 */
			if ($home) {
				if (!$homes[$home])
					throw new Exception("", self::ERR_HS_NOT_AVAILABLE);
				$free_home = $home + $map['world_id']*10000
					+ $map['map_id']*100;
			}
			/**
			 * Sonst wählen wir zufällig einen.
			 */
			else {
				$free_home = random_key($homes) + $map['world_id']*10000
					+ $map['map_id']*100;
			}

			$user = User::get_instance_by_id($this->uid);
			// Wartestatus: Er wartet auf ne volle Map
			$user->set_waiting_homesector($free_home);
			$user->set_waiting_state(IUser::STATE_WAITING_ON_MAP);

			/**
			 * Der normale State wird nicht angetastet, da der Waiting-State
			 * höherwertig ist. Der normale State wird nun genutzt, um den
			 * Status des Trainingsmapspiels zu speichern, das nur gespielt werden,
			 * während ein Spieler wartet.
			 * Dasselbe gilt für den Homesektor, der ebenso nicht angetastet wird,
			 * damit man ihn für eine Trainingskarte nutzen kann.
			 */
			// User der gewählten Karte hinzufügen:
			$this->available_maps[$map_index]['users_on_map'][] = $user->get_id();
			$this->chosen_map_index = $map_index;

			if ($color)
				$this->wished_color = $color;

			/**
			 * Beim Direkten Start schalten wir von "wartend" direkt auf
			 * "wartend und fertig" um, da es keinen Warteraum gibt, wo Spieler
			 * markieren müssen, dass sie fertig sind und auch wirklich sicher.
			 */
			if ($this->direct_start)
				$this->toggle_ready_state_logic();
			// MUTEX
			sem_release($sem_id);
			return $free_home;
		}
		catch (IllegalArgumentException $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
		catch (Exception $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
	}

	/**
	 * Mach die Mapwahl rückgängig. Der Spieler wird somit wieder in den Zustand
	 * zurückgesetzt, als habe er bislang noch keine Karte gewählt.
	 */
	public function unchoose_map() {
		if (self::DEBUG_VERBOSE > 0) {
			error_log("MapChooserFFA::\$INSTANCES[{$this->uid}]->unchoose_map($map_index) called");
		}
		$user = User::get_instance_by_id($this->uid);
		$user->set_waiting_homesector(0);
		$user->set_waiting_state(IUser::STATE_CHOOSING_MAPS);
		$n = count($this->available_maps[$this->chosen_map_index]['users_on_map']);
		if ($n > 0) {
			for ($i = 0; $i < $n; $i++) {
				$uid = $this->available_maps[$this->chosen_map_index]
														['users_on_map'][$i];
				if (!$uid || ($uid == $user->get_id())) {
					unset($this->available_maps[$this->chosen_map_index]
														['users_on_map'][$i]);
				}
			}
			// Es wurde nun mittendrin ein User rausgelöscht. Mit array_values()
			// werden alle Werte im Array neu indiziert und somit die
			// entstandene Lücke durch das Löschen geschlossen.
			$this->available_maps[$this->chosen_map_index]['users_on_map'] =
				array_values($this->available_maps[$this->chosen_map_index]
															['users_on_map']);
		}
		$this->reset_chosen_map_index();
	}

	/**
	 * Hilfsfunktion.
	 * @see toggle_ready_state
	 * @throws HackattackException
	 * @throws Exception
	 * @throws ExceptionWithErrorlog,
	 * @throws IllegalArgumentException
	 */
	protected function toggle_ready_state_logic() {
		$user = User::get_instance_by_id($this->uid);
		if ($user->get_waiting_state() == IUser::STATE_WAITING_ON_MAP) {
			$user->set_waiting_state(IUser::STATE_WAITING_ON_MAP_AND_READY);

			/**
			 * Bei direktem Start wird eine Karte direkt gestartet wenn leer,
			 * wenn bereits jemand drauf, dann soll ein User entsprechend auf
			 * die laufende Karte drauf.
			 */
			if ($this->direct_start) {
				$map_id =
					$this->available_maps[$this->chosen_map_index]['map_id'];
				$world_id =
					$this->available_maps[$this->chosen_map_index]['world_id'];

				/**
				 * Zwei Fälle. 1.: Wenn noch die Karte noch nicht gestartet,
				 * soll die Karte sofort gestartet werden (da direct_start)
				 */
				if (!MapDescription::get_instance($world_id, $map_id)
					->is_started())
				{
					$this->start_map($this->chosen_map_index);
				}
				/**
				 * Zweiter Fall: Der User muss auf die Karte hinzugesetzt
				 * werden.
				 */
				else {
					$this->put_user_on_running_map($this->chosen_map_index);
				}
			}
			/**
			 * Bei "Start-Wenn-Karte-Voll" wird nichts gemacht, es sei denn
			 * die Karte ist voll.
			 */
			else {
				if ($this->map_is_full($this->chosen_map_index))
				{	// Wenn die Karte voll ist, dann prüfen, ob alle bereit sind
					$all_ready = true;
					foreach ($this->available_maps[$this->chosen_map_index]
													['users_on_map'] as $uid)
					{
						if (User::get_instance_by_id($uid)->get_waiting_state()
							!= IUser::STATE_WAITING_ON_MAP_AND_READY)
						{
							$all_ready = false;
							break;
						}
					}
					if ($all_ready) // Sind alle bereit, dann Karte starten
						$this->start_map($this->chosen_map_index);
				}
			}
		}
		else if ($user->get_waiting_state() ==
				IUser::STATE_WAITING_ON_MAP_AND_READY)
			$user->set_waiting_state(IUser::STATE_WAITING_ON_MAP);
		else
			throw new HackattackException("Wrong state for Toggle!",
				$this->uid, 0);
	}

	/**
	 * Wenn ein Spieler auf einer Karte wartet, kann er sich als "bereit"
	 * markieren lassen. Sind alle Spieler bereit, startet die Karte automatisch.
	 * Mit Aufruf dieser Funktion wird der Spieler als "bereit" markiert, wenn er
	 * es noch nicht war, ansonsten wird dieser Bereit-Status wieder entzogen.
	 * Wartet der User gerade gar nicht auf einer Map, so wird eine
	 * Hackattack-Exception geworfen
	 */
	public function toggle_ready_state() {
		if (self::DEBUG_VERBOSE > 0) {
			error_log("MapChooserFFA::\$INSTANCES[{$this->uid}]->toggle_ready_state called");
		}
		// MUTEX: Keine zwei Kartenzusagen gleichzeitig!!
		// (sonst würden Homesektoren doppelt vergeben)
		$sem_id = sem_get(MUTEX_MAP_CHOOSE, 1, 0600, -1);
		sem_acquire($sem_id);
		try {
			$this->toggle_ready_state_logic();
			// MUTEX
			sem_release($sem_id);
		}
		catch (IllegalArgumentException $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
		catch (Exception $e) {
			// MUTEX
			sem_release($sem_id);
			throw ($e);
		}
	}


	/**
	 * Fügt die Startgebäude für den User im angegebenen Sektor ein.
	 * @param WorldCoordinates $sector_c
	 * @param uint $uid
	 */
	private function insert_start_buildings(WorldCoordinates $sector_c, $uid)
	{
		$t_now = Time::get_t_now();
		$b_obj = Buildings::get_instance($this->uid);
		$position = $b_obj->get_free_place(
			Buildings::MOTHER_FUNGUS, $sector_c
		);

		/**
		 * Die Gebäude müssen nacheinander gesetzt werden, da ansonsten
		 * get_free_place nicht funktioniert. Das könnte man noch optimieren,
		 * indem das Buildingsobjekt aktuallisiert wird und man somit
		 * DB-Zugriffe spart, aber hey: Es ist der Mapstart, das kann ein paar
		 * Sekunden dauern... :)
		 */
		query(
			'INSERT INTO building (uid, time_built, finish_time, world_id, '.
				'map_id, sector_id, time_build_started, type,cur_level, ulc_x,'.
				'ulc_y) '.
			'VALUES '.
			"($uid, ".
			"$t_now,".
			"$t_now,".
			"{$sector_c->get_world_id()},".
			"{$sector_c->get_map_id()},".
			"{$sector_c->get_sector_id()},".
			"$t_now,".
			Buildings::MOTHER_FUNGUS.",".
			"1, ".
			$position['x'].', '.
			$position['y'].')'
		);

		$position = $b_obj->get_free_place(
			Buildings::MOTHER_FUNGUS,
			$sector_c, $position
		);
		query(
			'INSERT INTO building (uid, time_built, finish_time, world_id, '.
				'map_id, sector_id, time_build_started, type,cur_level, ulc_x,'.
				'ulc_y) '.
			'VALUES '.
			"($uid, ".
			"$t_now,".
			"$t_now,".
			"{$sector_c->get_world_id()},".
			"{$sector_c->get_map_id()},".
			"{$sector_c->get_sector_id()},".
			"$t_now,".
			Buildings::TECHCENTER.",".
			"1, ".
			$position['x'].', '.
			$position['y'].')'
		);
	}

	/**
	 * Gibt die Werte in der entsprechend korrekten SQL-Syntax zurück für eine
	 * INSERT...(tribe_id, t_appearance, amount, map_id, path) VALUES-Anfrage
	 * zum Einfügen der Starteinheiten in die DB.
	 *
	 * @param WorldCoordinates $sector_c
	 * @param uint $uid
	 * @param bool &$first_units
	 * @return string
	 */
	private function get_insert_unit_values(WorldCoordinates $sector_c,
		$uid, &$first_units)
	{
		$sector_id = $sector_c->get_sector_id();
		if ($sector_id < 10)
			$sector_id = "0$sector_id";
		$t_now = Time::get_t_now();
		$tribes = Tribes::get_instance($uid);
		try {
			$tribes->insert_new_tribe(SC_ID_CLUB, "Clubber");
		}
		catch (Exception $e) {
			/**
			 * Es kann vorkommen, dass das Siedeln abgebrochen wurde, der Tribe
			 * aber schon drin ist. Also unsauberes Clean-Up beim
			 * Transaktionsspiel. In dem Fall wäre der SC_ID_SCLUB-Tribe schon
			 * da. Das macht ja nichts, deswegen ignorieren wir diesen Fehler.
			 */
			if ($e->getCode() != Tribes::E_SAME_TRIBE_CONFIG_TWICE)
				throw $e;
		}
		$club_tribes = $tribes->get_tribes_by_race(SC_ID_CLUB);
		$club_tribe = $club_tribes[0];

		$num_start_units = 2;
		$sql_units = '';
		for ($i = 0; $i < $num_start_units; $i++) {
			if ($first_units)
				$first_units = false;
			else
				$sql_units .= ',';
			$outerwear = $i & 1;	// bei ungeraden $1 wird das 1
			$trousers = $outerwear ^ 1; // bei geraden $1 wird das 1
			$sql_units .=
				"({$club_tribe->get_id()}, ".	// tribe_id
				"$t_now, ".						// t_appearance
				"1000, ".						// amount
				"{$sector_c->get_world_id()}, ".					// world_id
				"{$sector_c->get_map_id()}, ".					// map_id
				"'$sector_id',".				// path
				'0,'.							// sex
				"$outerwear,".					// outerwear
				"$trousers,".					// trousers
				"0)";							// hairhat
		}
		return $sql_units;
	}

	/**
	 * Setzt einen Spieler auf eine laufende Karte, sofern sie noch nicht voll
	 * ist.
	 *
	 * @param uint $map_index
	 * @throws Exception
	 */
	private function put_user_on_running_map($map_index) {
		$t_now = Time::get_t_now();

		if (!$this->direct_start)
			throw new Exception("DIRECT START IS OFF! NO RUNNING MAP JOIN!");
		$map_id = $this->available_maps[$map_index]['map_id'];
		$world_id = $this->available_maps[$map_index]['world_id'];

		$map_players = MapPlayers::get_instance($world_id, $map_id);
		$users_on_map = $map_players->get_users_on_map(true);

		// 1. User eintragen, dass er auf der Karte ist
		$map_players->register_player_on_map($this->uid, $this->wished_color);

		// 2. Den Status entsprechend setzen (wie bei Mapstart,
		//	aber ohne wartezeit)
		$user = User::get_instance_by_id($this->uid);
		// Lässt den Users direkt spielen
		$user->set_state(IUser::STATE_PLAYING);
		$user->set_homesector($user->get_waiting_homesector());

		$map_id = $user->get_waiting_map_id() % 100;
		$world_id = floor($user->get_waiting_map_id() / 100);
		$user->set_waiting_state(0);
		$user->set_waiting_finished(0);
		$user->set_waiting_homesector(0);
		DBCleanAndClear::get_instance($user->get_id())
			->clear_mapstart($world_id, $map_id);

		$sector_id = $user->get_homesector() % 100;

		$sector_c = new WorldCoordinates($world_id, $map_id, $sector_id);

		// 3. Startgebäude setzen
		$this->insert_start_buildings($sector_c, $user->get_id());

		// 4. Starteinheiten
		$sql_units =
			'INSERT INTO tribes_on_map (tribe_id, t_appearance, amount, '.
				'world_id, map_id, path, sex, outerwear, trousers, hairhat) '.
			'VALUES ';
		$first_units = true;
		$sql_units .= $this->get_insert_unit_values($sector_c,
				$user->get_id(), $first_units);
		if (!$first_units)
			query($sql_units);

		// 5. Alle User auf der Insel informieren per Mail
		Language::get_instance()->load_language_file('_mail_map_full');
		for ($i = 0, $n = count($users_on_map); $i < $n; $i++) {
			error_log("sending mail to {$users_on_map[$i]->get_nick()}.");
			$users_on_map[$i]->send_email(
				str_replace(
					array('<br/>', '[USER]'), 
					array("\n", $user->get_nick_main()),
					LNG_MAIL_MAP_FULL_TOPIC
				),
				str_replace(
					array('<br/>', '[USER]'), 
					array("\n", $user->get_nick_main()),
					LNG_MAIL_MAP_FULL_TEXT
				)
			);
		}

	}

	/**
	 * Startet eine Karte, d.h., dass keine weiteren Spieler mehr auf die
	 * Karte gesetzt werden kann (z.B. weil voll), und dass die Karte
	 * nach einer gewissen Verzögerung auch wirklich startet.
	 *
	 * @param int $map_index	Die Karte available_maps[$map_index] wird gestartet
	 * @throws ExceptionWithErrormail
	 */
	private function start_map($map_index) {

		if (self::DEBUG_VERBOSE > 0) {
			error_log("MapChooserFFA::\$INSTANCES[{$this->uid}]->start_map($map_index) called");
		}
		$t_now = Time::get_t_now();

		if (!$this->direct_start && !$this->map_is_full($map_index))
			throw new ExceptionWithErrormail("", $this->uid);

		/*$add_seconds = ($t_now % (24 * 3600) - 17 * 3600) * (-1);
		$cur_hour = floor($t_now % (24 * 3600) + 2 * 3600) / 3600;
		if ($cur_hour > 6)
			$add_seconds = 24 * 3600 + $add_seconds;*/
//		$add_seconds = 3600;
		$add_seconds = 0;

		$avg_level = 0;
		$map_id = $this->available_maps[$map_index]['map_id'];
		$world_id = $this->available_maps[$map_index]['world_id'];


		/**
		 * Bäume setzen :) Das geschieht zuerst, weil das ja "Terraforming" ist.
		 */
		$this->insert_trees($map_index);

		$sql_units =
			'INSERT INTO tribes_on_map (tribe_id, t_appearance, amount, '.
				'world_id, map_id, path, sex, outerwear, trousers, hairhat) '.
			'VALUES ';

		if (self::DEBUG_VERBOSE > 1) {
			error_log(print_r($this->available_maps[$map_index],true));
		}

		$first_units = true;
		foreach ($this->available_maps[$map_index]['users_on_map'] as $uid) {
			$user = User::get_instance_by_id($uid);
			$avg_level += $user->get_last_level();

			MapPlayers::get_instance($world_id, $map_id)
				->register_player_on_map($uid, $this->wished_color);

			$user->set_state(IUser::STATE_PLAYING);
			$user->set_homesector($user->get_waiting_homesector());
			if ($add_seconds > 0) {
				$user->set_waiting_state(IUser::STATE_MAP_IS_STARTING);
				$user->set_waiting_finished($t_now + $add_seconds);
			}
			else {
				$map_id = $user->get_waiting_map_id() % 100;
				$world_id = floor($user->get_waiting_map_id() / 100);
				$user->set_waiting_state(0);
				$user->set_waiting_finished(0);
				$user->set_waiting_homesector(0);
				DBCleanAndClear::get_instance($user->get_id())
					->clear_mapstart($world_id, $map_id);
			}

			if (!$this->direct_start) {
				pv_mail($user->get_email(),
					"Es geht los!",
					"Hallo, {$user->get_nick()}!\n\n".
					"Es haben sich alle Völker auf der von dir gewählten Insel ".
					"eingefunden. Die Insel kann jetzt besucht werden!\n".
					"Logge dich dazu unter http://www.mammun.com mit ".
					"deinen Benutzerdaten ein\n\n".
					"Deine Inselbewohner erwarten dich bereits..."
				);
			}

			$sector_id = $user->get_homesector() % 100;

			// Startgebäude setzen
			$sector_c = new WorldCoordinates($world_id, $map_id, $sector_id);
			$this->insert_start_buildings($sector_c, $user->get_id());

			// Starteinheiten
			$sql_units .= $this->get_insert_unit_values($sector_c,
				$user->get_id(), $first_units);

		}
		/**
		 * Nur ausführen, wenn mindestens ein Datum eingefügt werden soll.
		 */
		if (!$first_units)
			query($sql_units);

		$avg_level /= $this->available_maps[$map_index]['num_players'];
		$this->insert_artifacts($map_index, $avg_level);
		$host_control = HostControl::get_instance($world_id, $map_id);
		$host_control->drop_initial_guard_into_sectors($avg_level);
		/*TODO: Hostgebäude

		$this->build_host_buildings($map_id, $players_on_map);
		$this->create_briefing($map_id, $players_on_map);
		$this->add_mercenaries($map_id, $players_on_map);
		if ($this->type == SP_MAP)
			$this->drop_additional_workers($map_id);
		 */

		/**
		 * Das Bio wird nun angepasst. Je höher das Level ist, desto mehr Bio
		 * ist verfügbar. Der Grund hierfür ist einfach: Alles wird mit erhöhtem
		 * Fortschritt teurer. Ein alternatives Balancing über eine bessere
		 * Ausbeute der Sektoren ist aber auch denkbar.
		 */
		$bio_mult = sqrt($avg_level * 3) / 10 + 1;
		/* kann mit aktueller Formel nicht passieren
		if ($bio_mult < 1)
			$bio_mult = 1;*/
		$sql =
			'UPDATE maps SET state=0,started='.(Time::get_t_now() + $add_seconds).', '.
			"	avg_player_level=$avg_level ".
			"WHERE world_id=$world_id AND map_id=$map_id";
		query ($sql);
		$sql =
			"UPDATE sectors set bio=bio*$bio_mult ".
		 	"WHERE world_id=$world_id AND map_id=$map_id";
		query ($sql);
	}



	/**** KLEINE HELFER-FUNKTIONEN ****/
	/**
	 * Prüft ob die Karte mit dem übergebenen Index voll von Spielern ist. Gibt
	 * in diesem Fall true, sonst false zurück.
	 * @return bool
	 */
	private function map_is_full($map_index) {
		return ($this->available_maps[$map_index]['num_players'] ==
			count($this->available_maps[$map_index]['users_on_map']));
	}
}


?>
