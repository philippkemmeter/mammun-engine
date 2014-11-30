<?php
require_once(dirname(__FILE__).'/gen_funcs.inc');
require_once(dirname(__FILE__).'/consts.inc');
@include_once(dirname(__FILE__).'/__private_config.inc');
require_once(dirname(__FILE__).'/classes/Accesscontrol.inc');
require_once(dirname(__FILE__).'/shared/classes/PV_encryption.inc');
require_once(dirname(__FILE__).'/classes/DBCleanAndClear.inc');
require_once(dirname(__FILE__).'/shared/classes/server_com/ClientServer.inc');

Accesscontrol::secure_session_start();

if (!isset($pv_db)) { // xxxx => com_login.inc
	require_once(dirname(__FILE__).'/classes/Db.inc');

	$pv_db = new Db();
	$con = $pv_db->getConId();
}

if (!isset($cryptObj)) {
	require_once(dirname(__FILE__).'/shared/classes/PV_encryption.inc');
	$cryptObj = new PV_encryption();
}


$mapsettings = Mapsettings::get_instance();

if ((defined('PHILS_PRIVATE_SERVER') || defined('GAME_TEST_SERVER')) && $_GET['time_factor']) {
	$mapsettings = Mapsettings::get_instance();
	$mapsettings->set_timefactor($_GET['time_factor']);
}

$expected_data = array (
	'pin_number'		=> 'eint',
	'pin_time'			=> 'eint',
	'login'				=> 'bool',
	'nick'				=> 'nospacestring',
	'pass'				=> 'string',
	'rechoose_map'		=> 'ebool',
	'choose_training'	=> 'ebool',
	'sector_id'			=> '(e)int',
	'input_pin'			=> 'int',
	'msg'				=> 'string',
	'goon'				=> 'bool'
);
$GP_accesscontrol = $cryptObj->load_GET_POST($expected_data);

/**
 * Nur wenn Neulogin muss Accesscontrol initialisiert werden, ansonsten bleiben
 * die Daten durch die Session erhalten.
 */
if (!empty($GP_accesscontrol['login'])) {
	Accesscontrol::init(
		$GP_accesscontrol['nick'],
		md5($GP_accesscontrol['pass']),
		@$GP_accesscontrol['pin_number'],
		@$GP_accesscontrol['pin_time'],
		@$GP_accesscontrol['input_pin']
	);
}

try {
	$current_user = Accesscontrol::load_current_user();
}
catch(Exception $e) {
	Accesscontrol::kill_session($e->getCode());
}

//login erfolgreich!!!
Accesscontrol::do_login_stuff();