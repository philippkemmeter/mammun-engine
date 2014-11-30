<?php
require_once('_config.inc');
require_once(CONF_UNIT_TESTS_ROOT . '/extensions/PHPUnitInterfaceForGUI.inc');
require_once('../../include/shared/classes/PV_encryption.inc');
require_once('../../include/shared/classes/server_com/ClientServer.inc');
$cryptObj = new PV_encryption();
$expected_data_with_session = array();
$expected_data_without_session = array (
	'install_db_dump'							=>	'(e)serialized',
	'list_dump_files'							=>	'(e)serialized'
);
$GP1 = $cryptObj->load_GET_POST($expected_data_with_session);
$GP2 = $cryptObj->load_GET_POST($expected_data_without_session);
if ($GP1) {
	require_once ('../include/accesscontrol2.inc');
}
$GP = array_merge($GP1, $GP2);

$r = new ClientServerResponse();
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
else {
	foreach ($GP as $command => $params) {
		try {
			$str = is_array($params) ? "'" . implode(",'", $params) . '"'
				: "'$params'";

			eval("\$res = PHPUnitInterfaceForGui::$command($str);");
			$r->add_value($res);
		}
		catch (Exception $e) {
			error_log("File: {$e->getFile()}, Line: {$e->getLine()}, ".
				"Message: {$e->getMessage()}, Code: {$e->getCode()}");
			$r->add_exception($e);
		}
		catch (ExceptionWithErrorlog $e) {
			error_log("File: {$e->getFile()}, Line: {$e->getLine()}, ".
				"Message: {$e->getMessage()}, Code: {$e->getCode()}");
			$r->add_exception(new Exception($e->getMessage(), $e->getCode()));
		}
		catch (ExceptionWithErrormail $e) {
			error_log("File: {$e->getFile()}, Line: {$e->getLine()}, ".
				"Message: {$e->getMessage()}, Code: {$e->getCode()}");
			$r->add_exception(new Exception($e->getMessage(), $e->getCode()));
		}
		catch (HackattackException $e) {
			error_log("File: {$e->getFile()}, Line: {$e->getLine()}, ".
				"Message: {$e->getMessage()}, Code: {$e->getCode()}");
			$r->add_exception($e);
		}
	}
}
echo $r;
?>