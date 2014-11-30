<?


//achtung: die construct und get_free_place fkt-nen scheinen doch recht langsam
//zu laufen, bei 255 gebäuden in der building queue kam auf dem notebook ein
//30sec timeout wegen denen. das könnte durchaus an den datenbankzugriffen liegen
//wg. get_buildings_in_sector oder ähnlichen. kurzer test: auslagern der
//get_buildings_in_sector db zugriffe war es, dauert aber trotz änderung noch viel zu lang!.
//trotzdem unbedingt auf datenbankzugriffe testen!

require_once (dirname(__FILE__).'/../../shared/classes/Language.inc');
require_once (dirname(__FILE__).'/../map/Mapsettings.inc');
require_once (dirname(__FILE__).'/../Science.inc');
require_once (dirname(__FILE__).'/../User.inc');
require_once (dirname(__FILE__).'/../../consts.inc');
require_once (dirname(__FILE__).'/../../shared/classes/buildings/IBuildings.inc');
require_once (dirname(__FILE__).'/../../shared/classes/datastructures/'.
													'JSMouseEventStruct.inc');
require_once (dirname(__FILE__).'/Building.inc');
require_once (dirname(__FILE__).'/BuildingOnMap.inc');
require_once (dirname(__FILE__).'/../Resource.inc');
require_once (dirname(__FILE__).'/../exceptions/HackattackException.inc');

class Buildings implements IBuildings {
	const COL_OWN_BUILDING = "#404090";
	const COL_ALLIED_BUILDING = "#409060";
	const COL_ENEMY_BUILDING = "#A04040";

	const MAX_BUILDINGS_PER_SECTOR = 100;	// UNENDLICH => Deaktiviert

	private static $building_data_cache = array();
	private $userID;

	private static $area;
	private $buildings;

	const DEBUG = 0;
	static private $INSTANCE = array();	// instanzüberwachung. Nur eine Instanz pro UID innerhalb von PV
 										// Konstruktor ist private. Objekt statisch über get_instance($uid)
 										// anfordern! (s.u.)

 	/**
	 * Diese Statische Funtion als Konstruktor benutzen !
	 *
	 * @return Buildings
	 */
 	public static function get_instance($uid) {
 		if (self::DEBUG) {
			echo "Building: User id@get_instance: $uid<br>\n";
 		}
 		if (!self::$INSTANCE || !self::$INSTANCE[$uid]) {
 			self::$INSTANCE[$uid] = new Buildings($uid);
 		}
 		return self::$INSTANCE[$uid];
 	}

	private function __construct($uid) {
		if (self::DEBUG)
 			echo "Building: constructor called: $uid<br>\n";

		$this->userID = $uid;
		$this->buildings = array();
	}

	/**
	 * Gibt die User-ID zurück
	 *
	 * @return int user-ID
	 */
	public function get_user_id() {
		return $this->userID;
	}

	/**
	 * Lädt die DB-Zeile von $type in den Cache, wenn nötig, und gibt den
	 * Cacheinhalt zurück.
	 *
	 * @param uint $type
	 * @param unit $level
	 */
	private static function get_cache_data($type, $level=1) {
		$type = (int)$type;
		$level = (int)$level;
		if (!isset(self::$building_data_cache[$type][$level])) {
			$row = mysql_fetch_object(
				query(
					"SELECT base_size, height_factor, pic, const_key ".
					"FROM buildings WHERE type=$type AND level=$level"
				)
			);

			self::$building_data_cache[$type][$level] = $row;
		}
		return self::$building_data_cache[$type][$level];
	}

	public static function get_type_to_constkey_map() {
		$res = query(
			"SELECT DISTINCT type, const_key FROM buildings"
		);
		$result = null;
		while($row = mysql_fetch_row($res)) {
			$result[$row[0]] = $row[1];
		}
		return $result;
	}

	/**
	 * Gibt die Seitenlänge der Grundfläche des Gebäudes vom Typ $type zurück.
	 * Alle Gebäude sind quadratisch, daher ist eine Seitenlänge ausreichend.
	 * Die Seitenlänge wird in der Gebäuderastereinheit ganzzahlig zurückgegeben.
	 *
	 * @param int $type 	Typ des Gebäudes
	 * @param int $level	Level des Gebäudes
	 * @return int Seitenlängen der Grundfläche des gewählten Gebäudes
	 */
	public static function get_b_area_size($type, $level=1) {
		return self::get_cache_data($type, $level)->base_size;
	}

	/**
	 * Gibt den Const-Key des Gebäudes zurück.
	 *
	 * @param int $type 	Typ des Gebäudes
	 * @param int $level	Level des Gebäudes
	 * @return String
	 */
	public static function get_b_const_key($type, $level=1) {
		return self::get_cache_data($type, $level)->const_key;
	}

	/**
	 * Gibt die Höhe des Gebäudes vom Typ $type zurück. Die Höhe wird
	 * in Gebäuderastereinheit zurückgegeben.
	 *
	 * @param int $type Typ des Gebäudes
	 * @return float Höhe des Gebäudes
	 */
	public static function get_b_height($type, $level=1) {
		return self::get_cache_data($type, $level)->height_factor;
	}

	/**
	 * Gibt alle Gebäude zurück, die im Laufe des Spiels erforschbar sind.
	 *
	 * @return Building[]
	 */
	public static function get_all_non_custom_buildings() {
		$res = query(
			"SELECT type FROM buildings WHERE req_sc_id!=9999 AND level=1"
		);
		$result = NULL;
		while ($row = mysql_fetch_row($res)) {
			$type = $row[0];
			$result[$type] = Building::get_instance($type, 1);
		}
		return $result;
	}

