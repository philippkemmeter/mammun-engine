<?

/**
 * Diese Datei bindet serverunabhängige Konstanten ein, welche in ziemlich
 * allen Dateien gebraucht werden
 */

require_once (dirname(__FILE__).'/shared/constants/CIBal.inc');
require_once (dirname(__FILE__).'/_config.inc');
require_once (dirname(__FILE__).'/consts_science.inc');
require_once (dirname(__FILE__).'/shared/classes/ValueChecker.inc');

/* alliance ablaufzeiten (AKTUELL???):
nur noch 3 spieler auf map: 12h
embassy destroyed:12h
krieg erklären:12h
normales kündigen:48h
mofu destroy:0h
nap ablaufzeiten:
nur noch 3 spieler auf map: 12h
embassy destroyed:12h
krieg erklären:12h
normales kündigen:48h
mofu destroy:0h
*/
//general functions
//general options:
$SIGNUP_OK=1;

/**
 * TODO Diese Konstanten sollten alle in Klassen gekapselt werden.
 *
 * Achtung: An den folgenden Konstanten hängen DB-Abfragen. Wenn sie verändert
 * werden müssen, dann müssen Updates an der DB gemacht werden.
 */
define ('BUILDING_TRIBE_ID',		1000000000); //das muss immer höher als die gesamtanzahl der tribes im system sein!
define ('MAX_USER_COUNT_IN_DB', 1000000);	// auf BIGINT umstellen, dann: 500.000.000.000.000.000
define ('ITEM_SCIENCE_ID', 10000); // muss immer höher als die maximale sc_id in science sein


//MUTEX
// TODO: Das in eine Mutex-Klasse kapseln
define ('MUTEX_MAP_COMPUTE_OFFSET',	MAX_USER_COUNT_IN_DB);
define ('MUTEX_USER_OFFSET', 0);
define ('MUTEX_BUG_PIC_UPLOAD', 1);
define ('MUTEX_MAP_CHOOSE', 2);
define ('MUTEX_WORLD_MUTEX', 3);
define ('MUTEX_TRIBE_MORPH', 4);
define ('MUTEX_BUILDING_CHANGE_COLOR', 5);
define ('MUTEX_BUILDING_CONSTRUCT', 6);
define ('MUTEX_LEAVE_MAP', 7);
define ('MUTEX_TRIBES_DB', 8);
//general constants:

define('SP_MAP', 2);	// TODO SP-Maps raushauen und die Konstante löschen

// TODO: Diese Flags in eine Klasse Kapseln und auch benutzen ;-)
//Flags: FLAG_i = 2^(i-1). Bei 32 bit haben wir 32 flags
define ('FLAG_1', 	1);
define ('FLAG_2', 	2);
define ('FLAG_3',	4);
define ('FLAG_4', 	8);
define ('FLAG_5', 	16);
define ('FLAG_6', 	32);
define ('FLAG_7', 	64);
define ('FLAG_8', 	128);
define ('FLAG_9', 	256);
define ('FLAG_10', 	512);
define ('FLAG_11', 	1024);
define ('FLAG_12', 	2048);
define ('FLAG_13', 	4096);
define ('FLAG_14', 	8192);
define ('FLAG_15', 	16384);
define ('FLAG_16', 	32768);
define ('FLAG_17', 	65536);
define ('FLAG_18',	131072);
define ('FLAG_19',	262144);
define ('FLAG_20',	524288);
define ('FLAG_21',	1048576);
define ('FLAG_22',	2097152);
define ('FLAG_23',	4194304);
define ('FLAG_24',	8388608);
define ('FLAG_25',	16777216);
define ('FLAG_26',	33554432);
define ('FLAG_27',	67108864);
define ('FLAG_28',	134217728);
define ('FLAG_29',	268435456);
define ('FLAG_30',	536870912);
define ('FLAG_31',	1073741824);
//define ('FLAG_32',	2147483648);	// geht nicht, weil in PHP ist ein INT immer signed :/
?>
