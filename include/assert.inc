<?php
/*
 * Created on Jun 12, 2007 by Philipp Kemmeter
 */
define ('EMAIL_ASSERT_ACTIVE',		1);

if (!defined('ROADSEND_PHP')) {
	assert_options(ASSERT_ACTIVE, 1);			// assertion ist aktiviert
	assert_options(ASSERT_CALLBACK, 'assert_callback');	// assert_callback wird aufgerufen, wenn assertion fehlschlägt
}

function assert_callback($script, $line, $message) {
	echo "<b>PV Assertion Error</b> in script <b>$script</b>, line <b>$line</b>:<br>\n";
	echo ereg_replace('^.*//\*', '', $message)."<br><br>\n";
	echo "This is a bug. Please copy the message above and report it at our board.<br>\n";
	echo "--------\n<br>";
	echo "Dies ist ein Bug. Bitte kopiere die obige Nachricht und poste sie in unserem Forum.<br>\n";
	ob_flush();
	flush();
	die();
}

function pv_assert($bool, $error_string) {
	$b = (bool)$bool;	
	assert("$b; //*$error_string");
}

function pv_email_assert($bool, $error_msg, $recipient, $topic) {
	if (EMAIL_ASSERT_ACTIVE && !$bool) {
		errormail($error_msg, $recipient, $topic);
	}
}
?>