	/**
	 * Gibt die Gebäude zurück, die ein User bauen kann. Ist load_from_db gesetzt,
	 * so werden die Daten aus der DB geholt und nicht aus der Session. Dasselbe
	 * passiert, wenn $this->userID != $current_user->get_id() ist, und somit ein
	 * Sessionzugriff ein fehler wäre.
	 *
	 * @return Building[] die Liste der Gebäude, die gebaut werden dürfen
	 */
	public function get_buildable_buildings() {
		$world_id = User::get_instance_by_id($this->userID)->get_world_id();
		$map_id = User::get_instance_by_id($this->userID)->get_map_id();
		// Ein Christbaum darf errichtet werden, wenn heute der 24. - 26. Dezember ist
		$may_construct_christmas_tree = false;
		/*$month_of_today = (int) date('m', time());
		if ($month_of_today == 12) {
			$day_of_today = (int) date('d', time());
			if ($day_of_today >= 24 && $day_of_today <= 26)
				$may_construct_christmas_tree = true;
		}*/

		$types = array ();
		$techtree = Techtree::get_instance($this->userID);

		/**
		 * Zunächst die Special-Gebäude
		 */
		$noTechcenter = (mysql_num_rows(query(
			"SELECT ulc_x FROM building WHERE uid='{$this->userID}' AND ".
				"type=".self::TECHCENTER." AND world_id=$world_id AND ".
				"map_id=" . $map_id
			)) == 0);
		$techcenterBuiltOnce = !$noTechcenter;
		if ($noTechcenter)
			$types[self::TECHCENTER] = Building::get_instance(self::TECHCENTER, 1);
		if ($may_construct_christmas_tree) {
			$types[self::CHRISTMAS_TREE] = Building::get_instance(self::CHRISTMAS_TREE, 1);
		}
		/**
		 * Jetzt die, die er durch Forschung bauen darf
		 */
		if ($techcenterBuiltOnce) {
			// Alle Gebäudevorraussetzungen aus der DB laden
			$type_reqs = array();
			$res = query(
				'SELECT type, req_level, req_sc_id FROM buildings WHERE level=1'
			);
			while ($row = mysql_fetch_assoc($res)) {
				$type_reqs[$row['type']] = new stdClass();
				$type_reqs[$row['type']]->req_sc_id = $row['req_sc_id'];
				$type_reqs[$row['type']]->req_level = $row['req_level'];
			}
			for ($i = 2; $i < count($type_reqs) +1; $i++) {  // $i != self::TECHCENTER ist gewährleistet, da $i bei 2 startet und self::TECHCENTER == 1
				if (($i != self::MOTHER_FUNGUS)
					&& ($i != self::CHRISTMAS_TREE))
				{
					if (isset($type_reqs[$i])
						&& ($techtree->get_science_cur_level(
								$type_reqs[$i]->req_sc_id
						) >= $type_reqs[$i]->req_level))
					{
						try {
							$types[$i] = Building::get_instance($i, 1);
						}
						catch (Exception $e) {

						}
					}

				}
			}
		}
		/**
		 * Jetzt kommen die Gebäude, die er bauen darf, weil er sie in der
		 * Schatzkiste hat.
		 */
		$my_chest_buildings = Server::get_instance(CONF_PAYMENT_SERVER_URL,
				CONF_MAMMUN_AUTH_SECRET)->get_my_items(
					$this->get_user_id(),
					IIndividualization::TYPE_BUILDING
				);
		foreach ($my_chest_buildings as $buildings) {
			foreach ($buildings as $building) {
				$types[$building->id] = Building::get_instance($building->id,1);
			}
		}
		return ($types);
	}

	/***********************************************************************************
	 * Diese Funktion gibt zurück, ob der User (oder ein Alliierter) dominiert, oder
	 * nicht.
	 * @return TRUE bei Dominanz, sonst FALSE
	 **********************************************************************************/
	private function is_dominating(WorldCoordinates $sector_c) {
		return (Sector::get_instance($sector_c)->get_dominator_id()
			== $this->userID);
	}

	/***********************************************************************************
	 * Die Funktion can_add überprüft, ob ein Gebäude an der gewünschten Stelle gesetzt
	 * werden kann oder nicht.
	 *
	 * @param int $x:
	 *  x-Koordinate des zu setzenden Gebaeudes
	 * @param int $y:
	 *  y-Koordinate
	 * @param int $width:
	 *  Breite
	 * @param int $height:
	 *  Hoehe
	 * @param int $sID:
	 *  Die ID des Sektors (lange), wo das Gebaeude hin soll
	 * @param bool $dont_check_dom:
	 *  Wenn TRUE, dann wird NICHT überprüft, ob er durch Dominanz überhaupt zum Bauen
	 *  berechtigt ist. Standard ist FALSE.
	 * @return bool
	 *  Gibt TRUE zurueck, wenn gebaut werden darf, andernfalls FALSE
	 **********************************************************************************/

	private function can_add($x, $y, $width, $height,
		WorldCoordinates $sector_c,	$dont_check_dom = false)
	{
		if ($width <= 0)
			throw new IllegalArgumentException("width", "N+\{0}", $width);
		if ($height <= 0)
			throw new IllegalArgumentException("height", "N+\{0}", $height);

		if ($dont_check_dom || $this->is_dominating($sector_c)) {
			$area = self::get_area_of_sector($sector_c);

			/* Prüfen, ob das Gebäude nicht den Ramen des Sektors überschreitet */
			if ((($x + $width) < 9) && (($y + $height) < 9) && ($y >= 0) && ($x >= 0)) {

				for ($i = $x; $i < ($x + $width); $i++)
					for ($j = $y; $j < ($y + $height); $j++)
						if (@$area[$i][$j] > 0)
							return false; /* Prüfen, dass das Gebäude sich mit
											keinem vorhandenen überlappt */
			} else
				return false;

			return true;
		}
		else {
			error_log("NO DOMINATION!");
			throw new Exception("", self::ERR_NO_DOMINATION);
		}
		return false;
	}

	/***********************************************************************************
	 * Gibt die Position eines freien Platzes zum Bauen des übergebenen Gebäudetyps zurück.
	 *
	 * @param int $type:
	 *  Gebäudetyp
	 * @param int $sector:
	 *  in welchem Sector (Sector-ID sollte dabei die Map-ID enthalten!)
	 * @param int $startx:
	 *  bei welchem $x-Wert der algo beginnen soll
	 * @param int $startx:
	 *  bei welchem $y-Wert der algo beginnen soll
	 * @param int $algotype:
	 *  Welcher Algorithmus gewählt werden soll (standart ist GFP_ROW_BY_ROW).
	 *  Mögliche Werte: Buildings::GFP_ROW_BY_ROW, Buildings::GFP_SPIRAL,
	 *  Buildings::GFP_SCATTERED
	 * @param int run_number:
	 *  Hier kann übergeben werden, der wievielte Aufruf der Funktion innerhalb einer
	 *  Schleife das ist. Dies ist für den Algorithmus GFP_SCATTER nötig, damit er
	 *  ordnungsmäÂßig funktioniert. Ansonsten kann ein optimales Ergebnis nicht gewährleistet
	 *  werden.
	 * @return int[]
	 *  Die Position in einem Array: ['x']: x-position, ['y']: y-position
	 *  Zudem weitere Daten. Siehe dazu die einzelnen get_free_place-Algorithmen
	 **********************************************************************************/

