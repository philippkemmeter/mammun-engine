<?php

require_once(dirname(__FILE__).'/../../shared/classes/Tribe.inc');
/**
 * Repräsentiert einen Tribe vom Host. Im Gegensatz zum Tribe eines Spielers,
 * werden hier die meisten Operationen direkt ausgeführt und sind nicht an
 * Zahlungen oder "Zeitabwarten" geknüpft. Ein Morphen fällt z.B. ganz aus.
 * Zudem sind hier die Verwaltungsoptionen aller Tribes als statische Funktionen
 * hinzugefügt - anders als im Modell der Tribes-Klasse mit der nested
 * TribesInMF-Klasse.
 * Ein weiterer Unterschied ist, dass HostTribes alle von derselben Rasse sind,
 * und dass jeder HostTribe ohne Einschränkung alle Fähigkeiten haben kann.
 */
class HostTribe extends AbstractTribe {
	const STRENGTH_MULTIPLIER = 0.9;	// hier kann zentral gebalancet werden
	/**
	 * Hosttribes sind alle von gleicher Rasse. Über den Typ kann diese jedoch
	 * in den Attributen unterschieden werden (ähnlich Attacker vs. Defender)
	 */
	/** Attackwerte stärker ausgeprägt */
	const TRIBE_TYPE_AGRESSIVE = 1;
	/** Defensivwerte stärker ausgeprägt */
	const TRIBE_TYPE_DEFENSIVE = 2;
	/** Att und Def ca. gleichwertig */
	const TRIBE_TYPE_EQUAL = 3;

	private static $INSTANCES = array();

	/**
	 * Gibt die Instanz des Host-Tribes mit der angegebenen ID zurück.
	 *
	 * @param int $id
	 * @return HostTribe
	 */
	public static function get_instance($id) {
		(int)$id = $id;
		if (!self::$INSTANCES[$id]) {
			self::$INSTANCES[$id] = new self($id);
		}
		return self::$INSTANCES[$id];
	}

	/**
	 * Erzeugt ein neues HostTribe-Objekt.
	 *
	 * @param assoc/int $data	DB-Zeile oder die ID der Zeile
	 * @return HostTribe
	 */
	protected function __construct($data) {
		if (!is_array($data)) {
			$id = (int)$data;
			$res = query (
				'SELECT * FROM tribes WHERE id='.$id
			);
			$data = mysql_fetch_assoc($res);
		}
		$this->init_from_db($data);
	}

	/**
	 * Gibt die IDs der Tribes zurück, die dem Host mit der übergebenen ID
	 * gehören.
	 *
	 * @param int $host_id	ID des Hosts
	 * @return int[]		Tribe-IDs
	 */
	public static function get_tribe_ids_of_host($host_id) {
		$result = array();
		$res = query ("SELECT * FROM tribes WHERE uid=$host_id");
		while ($tribe = mysql_fetch_assoc($res)) {
			$tribe['tribe_id'] = $tribe['id'];

			if (!isset(self::$INSTANCES[$tribe['tribe_id']]))
				self::$INSTANCES[$tribe['tribe_id']] = new HostTribe($tribe);
			$result[] = $tribe['tribe_id'];
		}

		return $result;
	}
	/**
	 * Fügt einen Tribe für den Host mit der übergebenen ID hinzu. Da ein
	 * HostTribe jede Fähigkeitenkombination tragen kann, werden bei den
	 * Fähigkeiten keine Checks gemacht. Von den übergebenen Fähigkeiten wird
	 * jeweils nur eine zufällig gewählt und dem Tribe gegeben.
	 *
	 * @param int $host_id			ID des Hosts, dem der Tribe gehören soll
	 * @param string $name			Der Name, den der Tribe haben soll
	 * @param int $type				Der Typ: einer von self::TRIBE_TYPE_...
	 * @param int $strength			Die Stärke des Tribes. Daraus werden anhand
	 * 								des Typs die Attribute errechnet - zufällig.
	 * @param int[] $general_abils  Science-IDs der Grundfähigkeiten aus denen
	 * 								eine zufällig ausgewählt wird
	 * @param int[] $other_abils	Science-IDs anderer Fähigkeiten (z.B.
	 * 								special, agressive, defensive) aus denen
	 * 								eine zufällig ausgewählt wird
	 * @return int 	Die ID des neuen Tribes
	 * @throws ExceptionWithErrormail
	 */
	public static function add_tribe($host_id, $name, $type, $strength,
		$general_abils=array(), $other_abils=array())
	{

		$host_id = HostControl::USER_HOST_ID;

		$techtree = Techtree::get_instance($host_id);
		//hp, att, def:
		$strength *= self::STRENGTH_MULTIPLIER;
		$min = $strength * 5;
		$max = $strength * 6;
		$dice = rand($min, $max);
		$hp = $dice * 7;
		if ($type == self::TRIBE_TYPE_AGRESSIVE) {
			$att = $dice + $strength;
			$def = $dice - $strength;
		}
		elseif ($type == self::TRIBE_TYPE_DEFENSIVE) {
			$att = $dice + $strength;
			$def = $dice - $strength;
		}
		else
			$hp = $hp * 0.9;
		//abilities:

		$abilities = array();
		if (!empty($other_abils)) {
			$ability_0 = $other_abils[rand(0, count($other_abils) - 1)];
			$max_lev = Techtree::get_science_max_level($ability_0);
			if ($max_lev > 1)
				$max_lev--;
			$values = $techtree->get_science($ability_0, $max_lev);
			$att += $values['attack'];
			$def += $values['defend'];
			$hp += $values['hitpoints'];
			$abilities[] = $ability_0;
		}
		else
			$ability_0 = 0;

		if (!empty($general_abils)) {
			$ability_1 = $general_abils[rand(0, count($general_abils) - 1)];
			$max_lev = Techtree::get_science_max_level($ability_0);
			if ($max_lev > 1)
				$max_lev--;
			$values = $techtree->get_science($ability_0, $max_lev);
			$att += $values['attack'];
			$def += $values['defend'];
			$hp += $values['hitpoints'];
			$abilities[] = $ability_1;
		}
		else
			$ability_1 = 0;
		//checken und einfügen
		if ($att < 10)
			$att = 10;
		if ($def < 10)
			$def = 10;
		if ($hp < 120)
			$hp = 120;
		$data = array();
		$data['hp'] = $hp;
		$data['att'] = $att;
		$data['def'] = $def;
		$data['name'] = $name;
		$data['pic_url'] = 'def_cell.gif';
		$data['speed'] = 50;
		$data['race'] = SC_ID_DARK_UNIT;
		$data['cost'] = 7;
		$data['ability_0'] = $ability_0;
		$data['ability_1'] = $ability_1;
		$data['uid'] = $host_id;
		query (
			"INSERT INTO tribes SET pic_url='{$data['pic_url']}', ".
				"name='{$data['name']}', uid={$data['uid']}, ".
				"speed={$data['speed']}, race={$data['race']}, ".
				"cost={$data['cost']}, hp={$data['hp']}, att={$data['att']}, ".
				"def={$data['def']}"
		);
		$data['id'] = mysql_result(query(
			"SELECT id FROM tribes WHERE uid={$data['uid']} ".
				"AND name='{$data['name']}'"
		),0);

		foreach ($abilities as $abil) {
			query (
				"INSERT IGNORE INTO tribes_abilities ".
					"SET tribe_id={$data['id']}, ability_id={$abil}"
			);
		}

		self::$INSTANCES[$data['id']] = new HostTribe($data);
		return $data['id'];
	}
}
?>