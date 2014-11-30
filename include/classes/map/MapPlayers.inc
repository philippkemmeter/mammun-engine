<?php

/**
 * Mithilfe dieser Klasse lässt sich herausfinden, welche Spieler auf welcher
 * Karte gerade spielen (also nicht warten, sondern wirklich aktiv spielen),
 * und welchen Zustand die Spieler in Bezug auf diese Karte haben - z.B. ob
 * sie noch auf der Karte sind oder verdrängt wurden, oder ob sie vielleicht
 * sogar gewonnen haben :)
 */

require_once (dirname(__FILE__).'/../../shared/classes/IMapPlayers.inc');
require_once (dirname(__FILE__).'/../User.inc');

class MapPlayers implements IMapPlayers {
	private /*array*/ $player_data;
	private /*uint*/ $world_id;
	private /*uint*/ $map_id;
	private /*bool*/ $map_is_over;

	private static $INSTANCES;
	private static $LAST_MAP_LINKS;

	/**
	 * Erzeugt ein neues MapPlayers-Objekt, das sich auf die angegebene
	 * Karte auf der angegebenen Welt bezieht und die Spieler auf einer
	 * Karte verwaltet und gruppiert.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param bool $force		Alle Caches umgehen
	 * @return MapPlayers
	 */
	public static function get_instance($world_id, $map_id, $force=false) {
		$map_id_old = $map_id;
		$map_id = (int)$map_id;
		if ($map_id < 0) {
			error_log(print_r(debug_backtrace(false),true));
			throw new IllegalArgumentException("map_id", 'N0', $map_id_old);
		}
		$world_id_old = $world_id;
		$world_id = (int)$world_id;
		if ($world_id < 0) {
			error_log(print_r(debug_backtrace(false),true));
			throw new IllegalArgumentException("world_id", 'N0', $world_id_old);
		}
		if (!isset(self::$INSTANCES[$world_id][$map_id]))
			self::$INSTANCES[$world_id][$map_id] = new MapPlayers($world_id,
				$map_id);
		else if ($force) {
			self::$INSTANCES[$world_id][$map_id]->init_from_db();
		}
		return self::$INSTANCES[$world_id][$map_id];
	}

	/**
	 * Gibt die Instanz der letzten Karte des Spielers zurück.
	 *
	 * Die letzte Karte ist die, auf der er als letztes gespielt hat.
	 *
	 * @param uint $uid
	 * @param bool $force		Alle Caches umgehen
	 * @return MapPlayers
	 */
	public static function get_last_map_instance($uid, $force=false) {
		$uid = (int)$uid;
		$map_data = null;
		if (!self::$LAST_MAP_LINKS[$uid] || $force) {
			$res = query(
				"SELECT * FROM players_on_map ".
				"WHERE user_id=$uid ORDER BY kill_time DESC LIMIT 1;"
			);
			self::$LAST_MAP_LINKS[$uid] = new stdClass();
			if (mysql_num_rows($res) == 0) {
				self::$LAST_MAP_LINKS[$uid]->world_id = 0;
				self::$LAST_MAP_LINKS[$uid]->map_id = 0;
			}
			else {
				$map_data = mysql_fetch_assoc($res);
				self::$LAST_MAP_LINKS[$uid]->world_id = $map_data['world_id'];
				self::$LAST_MAP_LINKS[$uid]->map_id = $map_data['map_id'];
			}
		}
		$world_id = self::$LAST_MAP_LINKS[$uid]->world_id;
		$map_id = self::$LAST_MAP_LINKS[$uid]->map_id;
		if (!self::$INSTANCES[$world_id][$map_id])
			self::$INSTANCES[$world_id][$map_id] = new MapPlayers(
				$world_id, $map_id, $map_data);
		return self::$INSTANCES[$world_id][$map_id];
	}

	/**
	 * Der Konstruktor der Klasse ist nicht öffentlich.
	 *
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param assoc $data		Initialisierungsdaten als DB
	 				override
	 * @see MapPlayers#get_instance
	 */
	protected function __construct($world_id, $map_id, $data=null) {
		$this->world_id = $world_id;
		$this->map_id = $map_id;

		if ($data)
			$this->init_from_data($data);
		else
			$this->init_from_db();
	}