	public function get_free_place($type, WorldCoordinates $sector_c,
		$start_values = false, $algotype = self::GFP_ROW_BY_ROW, $run_number = 0)
	{
		self::$area = null;

		switch ($algotype) {
			case self::GFP_SPIRAL :
				if (is_array($start_values)) {
					if (!is_anint($start_values['internals']['dir']['x']) ||
						!is_anint($start_values['internals']['dir']['y']) ||
						!is_anint($start_values['internals']['min']['x']) ||
						!is_anint($start_values['internals']['min']['y']) ||
						!is_anint($start_values['internals']['max']['x']) ||
						!is_anint($start_values['internals']['max']['y']))
					{
						$start_values = false;
					}

				}
				return $this->get_free_place_spiral($type, $sector_c,
					$start_values);
				break;
			case self::GFP_SCATTERED :
				return ($this->get_free_place_scattered($type, $sector_c,
															0, 0, $run_number));
				break;
			default :
				if (!$start_values) {
					$start_values['x'] = 0;
					$start_values['y'] = 0;
				}
				return $this->get_free_place_rbr($type, $sector_c,
										$start_values['x'], $start_values['y']);
		}
	}

	/***********************************************************************************
	 * Gibt die Position eines freien Platzes zum Bauen des übergebenen Gebäudetyps zurück.
	 * Lineare suche von (0,0) bis (8,8)
	 *
	 * ********
	 * ********
	 * ****
	 *
	 *
	 * Wird _nur_ von get_free_place aufgerufen
	 * @param int $type:
	 *  Gebäudetyp
	 * @param int $sector:
	 *  in welchem Sector (Sector-ID sollte dabei die Map-ID enthalten!)
	 * @param int $startx:
	 *  bei welchem $x-Wert der algo beginnen soll
	 * @param int $startx:
	 *  bei welchem $y-Wert der algo beginnen soll
	 * @return int[]
	 *  Die Position in einem Array: ['x']: x-position, ['y']: y-position
	 **********************************************************************************/
	private function get_free_place_rbr($type, WorldCoordinates $sector_c,
		$startx = 0, $starty = 0)
	{
		// var_dump(debug_backtrace());
		// echo "type: $type";
		$size = self::get_b_area_size($type);
		$found = false;
		for ($y = $starty; $y < 9; $y++) {
			for ($x = $startx; $x < 9; $x++) {
				// echo "x: $x, y: $y, size: $size, sector: $sector<br>";
				if ($this->can_add($x, $y, $size, $size, $sector_c, true)) {
					$found = true;
					// echo "...seems to be free<br>";
					break 2; // Doppelte for-Schleife verlassen
				}
			}
			$startx = 0; // damit nicht ganze Spalten ausgelassen werden
		}

		return ($found) ? array (
			'x' => $x,
			'y' => $y
		) : false;
	}

	/***********************************************************************************
	 * Gibt die Position eines freien Platzes zum Bauen des übergebenen Gebäudetyps zurück.
	 * Spiralsuche, von außen nach innen. Gestartet oben links:
	 *
	 * ********
	 *        *
	 * *      *
	 * ********
	 *
	 * Wird _nur_ von get_free_place aufgerufen
	 * @param int $type:
	 *  Gebäudetyp
	 * @param int $sector:
	 *  in welchem Sector (Sector-ID sollte dabei die Map-ID enthalten!)
	 * @param int $startx:
	 *  bei welchem $x-Wert der algo beginnen soll
	 * @param int $startx:
	 *  bei welchem $y-Wert der algo beginnen soll
	 * @return int[][] | bool
	 *  Die aktuellen Daten zum weiterführen des Algorithmus an Abbruchstelle.
	 *  Im Fehlerfalle wird boolisch false zurückgegeben.
	 *  Das Array ist wie folgt aufgebaut:
	 *  $result['x']	x-Position, der gefundenen Stelle
	 *  $result['y'] 	y-Position, der gefundenen Stelle
	 *  $result['internals']['dir']['x'] x-Richtung (intern notwendiger Wert)
	 *  $result['internals']['dir']['y'] y-Richtung (intern notwendiger Wert)
	 *  $result['internals']['min']['x'] x-Minimum (intern notwendiger Wert)
	 *  $result['internals']['min']['y'] x-Minimum (intern notwendiger Wert)
	 *  $result['internals']['max']['x'] x-Minimum (intern notwendiger Wert)
	 *  $result['internals']['max']['y'] x-Minimum (intern notwendiger Wert)
	 **********************************************************************************/
	private function get_free_place_spiral($type, WorldCoordinates $sector_c,
		$start_values = false)
	{
		$size = self::get_b_area_size($type);

		/* Wenn ($startx, $starty) in # sind, dann ist dir=(1,0), bei | dir=(0,1),
		 * bei - dir=(-1,0), bei + dir=(0,-1).
		 *
		 *    01234567
		 *
		 * 0  ########
		 * 1  +######|
		 * 2  ++####||
		 * 3  +++##|||
		 * 4  +++-||||
		 * 5  ++---|||
		 * 6  +-----||
		 * 7  -------|
		 *
		 * //TODO:Dies effizent implementieren. Sowie max_(x|y) und min_(x|y)
		 */

		if (is_array($start_values)) {

			//print_r($start_values);
			$max = $start_values['internals']['max'];
			$min = $start_values['internals']['min'];
			$dir = $start_values['internals']['dir'];
			$x = $start_values['x'];
			$y = $start_values['y'];
		} else {
			$max['x'] = $max['y'] = 8 - $size;
			$min['x'] = 0;
			$min['y'] = $size;
			$x = 0;
			$y = 0;
			$dir['x'] = 1;
			$dir['y'] = 0;
		}

		$found = false;
		while ((($max['x'] >= $min['x']) || ($max['y'] >= $min['y'])) && ($x >= 0) && ($y >= 0)) {
			if ($this->can_add($x, $y, $size, $sector_c, $sector_id, true)) {
				$found = true;
				break;
			}
			//echo "x : $x, y: $y<br>";
			if ($dir['x'] == 1) {
				if ($x == $max['x']) {
					$max['x']--;
					//echo "max['x'] = {$max['x']}<br>";
					$dir['x'] = 0;
					$dir['y'] = 1;
				}
			} else
				if ($dir['x'] == -1) {
					if ($x == $min['x']) {
						$x = $min['x'];
						$min['x']++;
						//echo "min['x'] = {$min['x']}<br>";
						$dir['x'] = 0;
						$dir['y'] = -1;
					}
				} else
					if ($dir['y'] == 1) {
						if ($y == $max['y']) {
							$max['y']--;
							//echo "max['y'] = {$max['y']}<br>";
							$dir['x'] = -1;
							$dir['y'] = 0;
						}
					} else
						if ($dir['y'] == -1) {
							if ($y == $min['y']) {
								$y = $min['y'];
								$min['y']++;
								//echo "min['y'] = {$min['y']}<br>";
								$dir['x'] = 1;
								$dir['y'] = 0;
							}
						}
			$x += $dir['x'];
			$y += $dir['y'];
		}

		return ($found) ? array (
			'x' => $x,
			'y' => $y,
			'internals' => array (
				'max' => $max,
				'min' => $min,
				'dir' => $dir
			)
		) : false;
	}

