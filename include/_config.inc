<?
@include_once(dirname(__FILE__).'/../_private_config.inc');

define ('CONF_LANGUAGE_FILE_DIR',			dirname(__FILE__).'/../game/language');

// Registrierung
define ('CONF_MIN_NICK_LENGTH',				3);
define ('CONF_MAX_NICK_LENGTH',				13);
/**
 * Welche Zeichen nicht erlaubt sind.
 * ACHTUNG: Die Zeichen @, ' und " sind aus Sicherheitsgründen immer verboten!
 */
define ('CONF_FORBIDDEN_CHARS_PATTERN',		'[^a-zA-Z_0-9]');

define ('CONF_MAX_REGISTERED_USER_AMOUNT',	100000);

define ('CONF_SEND_BUG_REPORTS_TO',			"p.kemmeter@gausus.com");

if (!defined('CONF_PAYMENT_SERVER_MAIN_URL'))
	define ('CONF_PAYMENT_SERVER_MAIN_URL',		'http://localhost:8125/game');
define ('CONF_PAYMENT_SERVER_URL',	CONF_PAYMENT_SERVER_MAIN_URL.
													'/interface.php');
if (!defined('CONF_WCF_SERVER_URL'))
	define ('CONF_WCF_SERVER_URL', 'http://localhost:8124');


define('CONF_MAMMUN_AUTH_SECRET', '293489f41898d18a53a6cf49ab476b96');
?>