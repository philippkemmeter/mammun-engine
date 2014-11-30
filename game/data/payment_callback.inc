<?php
require_once ('../include/_config.inc');
require_once ('../include/shared/classes/PV_encryption.inc');
require_once ('../include/classes/Db.inc');
require_once ('../include/classes/EventMessages.inc');
require_once ('../include/shared/classes/server_com/TrustedServerAuth.inc');

$cryptObj = new PV_encryption();
$expected_data = array(
	'action'		=>	'string',
	'uid'			=>	'int',
	'to_uid'		=>	'int',
	'type'			=>	'int',
	'id'			=>	'int'
);

$GP = $cryptObj->load_GET_POST($expected_data);
if (!TrustedServerAuth::verify($_REQUEST, CONF_MAMMUN_AUTH_SECRET)) {
	die("AUTHENTIFICATION FAILED!");
}

switch ($GP['action']) {
	case 'make_a_gift':
		EventMessages::get_instance($GP['to_uid'])->add_entry(
			IEventLog::TYPE_GIFT_RECEIVED,
			$GP['uid'],
			"{$GP['type']}-{$GP['id']}"
		);
		break;
}

?>