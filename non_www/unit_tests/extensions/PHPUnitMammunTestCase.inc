<?php
require_once (dirname(__FILE__).'/../_config.inc');
require_once (CONF_GAME_ROOT . '/include/consts.inc');
require_once (CONF_GAME_ROOT . '/include/classes/Accesscontrol.inc');
require_once (CONF_UNIT_TESTS_ROOT . '/extensions/DbDumpInstaller.inc');


class PHPUnitMammunTestCase extends PHPUnit_Framework_TestCase {

	/**
	 * Loggt den Spieler mit dem angegebenen Nick ein. Im Testmodus wird
	 * kein Passwort benötigt.
	 *
	 * @param string $nick
	 * @throws Exception, IllegalArgumentException
	 */
	public static function login_user($nick) {
		ValueChecker::string($nick, 'nick');

		Accesscontrol::init($nick, '');
		Accesscontrol::load_current_user();
		Accesscontrol::do_login_stuff();
	}

	/**
	 * Spielt den angegebenen Datenbank-Dump ein.
	 *
	 * @param unknown_type $dump_file
	 * @throws Exception, IllegalArgumentException
	 */
	public static function install_db_dump($dump_file) {
		ValueChecker::string($dump_file, 'dump_file');

		DbDumpInstaller::source_db_dump($dump_file);
	}

	/**
	 * Nach jedem Testdurchlauf sollen die Logs und Punkte gelöscht werden.
	 *
	 * @see Framework/PHPUnit_Framework_TestCase::tearDown()
	 */
	public static function tearDownAfterClass() {
		DbDumpInstaller::source_db_dump('logs__empty');
		DbDumpInstaller::source_db_dump('score__empty');
	}

}