	/**
	 * Speichert alle Daten des Objekts in die Datenbank zurück
	 */
	private function save() {
		foreach ($this->player_data as $uid => $data) {
			$killer_id = (int)$data['killer_id'];
			$handicapped= (int)$data['handicapped'];
			$last_map_score = (int)$data['last_map_score'];
			$kill_time = (int)$data['kill_time'];
			$start_time = (int)$data['start_time'];
			query (
				"UPDATE players_on_map SET killer_id=$killer_id, ".
					"kill_time=$kill_time, handicapped=$handicapped, ".
					"start_time=$start_time, ".
					"last_map_score=$last_map_score ".
				"WHERE user_id=$uid AND world_id={$this->world_id} ".
					"AND map_id={$this->map_id}"
			);
		}
	}

	/**
	 * Initialisiert die Daten des Objekts aus der Datenbank anhand
	 * der bereits zugewiesenen World- und Map-ID.
	 */
	private function init_from_db() {
		$res = query (
			'SELECT * FROM players_on_map '.
				'WHERE world_id='.$this->world_id.' AND '.
				'map_id='.$this->map_id.' '.
			'ORDER BY user_id'
		);
		while ($row = mysql_fetch_assoc($res))
			$this->player_data[(int)$row['user_id']] = $row;

		if ($this->world_id && $this->map_id) {
			$this->map_is_over =
				(MapDescription::get_instance($this->world_id, $this->map_id)
					->get_state() == MapDescription::STATE_MAP_FINISHED);
		}
		else {
			$this->map_is_over = true;
		}
	}

	/**
	 * Setzt die Daten des Objektes initial gleich derer, die durch
	 * <code>$data</code> übergeben werden.
	 *
	 * @param assoc $data
	 */
	private function init_from_data($data) {
		foreach ($data as $key => $row)
			$this->player_data[$data['user_id']] = $row;

		if ($this->world_id && $this->map_id) {
			$this->map_is_over =
				(MapDescription::get_instance($this->world_id, $this->map_id)
					->get_state() == MapDescription::STATE_MAP_FINISHED);
		}
		else {
			$this->map_is_over = true;
		}
	}

	/**
	 * Registriert den Spieler mit der angegebenen UID, dass er auf dieser Karte
	 * spielt.
	 *
	 * ACHTUNG: Diese Funktion wird von MapChooser*-Klassen aufgerufen und
	 * VON SONST KEINEM! NIEMALS DIREKT AUFRUFEN!
	 * Leider kennt PHP keine Friends.... :'-(
	 *
	 * @param uint $uid
	 * @param ENUM $color
	 */
	public function register_player_on_map($uid, $color=0) {
		$map_id = $this->map_id;
		$world_id = $this->world_id;
		$uid = (int)$uid;

		/**
		 * Schwarz (Code 0) ist eine reservierte Farbe
		 */
		if ($color && ((strtolower($color) == 'black') ||
			$color == 1))
		{
				$color = 0;
		}

		$handicapped = (User::get_instance_by_id($uid)->get_cur_level()
				> World::get_instance($world_id)->get_maximum_level()) ? 1 : 0;


		$user = User::get_instance_by_id($uid);
		query("START TRANSACTION");
		if ($color) {
			/**
			 * Es wurde eine Wunschfarbe gewählt.
			 */
			$chosen_colors = null;
			$res = query(
				"SELECT color AS color_str, color+0 AS color_int ".
					"FROM players_on_map ".
				"WHERE world_id={$this->world_id} AND map_id={$this->map_id}"
			);
			while ($row = mysql_fetch_assoc($res)) {

				if (($color == $row['color_str']) ||
					($color == $row['color_int']))
				{
					/**
					 * Die Farbe kann nicht zugewiesen werden, weil sie schon
					 * vergeben ist
					 */
					$color = 0;
					$chosen_colors[$row['color_int']] = true;
				}
			}
			/**
			 * Alle Farben durchgehen und eine freie zuweisen.
			 */
			if (!$color) {
				for ($i = 2; $i < 10; $i++) {
					if (@!$chosen_colors[$i]) {
						$color = $i;
						break;
					}
				}
			}
		}
		/**
		 * Der Spieler hat keine Wunschfarbe
		 */
		else {
			$res = query(
				"SELECT color+0 AS color FROM players_on_map ".
				"WHERE world_id={$this->world_id} AND map_id={$this->map_id}"
			);
			$chosen_colors = null;
			while ($row = mysql_fetch_assoc($res)) {
				$chosen_colors[$row['color']] = true;
			}
			for ($i = 2; $i < 10; $i++) {
				if (@!$chosen_colors[$i]) {
					$color = $i;
					break;
				}
			}
		}
		// FIXME: get_wating_homesector is obsolete!
		query (
			"INSERT INTO players_on_map SET world_id=$world_id, ".
				"map_id=$map_id, ".
				"user_id=$uid, color='$color', ".
				"start_time=".Time::get_t_now().", ".
				"handicapped=$handicapped, ".
				"homesector=".($user->get_waiting_homesector()%100)
		);

		/**
		 * $color ist ein INT. Wir wollen aber den ENUM-String.
		 * Deswegen müssen wir die DB nochmal explizit danach fragen fragen,
		 * weil wir die Zuordnung von den INTs zum ENUM-String nicht kennen.
		 */
		$col_str = mysql_result(
			query(
				"SELECT color FROM players_on_map WHERE ".
					"world_id=$world_id AND map_id=$map_id ".
					"AND user_id=$uid"
			), 0
		);
		query("COMMIT");

		$this->player_data[$uid]['killer_id'] = 0;
		$this->player_data[$uid]['kill_time'] = 0;
		$this->player_data[$uid]['start_time'] = Time::get_t_now();
		// FIXME: get_waiting_homesector is obsolete
		$this->player_data[$uid]['homesector'] =
			$user->get_waiting_homesector()%100;
		$this->player_data[$uid]['handicapped'] = $handicapped;
		$this->player_data[$uid]['color'] = $col_str;
	}

