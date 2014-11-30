<?php
@include_once(dirname(__FILE__).'/../../_private_config.inc');
/**
 * Dieser Klasse liest DB-Dumps in die Datenbank ein.
 */
class DbDumpInstaller {

	private function __construct(){}

	/**
	 * Liest die angegebene SQL-Datei ein, wenn sie sich im
	 * <code>CONF_DB_BACKUP_DUMP_PATH</code> befindet.
	 *
	 * @param string $filename
	 * @throws Exception
	 */
	public static function source_db_dump($filename) {
		if (!defined("CONF_UNIT_TEST_MODE") || !CONF_UNIT_TEST_MODE) {
			throw new Exception(
				"This function is in available in unit test mode, only"
			);
		}

		if (substr($filename, -4) == '.sql') {
			throw new IllegalArgumentException(
				"Don't append '.sql' to filename."
			);
		}

		$f = explode('__', $filename);
		$db_name = $f[0];
		unset($f);

		$path_and_file = CONF_DB_BACKUP_DUMP_PATH . "/$filename.sql";
		if (!@file_exists($path_and_file)) {
			throw new IllegalArgumentException(
				"File '$filename' does not exist."
			);
		}

		exec(
			"mysql -uroot_test -p\\\$pA\\\$tI#t1 {$db_name}_unittest < "
				. CONF_DB_BACKUP_DUMP_PATH . "/$filename.sql", $foo, $result
		);

		if ($result != 0) {
			throw new Exception(
				"Dump '$filename' could not be sourced. This can have multiple ".
				"reasons. One is, that mysql is not installed or that the ".
				"dump file is corrupted."
			);
		}

	}

	/**
	 * Gibt alle SQL-Dateien im <code>CONF_DB_BACKUP_DUMP_PATH</code> zurück.
	 *
	 * @return string[]
	 */
	public static function list_dump_files() {
		$result = array();
		if ($handle = opendir(CONF_DB_BACKUP_DUMP_PATH)) {
			while (false !== ($file = readdir($handle))) {
				if (($file != ".") && ($file != "..")
					&& (substr($file, -4) == '.sql'))
				{
					$result[] = $file;
				}
			}
			closedir($handle);
		}
		return $result;
	}

}

?>