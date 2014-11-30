<?php
/**
 * File created by Philipp Kemmeter on 27.02.2009 at 11:50:37
 */

require_once(dirname(__FILE__).'/Science.inc');
require_once(dirname(__FILE__).'/ScienceChooser.inc');
require_once(dirname(__FILE__).'/Techtree.inc');
require_once(dirname(__FILE__).'/Tribes.inc');
require_once(dirname(__FILE__).'/EventMessages.inc');
require_once(dirname(__FILE__).'/User.inc');
require_once(dirname(__FILE__).'/map/MapCompute/MapCompute.inc');
require_once(dirname(__FILE__).'/buildings/Buildings.inc');
require_once(dirname(__FILE__).'/World.inc');
require_once(dirname(__FILE__).'/map/MapPlayers.inc');
require_once(dirname(__FILE__).'/map/Sector.inc');
require_once(dirname(__FILE__).'/map/MapDescription.inc');
require_once(dirname(__FILE__).'/exceptions/HackattackException.inc');

/**
 * Statische Klasse, die alle Admin-Funktionen enthält. Das sind Funktionen,
 * die besonders kritisch sind und nur von authorisierten Nutzern aufgerufen
 * werden dürfen.
 *
 * @author Philipp Kemmeter
 */
class Admin {
	/**
	 * Kein Konstruktor - statische Klasse!
	 */
	private function __construct() {}

	/**
	 * Gibt zurück, ob der aktuell eingeloggte User Adminrechte hat, und
	 * somit die hier aufgeführten Funktionen ausführen darf.
	 * @return bool
	 */
	public static function current_user_is_admin() {
		$current_user = User::get_current_user();
		$nick_lower = strtolower($current_user->get_nick());
		return ($nick_lower === 'philipp');
	}
	/**
	 * !!! Setzt alle Accounts auf den Initialzustand zurück!!!
	 * @return bool
	 * @throws HackattackException
	 */
	public static function reset_all_accounts() {
		if (!self::current_user_is_admin())
			throw new HackattackException(
				"Action not allowed",
				User::get_current_user()->get_id(),
				HackattackException::CODE_ADMIN_FUNCTION
			);
		Techtree::clear_of_all_users();
		Science::clear_of_all_users();
		ScienceChooser::clear_of_all_users();
		Tribes::clear_of_all_users();
		User::clear_of_all_users();
		UserMedals::clear_of_all_users();
		MapCompute::clear_of_all_users();
		Buildings::clear_of_all_users();
		TribeOnMap::clear_of_all_users();
		World::clear_of_all_users();
		MapDescription::clear_of_all_users();
		MapPlayers::clear_of_all_users();
		Sector::clear_of_all_users();
		Resource::clear_of_all_users();
		EventMessages::clear_of_all_users();
		LeagueScore::clear_of_all_users();
		MapScore::clear_of_all_users();
   		return true;
	}

	/**
	 * Generiert die Forschungsdaten und gibt sie aus (als PHP-Datei)
	 * @return bool
	 * @throws HackattackException
	 */
	public static function generate_science_data() {
		if (!self::current_user_is_admin())
			throw new HackattackException(
				"Action not allowed",
				User::get_current_user()->get_id(),
				HackattackException::CODE_ADMIN_FUNCTION
			);
		include_once(dirname(__FILE__).'/../ad_min/generate_science_data.inc');

		return get_general_science_data();
	}


	/**
	 * Gibt die Nutzerstatistik zurück.
	 *
	 */
	public static function get_user_statistics() {
		$result = new stdClass();
		$result->registered_user_amount = User::get_registered_user_amount();
		$result->registered_fb_user_amount = User::get_registered_user_amount(true);
		$result->active_user_amount = User::get_active_user_amount();
		$result->online_user_amount = User::get_online_user_amount();
		$result->active_fb_user_amount = User::get_active_user_amount(0, true);
		return $result;
	}
}
?>