	/**
	 * Wechselt die Farbe des Spielers.
	 * Gibt die neue Farbe als String zurück.
	 *
	 * @param N+ $uid
	 * @param ENUM $color
	 * @throws Exception
	 */
	public function change_color($uid, $color) {
		/**
		 * Schwarz (Code 0) ist eine reservierte Farbe
		 */
		if ((is_anint($color) && ($color == 0))
				|| (strtolower($color) == 'black'))
		{
			throw new Exception("Color ($color) already chosen by another player", 0);
		}
		query("START TRANSACTION");
		$res = query(
			"SELECT color AS color_str, color+0 AS color_int ".
				"FROM players_on_map ".
			"WHERE world_id={$this->world_id} AND map_id={$this->map_id}"
		);
		if (mysql_num_rows($res) == 0) {
			query("ROLLBACK");
			throw new Exception("User $uid is not on this Map ".
				"($this->world_id, $this->map_id)");
		}
		while ($row = mysql_fetch_assoc($res)) {

			if (($color == $row['color_str']) ||
				($color == $row['color_int']))
			{
				query("ROLLBACK");
				throw new Exception("Color ($color) already chosen by another player",0);
			}
		}
		try {
			query(
				"UPDATE players_on_map SET color='$color' ".
				"WHERE world_id={$this->world_id} AND map_id={$this->map_id} ".
					"AND user_id=$uid"
			);
		}
		catch(Exception $e) {
			query("ROLLBACK");
			throw new Exception("Farbe konnte nicht gesetzt werden: $color", 1);
		}

		if (is_anint($color)) {
			/**
			 * $color ist ein Integer und wir wollen den ENUM-String.
			 * Deswegen müssen wir die DB nochmal explizit danach fragen,
			 * weil wir die Zuordnung von den INTs zum ENUM-String nicht kennen.
			 */
			$col_str = mysql_result(
				query(
					"SELECT color FROM players_on_map ".
					"WHERE world_id={$this->world_id} ".
						"AND map_id={$this->map_id} ".
						"AND user_id=$uid"
				), 0
			);
		}
		query("COMMIT");
		$this->player_data[$uid]['color'] = $col_str;
		return $col_str;
	}

	/**
	 * Gibt zurück, wie der Zustand des Users auf der Karte gerade ist.
	 * Mögliche Rückgabewerte sind MapPlayers::STATE_STILL_ALIVE,
	 * MapPlayers::STATE_KILLED, MapPlayers::STATE_VICTOR,
	 * MapPlayers::STATE_WAS_NEVER_ON_MAP.
	 *
	 * @param uint $uid		Die ID des Users
	 * @return MapPlayers::STATE_...
	 */
	public function get_user_state_on_map($uid) {
		if (empty($this->player_data[$uid]))
			return self::STATE_WAS_NEVER_ON_MAP;
		if ($this->player_data[$uid]['killer_id'])
			return self::STATE_KILLED;
		if ($this->map_is_over)
			return self::STATE_VICTOR;
		else
			return self::STATE_STILL_ALIVE;
	}

