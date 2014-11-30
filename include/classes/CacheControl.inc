<?


require_once (dirname(__FILE__).'/../consts.inc');
require_once (dirname(__FILE__).'/../assert.inc');
class CacheControl {
	private $t_now;
	private $uid;
	private $cur_base_cache_string;	// unter welchem name das aktuelle Caching läuft (wird für start/stop caching benötigt)
									// für 304-caching unbedeutend

	const CACHE_INTO_SESSION = 0;
	const CACHE_INTO_FILE = 1;
	//const CACHE_INTO_DB = 2; // not supported yet

	const DEBUG = 0;
	const CACHE_PATH = '/var/kunden/webs/pv-game-us/c_a_ch_e/';	// wenn man cachen in dateien angegeben hat, dann werden die dateien dort abgelegt
	const MAX_CACHE_TIME = 1800;	// 30min
	const CACHE_STYLE = self::CACHE_INTO_SESSION;


	static private $INSTANCE = array();	// instanzüberwachung. Nur eine Instanz pro UID innerhalb von PV
 										// Konstruktor ist private. Objekt statisch über get_instance($uid)
 										// anfordern! (s.u.)

	/* Diese Statische Funtion als Konstruktor benutzen !*/
 	public static function get_instance($uid=0) {
 		if (self::DEBUG) {
			echo "CacheControl: User id@get_instance<br>\n";
 		}
 		if (!self::$INSTANCE || !self::$INSTANCE[$uid]) {
 			self::$INSTANCE[$uid] = new CacheControl($uid);
 		}
 		return self::$INSTANCE[$uid];
 	}

	private function __construct($uid) {
		if (self::DEBUG)
 			echo "CacheControl: constructor called<br>\n";

		$this->uid = $uid;
		$this->t_now = Time::get_t_now();
		$this->cur_base_cache_string = false;
	}
	/**
	 * gibt zurück, ob die Datei sich nicht geändert hat und somit ein 304 gesendet werden kann
	 * sobald also diese Funktion true zurückgibt, muss exit/die aufgerufen werden.
	 */
	public function check_file_cache() {
		if (self::DEBUG)
			echo time();

		$mtime = filemtime(FILE_SELF);
		$last_modified = gmdate('D, d M Y H:i:s', $mtime) . ' GMT';

		$etag = '"'.md5($last_modified).'"';
	    // Send the headers
	    header("Last-Modified: $last_modified");
	  //  header("ETag: $etag");

		$if_modified_since = isset($_SERVER['HTTP_IF_MODIFIED_SINCE']) ?
			stripslashes($_SERVER['HTTP_IF_MODIFIED_SINCE']) : false;
/*		$if_none_match = isset($_SERVER['HTTP_IF_NONE_MATCH']) ?
			stripslashes($_SERVER['HTTP_IF_NONE_MATCH']) : false;*/
	    if (!$if_modified_since /*&& !$if_none_match*/) {
			return false;
	    }/*
		// At least one of the headers is there - check them
	    else if ($if_none_match && $if_none_match != $etag) {
			return false; // etag is there but doesn't match
	    }
	    // if_modified_since gesandt, aber passt nicht zur Zeit
		else*/ if ($if_modified_since && $if_modified_since != $last_modified) {
			return false;
		}

		header('HTTP/1.0 304 Not Modified');

		return false;
	}


	/**
	 * Startet das Caching.
	 * Ist eine gecachte Datei vorhanden, die nicht älter ist, als self::MAX_CACHE_TIME
	 * und wird kein reload erzwungen mit $forced_reload, dann wird die Datei aus dem Cache
	 * geholt, ansonsten wird das Caching hier mit output-buffering gestartet.
	 *
	 * $base_cache_string kann angegeben werden, wenn man z.B. innerhalb einer datei mehrere
	 * Bereiche unabhängig voneinander cachen will. Ansonsten ist es einfach der Dateiname.
	 *
	 * WICHTIG: MAN KANN KEINE CACHEGESCHICHTEN VERSCHACHTELN !
	 */
	public function start_caching($base_cache_string=FILE_SELF, $forced_reload=false) {
		pv_assert(self::CACHE_STYLE == self::CACHE_INTO_SESSION || self::CACHE_STYLE == self::CACHE_INTO_FILE,
			'CACHE_STYLE = '.self::CACHE_STYLE.'. Not supported. in CacheControl::start_caching()');

		if (self::DEBUG)
			pv_assert(!$this->cur_base_cache_string , '$this->cur_base_cache_string IS set in CacheControl::start_caching(). ' .
				'Have you cascaded cache stuff like calling start_caching() while caching is still enabled?');

		$this->cur_base_cache_string = $base_cache_string;
		if (self::CACHE_STYLE == self::CACHE_INTO_SESSION) {
			if (!isset ($_SESSION)) {
				if (self::DEBUG)
					echo "<b>Warning:</b> Session was not started in CacheControl::start_caching().";
				session_start();
			}

			$cached_stuff = $_SESSION['cache'][$base_cache_string];	// entprechend gecachten Eintrag aus der Session lesen

			if ($forced_reload || !$cached_stuff)	// wenn cache verworfen werden soll ($foced_reload) oder kein cache ist
				ob_start();							// dann caching starten
			else {
				if (self::DEBUG)
					echo "cached output:<br>";
				die($cached_stuff);					// ansonsten cache ausgeben und ende
			}
		}
		else if (self::CACHE_STYLE == self::CACHE_INTO_FILE) {
			$filename = self::CACHE_PATH.md5($this->uid).'---'.md5($base_cache_string).'.cache';
			$cachefile_created = (file_exists($filename)) ? filemtime($filename) : false;
			clearstatcache();
			if ($forced_reload || !$cachefile_created || ((time() - self::MAX_CACHE_TIME) >= $cachefile_created))
				ob_start();
			else {
				readfile($filename);
				exit;
			}
		}
	}


