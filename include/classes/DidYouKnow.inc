<?

/*
 * DidYouKnow class creates a did you know box - contextsensitively
 */

require_once(dirname(__FILE__).'/../languages.inc');
$lang = get_language();
include_language_file('DidYouKnow', $lang);

require_once(dirname(__FILE__).'/../consts.inc');


class DidYouKnow {
	private static $AGGRESSIVE_HINTS = array (
		self::SIT_HELLO_ENTER => LNG_SIT_HELLO_ENTER,
		self::SIT_CRMAP_ENTER => LNG_SIT_CRMAP_ENTER,
		self::SIT_HS_ENTER => LNG_SIT_HS_ENTER,
		self::SIT_TRIBE_CREATE => LNG_SIT_TRIBE_CREATE,
		self::SIT_TRIBES_MF_LVL_0 => LNG_SIT_TRIBE_MF_LVL_0,
		self::SIT_HS_MF_JUST_BUILT => LNG_SIT_HS_MF_JUST_BUILT,
		self::SIT_VIMO_ENTER => LNG_SIT_VIMO_ENTER,
		self::SIT_VIMO_WORKERS_MAY_DO_BOA => LNG_SIT_VIMO_WORKERS_MAY_DO_BOA,
		self::SIT_MP_ENTER => LNG_SIT_MP_ENTER,
	);	

	private static $TEXTS_GENERAL = array (
		LNG_DYK_GENERAL_0,
		LNG_DYK_GENERAL_1,
		LNG_DYK_GENERAL_1_2,
		LNG_DYK_GENERAL_2,
		LNG_DYK_GENERAL_3,
		LNG_DYK_GENERAL_4
	);
	private static $TEXTS_CREATUREMAP = array(
		LNG_DYK_CRMAP_0,
		LNG_DYK_CRMAP_1,
		LNG_DYK_CRMAP_2
	);
	private static $TEXTS_SECTOR_VIEW = array (
		LNG_DYK_GENERAL_0
	);
	private static $TEXTS_HOME_SECTOR = array (
		LNG_DYK_HS_0,
		LNG_DYK_HS_1
	);
	private static $TEXTS_SCIENCE = array (
		LNG_DYK_SCIENCE_0,
		LNG_DYK_SCIENCE_0_1,
		LNG_DYK_SCIENCE_1,
		LNG_DYK_SCIENCE_2,
		LNG_DYK_SCIENCE_3,
		LNG_DYK_SCIENCE_4
		
	);
	private static $TEXTS_TRIBES = array(
		LNG_DYK_TRIBES_0,
		LNG_DYK_TRIBES_1,
		LNG_DYK_TRIBES_1_1
	);
	private static $TEXTS_MARKET_PLACE = array (
		LNG_DYK_MP_0,
		LNG_DYK_MP_1,
		LNG_DYK_MP_2
	);
	//...

	// pages (non-aggressive tipps will use those)
	const PAGE_NONE = 0;
	const PAGE_HELLO = 1;
	const PAGE_CREATUREMAP = 2;
	const PAGE_VIRAL_MOVEMENT = 3;
	const PAGE_SECTOR_VIEW = 4;
	const PAGE_HOME_SECTOR = 5;
	const PAGE_SCIENCE = 6;
	const PAGE_TRIBES = 7;
	const PAGE_MARKET_PLACE = 8;
	//...
	
	// situations (aggrassive hints will use those)
	const SIT_NONE = 0;
	const SIT_HELLO_ENTER = FLAG_1;
	const SIT_CRMAP_ENTER = FLAG_2;
	const SIT_HS_ENTER = FLAG_3;
	const SIT_TRIBE_CREATE = FLAG_4;
	const SIT_TRIBES_MF_LVL_0 = FLAG_5;
	const SIT_HS_MF_JUST_BUILT = FLAG_6;
	const SIT_VIMO_WORKERS_MAY_DO_BOA = FLAG_7;
	const SIT_MP_ENTER = FLAG_8;
	const SIT_VIMO_ENTER = FLAG_9;
	//...
	
	
	const DEBUG = 0;
	
	private static $INSTANCE = array();
	