	/**
	 * Entfernt den Spieler von der Karte.
	 * Wird $auto_finish auf FALSE gesetzt, läuft die Karte beim
	 * Verlassen des letzten Spielers weiter (was i.d.R. unsinnig ist,
	 * weswegen das Gegenteil der Normalfall ist. Wird hauptsächlich bei
	 * harten Mapkills benötigt (s. World::finish_map)).
	 *
	 * Gibt zurück, ob er der letzte auf der Karte war (und somit Sieger),
	 * oder nicht (und somit Verlierer).
	 *
	 * @param uint $uid
	 * @param bool $auto_finish
	 * @return bool
	 */
	public function leave_map($uid, $auto_finish=true) {
		$sem_id = sem_get(MUTEX_LEAVE_MAP, 1, 0600, -1);
		sem_acquire($sem_id);
		try {
			if (!$this->player_data[$uid]['killer_id']) {
				$this->player_data[$uid]['killer_id'] = $uid;
				$this->player_data[$uid]['kill_time'] = Time::get_t_now();
				$this->player_data[$uid]['last_map_score']
					= MapScore::get_instance($this->world_id, $this->map_id)->get_score($uid);
				$this->save();
			}

			//TODO Tribes Stärke anpassen
			$user = User::get_instance_by_id($uid);

			/**
			 * Check, ob der Spieler, der gerade verdrängt wird, auch aktuell
			 * auf dieser Insel spielt und nicht bereits auf einer anderen ist,
			 * und seine alten Truppen und sein altes Gebiet auf einer alten
			 * Insel gerade verdrängt wurden
			 */
			if (($user->get_map_id() == $this->get_map_id())
					&& ($user->get_state() == User::STATE_PLAYING))
			{
				/**
				 * Spieler ist auch aktuell auf der Insel: Seine aktuelle
				 * Insel wird für ihn beendet.
				 */
				// DB-Clear für diesen User (Map-End-Clear)
				DBCleanAndClear::get_instance($uid)->clear_mapend($this->world_id,
					$this->map_id);

				$old_user_data = new stdClass();
				$old_user_data->waiting_state = $user->get_waiting_state();
				$old_user_data->waiting_homesector = $user->get_waiting_homesector();
				$old_user_data->waiting_finished = $user->get_waiting_finished();
				//User-Daten anpassen
				$user->set_waiting_state(0);
				$user->set_waiting_homesector(0);
				$user->set_waiting_finished(0);
				//TODO $user->add_play_time($map_time);

				//Hauptgebäude abreißen:
				Buildings::get_instance($uid)->destroy_main_buildings();

				$user_is_on_this_map = true;
			}
			else
				$user_is_on_this_map = false;

			// Schauen, ob noch jemand auf der Map lebt
			$amount_still_living = 0;
			$last_man_standing = 0;
			foreach ($this->player_data as $uid => $jnk) {
				if ($this->get_user_state_on_map($uid) != self::STATE_KILLED) {
					$amount_still_living++;
					$last_man_standing = $uid;	// wenn $amount_still_living == 1
				}
			}

			$full = $this->get_map_is_full();

			/**
			 * Wenn die Insel voll ist
			 */
			if ($full) {
				/**
				 * Wenn keiner mehr auf der Karte ist, der User also der letzte
				 * war, dann ist er der Sieger, es sei denn, er ist gegangen,
				 * bevor die Karte voll war!
				 */
				if ($amount_still_living == 0) {
					/**
					 * Wenn der Spieler schon auf einer anderen Karte spielt,
					 * dann wird hier nix mehr am State geändert.
					 */
					if (!$user_is_on_this_map)
						return true;
					/**
					 * User hat GEWONNEN :)
					 */
					$user->set_state(User::STATE_CHOOSING_SCIENCES_WON_LAST_MAP);
					$user->set_maps_won($user->get_maps_won()+1);

					if ($auto_finish) {
						World::get_instance($user->get_world_id())
							->finish_map($this->map_id, Time::get_t_now(),
										$user->get_id(), $user->get_homesector());
					}
					return true;
				}
				else {
					/**
					 * Wenn nur noch einer lebt, dann schicken wir ihm eine
					 * Nachricht, dass er gewonnen hat und gehen kann :)
					 *
					 */
					if ($amount_still_living == 1) {
						EventMessages::get_instance($last_man_standing)->add_entry(
							EventMessages::TYPE_ALL_ENEMIES_DEFEATED
						);
					}
					/**
					 * Wenn der Spieler schon auf einer anderen Karte spielt,
					 * dann wird hier nix mehr am State geändert.
					 */
					if (!$user_is_on_this_map)
						return true;
					/**
					 * Wir wurden verdrängt :(
					 */
					$user->set_state(User::STATE_CHOOSING_SCIENCES_LOST_LAST_MAP);
					$user->set_maps_lost($user->get_maps_lost()+1);
					return false;
				}
			}
			else {
				/**
				 * Wenn der Spieler schon auf einer anderen Karte spielt,
				 * dann wird hier nix mehr am State geändert.
				 */
				if (!$user_is_on_this_map)
					return true;
				/**
				 * User wurde verdrängt :(
				 */
				$user->set_state(User::STATE_CHOOSING_SCIENCES_LOST_LAST_MAP);
				$user->set_maps_lost($user->get_maps_lost()+1);
				return false;
			}
		}
		catch (Exception $e) {
			/**
			 * Manuelles Rollback
			 */

			sem_release($sem_id);
			throw ($e);
		}
		sem_release($sem_id);
	}