	/**
	 * Beendet den Cachevorgang.
	 *
	 * NIEMALS vergessen! :)
	 *
	 * $base_cache_string muss mit start_cachings $base_cache_string übereinstimmen
	 * Sprich, start_caching muss mit denselben Paramtern aufgerufen werden, wie stop_caching
	 */
	public function stop_caching() {
		pv_assert(self::CACHE_STYLE == self::CACHE_INTO_SESSION || self::CACHE_STYLE == self::CACHE_INTO_FILE,
			'CACHE_STYLE = '.self::CACHE_STYLE.'. Not supported. in CacheControl::start_caching()');

		if (self::DEBUG)
			pv_assert($this->cur_base_cache_string, '$this->cur_base_cache_string emtpy or not set in CacheControl::stop_caching(). ' .
				'Have your called stop_caching() before start_caching()?');

		if (self::CACHE_STYLE == self::CACHE_INTO_SESSION) {
			if (!isset ($_SESSION)) {
				if (self::DEBUG)
					echo "<b>Warning:</b> Session was not started in CacheControl::stop_caching().";
				session_start();
			}

			$_SESSION['cache'][$this->cur_base_cache_string] = ob_get_contents();		// ausgabe in der Session speichern
		}
		else if (self::CACHE_STYLE == self::CACHE_INTO_FILE) {
			$filename = self::CACHE_PATH.md5($this->uid).'---'.md5($this->cur_base_cache_string).'.cache';
			if (file_exists($filename)) {
				chmod($filename, 777);
			}
			 // Now the script has run, generate a new cache file
			$fp = fopen($filename, 'w');
			// save the contents of output buffer to the file
			fwrite($fp, ob_get_contents());
			fclose($fp);
		}
		ob_end_flush(); 	// in jedem Fall den output buffer flushen und das puffern beenden
		$this->cur_base_cache_string = false;	// und den aktuellen base_cache_string wieder auf false setzen.
												// Somit ist klar, dass wir nicht mehr am cachen sind.
	}

	/**
	 * Löscht alle gecachten Dateien des im Konstruktor angegebenen Users.
	 */
	public function clear_whole_cache() {
		if (self::DEBUG)
			echo "clearing whole cache...<br>",
		pv_assert(self::CACHE_STYLE == self::CACHE_INTO_SESSION || self::CACHE_STYLE == self::CACHE_INTO_FILE,
			'CACHE_STYLE = '.self::CACHE_STYLE.'. Not supported. in CacheControl::start_caching()');

		if (self::CACHE_STYLE == self::CACHE_INTO_SESSION) {
			if (!isset ($_SESSION)) {
				if (self::DEBUG)
					echo "<b>Warning:</b> Session was not started in CacheControl::clear_cache().";
				session_start();
			}
			unset ($_SESSION['cache']);
		}
		else if (self::CACHE_STYLE == self::CACHE_INTO_FILE) {
			if ($handle = opendir(self::CACHE_PATH)) {
			    while (false !== ($file = readdir($handle))) {
			    	if ($file == '.' || $file == '..')
			    		continue;
			    	//echo $file.'<br>';
			    	if (strpos($file, md5($this->uid).'---') !== false) {
						//echo $file . ' deleted.<br>';
						unlink(self::CACHE_PATH . $file);
			    	}
			    }
			}
		}
	}

	/**
	 * Löscht den entsprechenden Eintrag basierend auf $base_cache_string
	 */
	public function clear_cached_entry($base_cache_string=FILE_SELF) {
		if (self::DEBUG)
			echo "clearing cache for $base_cache_string<br>";

		pv_assert(self::CACHE_STYLE == self::CACHE_INTO_SESSION || self::CACHE_STYLE == self::CACHE_INTO_FILE,
			'CACHE_STYLE = '.self::CACHE_STYLE.'. Not supported. in CacheControl::start_caching()');

		if (self::CACHE_STYLE == self::CACHE_INTO_SESSION) {
			if (!isset ($_SESSION)) {
				if (self::DEBUG)
					echo "<b>Warning:</b> Session was not started in CacheControl::clear_cache().";
				session_start();
			}
			unset ($_SESSION['cache'][$base_cache_string]);
		}
		else if (self::CACHE_STYLE == self::CACHE_INTO_FILE) {
			if ($handle = opendir(self::CACHE_PATH)) {
			    while (false !== ($file = readdir($handle))) {
			    	if ($file == '.' || $file == '..')
			    		continue;
			    	//echo $file.'<br>';
			    	if ($file === md5($this->uid).'---'.md5($base_cache_string).'.cache')
			    		unlink(self::CACHE_PATH.$file);
			    }
			}
		}
	}
}

?>