	/**
	 * Gibt die Position eines freien Platzes zum Bauen des übergebenen
	 * Gebäudetyps zurück. Hierzu wird ein möglichst großer Platzverlust beim
	 * Bauen angestrebt. Da es keine 1x1 große Gebäude gibt, wird also versucht,
	 * immer ein Feld frei zu lassen. Dass das aber auch möglichst viel Platz
	 * verschwendet, auch wenn man nur ein paar Fungi baut (z.B. nur 4 oder 5),
	 * ist dies nicht ganz so trivial.
	 *
	 * Der Algorithmus durchsucht den Sektor von den Ecken aus ab. Jenachdem
	 * welche $run_number übergeben wurde, beginnt die Suche in einer anderen
	 * Ecke. Dann versucht er bei gefundenem Platz, ein Feld von der Ecke weg
	 * diagonal zu bauen (dies wäre der meiste Platzverbraucher)
	 *
	 * Beispiel:
	 * oo....oo		<-- sektor
	 * oOx..xOo		o: gefundener platz zum bauen
	 * .xx..xx.		x: gebautes Gebäude
	 * ........		O: zuerst gefundener platz, welcher dann durch bauen beleget
	 * 					wurde (kombi von o und x)
	 * ........		.: freier platz
	 * .xx..xx.
	 * oOx..xOo
	 * oo....oo
	 *
	 * Hier wurde der Algorithmus 4x durchgeführt. Jedesmal begann er in einer
	 * anderen Ecke mit dem Suchen. Dieses Ergebnis ist für 4 Fungi optimal.
	 *
	 * @param int $type:
	 *  Gebäudetyp
	 * @param int $sector:
	 *  in welchem Sector (Sector-ID sollte dabei die Map-ID enthalten!)
	 * @param int $startx:
	 *  bei welchem $x-Wert der algo beginnen soll
	 * @param int $startx:
	 *  bei welchem $y-Wert der algo beginnen soll
	 * @param int run_number:
	 *  Hier kann übergeben werden, der wievielte Aufruf der Funktion innerhalb
	 *  einer Schleife das ist. Dies ist nötig, damit der Algorithmus
	 *  ordnungsmäßig funktioniert.
	 *  Ansonsten kann ein optimales Ergebnis nicht gewährleistet werden.
	 * @return int[]
	 *  Die Position in einem Array: ['x']: x-position, ['y']: y-position
	 */
	private function get_free_place_scattered($type, WorldCoordinates $sector_c,
		$startx = 0, $starty = 0, $run_number = 0)
	{
		$size = self::get_b_area_size($type);

		$LO_DIR['x'] = 1;
		$LO_DIR['y'] = 1;
		$RO_DIR['x'] = -1;
		$RO_DIR['y'] = 1;
		$LU_DIR['x'] = 1;
		$LU_DIR['y'] = -1;
		$RU_DIR['x'] = -1;
		$RU_DIR['y'] = -1;
		$MI_DIR['x'] = 0;
		$MI_DIR['y'] = 0;

		$lo_pos['x'] = 0;
		$lo_pos['y'] = 0;
		$ro_pos['x'] = 8 - $size;
		$ro_pos['y'] = 0;
		$lu_pos['x'] = 0;
		$lu_pos['y'] = 8 - $size;
		$ru_pos['x'] = 8 - $size;
		$ru_pos['y'] = 8 - $size;
		$mi_pos['x'] = 5 - $size;
		$mi_pos['y'] = 5 - $size;

		$i = 1; //($run_number +1) % 5;

		$found = false;
		do {
			switch ($i) {
				case 1 :
					$pos = $mi_pos;
					$dir = $MI_DIR;
					break;
				case 2 :
					$pos = $ro_pos;
					$dir = $RO_DIR;
					break;
				case 3 :
					$pos = $lu_pos;
					$dir = $LU_DIR;
					break;
				case 4 :
					$pos = $ru_pos;
					$dir = $RU_DIR;
					break;
				case 5 :
				case 0 :
					$pos = $lo_pos;
					$dir = $LO_DIR;
					break;
			}
			/*echo "$i<br>";
			echo "pos x: {$pos['x']}, pos y: {$pos['y']}<br>\n";
			echo "dir x: {$dir['x']}, dir y: {$dir['y']}<br>\n\n<br>";*/

			if ($this->can_add($pos['x'], $pos['y'], $size, $size, $sector_c,
				true))
			{
				//echo "i can add here<br><br>";
				$found = true;
				if ($this->can_add($pos['x'] + $dir['x'], $pos['y'] + $dir['y'],
					$size, $size, $sector_c,true))
				{
					$x = $pos['x'] + $dir['x'];
					$y = $pos['y'] + $dir['y'];
				} else
					if ($this->can_add($pos['x'] + $dir['x'], $pos['y'], $size,
						$size, $sector_c,true))
					{
						$x = $pos['x'] + $dir['x'];
						$y = $pos['y'];
					} else
						if ($this->can_add($pos['x'], $pos['y'] + $dir['y'],
							$size, $size, $sector_c,true))
						{
							$x = $pos['x'];
							$y = $pos['y'] + $dir['y'];
						} else {
							$x = $pos['x'];
							$y = $pos['y'];
						}
				break;
			}
			$pos['x'] += $dir['x'];
			if ($pos['x'] > 7) {
				$pos['x'] = 0;
				$pos['y'] += $dir['y'];
			} else
				if ($pos['x'] < 0) {
					$pos['x'] = 7 - $size;
					$pos['y'] += $dir['y'];
				}
			switch ($i) {
				case 1 :
					$mi_pos = $pos;
					$i++;
					break;
				case 2 :
					$ro_pos = $pos;
					$i++;
					break;
				case 3 :
					$lu_pos = $pos;
					$i++;
					break;
				case 4 :
					$ru_pos = $pos;
					$i++;
					break;
				case 5 :
				case 0 :
					$lo_pos = $pos;
				default :
					$i = 1;
					break;
			}

		} while (($pos['x'] >= 0) && ($pos['x'] <= 7) && ($pos['y'] >= 0)
			&& ($pos['y'] <= 7));

		//echo "<br>\n$found - pos x: {$pos['x']}, pos y: {$pos['y']}<br>\n";
		return ($found) ? array (
			'x' => $x,
			'y' => $y
		) : false;
	}

