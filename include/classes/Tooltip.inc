<?


/**
 * 	Diese Klasse ist als Namespace zu verstehn. Sie ist nicht zum instanziieren gedacht :)  
 */
class Tooltip {
	/**
	 * Ersetzt in einem String die vorkommenden Tips (in $tip) durch einen entsprechenden Tooltiplink
	 */
	static public function parse($tip, $string) {
		if (!is_array($tip))
			return preg_replace('/([^a-zA-Z]|^)('.$tip.')([^a-zA-Z]|$)/', '\\1<a class=tooltip href="javascript: openTooltip(\''.Tooltip :: encode($tip).'\');">'.$tip.'</a>\\3', $string);
		else {
			$search_tip = array();
			$replace_tip = array();
			foreach ($tip as $value) {
				$string = preg_replace('/([^a-zA-Z]|^)('.$value.')([^a-zA-Z]|$)/', '\\1<a class=tooltip href="javascript: openTooltip(\''.Tooltip :: encode($value).'\');">'.$value.'</a>\\3', $string);
			
/*				$search_tip[] = '/([^a-zA-Z])('.$value.')([^a-zA-Z])/';
				$replace_tip[] = '\\1<a class=tooltip href="javascript: openTooltip(\''.Tooltip :: encode($value).'\');">'.$value.'</a>\\3';*/
			}
			//return preg_replace($search_tip, $replace_tip, $string);
			return $string;
		}
	}

	/**
	 * Hebt den übergebenen Tipp hervor
	 */
	static public function highlight($tip, $string) {
		if (!is_array($tip))
			return str_replace($tip, '<i>'.$tip.'</i>', $string);
		else {
			foreach ($tip as $value) {
				$replace_tip[] = '<i>'.$value.'</i>';
			}
			return str_replace($tip, $replace_tip, $string);
		}
	}

	static public function encode($tip) {
		return str_replace(array('ä', 'Ä', '&auml;', '&Auml', 
								 'ö', 'Ö', '&ouml;', '&Ouml',
								 'ü', 'Ü', '&uuml;', '&Uuml;',
								 ' '), 
						   array('ae', 'Ae', 'ae', 'Ae',
						   		 'oe', 'Oe', 'oe', 'Oe',
						   		 'ue', 'Ue', 'ue', 'Ue',
						   		 '_'), $tip);
	}

	static public function decode($tip) {
		return str_replace(array('ae', 'Ae', 'oe', 'Oe', 'ue', 'Ue', '_'),
			array('&auml;', '&Auml', '&ouml;', '&Ouml', '&uuml;', '&Uuml', ' '), $tip);
	}

	/**
	 * Gibt ein Array zurück, das alle Strings enthält, welche durch einen entsprechenden Link
	 * zum tooltip des Strings ersetzt werden sollen.
	 * Die Funktion liest hierbei alle gesetzten Konstanten mittels get_defined_constats() ein
	 * und schreibt diese ohne das LNG_ und über Tooltip::decode formatiert ins Array.
	 * Auf diese Weise muss man nur noch Konstanten definieren, die den gleichen Namen haben,
	 * wie das zu ersetzende. Beispiel:
	 * LNG_TESTTEXT würde testtext ersetzen.
	 */
	static public function get_all_tips() {
		require_once ('../include/languages.inc');
		include_language_file('tooltip');

		$vers = phpversion();
		if ($vers{0} == 5) { // ab PHP5 kann man diese performantere Version nutzen 
			$consts = get_defined_constants(true);
			foreach ($consts['user'] as $const => $temp)
				if ($const{0} == 'T' && $const{1} == 'I' && $const{2} == 'P')
					$all_tips[] = Tooltip::decode(substr($const, 4));
		}
		else { // bei PHP 4 muss man jede Konstante noch von Hand testen
			$consts = get_defined_constants();
			foreach ($consts as $const => $temp)
				if ($const{0} == 'T' && $const{1} == 'I' && $const{2} == 'P')
					$all_tips[] = Tooltip::decode(substr($const, 4));
		}
		return $all_tips;
	}
}


?>