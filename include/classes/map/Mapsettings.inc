<?


class Mapsettings {
	private static $map_settings;		// garantiert synchronisierung zwischen
										// den instanzen
	private $auto_save;

	const DEBUG = 0;

	const MAX_TIME_FACTOR = 1000;		// Darüber wird die Engine Instabil!
	const NORMAL_TIME_FACTOR = 1;

	static private $INSTANCE = array();	// instanzüberwachung. Nur eine Instanz
										// pro UID.

	/* Diese Statische Funtion als Konstruktor benutzen !*/
 	public static function get_instance() {
 		if (self::DEBUG) {
			echo "Mapsettings: User id@get_instanc<br>\n";
 		}
 		if (!self::$INSTANCE) {
 			self::$INSTANCE = new Mapsettings();
 		}
 		return self::$INSTANCE;
 	}

	private function __construct () {
		$this->load_data();
		$this->auto_save = true;
	}

	function __destruct() {
		$this->save_data();
	}

	public function set_autosave($a) {
		$this->auto_save = $a;
	}

	public function get_autosave() {
		return $this->auto_save;
	}

	public function set_timefactor($time_factor) {
		self::$map_settings['time_factor'] = $time_factor;
		if ($this->auto_save) {
			$_SESSION['map_settings']['time_factor']
				= self::$map_settings['time_factor'];
		}
	}

	public function get_timefactor() {
		if (self::$map_settings['time_factor'] > self::MAX_TIME_FACTOR)
			return self::MAX_TIME_FACTOR;
		else
			return (self::$map_settings['time_factor'])
				? self::$map_settings['time_factor']
				: self::NORMAL_TIME_FACTOR;
	}

	private function save_data() {
		$_SESSION['map_settings'] = self::$map_settings;
	}

	private function load_data() {
		if (isset($_SESSION['map_settings']))
			self::$map_settings = $_SESSION['map_settings'];

		if (empty(self::$map_settings['time_factor']))
				self::$map_settings['time_factor'] = self::NORMAL_TIME_FACTOR;
	}
}

?>