	/***********************************************************************************
	 * Lässt die gewählte Anzahl von Gebäuden des gewählten Typs automatisch im Sektor
	 * bauen.
	 * @param int $type
	 *  der Gebäudetyp
	 * @param int $count
	 *  die Anzahl
	 * @param int $sector
	 *  in welchem Sektor. $sector muss sie lange ID sein
	 * @param int $algotype
	 *  welcher Algorithmus zum Bauen gewählt werden soll(standart ist GFP_ROW_BY_ROW).
	 *  Mögliche Werte: GFP_ROW_BY_ROW, GFP_SPIRAL, GFP_SCATTERED
	 * @return
	 *  true oder false - je nach Erfolg
	 */
	public function construct_automatically($type, $count, $sector_c,
		$algotype = self::GFP_ROW_BY_ROW)
	{
		$data['x'] = $data['y'] = 0;
		for ($i = 0; $i < $count; $i++) {
			if (($data = $this->get_free_place($type, $sector_c, $data,
												$algotype, $i)) !== false)
			{
				try {
					$this->construct($data['x'], $data['y'], $type, $sector_c);
				}
				catch (Exception $e) {
					throw new Exception("", self::ERR_COULD_NOT_BUILD_ALL);
				}
			}
			else {
				throw new Exception("", self::ERR_CANT_BUILD_THERE);
			}
		}
		return true;
	}

	/***********************************************************************************
	 * gibt ein Array zurück, indem die Gebäude position der Gebäude mit den Typnummern
	 * gespeichert ist.
	 *
	 * @param WorldCoordinates $sector_c	Sektor
	 * @return BuildingOnMap[]
	 *  Enthaelt Typ der Gebauede an entsprechender Position.
	 *  Dim 1 steht fuer die y, Dim 2 fuer die x Koordinate.
	 **********************************************************************************/
	public static function get_buildings_in_sector(WorldCoordinates $sector_c) {
		$ubuildings = query(
			"SELECT ulc_x,ulc_y FROM building WHERE ".
				"world_id={$sector_c->get_world_id()} AND ".
				"map_id={$sector_c->get_map_id()} AND ".
				"sector_id={$sector_c->get_sector_id()}"
		);
		$buildings_in_sector = array();
		if (mysql_num_rows($ubuildings) > 0) {
			while ($ubuilding = mysql_fetch_array($ubuildings)) {
				$buildings_in_sector[] = BuildingOnMap::get_instance(
					$sector_c, $ubuilding['ulc_x'], $ubuilding['ulc_y']
				);
			}
		}
		return $buildings_in_sector;
	}

	public static function get_area_of_sector(WorldCoordinates $sector_c) {
		$world_id = $sector_c->get_world_id();
		$map_id = $sector_c->get_map_id();
		$sector_id = $sector_c->get_sector_id();
		if (!isset(self::$area[$world_id][$map_id][$sector_id]))
		{
			$buildings = self::get_buildings_in_sector($sector_c);
			foreach ($buildings as $building) {
				$ulc = $building->get_ulc();
				for ($i = $ulc->x; $i < ($ulc->x + $building->get_size()); $i++)
				{
					for ($j = $ulc->y; $j < ($ulc->y + $building->get_size());
						$j++)
					{
						self::$area[$world_id][$map_id][$sector_id][$i][$j]
								= $building->get_type();
					}
				}
			}
		}
		return self::$area[$world_id][$map_id][$sector_id];
	}