	private $uid;
	private $page;		// Seite, auf der man sich gerade befindet. Muss eine der DidYouKnow::PAGE_...-Konstanten sein!
	private $situation;	// Situation, in der man sich gerade befindet. Muss eine der DidYouKnow::SIT_...-Konstanten sein!
	private $index;		// Aktueller Index - wo im Array definiert durch $page sind wir gerade? Nötig für rotation durchs array (s. get_next_text())
	private $not_wanted_tips;
	private $db_need_sync;	// ob die Datanbank beim zerstören des Objektes synchronisiert werden muss oder nicht. Sprich, ob man die Daten des Objekts in die DB schreiben muss


	/* Diese Statische Funtion als Konstruktor benutzen !*/
 	public static function get_instance($uid) {
 		if (!self::$INSTANCE || !self::$INSTANCE[$uid]) {
 			self::$INSTANCE[$uid] = new DidYouKnow($uid);
 		}
 		if (self::DEBUG)
 			echo "DIDYOUKNOW: User id@get_instance: $uid<br>\n";
 		return self::$INSTANCE[$uid];
 	}
	
	private function __construct($uid) {	// konstruktor nicht von außen ausführen - get_instance() benutzen
		global $userdata;
		if (self::DEBUG)
			echo "DIDYOUKNOW: Constructor called.<br>\n";
			
		$this->uid = (int)$uid;
		$this->page = self::PAGE_NONE;
		$this->situation = self::SIT_NONE;
		
		if ($uid) {
			if ($userdata['id'] == $uid) {
				if (self::DEBUG)
					echo "not_wanted_tips initialized by \$userdata.<br>";	
			
				$this->not_wanted_tips = $userdata['not_wanted_tips'];
			}
			else {
				if (self::DEBUG)
					$this->not_wanted_tips = mysql_result(query("SELECT not_wanted_tips FROM users WHERE id=$uid"),0);	
				else
					$this->not_wanted_tips = @mysql_result(query("SELECT not_wanted_tips FROM users WHERE id=$uid"),0);
			}
		}
		else
			$this->not_wanted_tips = 0;
		$this->index = 0;
	}
	
	public function __destruct() {
		if (self::DEBUG)
			echo "DIDYOUKNOW: Destructor called.<br>\n";
		unset(self::$INSTANCE[$this->uid]);
		$this->save_all();	
	}
	
	private function save_all() {
		global $pv_db;
		if ($this->db_need_sync && $this->uid) {
			query("UPDATE users SET not_wanted_tips={$this->not_wanted_tips} WHERE id={$this->uid}");
			$_SESSION['userdata']['not_wanted_tips'] = $this->not_wanted_tips;	// auch die Session aktuell halten
		}
	}
	
	/**
	 * Gibt einen zufälligen Text aus dem entsprechenden Array zurück. 
	 * Das Array muss vorher mit set_page() festgelegt werden, ansonsten 
	 * wird self::$TEXTS_GENERAL angenommen
	 */
	public function get_rand_text() {
		switch ($this->page) {
			case self::PAGE_HELLO:
				$this->index = rand(0, count(self::$TEXTS_HELLO)-1);
				return self::$TEXTS_HELLO[$this->index];
			case self::PAGE_CREATUREMAP:
				$this->index = rand(0, count(self::$TEXTS_CREATUREMAP)-1);
				return self::$TEXTS_CREATUREMAP[$this->index];
			case self::PAGE_SECTOR_VIEW:
				$this->index = rand(0, count(self::$TEXTS_SECTOR_VIEW)-1);
				return self::$TEXTS_SECTOR_VIEW[$this->index];
			case self::PAGE_HOME_SECTOR:
				$this->index = rand(0, count(self::$TEXTS_HOME_SECTOR)-1);
				return self::$TEXTS_HOME_SECTOR[$this->index];
			case self::PAGE_SCIENCE:
				$this->index = rand(0, count(self::$TEXTS_SCIENCE)-1);
				return self::$TEXTS_SCIENCE[$this->index];
			case self::PAGE_TRIBES:
				$this->index = rand(0, count(self::$TEXTS_TRIBES)-1);
				return self::$TEXTS_TRIBES[$this->index];
			case self::PAGE_MARKET_PLACE:
				$this->index = rand(0, count(self::$TEXTS_MARKET_PLACE)-1);
				return self::$TEXTS_MARKET_PLACE[$this->index];
			default:
				$this->index = rand(0, count(self::$TEXTS_GENERAL)-1);
				return self::$TEXTS_GENERAL[$this->index];
		}
	}
	
