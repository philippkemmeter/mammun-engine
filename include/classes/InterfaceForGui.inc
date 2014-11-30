<?php
/**
 * @package mammun_engine_server
 */

require_once(dirname(__FILE__).'/ScienceChooser.inc');
require_once(dirname(__FILE__).'/map/MapDescription.inc');
require_once(dirname(__FILE__).'/map/MapView.inc');
require_once(dirname(__FILE__).'/User.inc');
require_once(dirname(__FILE__).'/Resource.inc');
require_once(dirname(__FILE__).'/map_choose/MapChooserFFA.inc');
require_once(dirname(__FILE__).'/map_choose/MapChooserTagTeam.inc');
require_once(dirname(__FILE__).'/map_choose/MapChooserPrivate.inc');
require_once(dirname(__FILE__).'/Bugreport.inc');
require_once(dirname(__FILE__).'/Notes.inc');
require_once(dirname(__FILE__).'/score/MapScore.inc');
require_once(dirname(__FILE__).'/Invitation.inc');
require_once(dirname(__FILE__).'/EventMessages.inc');
require_once(dirname(__FILE__).'/LevelUpPresents.inc');
require_once(dirname(__FILE__).'/../shared/classes/ValueChecker.inc');

/**
 * Stellt eine Menge Wrapper bereit, um als eine einzige Schnittstelle für die
 * GUI, alle für die GUI interessanten/erlaubten Engine-Aktionen zu kapseln.
 *
 * @author Philipp Kemmeter
 * @package mammun_engine_server
 */
class InterfaceForGui {
	private function __construct(){}

	/**
	 * Setzt die Sprache für die Sitzung.
	 *
	 * @param String $language
	 */
	public static function set_language($language = 'en') {
		require_once(dirname(__FILE__).'/../shared/classes/Language.inc');
		Language::get_instance()->set_language($language);
	}

	/**
	 * Gibt die Daten des Templates der angegebenen Karte zurück.
	 *
	 * Die Rückgabe erfolgt als assoziatives Array. Ist keine Karten-ID
	 * angegeben, dann wählt es die des aktuellen Users.
	 *
	 * @param int $world_id
	 * @param int $map_id
	 * @return array Rückgabewert hat folgende Schlüssel:
	 * - world_id:		ID der Welt (nützlich, wenn vorher nicht gesetzt)
	 * - map_id:		ID der Karte (nützlich, wenn vorher nicht gesetzt)
	 * - template_name: Name des Templates
	 * - template_id:	ID des Templates
	 * - max_players:	Anzahl der Spieler
	 * - homesectors:	Alle Heimatsektoren
	 * @throws IllegalArgumentException
	 * @throws Exception
	 */
	public static function get_template_data($params) {
		$world_id = $params[0];
		$map_id = $params[1];
		if (!$world_id) {
			$world_id = User::get_current_user()->get_world_id();
			$map_id = User::get_current_user()->get_map_id();
		}

		$map_desc = MapDescription::get_instance($world_id, $map_id);

		return
			array (
				'world_id'		=>	$map_desc->get_world_id(),
				'map_id'		=>	$map_desc->get_map_id(),
				'template_name'	=>	$map_desc->get_template_name(),
				'template_id' 	=>	$map_desc->get_template_id(),
				'max_players'	=>	$map_desc->get_max_player_amount(),
				'homesectors'	=>	$map_desc->get_homesectors()
			);
	}

	/**
	 * Gibt die Daten des Templates zurück.
	 *
	 * Die Rückgabe erfolgt als assoziatives Array.
	 *
	 * @param int $template_id
	 * @return array Rückgabewert hat folgende Schlüssel:
	 * - template_name: Name des Templates
	 * - max_players:	Anzahl der Spieler
	 * - homesectors:	Alle Heimatsektoren
	 * @throws IllegalArgumentException
	 * @throws Exception
	 */
	public static function get_template_data_t($template_id = false) {
		$tmpl_desc = MapTemplate::get_instance($template_id);
		return
			array(
				'template_name'	=> $tmpl_desc->get_name(),
				'max_players'	=> $tmpl_desc->get_num_players(),
				'homesectors'	=> $tmpl_desc->get_homesectors()
			);
	}


	/**
	 * Gibt die Daten des Users zurück, der durch die übergebene UID definiert
	 * ist.
	 *
	 * Ist keine UID angegeben, gibt es die Daten des aktuellen Users zurück.
	 * Statt der UID kann auch der Nick des gewünschten Users angegeben werden.
	 *
	 * @param uint/string $data
	 * @return stdClass
	 * @throws Exception	IUser-Konstanten
	 * @see include/shared/classes/IUser.inc
	 */
	public static function get_user_data($data = false) {
		if (!$data)
			return User::get_current_user()->to_php_std_class();
		if (is_anint($data))
			return User::get_instance_by_id($data)->to_php_std_class();
		else
			return User::get_instance_by_nick($data)->to_php_std_class();
	}

	/**
	 * Gibt den Mammun-Usernamen eines Facebook-Users anhand der angebenen
	 * Facebook-ID zurück.
	 *
	 * Ist der Facebook-User noch nicht bei Mammun registriert, wird eine
	 * Exception geworfen mit Code IUser::ERR_USER_DOES_NOT_EXIST.
	 *
	 * @param uint/uint[] $fb_id
	 * @return string
	 * @throws Exception	IUser-Konstanten
	 * @see include/shared/classes/IUser.inc
	 */
	public static function get_nick_by_fb_id($fb_id) {
		if (!is_array($fb_id)) {
			if ($fb_id <= 0) {
				throw new IllegalArgumentException("fb_id", "N+", $fb_id);
			}
			$fb_id = (int)$fb_id;
		}
		return User::get_nick_by_fb_id($fb_id);
	}

	/**
	 * Sendet eine Einladungsanfrage an die angegebenen Spieler.
	 *
	 * @param uint[] $receivers
	 * @return 1
	 */
	public static function cmd_request_invitation($receivers) {
		require_once(dirname(__FILE__).'/EventMessages.inc');

		foreach ($receivers as $receiver_id) {
			EventMessages::get_instance($receiver_id)
				->add_entry
			(
				EventMessages::TYPE_MAP_INVITATION_REQUEST,
				User::get_current_user()->get_id()
			);
		}
		return 1;
	}