	/**
	 * Neukonstruktion von Gebäuden
	 *
	 * @param int WorldCoordinates $sector_c
	 * 	ID des Sektors wo gebaut werden soll in Welt-Koordinaten
	 * @param int $x
	 * 	x-Koordinate des zu bauenden Gebäudes
	 * @param int $y
	 *  y-Koordinate des zu bauenden Gebäudes
	 * @param int $b_type
	 *  Typ des zu bauenden Gebäudes
	 * @param bool $skip_checks
	 *  Forschungsstandabfragen ignorieren (nur true setzen, wenn bereits
	 *  woanders geprüft (Aufruf z.B. von gen_funcs::check_worker_send_and_build_queue() )
	 * @param int $buildtime
	 *  Der Zeitpunkt, wann das Gebäude gebaut werden soll. Wenn 0 [default], dann wird
	 *  $t_now genommen.
	 * @return uint
	 * 	Gibt zurück, wie viele Punkte die Aktion dem Spieler brachte.
	 *
	 * @throws Exception
	 *  im normalen Fehlerfall, wenn z.B. kein Platz zum Bauen etc
	 * @throws ExceptionWithErrormail
	 *  Bei fehlerhaftem Aufruf der Funktion/Designfehlern
	 */
	public function construct(WorldCoordinates $sector_c, $x, $y, $b_type,
		$color=0, $skip_checks = false, $buildtime = 0)
	{
		$sem_id = sem_get(MUTEX_BUILDING_CONSTRUCT, 1, 0600, -1);
		sem_acquire($sem_id);

		if (empty ($x))
			$x = 0;
		if (empty ($y))
			$y = 0;

		$world_id = $sector_c->get_world_id();
		$map_id = $sector_c->get_map_id();
		$sector_id = $sector_c->get_sector_id();

		if (!$color || ($color < 0))
			$color = 0;

		if (empty($b_type)) {
			sem_release($sem_id);
			throw new ExceptionWithErrormail("", $this->uid, 'All',
				'Buildings::construct - no $b_type given',
				"\$b_type=$b_type");
		}

		if (($b_type >= Buildings::CUSTOM_BUILDINGS_BEGIN)
			&& ($b_type <= Buildings::CUSTOM_BUILDINGS_END))
		{
			$color = 0;
		}

		$num_buildings_in_sector = mysql_result(
			@query(
				"SELECT COUNT(*) FROM building WHERE ".
					"world_id=$world_id AND map_id=$map_id AND ".
					"sector_id=$sector_id"
			),
			0
		);

		if ($num_buildings_in_sector >= self::MAX_BUILDINGS_PER_SECTOR) {
			sem_release($sem_id);
			throw new Exception("", self::ERR_TOO_MANY_BUILDINGS_IN_SECTOR);
		}

		if (!$buildtime)
			$buildtime = Time::get_t_now();
		/* Bei einer Neukonstruktion wird ein upgrade von Level 0 aus vollzogen.
		 * Um dies zu bewirken wird vor
		   dem Upgrade die entsprechende Zeile erzeugt. Diese wird wiedergefunden
		   => das upgrade beginnt.
		   Auf diese Weise spar ich mir viel Code. Nachteil: In einem Fehlerfall,
		   muss das Gebäude wieder
		   vom Script gelöscht werden.
		   Es werden wohl mehr Leute nur dann bauen, wenn sie es auch können.... */
		/*if ((!$this->is_dominating($short_sID)) && ($worker_num === false))
			throw new Exception("", self::ERR_NO_DOMINATION);
		else {*/
			$mapsettings = Mapsettings::get_instance();

			$building = mysql_fetch_array(query(
				"SELECT grow FROM buildings WHERE type=$b_type AND level='1'"
			));
			$finished = (100 * $building['grow'])
							/ $mapsettings->get_timefactor() + $buildtime;

			// man darf keine zwei bauen - hier nochmal prüfen!
			if (($b_type == self::MOTHER_FUNGUS) || ($b_type == self::TECHCENTER)) {
				if (mysql_num_rows(query(
					"SELECT ulc_x FROM building WHERE world_id=$world_id AND ".
						"map_id=$map_id AND ".
						"type=$b_type AND uid={$this->userID}")) > 0)
				{
					sem_release($sem_id);
					throw new Exception("", self::ERR_CANT_BUILD_THERE);
				}
			}

			if ($this->can_add($x, $y, self::get_b_area_size($b_type),
				self::get_b_area_size($b_type), $sector_c, $skip_checks))
			{
				if ($color > 0) {
					/**
					 * Farben sind nicht kostenlos. Klunker abziehen.
					 */
					try {
						$kkr = Server::get_instance(CONF_PAYMENT_SERVER_URL,
							CONF_MAMMUN_AUTH_SECRET)
								->substract_klunker($this->userID,
									self::KKR_PAINT, 100+$color);
					}
					catch (Exception $e) {
						sem_release($sem_id);
						error_log($e);
						throw new Exception("Not enough Klunker",
							self::ERR_NOT_ENOUGH_KLUNKER);
					}
				}

				query("INSERT INTO building SET uid='{$this->userID}', ".
					"world_id=$world_id, map_id=$map_id, sector_id=$sector_id,".
					"ulc_x=$x, ulc_y=$y, ".
					"type='$b_type', cur_level='0', time_built='$finished', ".
					"color=$color, ".
					"finish_time='$finished', time_build_started='$buildtime'");
			}
			else {
				sem_release($sem_id);
				throw new Exception("", self::ERR_CANT_BUILD_THERE);
			}


			// Wenn noch kein Fehler auftrat zuvor, dann ergebnis Upgrade zurückgeben
			try {
				$this->upgrade($sector_c, $x, $y, true,
					$skip_checks, $buildtime);
				if (isset($kkr))
					return $kkr;
				else
					return false;
			}
			catch (Exception $e) {
				// Gab es einen Upgradefehler, dann die vorherige INSERT-Query Rückgängig machen
				query(
					"DELETE FROM building WHERE ".
						"world_id=$world_id AND ".
						"map_id=$map_id AND sector_id=$sector_id AND ".
						"ulc_x=$x AND ulc_y=$y"
				);
				sem_release($sem_id);
				throw $e;	// Exception weiterwerfen

			}
		/*}*/
		sem_release($sem_id);
	}

	/**
	 * Baut höchstens so viele Pilze eines Type im gewählten Sektor aus, wie
	 * angegeben. Dies ist das Rudelupgrade :)
	 *
	 * @param int $type Welcher Pilztyp ausgebaut werden soll (wenn $type=0,
	 *                  dann alle Typen).
	 * @param int $count Gibt an, wie viele Pilze höchsten ausgebaut werden sollen.
	 * @param int $sector_c Sektor, in dem ausgebaut werden soll
	 */
	function upgrade_automatically($type, $count, WorldCoordinates $sector_c) {
		$world_id = $sector_c->get_world_id();
		$map_id = $sector_c->get_map_id();
		$sector_id = $sector_c->get_sector_id();
		if ($type) {
			$res = query("SELECT * FROM building WHERE uid='{$this->userID}' ".
						"AND world_id=$world_id AND ".
						"map_id=$map_id AND sector_id=$sector_id ".
						"AND type='$type'");
		}
		else {
			$res = query("SELECT * FROM building WHERE uid='{$this->userID}' ".
						"AND world_id=$world_id AND ".
						"map_id=$map_id AND sector_id=$sector_id");
		}

		if (mysql_num_rows($res) == 0) {
			return;	// keine da, kein Fehler, aber auch kein Upgrade
		}
		$cur_b_res = query("SELECT * FROM buildings WHERE type='$type'");
		$cur_building_attr = array();
		while ($cur_building = mysql_fetch_assoc($cur_b_res))
			$cur_building_attr[$cur_building['level']] = $cur_building;

		$i = 0;
		while (($b2upgrade = mysql_fetch_assoc($res)) && ($i < $count)) {
			if ($b2upgrade['to_build'] > 0)	// die, die schon am upgrade sind
				continue;					// einfach überspringen
			$next_building = $cur_building_attr[$b2upgrade['cur_level']+1];
			if (!$next_building)
				continue;					// schon auf max level (überhüppeln)
			try {
				/** TODO: REWRITE VON DO UPGRADE *
				$this->do_upgrade($b2upgrade, $next_building, $sector_c);
				*/
			}
			catch (Exception $e) {
				continue;
			}
			$i++;
		}
	}


