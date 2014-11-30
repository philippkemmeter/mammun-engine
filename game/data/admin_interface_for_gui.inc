<?php
require_once ('../include/shared/classes/server_com/ClientServer.inc');
require_once ('../include/classes/Admin.inc');
require_once ('../include/accesscontrol2.inc');
$cryptObj = new PV_encryption();

$expected_data = array (
	'reset_all_accounts'	=>		'(e)serialized',
	'generate_science_data'	=>		'(e)serialized',
	'get_user_statistics'	=>		'(e)serialized'
);

$GP = $cryptObj->load_GET_POST($expected_data);
if (empty($GP)) {
	echo "<b>Warning</b>: No known command transmitted.<br/>\n";
	$data = (!empty($_GET)) ? $_GET : $_POST;
	echo "<br/>\nThe following transmitted commands are unknown:<br/>\n";
	foreach ($data as $command => $params) {
		if ($command === 'PHPSESSID')
			continue;
		$params = unserialize($params);
		if (is_array($params))
			echo "- $command(".implode(', ', $params).")<br/>\n";
		else
			echo "- $command($params)<br/>\n";
	}
	echo "<br/>\n";
}
$r = new ClientServerResponse();
foreach ($GP as $command => $params) {
	try {
		if (!is_array($params))
			$r->add_value(Admin::$command($params));
		else {
			switch(count($params)) {
				case 1:
					$r->add_value(Admin::$command($params[0]));
					break;
				case 2:
					$r->add_value(Admin::$command($params[0], $params[1]));
					break;
				case 3:
					$r->add_value(Admin::$command($params[0], $params[1],
						$params[2]));
					break;
				case 4:
					$r->add_value(Admin::$command($params[0], $params[1],
						$params[2], $params[3]));
					break;
				case 5:
					$r->add_value(Admin::$command($params[0], $params[1],
						$params[2], $params[3], $params[4]));
					break;
				case 6:
					$r->add_value(Admin::$command($params[0], $params[1],
						$params[2], $params[3], $params[4], $params[5]));
					break;
				default:
					echo "<b>Warning</b>: Too many Params transmitted, only 6 allowed.<br/>\n";
			}
		}
	}
	catch (Exception $e) {
		$r->add_exception($e);
	}
	catch (ExceptionWithErrormail $e) {
		$r->add_exception(new Exception($e->getMessage(), $e->getCode()));
	}
	catch (HackattackException $e) {
		$r->add_exception($e);
	}
}
echo $r;
?>