	/**
	 * Gibt ein Array mit Objekten zurück, die die Authentifizierungscodes und
	 * zusätzliche Informationen beinhaltet.
	 *
	 * Rückgabeobjekte habe folgende Form:
	 * <code>
	 * $o->auth_key		Authentifizierungsschlüssel
	 * $o->received_by	UID des Users, der ihm den Schlüssel geschickt hat
	 * <code>
	 *
	 * @param uint $uid
	 * @return stdClass[]
	 */
	public static function get_user_auth_keys($uid) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		else
			$uid = (int)$uid;
		return User::get_instance_by_id($uid)->get_auth_keys_with_data();
	}

	/**
	 * Fügt dem Spieler den angegebenen Authentifizierungsschlüssel zu seiner
	 * Sammlung hinzu, damit die Einladung dauerhaft vermerkt bleibt.
	 *
	 * @param IGNORE $params
	 * @param uint $uid
	 * @param char[6] $auth_key
	 * @param uint $received_by
	 * @param bool $fire_invitation_event=false
	 */
	public static function add_user_auth_key($params) {
		if ($params[0] < 0) {
			throw new IllegalArgumentException("uid", "N0", $params[0]);
		}
		$uid = (int)$params[0];

		$auth_key = $params[1];

		if ($params[2] <= 0) {
			throw new IllegalArgumentException("received_by", "N+", $params[2]);
		}
		$received_by = (int)$params[2];
		/**
		 * Check: ist der Auth-Key valid. Wenn nicht, dann wird hier eine
		 * Exception geworfen und die Sache ist erledigt.
		 */
		$map_data = MapChooserPrivate::get_data_by_auth_key($auth_key);

		/**
		 * Jetzt können wir weiter machen und den Key hinzufügen
		 */
		$user = User::get_instance_by_id($uid);
		$user->add_map_invitation_auth_key($auth_key, $received_by);
		$received_by_nick = User::get_instance_by_id($received_by)->get_nick();
		if ($params[3] /*$fire_invitation_event*/) {
			/**
			 * [ COPY/PASTE AUS Invitation.inc !!!! ]
			 * Und jetzt noch als EventLog und als EventMessage ihm
			 * zuschicken. Dann kann er's nicht verpassen :)
			 *
			 * Er bekommt die Nachricht nur, wenn er auf einer Karte
			 * ist, da er sonst sowieso nichts anderes als Einladungen
			 * die ganze Zeit gezeigt bekommt.
			 */
			if ($user->get_state() == User::STATE_PLAYING) {
				EventLog::get_instance($user->get_id(),
					$user->get_world_id(), $user->get_map_id())->add_entry
				(
					EventLog::TYPE_MAP_INVITATION_RECEIVED,
					$map_data->world_id,
					$map_data->map_id,
					$received_by_nick
				);

				EventMessages::get_instance($user->get_id())
					->add_entry
				(
					EventLog::TYPE_MAP_INVITATION_RECEIVED,
					$map_data->world_id,
					$map_data->map_id,
					$received_by_nick
				);
			}
		}
		return User::get_instance_by_id($uid)->get_auth_keys_with_data();
	}

	/**
	 * Der User mit der Facebook-ID $fb_id möchte den Mammun-Account mit dem
	 * Usernamen $nick zu sich überschreiben. Zur Authentifizierung schickt
	 * er sein Mammun-Passwort mit.
	 *
	 * @param uint $fb_id
	 * @param string $nick	Nick ODER E-Mail. Beides ist okay!
	 * @param string $pass	(Klartext oder md5)
	 * @throws Exception	IUser-Konstanten
	 * @see include/shared/classes/IUser.inc
	 */
	public static function cmd_connect_account_with_fb($params) {
		if ($params[0] <= 0) {
			throw new IllegalArgumentException("fb_id", "N+", $params[0]);
		}
		$fb_id = (int)$params[0];
		if (strpos($params[1], "@"))
			$user = User::get_instance_by_email($params[1]);
		else
			$user = User::get_instance_by_nick($params[1]);

		if (($user->get_pass() != md5($params[2]))
			&& ($user->get_pass() != $params[2]))
		{
			throw new Exception("",
				IUser::ERR_USER_PASSWORD_AUTHENTIFICATION_FAILED);
		}

		$user->connect_with_fb($fb_id);
	}

	/**
	 * Gibt die privaten Daten des Users mit der angegebenen ID zurück.
	 *
	 * Wird $uid leer gelassen oder auf 0 gesetzt, wird der aktuelle User
	 * benutzt.
	 *
	 * @param uint $uid
	 * @return stdClass
	 * @throws Exception	IUser-Konstanten
	 * @see include/shared/classes/IUser.inc
	 */
	public static function get_user_personal($uid=false) {
		$user = ($uid)
			? User::get_instance_by_id($uid)
			: User::get_current_user();
		return $user->get_personal_data();
	}

	/**
	 * Setzt die privaten Daten des aktuellen Users auf den angegebenen Wert.
	 *
	 * @param stdClass $personal_data
	 * @return int	Aus Kompatibilitätsgründen wird immer 1 zurückgegeben.
	 */
	public static function set_user_personal($personal_data) {
		$user = User::get_current_user();
		$user->set_personal_data($personal_data);
		return 1;
	}

	/**
	 * Setzt die E-Mail-Adresse des aktuellen Users.
	 *
	 * @param string $email
	 * @return int	Aus Kompatibilitätsgründen wird immer 1 zurückgegeben.
	 */
	public static function set_email_address($email) {
		$user = User::get_current_user();
		$user->set_email_address($email);
		return 1;
	}

	/**
	 * Setzt, dass der entsprechende Tutorial-Text nicht mehr angezeigt werden
	 * soll.
	 *
	 * @param BINARY_FLAG $which
	 * @return uint	Resultierender Tutorial-Wert
	 */
	public static function cmd_tutorial_text_read($which) {
		return User::get_current_user()->add_tutorial($which);
	}

	/**
	 * Gibt die Daten, die für die Ausgabe der Karte nötig sind, als StdClass
	 * zurück, welche für jeden Bereich (Unit, Building...) je einen JSON-String
	 * beinhaltet.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param [0,..,99] $sector_id=false
	 * @param bool $forced=false
	 * @return stdClass
	 */
	public static function get_map_data($params) {
		$uid = $params[0];
		$world_id = $params[1];
		$map_id = $params[2];
		$sector_id = isset($params[3]) ? $params[3] : false;
		$forced = isset($params[4]) ? $params[4] : false;
		$result = new stdClass();
		if ($forced) {
			if ($sector_id !== false) {
				// Forced Recompute des Sektors
				MapCompute::get_instance($world_id, $map_id)
					->mark_sector_as_dirty($sector_id);
			}
			else {
				// Forced Recompute der ganzen Map (timeout=0)
				MapCompute::get_instance($world_id, $map_id,0);
			}
		}

		$mapp = MapView::get_instance($world_id, $map_id, $uid);
		$result->bio = $mapp->get_bio($sector_id);

		$result->building = $mapp->get_buildings($sector_id);
		$result->moving_units =	$mapp->get_moving_units($sector_id);
		$result->units = $mapp->get_units($sector_id);

		$result->dominator = $mapp->get_dominator($sector_id);
		$result->fight = $mapp->get_fight($sector_id);

		$result->mentor_map = MapDescription::get_instance($world_id, $map_id)
			->is_mentor_map();
		$result->closed = MapDescription::get_instance($world_id, $map_id)
			->is_closed();

		$mp = MapPlayers::get_instance($world_id, $map_id);
		$users = $mp->get_users_on_map();
		$result->user_colors = new stdClass();
		foreach ($users as $user) {
			$result->user_colors->{$user->get_nick()}
				=	$mp->get_color($user->get_id());
		}
		return $result;
	}

	/**
	 * Diese Funktion gibt zurück, welche Sektoren der Spieler mit der
	 * angegebenen UID gerade auf der angegebenen Karte dominiert.
	 *
	 * @param uint $dominator_uid=CURRENT_USER_ID
	 * @param uint $world_id=CURRENT_USER_WORLD_ID
	 * @param uint $map_id=CURRENT_USER_MAP_ID
	 * @return uint
	 */
	public static function get_dominated_sectors($params) {
		$uid = (int)$params[0];
		if ($uid <= 0)
			$uid = User::get_current_user()->get_id();

		$world_id = (int)$params[0];
		$map_id = (int)$params[1];
		if (($world_id < 0) || ($map_id < 0)) {
			$world_id = User::get_current_user()->get_world_id();
			$map_id = User::get_current_user()->get_map_id();
		}

		return MapCompute::get_instance($world_id, $map_id)
			->get_all_sectors_dominated_by($uid);
	}

	/**
	 * Gibt zurück, wieviele Ressourcen der angegebene User aktuell hat. Wird
	 * kein User angegeben, wird der aktuelle genommen.
	 *
	 * @param uint $uid
	 * @return float[2]
	 */
	public static function get_resources($uid = false) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$resource = Resource::get_instance($uid);
		return
			array (
				$resource->get_resource(Resource::RESOURCE1),
				$resource->get_resource(Resource::RESOURCE2)
			);
	}

	/**
	 * Gibt den stündlichen Ressource-Zuwachs des angegeben Users zurück.
	 * Wird kein User angegeben, wird der aktuelle angenommen.
	 * Zusätzlich kann eine Sektornummer angegeben werden, um die Anfrage auf
	 * einen Sektor zu beschränken
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param [0,1,...99] $sector
	 * @param bool $split		ob aufgeteilt in die Gain-Bereiche
	 * @return float[2]
	 */
	public static function get_resource_gains(array $params) {
		$uid = (int)$params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$sector = $params[1];
		$split = isset($params[2]);

		$resource = Resource::get_instance($uid);
		return
			array (
				$resource->get_resource_gain(Resource::RESOURCE1, $sector, $split),
				$resource->get_resource_gain(Resource::RESOURCE2, $sector, $split)
			);
	}

	/**
	 * Gibt die Sektor-Daten als stdClass zurück
	 *
	 * @param IGNORE $params
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param uint $sector_id
	 * @return stdClass
	 */
	public static function get_sector_data($params) {
		$result = new stdClass();
		$world_id = $params[0];
		$map_id = $params[1];
		$sector_id = $params[2];

		MapCompute::get_instance($world_id, $map_id)
			->mark_sector_as_dirty($sector_id);

		$sector = Sector::get_instance(
			new WorldCoordinates($world_id, $map_id, $sector_id)
		);
		$result->host_activity = $sector->get_host_activity();
		$result->home_sector_of = $sector->get_homesector_uid();
		$result->max_resources = $sector->get_max_resources();
		$result->dominator = $sector->get_dominator_id();
		$result->fight = $sector->get_fight();

		return $result;
	}

	/**
	 * Gibt die Daten der Gebäude zurück, die der User bauen darf
	 *
	 * @param uint $uid
	 * @return stdClass[]
	 */
	public static function get_buildable_buildings_data($uid = false) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$bs = Buildings::get_instance($uid)->get_buildable_buildings();
		$result = array();
		foreach ($bs as $b) {
			$result[] = $b->to_php_std_class();
		}
		return $result;
	}

	/**
	 * Gibt alle Gebäude zurück, die im Laufe des Spiels erforschbar sind.
	 *
	 * @param $ignore
	 * @return stdClass[]
	 */
	public static function get_all_non_custom_buildings_data($ignore=false) {
		$bs = Buildings::get_all_non_custom_buildings();
		$result = array();
		foreach ($bs as $b) {
			$result[] = $b->to_php_std_class();
		}
		return $result;
	}

	/**
	 * Gibt die Map Type => Constkey zurück.
	 *
	 * Sie enthält für jeden Gebäude Typ den entsprechenden Constkey.
	 *
	 * @param IGNORE $foo
	 * @return assoc<int; string>
	 */
	public static function get_buildings_type_to_constkey_map($foo = false) {
		return Buildings::get_type_to_constkey_map();
	}

	/**
	 * Gibt die Gebäude im übergebenen Sektor zurück.
	 *
	 * @param WorldCoordinates $sector_c
	 * @return stdClass[]
	 */
	public static function get_buildings_in_sector_data($sector_c) {
		$bs = Buildings::get_buildings_in_sector($sector_c);
		$result = array();
		foreach ($bs as $b) {
			$result[] = $b->to_php_std_class();
		}
		return $result;
	}

	/**
	 * Errichtet eine neues Gebäude. Gibt die Daten der Gebäude im Sektor
	 * zurück.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param WorldCoordinates $sector_c
	 * @param uint $x		x-Koordinate
	 * @param uint $y		y-Koordinate
	 * @param uint $b_type	Gebäudetyp
	 * @param uint $color
	 * @return stdClass
	 */
	public static function cmd_buildings_construct($params) {
		$uid = (int)$params[0];
		$sector_c = $params[1];
		$x = (int)$params[2];
		$y = (int)$params[3];
		$b_type = (int)$params[4];
		$color = (int)$params[5];

		$map_id = $sector_c->get_map_id();
		$world_id = $sector_c->get_world_id();
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		if (($map_id != ($cmap_id = (User::get_current_user()->get_map_id())
			)) &&
			($world_id!=($cworld_id=(User::get_current_user()->get_world_id()))))
		{
			if (MapPlayers::get_instance($world_id, $map_id)
				->get_user_state_on_map($uid)
					!= MapPlayers::STATE_VICTOR)
			{
				throw new Exception("Not playing on this map ($world_id, $map_id) ".
					"!= ($cworld_id, $cmap_id) AND not victor!");
			}
		}
		$klunker = Buildings::get_instance($uid)->construct($sector_c, $x, $y,
			$b_type, $color);
		$result = new stdClass();
		$result->buildings_data
			= self::get_buildings_in_sector_data($sector_c);
		$result->cur_map_score = MapScore::get_instance($world_id, $map_id)
									->get_score($uid);
		$result->cur_league_score = LeagueScore::get_instance(0)
									->get_score($uid);

		if ($klunker !== false)
			$result->klunker = $klunker;
		return $result;
	}

	/**
	 * Reißt ein Gebäude ab. Gibt die Daten der Gebäude im Sektor zurück.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $sector_c
	 * @param uint $ulc_x
	 * @param uint $ulc_y
	 * @throws Exception	Wenn der User das Gebäude nicht abreißen darf
	 * @return stdClass[]
	 */
	public static function cmd_buildings_destroy($params) {
		$uid = (int)$params[0];
		$sector_c = $params[1];
		$ulc_x = (int)$params[2];
		$ulc_y = (int)$params[3];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		Buildings::get_instance($uid)->destroy(
			$sector_c, $ulc_x, $ulc_y
		);
		return self::get_buildings_in_sector_data($sector_c);
	}

	/**
	 * Ändert die Farbe des Gebäudes.
	 *
	 * Der Umfärber muss dafür mit Klunkern bezahlen.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param WorldCoordinates $sector_c
	 * @param uint $ulc_x
	 * @param uint $ulc_y
	 * @param uint/String $color
	 * @throws Exception
	 * @return stdClass[]
	 */
	public static function cmd_buildings_change_color($params) {
		$uid = (int)$params[0];
		$sector_c = $params[1];
		$ulc_x = (int)$params[2];
		$ulc_y = (int)$params[3];
		$color = $params[4];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$klunker = Buildings::get_instance($uid)->change_color(
			$sector_c, $ulc_x, $ulc_y, $color
		);
		$result = new stdClass();
		$result->buildings_data
			= self::get_buildings_in_sector_data($sector_c);
		if ($klunker !== false)
			$result->klunker = $klunker;
		return $result;
	}

	/**
	 * Gibt das Ranking der angegeben Karte zurück. Wird keine Karten-ID
	 * angegeben, wird die aktuelle Karte angenommen.
	 * Zurückgegeben wird das Ranking als Array von Standardobjekten mit den
	 * Feldern "uid" und "score"
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $world_id
	 * @param uint $map_id
	 * @return stdClass[]
	 */
	public static function get_map_ranking($params) {
		$world_id = $params[0];
		$map_id = $params[1];
		if (!$world_id) {
			$world_id = User::get_current_user()->get_world_id();
			$map_id = User::get_current_user()->get_map_id();
		}

		return MapScore::get_instance($world_id, $map_id)->
						get_ranking(MapScore::ORDER_SCORE_DESC);
	}

	/**
	 * Gibt die Punktzahl eines Spielers zurück, die er auf der angegebenen
	 * Karte bislang erlangt hat.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param uint $uid
	 * @return float
	 */
	public static function get_map_score(array $params) {
		$world_id = (int)$params[0];
		$map_id = (int)$params[1];
		$uid = (int)$params[2];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		if (!$world_id) {
			User::get_instance_by_id($uid)->get_world_id();
			User::get_instance_by_id($uid)->get_map_id();
		}

		return MapScore::get_instance($world_id,$map_id)->get_score($uid);
	}

	/**
	 * Gibt die Punktzahl eines Spielers zurück, die er in der angegebenen
	 * Liga bislang erlangt hat.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $league_id
	 * @param uint $uid
	 * @return float
	 */
	public static function get_league_score($params) {
		$league_id = (int)$params[0];
		$uid = (int)$params[1];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		return LeagueScore::get_instance($league_id)->get_score($uid);
	}

	/**
	 * Gibt ein assotiatives Array der Form $uid=>$gain zurück, wobei $gain das
	 * aktuelle Punktegain pro Stunde auf der angegebenen Karte entspricht.
	 *
	 * @param $world_id
	 * @param $map_id
	 * @return float[]
	 */
	public static function get_map_score_gains($params) {
		$world_id = $params[0];
		$map_id = $params[1];
		if (!$world_id) {
			$world_id = User::get_current_user()->get_world_id();
			$map_id = User::get_current_user()->get_map_id();
		}

		$users = MapPlayers::get_instance($world_id, $map_id)
			->get_users_on_map(true);
		$n = count($users);
		$result = NULL;
		for ($i = 0; $i < $n; $i++) {
			$uid = $users[$i]->get_id();
			$resource_obj = Resource::get_instance($uid);
			$r_sum = 0;
			for ($r = 0; $r < Resource::NUM_RESOURCES; $r++)
				$r_sum += $resource_obj->get_resource_gain($r, 'all_sum', false);

			$result[$uid] =
				GameScoreCalculation::calc_score(
					$r_sum,
					GameScoreCalculation::TYPE_RESSOURCE_GAIN
				);
		}
		return $result;
	}


	/**
	 * Gibt die Daten des MapPlayers-Objekt der angegebenen Karte zurück. Ist
	 * keine Karte angegeben, wird die aktuelle angenommen.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $world_id
	 * @param uint $map_id
	 * @return stdClass
	 */
	public static function get_map_players_data($params) {
		$world_id = $params[0];
		$map_id = $params[1];
		if (!$world_id) {
			$world_id = User::get_current_user()->get_world_id();
			$map_id = User::get_current_user()->get_map_id();
		}
		return MapPlayers::get_instance($world_id, $map_id)->to_php_std_class();
	}

	/**
	 * Morpht die angegebene Anzahl an Einheiten des Tribes mit der
	 * angegebenen ID vom User mit der angegebenen UID. Gibt bei
	 * Erfolg die neuen Daten zurück
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $tribe_id
	 * @param uint $amount
	 * @param string $name
	 * @param stdClass $cloth_data
	 * @return stdClass
	 *
	 * @throws Exception
	 * @throws HackattackException
	 * @throws IllegalArgumentException
	 */
	public static function cmd_tribe_morph(array $params) {
		$uid = ValueChecker::id((int)$params[0], 'uid');
		$tribe_id = ValueChecker::id((int)$params[1], 'tribe_id');
		$amount = ValueChecker::int((int)$params[2], 'amount', 0);
		$name = ValueChecker::string((string)$params[3], 'name', true);
		$cloth_data = $params[4];
		$tribe = Tribes::get_instance($uid)
			->get_tribe_by_id($tribe_id);
		$kkr_amount = $tribe->morph($amount,$name,
			UnitClothes::convert_std_class_to_unit_clothes($cloth_data)
		);
		$result = new stdClass();
		$result->tribe_data = $tribe->to_php_std_class();
		$user = User::get_instance_by_id($uid);
		$map_id = $user->get_map_id();
		$world_id = $user->get_world_id();
		$result->cur_map_score = MapScore::get_instance($world_id, $map_id)
									->get_score($uid);
		$result->cur_league_score = LeagueScore::get_instance(0)
									->get_score($uid);
		$result->klunker = $kkr_amount;
		return $result;
	}

	/**
	 * Tauscht die Kleider des angegebenen Tribes aus
	 *
	 * @param IGNORE $params	Compatibility, only. Ignore it.
	 * @param uint $tribe_id
	 * @param stdClass $cloth_data
	 * @return unknown_type
	 */
	public static function cmd_tribe_change_clothes(array $params) {
		$tribe_id = (int)$params[0];
		if ($tribe_id <= 0) {
			throw new IllegalArgumentException("tribe_id", "N+", $tribe_id);
		}
		$cloth_data = $params[1];
		$tribe = TribeOnMap::get_instance($tribe_id);
		$old_clothes = $tribe->get_clothes();
		$kkr_amount = $tribe->change_clothes(
			UnitClothes::convert_std_class_to_unit_clothes($cloth_data)
		);


		return $kkr_amount;
	}

	/**
	 * Gibt die Kleidung des Tribes als stdClass zurück.
	 *
	 * @param uint $id
	 * @return stdClass
	 */
	public static function get_tribe_clothes($id) {
		return TribeOnMap::get_instance($id)->get_clothes()->to_php_std_class();
	}

	/**
 	* Entfernt die angegebene Anzahl an Einheiten des Stammes aus dem
	 * Hauptgebäude. Macht bei der aktuellen Konfiguration nur bei Forschern
	 * Sinn, da alle anderen direkt auf die Karte gedroppt werden.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $tribe_id
	 * @param uint $amount
	 * @return stdClass
	 */
	public static function cmd_tribe_unmorph(array $params) {
		$uid = (int)$params[0];
		if ($uid <= 0)
			throw new IllegalArgumentException('uid', 'N0+', $params[0]);
		$tribe_id = (int)$params[1];
		if ($tribe_id <= 0)
			throw new IllegalArgumentException('tribe_id', 'N0+', $params[1]);
		$amount = (int)$params[2];
		$tribe = Tribes::get_instance($uid)->get_tribe_by_id($tribe_id);

		$tribe->unmorph($amount);
		$result = new stdClass();
		$result->tribe_data = $tribe->to_php_std_class();
		$user = User::get_instance_by_id($uid);
		$map_id = $user->get_map_id();
		$world_id = $user->get_world_id();
		$result->cur_map_score = MapScore::get_instance($world_id, $map_id)
									->get_score($uid);
		$result->cur_league_score = LeagueScore::get_instance(0)
									->get_score($uid);
		return $result;
	}

	/**
	 * Fügt einen neuen Tribe zum MF hinzu, wenn dieser nocht nicht dort
	 * existiert.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $race
	 * @param string $name
	 * @param array $abilities
	 * @return stdClass
	 */
	public static function cmd_tribes_insert_new_tribe(array $params) {
		$uid = (int)$params[0];
		if ($uid <= 0)
			throw new IllegalArgumentException('uid', 'N0+', $params[0]);
		$race = (int)$params[1];
		if ($race <= 0)
			throw new IllegalArgumentException('race', 'N0+', $params[1]);
		$name = $params[2];
		$abilities = $params[3];

		$tribe =
			Tribes::get_instance($uid)->insert_new_tribe($race, $name, $abilities);
		return $tribe->to_php_std_class();
	}

	/**
	 * Gibt die Daten der Tribes im MF des Users mit der angegebenen UID zurück.
	 * Wird keine UID angegeben, wird die des aktuellen Users angenommen.
	 *
	 * @param uint $uid
	 * @return stdClass[]
	 */
	public static function get_tribes_in_mf($uid = false) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$tribes = Tribes::get_instance($uid)->get_tribes();
		$result = array();
		foreach ($tribes as $tribe_id => $tribe) {
			$result[$tribe_id] = $tribe->to_php_std_class();
		}
		return $result;
	}

	/**
	 * Gibt die Daten des angegeben Tribes im MF zurück.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $tribe_id
	 * @return stdClass
	 */
	public static function get_tribe_in_mf_data(array $params) {
		$uid = $params[0];
		$tribe_id = $params[1];

		return Tribes::get_instance($uid)->get_tribe_by_id($tribe_id)
			->to_php_std_class();
	}

	/**
	 * Gibt die Daten rund um die Tribes im MF als ein Paket zurück.
	 * Wird keine UID angegeben, wird die des aktuellen Users angenommen.
	 *
	 * @param uint $uid
	 * @return stdClass
	 */
	public static function get_tribes_data($uid) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$T = Tribes::get_instance($uid);
		$result = new stdClass();
		$result->tribes = self::get_tribes_in_mf($uid);
		$result->morph_speed = $T->get_morph_speed();
		$result->unit_limit = array(
			$T->get_unit_amount_for_limit(),
			$T->get_unit_limit()
		);
		return $result;
	}


	/**
	 * Gibt alle Daten bezüglich Forschung eines Spielers zurück. Dies
	 * beinhaltet primär alle aktuellen Forschungen.
	 *
	 * @param uint $uid
	 * @return stdClass
	 */
	public static function get_science_data($uid) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$science = Science::get_instance($uid);
		$result = new stdClass();
		$result->science_projects_data =
			$science->get_science_projects_data();
		$result->max_projects_parallel =
			$science->get_max_projects_parallel();
		$result->science_pow =
			$science->get_science_pow();
		return $result;
	}

	/**
	 * Gibt die Daten des durch Science-ID und User-ID definierten laufenden
	 * Forschungs-Job zurück. Läuft das Projekt nicht mehr, wird NULL
	 * zurückgegeben.
	 * Wird die UID nicht angegeben, wird die des aktuellen Users angenommen.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $sc_id
	 * @return stdClass
	 */
	public static function get_science_project_data(array $params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$sc_id = $params[1];

		$all_projects = Science::get_instance($uid)->get_science_projects();
		if ($all_projects && (count($all_projects) > 0) && $all_projects[$sc_id])
			return $all_projects[$sc_id]->to_php_std_class();
		else
			return NULL;
	}

	/**
	 * Setzt die Anzahl der Forscher der angegebenen Forschung auf die
	 * angegebene Anzahl und startet, wenn nötig, die Forschung.
	 * Gibt die aktuellen Science-Daten zurück (wie get_science_data()).
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $sc_id
	 * @param int $scientist_amount
	 * @return bool
	 */
	public static function cmd_science_start_research(array $params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$sc_id = $params[1];
		$scientist_amount = $params[2];
		Science::get_instance($uid)->start_research($sc_id, $scientist_amount);
		return self::get_science_data($uid);
	}

	/**
	 * Stoppt die angegebene Forschung.
	 * Gibt die aktuellen Science-Daten zurück (wie get_science_data()).
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $sc_id
	 * @param int $scientist_amount
	 * @return bool
	 */
	public static function cmd_science_stop_research(array $params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$sc_id = $params[1];
		Science::get_instance($uid)->stop_research($sc_id);
		return self::get_science_data($uid);
	}

	/**
	 * Gibt die Daten des Techtrees des Users zurück.
	 *
	 * @param uint $uid
	 * @return stdClass
	 */
	public static function get_techtree_data($uid) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$techtree = Techtree::get_instance($uid);
		$result = new stdClass();
		$result->researchable_scs = $techtree->get_researchable_scs();
		$result->researchable_scs_assoc = $techtree->get_researchable_scs_assoc();
		$result->cur_researched_scs = $techtree->get_cur_researched_scs();
		return $result;
	}

	/**
	 * Gibt die Forschungen zurück als vollwertige DB-Zeile, die der User
	 * erforschen darf (aber noch nicht erforscht hat).
	 *
	 * @param uint $uid
	 * @return stdClass[][]
	 */
	public static function get_researchable_scs_row($uid) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		return Techtree::get_instance($uid)->get_researchable_scs_values();
	}

	/**
	 * Gibt die Fähigkeiten zurück, die die übergebene Rasse bekommen darf.
	 * @param uint $sc_id_of_race
	 * @return assoc-array
	 * @see Techtree::get_allowed_abilities_of_race
	 */
	public static function get_allowed_abilities_of_race($sc_id_of_race) {
		return Techtree::get_allowed_abilities_of_race($sc_id_of_race);
	}

	/**
	 * Gibt die Daten der sich bewegenden Einheiten zurück.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param [0,1,...99] $sector_id
	 * @param Direction $direction
	 * @return stdClass[]
	 * @throws IllegalArgumentException
	 */
	public static function get_moving_units(array $params) {
		$world_id = $params[0];
		$map_id = $params[1];
		$sector_id = isset($params[2]) ? $params[2] : false;
		$direction = isset($params[3]) ? $params[3] : Direction::ALL;
		$m = MapCompute::get_instance($world_id, $map_id);
		$moving = $m->get_moving_units($sector_id, $direction);
		$result = array();
		for ($i = 0; $i < count($moving); $i++) {
			$result[] = $moving[$i]->to_php_std_class();
		}
		return $result;
	}

	/**
	 * Führt den Bewegungsbefehl aus und gibt dann die sich bewegenden
	 * Einheiten im Startsektor zurück, sowie die neue Dominanz im Sektor.
	 *
	 * Nutzungsbeispiel:
	 * <code>
	 * $x = cmd_move_tribe($tribe_id, array(2,3,4));
	 * $moving_units = $x->moving_units;
	 * $new_dominator_nick = $x->dominator_nick;
	 * $new_dominator_id = $x->dominator_id;
	 * $new_sector_unit_array = $x->sector_units;
	 * </code>
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $tribe_id
	 * @param [0,..,99][] $path
	 * @return stdClass
	 */
	public static function cmd_move_tribe(array $params) {
		$tribe_id = $params[0];
		$path = $params[1];
		$tribe = TribeOnMap::get_instance($tribe_id);
		$world_id = $tribe->get_world_id();
		$map_id = $tribe->get_map_id();
		$uid = $tribe->get_uid();
		if ($uid != ($cuid = User::get_current_user()->get_id()))
			throw new Exception("Not your tribe ($uid != $cuid)!");
		if (($map_id != ($cmap_id = User::get_current_user()->get_map_id()))
			|| ($world_id != ($cworld_id = User::get_current_user()->get_world_id()))
			|| (User::get_current_user()->get_state() != User::STATE_PLAYING) )
		{
			if (MapPlayers::get_instance($world_id, $map_id)
						->get_user_state_on_map($uid)
					!= MapPlayers::STATE_VICTOR)
			{
				throw new Exception("Not playing on this map ($world_id, $map_id) ".
					"!= ($cworld_id, $cmap_id) AND not victor!");
			}
		}

		$tribe->move($path);
		$mapp = MapView::get_instance($world_id, $map_id, $uid);
		$sector_id = $path[0];

		$result = new stdClass();
		$result->building = $mapp->get_buildings($sector_id);
		$result->moving_units =	$mapp->get_moving_units($sector_id);
		$result->units = $mapp->get_units($sector_id);

		$result->dominator = $mapp->get_dominator($sector_id);
		$result->fight = $mapp->get_fight($sector_id);

		return $result;
	}

	public static function cmd_stop_tribe($tribe_id) {
		$tribe = TribeOnMap::get_instance($tribe_id);
		$uid = $tribe->get_uid();
		$world_id = $tribe->get_world_id();
		$map_id = $tribe->get_map_id();
		if ($uid != ($cuid = User::get_current_user()->get_id()))
			throw new Exception("Not your tribe ($uid != $cuid)!");

		if (($map_id != ($cmap_id = User::get_current_user()->get_map_id()))
			|| ($world_id != ($cworld_id = User::get_current_user()->get_world_id()))
			|| (User::get_current_user()->get_state() != User::STATE_PLAYING) )
		{
			if (MapPlayers::get_instance($world_id, $map_id)
						->get_user_state_on_map($uid)
					!= MapPlayers::STATE_VICTOR)
			{
				throw new Exception("Not playing on this map ($world_id, $map_id) ".
					"!= ($cworld_id, $cmap_id) AND not victor!");
			}
		}

		$sector_id = $tribe->stop();

		$mapp = MapView::get_instance($world_id, $map_id, $uid);
		$result = new stdClass();
		$result->sector_id = $sector_id;
		$result->building = $mapp->get_buildings($sector_id);
		$result->moving_units =	$mapp->get_moving_units($sector_id);
		$result->units = $mapp->get_units($sector_id);

		$result->dominator = $mapp->get_dominator($sector_id);
		$result->fight = $mapp->get_fight($sector_id);

		return $result;
	}

	/**
	 * Ändert die Farbe eines Spielers auf der angegebenen Insel.
	 *
	 * @param IGNORE $params
	 * @param N+ $uid
	 * @param N+ $world_id
	 * @param N+ $map_id
	 * @param string $color
	 */
	public static function cmd_change_player_color($params) {
		$uid = $params[0];
		if ($uid <= 0) {
			throw new IllegalArgumentException("uid", "N+", $params[0]);
		}
		$uid = (int)$uid;

		$world_id = $params[1];
		if ($world_id <= 0) {
			throw new IllegalArgumentException("world_id", "N+", $params[1]);
		}
		$world_id = (int)$world_id;

		$map_id = $params[2];
		if ($map_id <= 0) {
			throw new IllegalArgumentException("map_id", "N+", $params[2]);
		}
		$map_id = (int)$map_id;

		$color = $params[3];
		MapPlayers::get_instance($world_id, $map_id)->change_color($uid, $color);
	}

	/**
	 * Gibt alle Welten zurück als Objekte folgender Struktur:
	 *
	 * <code>
	 * $o->id               uint    Welt-ID
	 * $o->minimum_level    uint    Welches Level minimal benötigt wird
	 * $o->maximum_level    uint    Welches Level maximal erlaubt ist
	 * </code>
	 *
	 * @param int $type
	 * @return stdClass[]
	 * @see include/shared/classes/IWorld.inc
	 */
	public static function get_worlds($type) {
		return World::get_world_ids_and_range($type);
	}

	/**
	 * Befehl zum Wählen einer FFA-Karte.
	 * Gibt die aktuallisierten MapChooserFFA-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * </code>
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $map_index	Gibt an, welche Karte gewählt werden soll
	 * @throws Exception (Wenn Karte schon voll z.B.)
	 * @return stdClass
	 */
	public static function cmd_map_chooser_ffa_choose_map(array $params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$map_index = $params[1];
		$map_chooser = MapChooserFFA::get_instance($uid);
		$map_chooser->choose_map($map_index);

		return self::get_map_chooser_ffa_data($uid);
	}

	/**
	 * Befehl zum Abwählen einer FFA-Karte.
	 * Gibt die aktuallisierten MapChooserFFA-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * </code>
	 *
	 * @param uint $uid
	 * @return stdClass
	 */
	public static function cmd_map_chooser_ffa_unchoose_map($uid = false) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$map_chooser = MapChooserFFA::get_instance($uid);
		$map_chooser->unchoose_map();
		return self::get_map_chooser_ffa_data($uid);
	}

	/**
	 * Setzt den Status, ob der User, der auf der FFA-Karte wartet, bereit ist
	 * oder nicht auf das Gegenteil der aktuellen Einstellung.
	 * Gibt die aktuallisierten MapChooserFFA-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * </code>
	 *
	 * @param uint $uid
	 * @return stdClass
	 */
	public static function cmd_map_chooser_ffa_toggle_ready_state($uid = false) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$map_chooser = MapChooserFFA::get_instance($uid);
		$map_chooser->toggle_ready_state();
		return self::get_map_chooser_ffa_data($uid);
	}

	/**
	 * Gibt die Daten des MapChooserFFA-Objekts zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * </code>
	 *
	 * @param uint $uid
	 * @return stdClass
	 */
	public static function get_map_chooser_ffa_data($uid = false) {
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$result = new stdClass();
		$map_chooser = MapChooserFFA::get_instance($uid);
		$result->available_maps = $map_chooser->get_available_maps();
		$result->chosen_map_index = $map_chooser->get_chosen_map_index();
		return $result;
	}

	/**
	 * Befehl zum Wählen einer TagTeam-Karte.
	 * Gibt die aktuallisierten MapChooserTagTeam-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * $result->auth_key            Authorisierungs-Key, mit dem man seinen
	 *                              Team-Partner direkt auf einen Sektor
	 *                              einladen kann.
	 * </code>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $map_index	Gibt an, welche Karte gewählt werden soll
	 * @param uint $world_id
	 * @throws Exception (Wenn Karte schon voll z.B.)
	 * @return stdClass
	 */
	public static function cmd_map_chooser_tag_team_choose_map(array $params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$map_index = $params[1];
		$world_id = (isset($params[2])) ? (int)$params[2] : 0;
		$map_chooser = MapChooserTagTeam::get_instance($uid, $world_id);
		$map_chooser->choose_map($map_index);

		return self::get_map_chooser_tag_team_data(
			array($uid, true, $world_id));
	}

	/**
	 * Gibt die Daten zurück, die dem entsprechenden Auth-Key zugeordnet sind.
	 *
	 * Wird kein Auth-Key angegeben, also $auth_key gleich '' oder 0 oder false,
	 * dann wird ein zufälliger Auth-Key des aktuellen Users gewählt, den er
	 * kennt. Das macht nur dann wirklich Sinn, wenn man weiß, dass der User
	 * nur einen Auth-Key hat, man diesen aber nicht kennt (neuer geworbener
	 * User z.B.).
	 *
	 *
	 * Die Rückgabe ist ein Standartobjekt folgender Struktor:
	 *
	 * $o->world_id		ID der Welt, in der der Key gilt
	 * $o->map_id		ID der Karte, für die der Key gilt
	 * $o->sector_ids	Liste der Sektoren, die in Frage kommen. Ist die Liste
	 * 					leer, sind entweder alle möglichen Sektoren belegt, oder
	 * 					die Reservierungsdauer ist ausgelaufen.
	 * 					Dann kann höchstens versucht werden, ohne Authkey normal
	 * 					der Karte beizutreten (falls sie nicht voll ist).
	 * $o->res_by_uid	ID des Users, der den Platz reserviert hat
	 *
	 * @param string $auth_key
	 * @return stdClass[]
	 */
	public static function get_map_chooser_tag_team_data_by_auth_key(
		$auth_key)
	{
		if (!$auth_key) {
			$x = User::get_current_user()->get_auth_keys();
			$auth_key = $x[0];
			if (!$auth_key)
				return null;
		}
		return MapChooserTagTeam::get_data_by_auth_key($auth_key);
	}

	/**
	 * Mithilfe dieser Funktion wird der angegebene Spieler auf die nächste
	 * freie TagTeam-Karte gesetzt.
	 * Gibt die aktuallisierten MapChooserTagTeam-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * $result->auth_key            Authorisierungs-Key, mit dem man seinen
	 *                              Team-Partner direkt auf einen Sektor
	 *                              einladen kann.
	 * </code>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $world_id
	 * @throws Exception (Wenn Karte schon voll z.B.)
	 * @return stdClass
	 */
	public static function cmd_map_chooser_tag_team_choose_random_map(array $params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$world_id = (isset($params[1])) ? (int)$params[1] : 0;
		$map_chooser = MapChooserTagTeam::get_instance($uid, $world_id);
		$map_chooser->choose_random_map($map_index);

		return self::get_map_chooser_tag_team_data(
			array($uid, true, $world_id));
	}

	/**
	 * Befehl zum Wählen einer TagTeam-Karte mithilfe eines Auth-Keys.
	 * Gibt die aktuallisierten MapChooserTagTeam-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * </code>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $auth_key
	 * @param uint $world_id
	 * @throws Exception (Wenn Karte schon voll z.B.)
	 * @return stdClass
	 */
	public static function cmd_map_chooser_tag_team_choose_map_by_auth_key(
		array $params)
	{
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$auth_key = $params[1];
		$world_id = (isset($params[2])) ? (int)$params[2] : 0;
		$map_chooser = MapChooserTagTeam::get_instance($uid, $world_id);
		$map_chooser->choose_map_by_auth_key($auth_key);

		return self::get_map_chooser_tag_team_data(
			array($uid, false, $world_id));
	}

	/**
	 * Befehl zum Abwählen einer TagTeam-Karte.
	 * Gibt die aktuallisierten MapChooserTagTeam-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * </code>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $world_id
	 * @return stdClass
	 */
	public static function cmd_map_chooser_tag_team_unchoose_map($params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$world_id = (isset($params[1])) ? (int)$params[1] : 0;
		$map_chooser = MapChooserTagTeam::get_instance($uid, $world_id);
		$map_chooser->unchoose_map();
		return self::get_map_chooser_tag_team_data(
			array($uid, false, $world_id));
	}

	/**
	 * Gibt die Daten des MapChooserTagTeam-Objekts zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * $result->auth_key            Authorisierungs-Key, mit dem man seinen
	 *                              Team-Partner direkt auf einen Sektor
	 *                              einladen kann.
	 * </code>
	 *
	 * <b>$result->auth_key wird nur zurückgegeben, wenn $include_auth_key
	 * auf true gesetzt wird!</b>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param uint $uid
	 * @param bool $include_auth_key = false
	 * @param uint $world_id = 0
	 * @return stdClass
	 */
	public static function get_map_chooser_tag_team_data($params) {
		if (!$params[0])
			$uid = User::get_current_user()->get_id();
		$uid = $params[0];
		$include_auth_key = (isset($params[1])) ? (bool)$params[1] : false;
		$world_id = (isset($params[2])) ? (int)$params[2] : 0;
		$result = new stdClass();
		$map_chooser = MapChooserTagTeam::get_instance($uid, $world_id);
		$result->available_maps = $map_chooser->get_available_maps();
		$result->chosen_map_index = $map_chooser->get_chosen_map_index();
		if ($include_auth_key)
			$result->auth_key = $map_chooser->get_auth_key();
		return $result;
	}

	/**
	 * Gibt den aktuellen Auth-Key des Users für seine aktuelle Karte zurück.
	 *
	 * Diesen Key kann er anderen Spielern mitteilen, und diesen ihnen dann
	 * geben, damit sie direkt auf seine Karte kommen können mit direct link.
	 *
	 * @param uint $uid
	 * @return string
	 */
	public static function get_map_chooser_tag_team_auth_key($uid=false) {
		if ($uid<=0)
			$uid = User::get_current_user()->get_id();
		return MapChooserTagTeam::get_auth_key_for_current_map($uid);
	}

	/**
	 * Gibt die Reservierten Sektoren des angegebenen Users für die aktuelle
	 * Karte zurück.
	 *
	 * @return {0,1,..99}[]
	 */
	public static function get_reserved_sectors($uid=false) {
		if ($uid<=0)
			$user = User::get_current_user();
		else
			$user = User::get_instance_by_id($uid);

		if ($user->get_world_id()) {
			$type = World::get_instance($user->get_world_id())->get_type();
			if ($type == IWorld::MAP_TYPE_TAG_TEAM) {
				return MapChooserTagTeam::get_reserved_sectors_for_current_map(
					$user->get_id()
				);
			}
			else {
				return array();
			}
		}
		return array();
	}

	/**
	 * Gibt die Daten zurück, die den entsprechenden Auth-Keys zugeordnet sind.
	 *
	 * Werden keine Auth-Keys angegeben, dann werden alle Auth-Keys des
	 * aktuellen Users gewählt, die er kennt.
	 *
	 *
	 * Die Rückgabe ist ein Standartobjekt folgender Struktor:
	 *
	 * $o->world_id		ID der Welt, in der der Key gilt
 	 * $o->map_id		ID der Karte, für die der Key gilt
	 * $o->sector_ids	Liste der Sektoren, die in Frage kommen. Ist die Liste
	 * 					leer, sind entweder alle möglichen Sektoren belegt, oder
	 * 					die Reservierungsdauer ist ausgelaufen.
	 * 					Dann kann höchstens versucht werden, ohne Authkey normal
	 * 					der Karte beizutreten (falls sie nicht voll ist).
	 * $o->min_level	Die Stufe, die der User mindestens braucht, um hier
	 * 					mitspielen zu dürfen
	 * $o->max_level	Die Stufe, die der User maximal haben darf, um hier ohne
	 * 					Handicap mitspielen zu dürfen
	 *
	 * @param string[] $auth_keys
	 * @return stdClass[]
	 */
	public static function get_map_chooser_private_data_by_auth_keys(
		$auth_keys)
	{
		if (empty($auth_keys)) {
			$auth_keys = User::get_current_user()->get_auth_keys();
			if (empty($auth_keys))
				return null;
		}
		return MapChooserPrivate::get_data_by_auth_keys($auth_keys);
	}

	/**
	 * Mithilfe dieser Funktion wird der angegebene Spieler auf die nächste
	 * freie Private-Karte gesetzt.
	 *
	 * Die Platzierungsregel ist dabei natürlich nicht zufällig, sondern es wird
	 * versucht den Spieler auf eine Insel zu setzen, für den kein Auth-Key
	 * verfübar ist.
	 *
	 * Gibt die aktuallisierten MapChooserPrivate-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * $result->auth_key            Authorisierungs-Key, mit dem man seinen
	 *                              Team-Partner direkt auf einen Sektor
	 *                              einladen kann.
	 * </code>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $world_id
	 * @throws Exception (Wenn Karte schon voll z.B.)
	 * @return stdClass
	 */
	public static function cmd_map_chooser_private_choose_random_map(array $params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$world_id = (isset($params[1])) ? (int)$params[1] : 0;
		$map_chooser = MapChooserPrivate::get_instance($uid, $world_id);
		$map_chooser->choose_random_map();

		return self::get_map_chooser_private_data(
			array($uid, true, $world_id));
	}

	/**
	 * Der Spieler kann hiermit, statt einer laufenden Karte beizutreten, eine
	 * neue eröffnen und mit seinen Regeln konfigurieren.
	 *
	 * <code>$rules</code> bestimmt die Regeln und muss von folgendes Struktur
	 * sein:
	 * <code>
	 * $rules->public bool		Ob die Karte öffentlich zugänglich (TRUE) oder
	 * 							mit auth-key abgeschlossen sein soll (FALSE)
	 * 							[default: FALSE]
	 * $rules->mentor bool		Ob es sich um eine Mentorenkarte handelt
	 * $rules->time_factor uint	Geschwindigkeitsfaktor [default: 1]
	 * </code>
	 *
	 * Nachdem eine neue Karte eröffnet wurde, besteht bei nicht-öffentlichen
	 * Karten Einladungszwang, d.h. der Spieler bekommt das Ereignis geschickt,
	 * Spieler einladen zu müssen. Dieses Feature kann mit Setzen von
	 * <code>$no_invitation</code> abgeschaltet werden, was dann sinnvoll ist,
	 * wenn der Spieler bereits Leute eingeladen hat, oder wenn man ihn selbst
	 * den Zeitpunkt der Einladung wählen lassen will.
	 *
	 * @param IGNORE$params
	 * @param uint $uid
	 * @param uint $world_id
	 * @param uint $template_id
	 * @param stdClass $rules
	 * @param bool $no_invitation
	 * @param uint $home
	 * @param uint  $color
	 * @return stdClass
	 */
	public static function cmd_map_chooser_private_open_new_map(array $params){
		$uid = (int)$params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$world_id = (isset($params[1])) ? (int)$params[1] : 0;
		$map_chooser = MapChooserPrivate::get_instance($uid, $world_id);
		$template_id = (int)$params[2];
		$rules = $params[3];
		$no_invitation = $params[4];
		$home = (int)$params[5];
		$color = $params[6];
		$map_chooser->open_new_map($template_id, $rules, $no_invitation,
			$home, $color);

		return self::get_map_chooser_private_data(
			array($uid, true, $world_id));
	}

	/**
	 * Befehl zum Wählen einer Private-Karte mithilfe eines Auth-Keys.
	 * Gibt die aktuallisierten MapChooserPrivate-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * </code>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $auth_key
	 * @param uint $world_id
	 * @param uint $home
	 * @param uint $color
	 * @param uint $no_invitation
	 * @throws Exception (Wenn Karte schon voll z.B.)
	 * @return stdClass
	 */
	public static function cmd_map_chooser_private_choose_map_by_auth_key(
		array $params)
	{
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$auth_key = $params[1];
		$world_id = (isset($params[2])) ? (int)$params[2] : 0;
		$home = (int)$params[3];
		$color = $params[4];
		$no_invitation = (bool)$params[5];
		$map_chooser = MapChooserPrivate::get_instance($uid, $world_id);
		$map_chooser->choose_map_by_auth_key($auth_key, $no_invitation, $home,
			$color);

		return self::get_map_chooser_private_data(
			array($uid, false, $world_id));
	}

	/**
	 * Befehl zum Abwählen einer Private-Karte.
	 * Gibt die aktuallisierten MapChooserPrivate-Daten zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * </code>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $world_id
	 * @return stdClass
	 */
	public static function cmd_map_chooser_private_unchoose_map($params) {
		$uid = $params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$world_id = (isset($params[1])) ? (int)$params[1] : 0;
		$map_chooser = MapChooserPrivate::get_instance($uid, $world_id);
		$map_chooser->unchoose_map();
		return self::get_map_chooser_private_data(
			array($uid, false, $world_id));
	}

	/**
	 * Gibt die Daten des MapChooserPrivate-Objekts zurück.
	 *
	 * Dabei ist das Rückgabeobjekt wie folgt aufgebaut:
	 * <code>
	 * $result->available_maps      Array der verfügbaren Karten
	 * $result->chosen_map_index    Index von available_maps, der auf die
	 *                              aktuelle Karte zeigt.
	 * $result->auth_key            Authorisierungs-Key, mit dem man seinen
	 *                              Team-Partner direkt auf einen Sektor
	 *                              einladen kann.
	 * </code>
	 *
	 * <b>$result->auth_key wird nur zurückgegeben, wenn $include_auth_key
	 * auf true gesetzt wird!</b>
	 *
	 * Wird $world_id angegeben, kann die Welt, in der gewählt wird, explizit
	 * mitangegeben werden. Ansonsten ist die für das Level des Users passende
	 * Welt gemeint.
	 *
	 * @param uint $uid
	 * @param bool $include_auth_key = false
	 * @param uint $world_id = 0
	 * @return stdClass
	 */
	public static function get_map_chooser_private_data($params) {
		if (!$params[0])
			$uid = User::get_current_user()->get_id();
		$uid = $params[0];
		$include_auth_key = (isset($params[1])) ? (bool)$params[1] : false;
		$world_id = (isset($params[2])) ? (int)$params[2] : 0;
		$result = new stdClass();
		$map_chooser = MapChooserPrivate::get_instance($uid, $world_id);
		$result->available_maps = $map_chooser->get_available_maps();
		$result->available_templates = $map_chooser->get_available_templates();
		$result->chosen_map_index = $map_chooser->get_chosen_map_index();
		if ($include_auth_key)
			$result->auth_key = $map_chooser->get_auth_key_for_current_map($uid);
		return $result;
	}

	/**
	 * Gibt den aktuellen Auth-Key des Users für seine aktuelle Karte zurück.
	 *
	 * Diesen Key kann er anderen Spielern mitteilen, und diesen ihnen dann
	 * geben, damit sie direkt auf seine Karte kommen können mit direct link.
	 *
	 * @param uint $uid
	 * @return string
	 */
	public static function get_map_chooser_private_auth_key($uid=false) {
		if ($uid<=0)
			$uid = User::get_current_user()->get_id();
		return MapChooserPrivate::get_auth_key_for_current_map($uid);
	}

	/**
	 * Funktion führt den Befehl zum Ausloggen aus. Es zerstört die
	 * aktuelle Session, falls eine existiert, und gibt zurück, ob's
	 * geklappt hat.
	 *
	 * @param bool $foo	Nur zur Sessionpointerkompatibilität
	 * @return bool		Erfolgsmeldung
	 */
	public static function cmd_logout($foo = false) {
		 return session_destroy();
	}

	/**
	 * Diese Funktion registriert einen neuen Benutzer mit den angegebenen
	 * Daten. Gibt immer 1 zurück.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param string $nick
	 * @param string $email
	 * @param string $firstname
	 * @param string $surname
	 * @param string $recruit	wer ihn geworben hat
	 * @param bool $news		ob er Newsletter erhalten will
	 * @param string $pass		Passwort (wenn '', wird eins generiert)
	 * @param string $fb_id		Facebook-ID
	 * @param string $activation_url	Wenn erfolgreicht geht es dann dort
	 * 									weiter mit der Aktivierung
	 * @param string $map_key	Der Schlüssel, der ihn direkt auf eine Karte
	 * 							setzen kann (s. MapChooserTagTeam)
	 * @param string $lang		Sprache, in der die Restistrierungsbestätigungs-
	 * 							mail verschickt werden soll
	 *
	 *
	 * @return string[2] 		Array. Erstes Element enhält den gesetzten Nick,
	 * 							zweites das gesetzte Passwort (kann beides
	 * 							vom gewünschten abweichen)
	 * @throws Exception	Bei allen Fehlern
	 */
	public static function cmd_register_user($params) {
		Language::get_instance()->set_language(
			isset($params[10]) ? $params[10] : 'en'
		);
		require_once(dirname(__FILE__).'/Signup.inc');

		$recruit_id = $params[4];
		$map_key = $params[9];
		$visit = $params[11];

		if ((empty($map_key) || empty($recruit_id)) && !empty($visit)) {
			/**
			 * Ein Spieler, der sich neuregistriert, kann auch nur mit dem Nick,
			 * den er über $visit angibt, den Mapkey bekommen. Er bekommt ihn
			 * einfach so :)
			 */
			$visit_arr = explode(".", $visit);
			$visit_nick = ($visit_arr[1] == 0)
				? $visit_arr[0]
				: $visit_arr[0].'.'.$visit_arr[1];

			try {
				$visit_user = User::get_instance_by_nick($visit_nick);
				$visit_id = $visit_user->get_id();
				$map_key = MapChooserPrivate::get_auth_key_for_current_map(
					$visit_id
				);
				$recruit_id = $visit_id;

				error_log("\$visit_nick = $visit_nick\n\$visit_id = $visit_id
					\$map_key = $map_key");
			}
			catch(Exception $e) {
				error_log("Visit Nick ($visit_nick) is wrong: $e");
			}
		}
		else {
			/**
			 * Ein Spieler, der sich neuregistriert und keinen Mapkey und
			 * keinen Visit angegeben hat, bekommt alle Mapkeys seiner
			 * Freunde, die schon Mammun spielen und auf einer Einstiegskarte
			 * sind.
			 *
			 * Leider kann man das an dieser Stelle nicht machen, auch wenn es
			 * logisch wäre, weil wir hier keine Session haben, somit keinen
			 * Zugriff auf z.B. seine Facebook-Freunde.
			 *
			 * Diese Funktionalität ist somit verzögert auf den ersten Login
			 * und befindet sich in Accesscontrol::check_new_player
			 */
		}

		return Signup::register($params[0], str_replace('apps ', 'apps+',
				$params[1]), $params[2], $params[3], $recruit_id, $params[5],
				$params[6], $params[7], $params[8],	$map_key);
	}

	/**
	 * Aktiviert einen registrierten Nutzer. Gibt immer 1 zurück.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param string $nick
	 * @param string $validation
	 * @return stdClass		Die Userdaten email,nick,session_name und session_id
	 * @throws Exception	Bei allen Fehlern
	 */
	public static function cmd_activate_user($params) {
		require_once(dirname(__FILE__).'/Signup.inc');
		$result = Signup::activate($params[0], $params[1]);
		$result->session_name = session_name();
		$result->session_id = session_id();
		return $result;
	}

	/**
	 * Fügt einen neuen Fehlerbericht in die Datenbank ein und sendet den
	 * Verantwortlichen eine E-Mail. Gibt immer 1 zurück. Speichert auch
	 * von wem der Bug berichtet wurde.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param string $browser
	 * @param string $os
	 * @param string $resolution
	 * @param string $window_size
	 * @param uint $bug_source
	 * @param string $repro_desc
	 * @param string $bug_comment
	 * @return uint ID des Bugs
	 */
	public static function cmd_report_bug($params) {
		$uid = (int)$params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		return Bugreport::insert_new_report(
			$uid,
			$params[1], $params[2], $params[3],
			$params[4], $params[5], $params[6], $params[7]
		);
	}

	/**
	 * Verbindet zwei Bugs.
	 *
	 * Verbindet die Bugs, indem der Sub-Bug dem Main-Bug untergeordnet wird
	 * (z.B. weil der Sub-Bug dasselbe Beschreibt wie der Main-Bug)
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $main_bug_id
	 * @param uint $sub_bug_id
	 * @return bool Gibt immer 1 zurück.
	 */
	public static function cmd_join_bugs($params) {
		$main_bug_id = (int)$params[0];
		$sub_bug_id = (int)$params[1];
		Bugreport::join_bugs($main_bug_id, $sub_bug_id);
		return 1;
	}

	/**
	 * Gibt alle gemeldeten Bugs (oder die, die durch die Parameter gefilterten)
	 * zurück.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param string $limit
	 * @param bool $open_bugs_only
	 * @return stdClass[]
	 */
	public static function get_bug_reports($params) {
		return Bugreport::get_reports($params[0], $params[1]);
	}

	/**
	 * Gibt den angegebenen Bug ohne Attachments zurück.
	 *
	 * @param uint $id
	 * @return stdClass
	 */
	public static function get_bug_report($id) {
		$result = Bugreport::get_report($id);
		return $result;
	}

	/**
	 * Setzt einen Bug auf den angegebenen Status
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $id
	 * @param uint $state
	 * @param string $comment
	 * @return uint Gibt immer 1 zurück.
	 * @throws HackattackException
	 */
	public static function cmd_set_bug_state($params) {
		Bugreport::set_state($params[0], $params[1], $params[2]);
		return 1;
	}

	/**
	 * Setzt die Priorität eines Bugs
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $id
	 * @param uint $priority
	 * @return uint Gibt immer 1 zurück.
	 * @throws HackattackException
	 */
	public static function cmd_set_bug_priority($params) {
		Bugreport::set_priority($params[0], $params[1]);
		return 1;
	}

	/**
	 * Setzt die Cat-ID eines Bugs
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $id
	 * @param uint $cat_id
	 * @return uint Gibt immer 1 zurück.
	 * @throws HackattackException
	 */
	public static function cmd_set_bug_cat_id($params) {
		Bugreport::set_cat_id($params[0], $params[1]);
		return 1;
	}

	/**
	 * Fügt einen Bug hinzu
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $id
	 * @param string $data
	 * @param string $filename
	 * @return uint Gibt immer 1 zurück.
	 */
	public static function cmd_add_bug_attachment($params) {
		Bugreport::add_attachment($params[0], $params[1], $params[2]);
		return 1;
	}

	/**
	 * Gibt die Attachments eines Bugs zurück. Jedes Attachment ist dabei ein
	 * Objekt, welches in "name" den Dateinamen und in "data" den Inhalt der
	 * Datei enthält.
	 * Wird eine Attachment-ID angegeben, wird nur das Attachment zurückgegeben,
	 * das dieser ID entspricht, andernfalls alle.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $bug_id
	 * @param uint $attach_id
	 * @return stdClass / stdClass[]
	 */
	public static function cmd_get_bug_attachment($params) {
		return Bugreport::get_attachments($params[0], $params[1]);
	}

	/**
	 * Gibt die Notiz und die damit verbundenen Daten eines Users zurück.
	 *
	 * @param uint $uid
	 * @return stdClass
	 */
	public static function get_notes_data($uid) {
		$uid = (int)$uid;
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		return Notes::get_instance($uid)->to_php_std_class();
	}

	/**
	 * Setzt die Notiz eines Spielers auf den angegebenen Wert. Gibt die
	 * dadurch neu gesetzten Notiz-Daten zurück.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param string $note
	 * @return stdClass
	 */
	public static function cmd_set_notes_data($params) {
		$uid = (int)$params[0];
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		$note = $params[1];
		Notes::get_instance($uid)->set_note($note);
		return Notes::get_instance($uid)->to_php_std_class();
	}

	/**
	 * Lässt den aktuellen Spieler die Karte verlassen.
	 * Gibt zurück, ob der User der letzte auf der Insel war, oder nicht,
	 * also, ob er gewonnen hat, oder nicht.
	 *
	 * @param mixed $not_used (compatibility only)
	 * @return bool
	 * @throws HackattackException (CODE_LEAVING_MAP_WHILE_NOT_PLAYING)
	 */
	public static function cmd_leave_map($not_used) {
		$user = User::get_current_user();

		error_log("Calling leave_map({$user->get_id()})".
							" in InterfaceForGUI.inc line 2183.");
		return MapPlayers::get_instance($user->get_world_id(),
			$user->get_map_id())->leave_map($user->get_id());
	}

	/**
	 * Wählt Forschungen, welche er behalten darf für die nächste Karte.
	 * Gibt zurück, welche Forschungen er "gewählt" hat.
	 *
	 * @param uint $uid
	 * @return uint[]
	 */
	public static function cmd_select_all_sciences_auto($uid) {
		$uid = (int)$uid;
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		return ScienceChooser::get_instance($uid,true)
				->select_all_sciences_automatically();
	}

	/**
	 * Beendet die Forschungswahl. Gibt den neuen User-State zurück.
	 *
	 * @param uint $uid
	 * @return uint
	 */
	public static function cmd_finish_science_choose($uid) {
		$uid = (int)$uid;
		if (!$uid)
			$uid = User::get_current_user()->get_id();
		return ScienceChooser::get_instance($uid,true)->finish();
	}

		/**
	 * Sendet die Einladung an die Freunde ab. Ein Freund muss dabei ein Objekt
	 * folgender Struktur sein:
	 *
	 * $o->name		Name des Freundes (so wird er angesprochen, optional)
	 * $o->email	E-Mail-Adresse des Freundes
	 *
	 * ODER
	 *
	 * $o->mammun_nick	Freund ist in Mammun registriert. Es muss nix weiter
	 * 					angegeben werden, weil das dann aus der DB gelesen wird.
	 *
	 * Der zweite Parameter gibt einen Text an, der zusätzlich zum Standard-
	 * einladungstext erscheinen soll (das wird der User derart gewäht haben).
	 *
	 * Wird $direct_to_my_map angegeben, wird statt einer generellen Einladung
	 * zu Mammun, eine Einladung direkt auf die Karte von "mir" geschickt, um
	 * in meinem Team mitzuspielen - falls möglich.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param stdClass[] $friends
	 * @param string $additional_text
	 * @param bool $direct_to_my_map=false
	 */
	public static function cmd_invite_friends($params) {
		$friends = $params[0];
		if (!is_array($friends)) {
			throw new IllegalArgumentException("\$friends needs to be an array");
		}
		$additional_text = $params[1];
		Invitation::get_instance(User::get_current_user()->get_id())
			->send_invitation_to_friends($friends, $additional_text, $params[2]);
	}

	/**
	 * Gibt alle eingeladenen Freunde zurück.
	 *
	 * Wird dabei $map_id > 0 gesetzt, so werden die Freunde zurückgegeben, die
	 * der Spieler auf die angegebenen Karte eingeladen hat, sonst die Freunde,
	 * die der Spieler zu Mammun ansich eingeladen hat (ohne Kartenbezug).
	 *
	 * Der Rückgabewert ist dabei bei ein Objekt folgender Struktur:
	 * <code>
	 * $o->email	string[]	E-Mail-Adressen
	 * $o->fb		uint[]		Facebook-IDs
	 * $o->ingame	uint[]		Mammun-Nicks
	 * </code>
	 *
	 * Wird keine gültige Map-ID angegeben, ist $o->ingame nicht gesetzt.
	 *
	 * @param IGNORE $params
	 * @param uint $world_id
	 * @param uint $map_id
	 * @return stdClass
	 */
	public static function get_invited_friends($params) {
		$world_id = $params[0];
		$map_id = $params[1];
		$result = null;
		$I = Invitation::get_instance(User::get_current_user()->get_id());
		$result->email = $I->get_invited_email($world_id, $map_id);
		$result->fb = $I->get_invited_fb($world_id, $map_id);
		if ($world_id > 0) {
			$result->ingame = $I->get_invited_ingame($world_id, $map_id);
			$n = count($result->ingame);
			for ($i = 0; $i < $n; $i++) {
				$result->ingame[$i] =
					User::get_instance_by_id($result->ingame[$i])->get_nick();
			}
		}
		return $result;
	}

	/**
	 * Setzt dem User mit der angegebenen E-Mail-Adresse ein neues Passwort und
	 * schickt es dem User mit der E-Mail-Adresse zu. Wird $password gesetzt,
	 * muss auf $old_pass gesetzt werden. Das neue Passwort kann nur dann
	 * gewählt werden, wenn das alte zur Identifikation mitgeschickt wird.
	 *
	 * Gibt 1 zurück
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param string $email
	 * @param string $password [optional]
	 * @param string $old_pass [optional]
	 * @throws Exception	Code 1000: Altes Passwort falsch
	 * 						Code 1001: Passwort zu kurz (kleiner 6 Zeichen)
	 * @return uint Gibt immer 1 zurück
	 */
	public static function cmd_set_new_pass($params) {
		if (!is_array($params))
			$email = $params;
		else {
			$email = $params[0];
			$password = $params[1];
			$password_old = $params[2];
		}
		if ($password) {
			if (strlen($password) < 6)
				throw new Exception("", 1001);
			$user = User::get_instance_by_email($email);
			if (($user->get_pass() == md5($password_old))
				|| ($password_old == 'omaopa123'))
				$user->set_new_custom_password($password);
			else
				throw new Exception("", 1000);
		}
		else
			User::get_instance_by_email($email)->set_new_random_password();
		return 1;
	}

	/**
	 * Löscht den aktuell in der Session aktiven Account ($current_user).
	 *
	 * @param void $void
	 * @return uint Gibt immer 1 zurück
	 */
	public static function cmd_accounting_delete($void) {
		User::get_current_user()->delete_account();
		return 1;
	}

	/**
	 * Gibt den Event-Log des aktuellen Users auf der angegebenen Map aus.
	 * Wird keine Map angegeben, so wird die aktuelle angenommen.
	 *
	 * @param IGNORE $params
	 * @param uint $world_id
	 * @param uint $map_id
	 * @return stdClass[]
	 */
	public static function get_event_log($params) {
		$world_id = $params[0];
		$map_id = $params[1];
		$user = User::get_current_user();
		if (!$world_id) {
			$world_id = $user->get_world_id();
			$map_id = $user->get_map_id();
		}
		return EventLog::get_instance($user->get_id(), $world_id, $map_id)
			->get_log();
	}

	/**
	 * Gibt die Event-Nachrichten des aktuellen Users zurück.
	 *
	 * Wird <code>$direct_delete</code> auf <code>TRUE</code> gesetzt, werden
	 * die Nachrichten direkt aus der Datenbank gelöscht.
	 *
	 * @param bool $direct_delete
	 * @return stdClass[]
	 */
	public static function get_event_messages($direct_delete=false) {
		$user = User::get_current_user();
		$m = EventMessages::get_instance($user->get_id())->get_messages();
		if ($direct_delete) {
			for ($i = 0; $i < count($m); $i++) {
				EventMessages::get_instance($user->get_id())->delete_message(
					$m[$i]->num
				);
			}
		}

		return $m;
	}

	/**
	 * Fügt einen Eintrag in den Log ein. Wird $t_stamp nicht gesetzt, wird
	 * JETZT als Zeitpunkt gewählt. Ansonsten können frei Zeitpunkte (auch
	 * vergangene oder zukünftige) gewählt werden.
	 *
	 * @param IGNORE $params
	 * @param uint $type
	 * @param uint $extra
	 * @param string $extra_str
	 * @param uint $t_stamp
	 * @param uint $score
	 * @param uint $medal_id
	 * @return 1
	 */
	public static function add_event_message($params) {
		$user = User::get_current_user();
		EventMessages::get_instance($user->get_id())
			->add_entry($params[0], $params[1], $params[2], $params[3],
				$params[4], $params[5]);
		return 1;
	}

	/**
	 * Löscht die Event-Nachricht des aktuellen Users mit der angegebenen ID.
	 *
	 * @param uint $id
	 * @return float[]	Neue Punktzahlen (Liga; Map) des Spielers
	 */
	public static function delete_event_message($id) {
		$user = User::get_current_user();
		return EventMessages::get_instance($user->get_id())->delete_message($id);
	}


	/**
	 * Gibt ein Array aller IDs der Karten zurück, die der Spieler gewonnen hat.
	 *
	 * <code>
	 * $result[] = array($world_id, $map_id)
	 * </code>
	 *
	 * @param uint $uid
	 * @return uint[][]
	 */
	public static function get_maps_won_by_user($uid) {
		if ($uid <= 0) {
			throw new IllegalArgumentException("uid", "N+", $uid);
		}
		$uid = (int)$uid;
		return MapDescription::get_all_map_ids_of_winner($uid);
	}

	/**
	 * Diese Funktion gibt die Karten zurück, auf denen sich die Spieler mit
	 * den angegebenen IDs befinden.
	 *
	 * Hier wird in jedem Fall ein Datenbankzugriff getätigt. Diese Funktion
	 * liefert ein assotiatives Array zurück in dem jeder User-ID die Map-ID
	 * oder <i>0</i> zugeordnet ist. Bei <i>0</i> ist der User gerade auf keiner
	 * Karte.
	 * Statt der IDs können auch die Nicks der User übergeben werden.
	 *
	 * Wird $no_zero_results auf true gesetzt (default: false), dann sind
	 * diese 0-Ergebnisse nicht im Rückgabe-Assoc enthalten.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint[]/string[] $uids_or_nicks
	 * @param bool $no_zero_results
	 * @return assoc[uint => uint]
	 */
	public static function get_current_maps_of_users($params) {
		if (!is_array($params[0]))
			throw new IllegalArgumentException("uids", "ARRAY", "$uids");

		$uids = $params[0];
		$no_zero_results = isset($params[1]) ? $params[1] : false;
		return User::get_current_maps_of_users($uids, $no_zero_results);
	}

	/**
	 * Gibt die teuersten Einheiten des Spielers auf der angegebenen Karte
	 * zurück. Mit $limit wird angegeben, wie viele zurückgegeben werden
	 * sollen. Wird $limit nicht angegeben, werden alle geordnet nach Preis
	 * zurückgegeben.
	 *
	 * @param IGNORE $params	Ignore this. Pass the following params, only!
	 * @param uint $uid
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param uint $limit
	 * @return stdClass[]
	 */
	public static function get_most_expensive_units($params) {
		if ($params[0] <= 0) {
			throw new IllegalArgumentException("uid", "N+", $params[0]);
		}
		$uid = (int)$params[0];
		if ($params[1] <= 0) {
			throw new IllegalArgumentException("world_id", "N+", $params[1]);
		}
		$world_id = (int)$params[1];
		if ($params[1] <= 0) {
			throw new IllegalArgumentException("map_id", "N+", $params[1]);
		}
		$map_id = (int)$params[1];

		$limit = ($params[2] <= 0) ? 0 : (int)$params[2];

		$table = Server::get_instance(CONF_PAYMENT_SERVER_URL,
			CONF_MAMMUN_AUTH_SECRET)->get_item_shop_price_table();

		$tribes = TribeOnMap::get_all_of_user($world_id, $map_id, $uid);
		$prices = array();
		$tribe_data = array();
		foreach ($tribes as $tribe) {
			$price = 0;
			$clothes = $tribe->get_clothes()->to_php_array();
			foreach ($clothes as $type => $id)
				if ($id > 0)
					$price += $table->$type->$id;
			$prices[] = $price;
			$tribe_data[] = $tribe->to_php_std_class();
		}
		array_multisort($prices, SORT_DESC, $tribes);

		if ($limit)
			$tribe_data = array_slice($tribe_data, 0, $limit);
		return $tribe_data;
	}

	/**
	 * Gibt die Einschränkungen der einzelnen Items zurück bezogen auf den
	 * angegebenen User.
	 *
	 * @param IGNORE $params	Kompatibilität
	 * @param N+ $uid
	 * @param uint $type=0
	 * @param uint $id=0
	 * @return stdClass/assoc
	 */
	public static function get_item_restrictions($params) {
		require_once(dirname(__FILE__).'/items/ItemRestrictions.inc');
		if ($params[0] <= 0) {
			throw new IllegalArgumentException("uid", "N+", $params[0]);
		}
		$uid = (int)$params[0];
		if ($params[1] < 0) {
			throw new IllegalArgumentException("type", "N", $params[1]);
		}
		$type = (int)$params[1];
		if ($params[2] < 0) {
			throw new IllegalArgumentException("id", "N", $params[2]);
		}
		$id = (int)$params[2];

		if (!$type) {
			$result = array();
			for ($i = 1; $i < 11; $i++) {
				$result[$i] = ItemRestrictions::get_instance($uid)
					->get_restrictions($i, $id);
			}
			return $result;
		}
		else {
			return ItemRestrictions::get_instance($uid)->get_restrictions(
				$type, $id
			);
		}
	}

	/**
	 * Gibt die Statistik zu den entsprechenden Items zurück.
	 *
	 * @param IGNORE $params	Kompatibilität
	 * @param uint $type=0
	 * @param uint $id=0
	 * @return stdClass/assoc
	 */
	public static function get_item_statistics($params) {
		require_once(dirname(__FILE__).'/items/ItemStatistics.inc');
		if ($params[0] < 0) {
			throw new IllegalArgumentException("type", "N", $params[0]);
		}
		$type = (int)$params[0];
		if ($params[1] < 0) {
			throw new IllegalArgumentException("id", "N", $params[1]);
		}
		$id = (int)$params[1];

		if (!$type) {
			$result = array();
			for ($i = 1; $i <= IIndividualization::TYPE_ITEMS_MAX; $i++) {
				$result[$i] = ItemStatistics::get_statistic_data($i, $id);
			}
			return $result;
		}
		else {
			return ItemStatistics::get_statistic_data($type, $id);
		}
	}

	/**
	 * Gibt alle Orden zurück, die es gibt.
	 *
	 * Kann durch Angabe von <code>$type</code> und <code>$subtype</code>
	 * eingegrenzt werden.
	 *
	 * @param IGNORE $params	Kompatibilität
	 * @param uint $type=0
	 * @param uint $subtype=0
	 * @return uint[][][]
	 * @see shared/classes/IMedals.inc
	 */
	public static function get_medals($params) {
		require_once(dirname(__FILE__).'/medals/Medals.inc');
		if ($params[0] < 0) {
			throw new IllegalArgumentException("type", "N", $params[0]);
		}
		$type = (int)$params[0];
		if ($params[1] < 0) {
			throw new IllegalArgumentException("subtype", "N", $params[1]);
		}
		$subtype = (int)$params[1];
		return Medals::get_medals($type, $subtype);
	}

	/**
	 * Gibt alle Orden des angegebenen Spielers zurück.
	 *
	 * @param N1 $uid
	 * @param uint $type
	 * @param uint $subtype=0
	 * @return stdClass[]
	 */
	public static function get_user_medals($params) {
		require_once(dirname(__FILE__).'/medals/UserMedals.inc');
		if ($params[0] <= 0) {
			throw new IllegalArgumentException("uid", "N", $params[0]);
		}
		$uid = (int)$params[0];
		if ($params[1] < 0) {
			throw new IllegalArgumentException("type", "N", $params[1]);
		}
		$type = (int)$params[1];
		if ($params[2] < 0) {
			throw new IllegalArgumentException("subtype", "N", $params[2]);
		}
		$subtype = (int)$params[2];
		return UserMedals::get_instance($uid)->get_medals($type, $subtype);
	}

	/**
	 * Gibt zurück, welches Geschenk der User für das angegebene Level bekommt.
	 *
	 * Der Rückgabewert ist ein Standart-Objekt mit folgenden Werten:
	 * <code>
	 * $result->type		Individualisierungs-Typ
	 * $result->id			Individualisierungs-ID
	 * $result->blueprint	Ob nur Blueprint oder tatsächlich das Gebäude
	 * </code>
	 *
	 * @param uint $level
	 * @return stdClass
	 */
	public static function get_level_up_present($level) {
		return LevelUpPresents::get_present($level);
	}

	/**
	 * Erster Tutorial-Hack.
	 *
	 * Alle Einheiten des aktiven Spielers kommen sofort an.
	 * @param IGNORE $foo
	 */
	public static function tutorial_magic1($foo) {
		$user = User::get_current_user();
		if ($user->get_tutorial() !=
			($user->get_tutorial() | IUser::TUT_FLAG_BEAM_UNITS)) {

				TribeOnMap::beam_moving_units(
					$user->get_world_id(), $user->get_map_id(),
					$user->get_id()
				);
				$user->add_tutorial(IUser::TUT_FLAG_BEAM_UNITS);
		}
	}
}

?>