	/**
	 * Gibt zurück, ob die Karte voll besetzt ist.
	 *
	 * Dabei können durchaus noch Plätze frei sein, die aber niemals belegt
	 * werden können, weil die Insel permanent abgeschlossen wurde.
	 *
	 * @return bool
	 * @see MapDescription#is_closed
	 */
	public function get_map_is_full() {
		$map_desc = MapDescription::get_instance($this->world_id, $this->map_id);
		if ($map_desc->is_closed()) {
			return true;
		}
		else {
			$num_players = count($this->player_data);
			$max_players = $map_desc->get_max_player_amount();
			return ($num_players >= $max_players);
		}
	}

	/**
	 * Gibt die ID der Welt zurück, in der die Karte sich befindet, auf die sich
	 * das Objekt bezieht.
	 *
	 * @return uint
	 */
	public function get_world_id() {
		return $this->world_id;
	}

	/**
	 * Gibt die ID der Karte zurück, auf die sich das Objekt bezieht.
	 *
	 * @return uint
	 */
	public function get_map_id() {
		return $this->map_id;
	}


	/**
	 * Gibt die ID des Killers zurück, der den Spieler gekillt hat oder FALSE
	 * wenn er noch am Leben ist oder gar nicht auf der Karte war.
	 *
	 * @param uint $uid		Die ID des Users
	 * @return uint/FALSE
	 */
	public function get_killer_id($uid) {
		if (empty($this->player_data[$uid]) ||
			!($this->player_data[$uid]['killer_id']))
		{
			return false;
		}
		else
			return $this->player_data[$uid]['killer_id'];
	}

	/**
	 * Gibt zurück, wann der Spieler gestorben ist, oder FALSE, wenn er noch
	 * lebt oder gar nicht auf der Karte jemals war.
	 *
	 * @param uint $uid		Die ID des Users
	 * @return uint/FALSE	Zeitpunkt des Todes
	 */
	public function get_kill_time($uid) {
		if (empty($this->player_data[$uid]) ||
			!($this->player_data[$uid]['kill_time']))
		{
			return false;
		}
		else
			return $this->player_data[$uid]['kill_time'];
	}

	/**
	 * Gibt zurück, wann der Spieler diese Karte betreten hat.
	 *
	 * @return uint
	 */
	public function get_start_time($uid) {
		if (empty($this->player_data[$uid]) ||
			!($this->player_data[$uid]['start_time']))
		{
			return false;
		}
		return $this->player_data[$uid]['start_time'];
	}

	/**
	 * Gibt zurück, wieviele Punkte der Spieler am Ende der Karte hatte oder
	 * FALSE, wenn der Spieler noch auf der Karte ist oder niemals auf der
	 * Karte war.
	 *
	 * @param uint $uid		Die ID des Users
	 * @return int/FALSE
	 */
	public function get_last_score($uid) {
		if (empty($this->player_data[$uid]) ||
			!($this->player_data[$uid]['last_map_score']))
		{
			return false;
		}
		else
			return $this->player_data[$uid]['last_map_score'];
	}

