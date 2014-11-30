<?php

require_once(dirname(__FILE__).'/MapChooserFFA.inc');

abstract class MapChooserAuthKey extends MapChooserFFA
{
	protected $auth_key;

	/**
	 * Gibt die Daten zurück, die dem entsprechenden Auth-Key zugeordnet sind.
	 *
	 * @param string $auth_key
	 * @return stdClass[]
	 */
	//abstract public static function get_data_by_auth_key($auth_key);

	/**
	 * Gibt den Authentifizierungsschlüssel zurück, den der angebene User für
	 * die Karte, auf der er gerade spielt, zugewiesen bekommen hat.
	 *
	 * Mit diesem Schlüssel kann er weitere Spieler [in sein Team] auf der Karte
	 * einladen.
	 *
	 * @param {1, 2, ...} $uid
	 * @return string
	 */
	//abstract public static function get_auth_key_for_current_map($uid);

	/**
	 * Versucht den Spieler auf die Karte zu setzen, die sein Auth-Key angibt.
	 *
	 * Je nach Auth-Key bewirbt sich der Spieler als Tag-Team-Partner eines
	 * Spielers. Er wurde also eingeladen. Dieser Tag-Team-Platz ist jedoch
	 * nicht sicher, da sich ggf. mehrere Leute auf den Platz beworben haben.
	 * In dem Fall wird die Exception "self::ERR_MAP_ALREADY_FULL" geworfen.
	 *
 	 * @param string $auth_key	Die Hälfte der Heimatsektoren sind reserviert.
	 * 							Der $auth_key entscheidet, für welchen dieser
	 * 							Sektoren er sich bewirbt (könnte schon belegt
	 * 							sein)
	 * @return uint				Die komplette Sektor-ID des Heimatsektors, der
	 * 							dem User zugewiesen wurde.
	 */
	abstract public function choose_map_by_auth_key($auth_key);

	/**
	 * Hilfsfunktion zum Erstellen eines gültigen, möglichst kollisionsfreien
	 * Auth-Keys auf möglichst kurzer Zeichenzahl.
	 *
	 * @return char[6]
	 */
	protected static function create_new_auth_key() {
		$chars =
			'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_';
	    $chars_length = strlen($chars);

	    $string = '';

	    // Generate random string
	    for ($i = 0; $i < 6; $i++)
	        // Grab a random character from our list
	        $string .= $chars{rand(0, $chars_length-1)};

	    // Return the string
	    return $string;
	}
}
?>