	/**
	 * Upgrade von Gebäuden
	 *
	 * @param WorldCoordinates $sector_c
	 *  Sektor, wohin gebaut werden soll (lange ID)
	 * @param uint $x
	 * 	x-Koordinate des Sektors wo das Gebäude steht
	 * @param uint $y
	 * 	y-Koordinate des Sektors wo das Gebäude steht
	 * @param bool $construct
	 *  Ob die Funktion im Rahmen von einer Neukonstruktion aufgerufen wurde
	 * @param bool $skip_checks
	 *  Forschungsstandabfragen ignorieren (nur true setzen, wenn bereits
	 *  woanders geprüft (Aufruf z.B. von gen_funcs::check_worker_send_and_build_queue() )
	 * @param int $buildtime
	 *  Der Zeitpunkt, wann das Gebäude ausgebaut werden soll. Wenn 0 [default],
	 *  dann wird $t_now genommen [$buildtime liegt normalerweise in der Vergangenheit].
	 * @return int
	 *  Gibt zurück, wie viele Punkte die Aktion dem Spieler brachte.
	 * @throws Exception
	 *  im normalen Fehlerfall
	 */
	public function upgrade(WorldCoordinates $sector_c, $x, $y,
		$construct = false,	$skip_checks = false, $buildtime = 0)
	{
		try {
			BuildingOnMap::get_instance($sector_c, $x, $y)
				->_upgrade($buildtime);
		}
		catch (Exception $e) {
			/**
			 * Die geworfenen Exceptions werden entsprechend der eigenen
			 * Codes umgewandelt, da BuildingsOnMap andere Codes hat als
			 * IBuildings, bei Verwendung von Funktionen dieser Klasse hier
			 * aber natürlich IBuildings-Konstanten erwartet werden.
			 */
			switch ($e->getCode()) {
				case BuildingOnMap::ERR_BUILDING_DOES_NOT_EXIST:
					throw new Exception($e->getMessage(),
							self::ERR_BUILDING_NOT_FOUND);
					break;
				case BuildingOnMap::ERR_NOT_ENOUGH_RESOURCES:
					throw new Exception($e->getMessage(),
							self::ERR_NOT_ENOUGH_RESOURCES);
					break;
				case BuildingOnMap::ERR_NO_DOMINATION:
					throw new Exception($e->getMessage(),
							self::ERR_NO_DOMINATION);
					break;
				case BuildingOnMap::ERR_REQ_NOT_MET:
					throw new Exception($e->getMessage(),
							self::ERR_REQ_NOT_MET);
					break;
				default:
					throw $e;
			}
		}
	}

	/**
	 * Zerstört ein Gebäude
	 * @param WorldCoordinates $sector_c 	Weltkoordinaten des Sektors, in dem
	 * 										das Gebäude steht
	 * @param uint $x	x-Koordinate im Sektor, wo das Gebäude steht
	 * @param uint $y	y-Koordinate im Sektor, wo das Gebäude steht
	 * @throws HackattackException im Falle von illegalem Abrissversuch
	 * @throws Exception
	 */
	public function destroy(WorldCoordinates $sector_c, $x, $y) {
		$building_to_destroy = BuildingOnMap::get_instance($sector_c, $x, $y);
		if ($building_to_destroy->get_uid() != $this->userID) {
			throw new HackattackException("", $this->userID,
				HackattackException::CODE_DESTROY_BUILDING_OF_OTHER);
		}
		try {
			$building_to_destroy->_destroy();
		}
		catch (Exception $e) {
			error_log($e);
			switch ($e->getCode()) {
				case BuildingOnMap::ERR_DONT_DESTROY_MAIN_BUILDINGS:
					throw new Exception("", self::ERR_DONT_DESTROY_MAIN_BUILDINGS);
					break;
				default:
					throw new Exception("", self::ERR_UNKNOWN);

			}
		}
	}