	/**
	 * Gibt den nächsten Text im entprechenden Array zurück.
	 * Das Array muss vorher mit set_page festgelegt werden, ansonsten 
	 * wird self::$TEXTS_GENERAL angenommen.
	 * Ebenso muss der aktuelle Index bestimmt sein. Dies kann direkt mit
	 * set_index() getan werden, oder wird implizit durch aufruf von get_*_text()
	 * gemacht
	 * 
	 * Der Witz bei der Funktion ist, dass er durch das Array rotiert. Dies kann
	 * nicht außerhalb durch JS passieren, weil die Arraygrößen nicht bekannt sind.
	 */
	public function get_next_text() {
		switch ($this->page) {
			case self::PAGE_HELLO:
				$this->index = ($this->index >= count(self::$TEXTS_HELLO)-1) ? 0 : $this->index+1;
				return self::$TEXTS_HELLO[$this->index];
			case self::PAGE_CREATUREMAP:
				$this->index = ($this->index >= count(self::$TEXTS_CREATUREMAP)-1) ? 0 : $this->index+1;
				return self::$TEXTS_CREATUREMAP[$this->index];
			case self::PAGE_SECTOR_VIEW:
				$this->index = ($this->index >= count(self::$TEXTS_SECTOR_VIEW)-1) ? 0 : $this->index+1;
				return self::$TEXTS_SECTOR_VIEW[$this->index];
			case self::PAGE_HOME_SECTOR:
				$this->index = ($this->index >= count(self::$TEXTS_HOME_SECTOR)-1) ? 0 : $this->index+1;
				return self::$TEXTS_HOME_SECTOR[$this->index];
			case self::PAGE_SCIENCE:
				$this->index = ($this->index >= count(self::$TEXTS_SCIENCE)-1) ? 0 : $this->index+1;
				return self::$TEXTS_SCIENCE[$this->index];
			case self::PAGE_TRIBES:
				$this->index = ($this->index >= count(self::$TEXTS_TRIBES)-1) ? 0 : $this->index+1;
				return self::$TEXTS_TRIBES[$this->index];
			case self::PAGE_MARKET_PLACE:
				$this->index = ($this->index >= count(self::$TEXTS_MARKET_PLACE)-1) ? 0 : $this->index+1;
				return self::$TEXTS_MARKET_PLACE[$this->index];
			default:
				$this->index = ($this->index >= count(self::$TEXTS_GENERAL)-1) ? 0 : $this->index+1;
				return self::$TEXTS_GENERAL[$this->index]; 
		}
	}
	
	/**
	 * Gibt einen aggressiven Text zurück. Aggressive Texte sind solche, die in jedem Fall aufpoppen,
	 * wenn eine gewisse Situation erreicht ist und vom User dieser Tipp nicht aktuell als unerwünscht
	 * angehakt wurde.
	 * $this->set_situation(..) muss vorher aufgerufen worden sein. Ansonsten wird false zurückgeliefert!
	 */
	public function get_aggressive_text() {
		if (($this->situation > self::SIT_NONE) && ($this->situation == ($this->not_wanted_tips & $this->situation)))
			return false;	// not wanted or not set!
		else
			return self::$AGGRESSIVE_HINTS[$this->situation];
	}
	
	public function dont_show_again($situation_id) {
		$this->not_wanted_tips = $this->not_wanted_tips | $situation_id;
		$this->db_need_sync = true;
	}
	
	/** SETTER */
	public function set_page($page) {
		$this->page = (int)$page;
	}
	
	public function set_index($index) {
		$this->index = (int)$index;
	}
	public function set_situation($sit) {
		if ($sit == ($this->not_wanted_tips & $sit))
			return false;	// not wanted situation tip	
		else {
			$this->situation = (int)$sit;
			return true;
		}
	}
	
	/** GETTER */
	public function get_index() {
		return $this->index;
	}
	
	public function get_page() {
		return $this->page;
	}
	public function get_situation() {
		if ($this->situation == ($this->not_wanted_tips & $this->situation))
			return self::SIT_NONE;
		else
			return $this->situation;
	}
}

?>