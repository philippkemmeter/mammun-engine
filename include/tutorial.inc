<?
require_once(dirname(__FILE__).'/languages.inc');
if (!$language)
	$language = get_language();
include_language_file('tutorial');

if (isset ($tut_page)) {
	include_once ('templates/header.tmpl.inc'); //macht das sinn???
	tutorial($tut_page, $tut_num);
}
function tutorial($page, $num = -1) {
	global $userdata;
	/*  $npc_name['hello'] 		= 'Mo';
	$npc_name['science']		= 'Mu';
	$npc_name['buildings']	= 'Ma';
	$npc_name['creaturemap']	= 'Ki';
	$npc_name['viral_movement']	= 'Ki';
	$npc_name['trade']		= 'Mo';*/ //hab sie alle rausgenommen weil kein bock sie überall einzubauen ^^
	switch ($page) {
		case 'hello' :
			$off_num = 3;
			$str = LNG_HELLO_0;
			break;
		case 'tribes' :
			$off_num = 3;
			$str = @constant('LNG_TRIBES_' . $num);
			break;
		case 'science' :
			$off_num = 4;
			$str = @constant('LNG_SCIENCE_' . $num);
			break;
		case 'buildings' :
			$off_num = 5;
			$str  = LNG_GREAT_VIRAL_LEADER;
			$str .= constant('LNG_BUILDINGS_' . $num);
			break;
		case 'viral_movement' :
			$off_num = 1;
			$str = @constant('LNG_VIRAL_MOVEMENT_' . $num);
			break;
		default :
			$str = LNG_TRIBES_DEFAULT;
	}
	if ($page == 'science')
		officer($off_num, $str);
	return $str;
}
?>