	/**
	 * Ändert die Farbe des Gebäudes.
	 *
	 * Der Umfärber muss dafür mit Klunkern bezahlen.
	 *
	 * @param WorldCoordinate $sector_c
	 * @param uint $x
	 * @param uint $y
	 * @param uint/String $color
	 * @throws Exception
	 * @return uint
	 */
	public function change_color(WorldCoordinates $sector_c, $x, $y, $color) {
		$sem_id = sem_get(MUTEX_BUILDING_CHANGE_COLOR, 1, 0600, -1);
		sem_acquire($sem_id);
		$building = BuildingOnMap::get_instance($sector_c, $x, $y);

		if ($building->is_custom_building()) {
			sem_release($sem_id);
			throw new Exception(
				"Colors of custom buildings must not be changed.",
				self::ERR_CANNOT_CHANGE_COLOR
			);
		}

		if ($color == $building->get_color()) {
			// nothing to do
			sem_release($sem_id);
			return;
		}

		if ($building->get_uid() != $this->get_user_id()) {
			if ($building->get_color() > 0) {
				sem_release($sem_id);
				throw new HackattackException("Overpainting others' buildings",
					$this->get_user_id(),
					HackattackException::CODE_OVERPAINTIN_OTHERS_BUILDINGS);
			}
		}

		/**
		 * Farben sind nicht kostenlos. Klunker abziehen.
		 */
		if ($color) {
			try {
				$kkr = Server::get_instance(CONF_PAYMENT_SERVER_URL,
					CONF_MAMMUN_AUTH_SECRET)
						->substract_klunker($this->userID,
							self::KKR_PAINT, 100+$color);
			}
			catch (Exception $e) {
				sem_release($sem_id);
				error_log($e);
				throw new Exception("Not enough Klunker",
					self::ERR_NOT_ENOUGH_KLUNKER);
			}
		}

		try {
			$building->_change_color($color, $this->get_user_id());
		}
		catch(Exception $e) {
			sem_release($sem_id);
			error_log($e);
			throw new Exception("", self::ERR_UNKNOWN);
		}
		sem_release($sem_id);

		/**
		 * Ereignis melden :)
		 */
		if ($this->get_user_id() != $building->get_uid()) {
			/**
			 * Ich male ein Gebäude von jemand andrem an. Demjenigen eine
			 * Nachricht schicken.
			 */
			$ulc = $building->get_ulc();
			EventMessages::get_instance($building->get_uid())
				->add_entry(
					EventMessages::TYPE_BUILDING_HAS_BEEN_PAINTED,
					$this->get_user_id(),
					$building->get_world_id().'-'.$building->get_map_id().'-'.
						$building->get_sector_id().'-'.
						$ulc->x.'-'.$ulc->y.'-'.$building->get_color()
				);
			EventLog::get_instance($building->get_uid(),
				User::get_instance_by_id($building->get_uid())->get_world_id(),
				User::get_instance_by_id($building->get_uid())->get_map_id())
				->add_entry(
					EventMessages::TYPE_BUILDING_HAS_BEEN_PAINTED,
					$this->get_user_id(),
					$building->get_world_id().'-'.$building->get_map_id().'-'.
						$building->get_sector_id().'-'.
						$ulc->x.'-'.$ulc->y.'-'.$building->get_color()
				);
		}
		else if ($building->get_painted_by_uid() != $building->get_uid()) {
			/**
			 * Ich übermale die Farbe auf meinem Gebäude, die jemand
			 * anderes draufgetan hat => er wird informiert, dass dem
			 * Besitzer die Farbe wohl nicht gefallen hat ;-)
			 */
			$ulc = $building->get_ulc();
			EventMessages::get_instance($building->get_painted_by_uid())
				->add_entry(
					EventMessages::TYPE_REPAINTED_HIS_MY_COLORED_BUILDING,
					$this->get_user_id(),
					$building->get_world_id().'-'.$building->get_map_id().'-'.
						$building->get_sector_id().'-'.
						$ulc->x.'-'.$ulc->y.'-'.$building->get_color()
				);
			EventLog::get_instance($building->get_painted_by_uid(),
				User::get_instance_by_id($building->get_painted_by_uid())
					->get_world_id(),
				User::get_instance_by_id($building->get_painted_by_uid())
					->get_map_id())
				->add_entry(
					EventMessages::TYPE_REPAINTED_HIS_MY_COLORED_BUILDING,
					$building->get_uid(),
					$building->get_world_id().'-'.$building->get_map_id().'-'.
						$building->get_sector_id().'-'.
						$ulc->x.'-'.$ulc->y.'-'.$building->get_color()
				);
		}

		return isset($kkr) ? $kkr : false;
	}

	/**
	 * Reißt die Hauptgebäude des Users ab!
	 */
	public function destroy_main_buildings() {
		$b1 = BuildingOnMap::get_all_of_user($this->userID,
			Buildings::MOTHER_FUNGUS);
		$b1 = $b1[0];
		$b2 = BuildingOnMap::get_all_of_user($this->userID,
			Buildings::TECHCENTER);
		$b2 = $b2[0];

		if ($b1)
			$b1->_destroy(true);
		if ($b2)
			$b2->_destroy(true);
	}

	/**
	 * Gibt dem User das Weihnachtsgeschenk vom CCO :)
	 */
	public function give_chrismas_present() {
		/*Habs rausgenommen mit dem Update des MF wegen der Problematik, dass man mehrfach das Update erlangen
			könnte. Baum ist auch cool ;)*/
		return false;

		// MF Upgrade (beinahe fertiggestellt) wenn fkt aufgerufen
		$home_sector = mysql_result(query("SELECT homesector FROM users WHERE id={$this->userID}"), 0);
		$map_id = floor($home_sector / 100);
		$mother_id = mysql_result(query("SELECT ulc_x FROM building WHERE uid='{$this->userID}' AND type=" . self::MOTHER_FUNGUS . " AND map_id=" . $map_id), 0);
		$finished = Time::get_t_now() + 10;
		$buildtime = Time::get_t_now() - 100000;
		query("UPDATE building SET finish_time='$finished' , to_build='1', t_stamp='$buildtime', progress='99' WHERE uid='{$this->userID}' AND type='" . self::MOTHER_FUNGUS . "' AND id='$mother_id'");

		// Und einen Christbaun errichten, wenn möglich
		$pos = $this->get_free_place(self::CHRISTMAS_TREE, $home_sector);
		return $this->construct($pos['x'], $pos['y'], self::CHRISTMAS_TREE, $map_id, $home_sector, 0, true, $buildtime);
	}

	/**
	 * Weist die Gebäude des übergebenen Sektors auf der angegebenen Karte dem
	 * entsprechenden User zu.
	 *
	 * @param WorldCoordinates $sector_c
	 * @param uint $uid
	 */
	public static function assign_buildings_in_sector_to_user(
		WorldCoordinates $sector_c, $uid)
	{
		self::$INSTANCE = array();

		// Techcenter und Motherfungus können nicht übernommen werden
		query (
			"UPDATE building SET uid=$uid WHERE ".
				"world_id={$sector_c->get_world_id()} AND ".
				"map_id={$sector_c->get_map_id()} AND ".
				"sector_id={$sector_c->get_sector_id()} AND ".
				'type!='.self::TECHCENTER.' AND type!='.self::MOTHER_FUNGUS
		);
	}

	/**
	 * !!!ACHTUNG: Setzt alle Compute-Daten aller User unwiederruflich zurück!
	 */
	public static function clear_of_all_users() {
		self::$INSTANCE = array();
		BuildingOnMap::clear_of_all_users();
	}
}

?>