	/**
	 * Gibt zurück, welchen Heimatsektor der User auf dieser Karte aktuell hat
	 * oder mal hatte. Sprich: Dieser Wert bleibt auch nach Verlassen der Karte
	 * gültig im Gegensatz zu dem, den das User-Objekt liefert.
	 * Es wird FALSE zurückgeliefert, falls der User niemals auf dieser Karte
	 * gespielt hat.
	 *
	 * @param uint $uid
	 * @return [0,1,...99]/FALSE
	 */
	public function get_homesector($uid) {
		if (empty($this->player_data[$uid]) ||
			!($this->player_data[$uid]['homesector']))
		{
			return false;
		}
		else
			return $this->player_data[$uid]['homesector'];
	}

	/**
	 * Entspricht get_homesector. Gibt den Sektor jedoch als WorldCoordinates
	 * zurück.
	 *
	 * @param uint $uid
	 * @return WorldCoordinates
	 * @see MapPlayers#get_homesector
	 */
	public function get_homesector_c($uid) {
		$hs = $this->get_homesector($uid);
		if (!$hs)
			return new WorldCoordinates(0, 0, 0);

		return new WorldCoordinates(
			$this->world_id, $this->map_id, $hs
		);
	}

	/**
	 * Gibt zurück, welche Farbe dem User auf dieser Karte zugewiesen war oder
	 * ist. Rückgabewerte sind Farbstrings.
	 * Es wird FALSE zurückgeliefert, falls der User niemals auf dieser Karte
	 * gespielt hat.
	 *
	 * @param uint $uid
	 * @return {'blue','green','orange','pink','purple','red','white','yellow'}
	 * 						/ FALSE
	 */
	public function get_color($uid) {
		if (empty($this->player_data[$uid]) ||
			!($this->player_data[$uid]['color']))
		{
			return false;
		}
		else
			return $this->player_data[$uid]['color'];
	}

	/**
	 * Gibt alle User auf der Karte zurück. Wird $only_alive_ones=true gesetzt,
	 * werden die bereits gestorbenen Spieler ignoriert.
	 *
	 * @param bool $only_alive_ones=false	Nur die lebenden Spieler
	 * @return User[]
	 */
	public function get_users_on_map($only_alive_ones=false) {
		$result = array();
		if (is_array($this->player_data)) {
			foreach ($this->player_data as $uid => $jnk) {
				if (!$only_alive_ones ||
					($this->get_user_state_on_map($uid) != self::STATE_KILLED))
				{
					$result[] = User::get_instance_by_id($uid);
				}
			}
		}
		return $result;
	}

	/**
	 * Gibt alle IDs der User auf der Karte zurück. Wird $only_alive_ones=true
	 * gesetzt, werden die bereits gestorbenen Spieler ignoriert.
	 * @param bool $only_alive_ones=false
	 * @return uint[]
	 */
	public function get_uids_on_map($only_alive_ones=false) {
		$result = array();
		foreach ($this->player_data as $uid => $jnk) {
			if (!$only_alive_ones ||
				($this->get_user_state_on_map($uid) != self::STATE_KILLED))
			{
				$result[] = $uid;
			}
		}
		return $result;
	}

	/**
	 * Gibt zurück, ob der Spieler mit Handicap spielt, oder nicht
	 *
	 * @return bool
	 */
	public function get_handicapped($uid) {
		if (empty($this->player_data[$uid]) ||
			!($this->player_data[$uid]['handicapped']))
		{
			return false;
		}
		else
			return $this->player_data[$uid]['handicapped'];
	}

	/**
	 * !!!ACHTUNG: Setzt alle Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCES = array();
		/*unset($_SESSION['MapPlayers']);*/
		query("DELETE FROM players_on_map");
	}

	/**
	 * @see IConvertablePHPStdClass#to_php_std_class
	 */
	public function to_php_std_class() {
		$x = new stdClass();
		$x->world_id = $this->world_id;
		$x->map_id = $this->map_id;
		$x->player_data = $this->player_data;
		$x->map_is_over = $this->map_is_over;
		$x->map_is_full = $this->get_map_is_full();
		return $x;
	}

	/**
	 * @see IConvertablePHPAssocArray#to_php_assoc_array
	 */
	public function to_php_assoc_array() {
		return array (
			'world_id'		=>	$this->world_id,
			'map_id'		=>	$this->map_id,
			'player_data'	=>	$this->player_data,
			'map_is_over'	=>	$this->map_is_over,
			'map_is_full'	=>	$this->get_map_is_full()
		);
	}
}

?>
