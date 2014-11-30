<?
@include_once (dirname(__FILE__).'/../__private_config.inc');
require_once(dirname(__FILE__).'/../gen_funcs.inc');
require_once(dirname(__FILE__).'/../shared/classes/exceptions/'.
										'ExceptionWithErrorlog.inc');

/**
 * Wrapper-Funktion. Damit man weiterhin nur query eingeben kann.
 * Dies wird dann auf $pv_db::query() gelinkt mit globalem $pv_db.
 *
 * @param String $sql_code
 * @return SQL-Resource
 * @throws ExceptionWithErrorlog
 * @throws Exception
 */
function query($sql_code, $no_log=false) {
	global $pv_db, $con;
	if (!$pv_db) {
		$con = connect_db();
	}

	return $pv_db->query($sql_code, $no_log);
}

/**
 * Wrapper-Funktion. Damit man weiterhin nur $con=connect_db() eingeben kann.
 * Das wird dann umgeleitet etc :)
 *
 * @return uint		Connection ID
 */
function connect_db() {
	global $pv_db;
	$pv_db = new Db();
	return $pv_db->getConId();
}

class Db {
	const GAME_HOST = 'localhost';
	const GAME_NAME = 'evo';
	const GAME_SELECT_DB = 'pvgame';
	const GAME_PW = 'C#1sSh1t';

	const BUG_HOST = self::GAME_HOST;	// Muss so sein! Cross-Queries!
	const BUG_NAME = 'bug';
	const BUG_SELECT_DB = 'bugreports';
	const BUG_PW = 'fOo1b0rG_o$';

	const SCORE_HOST = 'localhost';
	const SCORE_NAME = 'score';
	const SCORE_SELECT_DB = 'score';
	const SCORE_PW = 'h27zh&ma9A';

	const LOGS_HOST = 'localhost';
	const LOGS_NAME = 'logs';
	const LOGS_SELECT_DB = 'logs';
	const LOGS_PW = 'Fpa9_Isj#98';

	const PAYMENT_HOST = 'localhost';
	const PAYMENT_NAME = 'payment';
	const PAYMENT_SELECT_DB = 'payment';
	const PAYMENT_PW = 'ja8s+74u8Q';

	const USER_PERSONAL_HOST = 'localhost';
	const USER_PERSONAL_SELECT_DB = 'user_personal';

	const COM_HOST = 'localhost';
	const COM_NAME = 'wcf';
	const COM_SELECT_DB = 'wcf';
	const COM_PW = 'Bl0eD1an';


	const GAME_LOCAL_TEST_SELECT_DB = 'evo_test_db';

	private $con_id;
	private $is_admin;
	private $res;
	private $host;
	private $name;
	private $pw;
	private $select_db;
	private $persistant;

	private $querycounter = 1;

	const DEBUG = 0;

	public function __construct($host = self::GAME_HOST, $name = self::GAME_NAME,
		$pw = self::GAME_PW, $select_db = self::GAME_SELECT_DB,
		$persistant = false, $suppress_errors = false, $is_admin = false)
	{
		/**
		 * An zentraler Stelle wird hier die Datenbank im Falle vom Testserver
		 * und im Falle von Unit-Test-Mode gewechselt. Somit kann nichts
		 * passieren.
		 */
		if (defined('TEST_SERVER') && TEST_SERVER) {
			$select_db .= '_test';
		}
		elseif (defined('CONF_UNIT_TEST_MODE') && CONF_UNIT_TEST_MODE) {
			$select_db .= '_unittest';
		}

		$this->con_id = mysql_connect($host, $name, $pw);

		if ((!$this->con_id || !@ mysql_select_db($select_db, $this->con_id))
				 && !$suppress_errors) {
			error_log('NO CONNECT POSSIBLE!: '.$select_db);
			die('<html><head><title>' .
			LNG_DB_ERROR . '</title></head><body><center><font color=red>' . LNG_ERR_NO_DB_CON . '</font></center></body></html>');
		 }
		else if (self::DEBUG)
			error_log("DB connected to $select_db.");

		$this->host = $host;
		$this->name = $name;
		$this->pw = $pw;
		$this->select_db = $select_db;
		$this->persistant = $persistant;
	}

	public function __destruct() {
		@ mysql_free_result($this->res);
		@ mysql_close();
	}

	public function set_admin($v) {
		$is_admin = $v;
	}

	/**
	 * Führt die SQL-Query aus und gibt eine SQL-Resource zurück
	 *
	 * @param String $sql_code
	 * @return SQL-Resource
	 * @throws ExceptionWithErrorlog
	 * @throws Exception
	 */
	public function query($sql_code, $no_log=false) {
		if (defined("CONF_DB_READ_ONLY") && CONF_DB_READ_ONLY
			&& (strtoupper(substr($sql_code, 0, 6)) != 'SELECT')
			&& (strtoupper(substr($sql_code, 0, 13)) != "UPDATE USERS ")
			)
		{
			error_log("PREVENTING $sql_code DUE TO READ ONLY MODE");

		}
		$t = microtime(true);
		$this->res = mysql_query($sql_code, $this->con_id);
		$took = (microtime(true) - $t);
		if (self::DEBUG) {
			error_log("sql-query" . $this->querycounter++ . ": $sql_code ".
				"(Duration: $took)");
		}
		if (!$this->res) {
			if ($no_log) {
				throw new Exception(
					"Database Error! Time:".date("Y-m-d, h:i:s",Time::get_t_now())
				);
			}
			else {
				$current_user = User::get_current_user();
				$uid = ($current_user) ? User::get_current_user()->get_id() : 0;
				throw new ExceptionWithErrorlog(
					"Database Error! Time:".date("Y-m-d, h:i:s",Time::get_t_now()),
					$uid,
					"DB ERROR: '$sql_code' : " . mysql_error()
				);
			}
		}
		if ($took >= 500)
			error_log("DB WARNING: Long Query duration: '$sql' took $took ms.");
		return $this->res;
	}

	public function getConId() {
		return $this->con_id;
	}
	public function getHost() {
		return $this->host;
	}
	public function getName() {
		return $this->name;
	}
	public function getPW() {
		return $this->pw;
	}
	public function getSelectDb() {
		return $this->select_db;
	}
	public function getPersistant() {
		return $this->persistant;
	}
}

?>