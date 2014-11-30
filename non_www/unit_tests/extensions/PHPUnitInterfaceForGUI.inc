<?php
class PHPUnitInterfaceForGUI {
	private function __construct(){}

	/**
	 * Liest die angegebene SQL-Datei ein, wenn sie sich im
	 * <code>CONF_DB_BACKUP_DUMP_PATH</code> befindet.
	 *
	 * @param string $filename
	 * @throws Exception
	 */
	public static function install_db_dump($filename) {
		require_once(dirname(__FILE__).'/DbDumpInstaller.inc');

		DbDumpInstaller::source_db_dump($filename);
		return true;
	}

	/**
	 * Liest die angegebene SQL-Datei ein, wenn sie sich im
	 * <code>CONF_DB_BACKUP_DUMP_PATH</code> befindet.
	 *
	 * @param string $filename
	 * @throws Exception
	 */
	public static function list_dump_files() {
		require_once(dirname(__FILE__).'/DbDumpInstaller.inc');

		return DbDumpInstaller::list_dump_files();
	}

}
?>