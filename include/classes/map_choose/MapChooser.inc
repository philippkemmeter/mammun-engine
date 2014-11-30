<?php
require_once(dirname(__FILE__).'/../../shared/classes/IMapChooser.inc');
require_once(dirname(__FILE__).'/../host/HostControl.inc');

/**
 * Abstrakte Klasse, die die Schnittstellen für MapChooser-Klassen, wie
 * MapChooserFFA, bereitstellt.
 */

abstract class MapChooser implements IMapChooser {
	protected $uid;	// Die User-ID
	protected $world_id = null;
	protected $available_maps = array();
	protected $chosen_map_index = -1;
	protected $wished_color = 0;

	/**
	 * Achtung: Das ist hier nicht ohne Grund private. Diese Positionen sind
	 * sehr bedacht gewählt. Es hängen ein paar Entscheidungen von der
	 * Baumpositionswahl ab, z.B. wie viele Bäume in einem Heimatsektor
	 * maximal gesetzt werden können.
	 */
	private static $tree_positions =
			array (
				array(2,2),
				array(5,6),
				array(5,1),
				array(4,4),
				array(2,5),
				array(1,4)
			);


	//abstract static public function get_instance($uid, $world_id=0);

	abstract public function choose_map($map_index);
	abstract public function unchoose_map();

	public function change_world($world_id) {
		$this->world_id = $world_id;
	}


	protected function reset_chosen_map_index() {
		$this->chosen_map_index = -1;
	}

	/**
	 * Fügt der durch den übergebenen Index definierten Karte die Artefakte
	 * hinzu, in Abhängigkeit vom Durchschnittslevel der Spieler.
	 *
	 * @param int $map_index
	 * @param float $average_player_level
	 */
	protected function insert_artifacts($map_index, $average_player_level) {
		return 0;	// TODO

		$BEGINNER_ARTES = array (
			SC_ID_SILVER_PARTICLE,
			SC_ID_GOLD_PARTICLE,
			SC_ID_CAT_DNA,
			SC_ID_ERBIUM_PARTICLE,
			SC_ID_SAEBORGIUM_PARTICLE
		);
		$world_id = $this->available_maps[$map_index]['world_id'];
		$map_id = $this->available_maps[$map_index]['map_id'];
		$sql =
			'SELECT t.difficulty FROM template_maps_1 AS t, maps AS m '.
			'WHERE t.template_id=m.template_id '.
				"AND m.world_id=$world_id AND m.map_id=$map_id";
		$difficulty = mysql_result(query($sql), 0);
		if ($difficulty == 2)
			$skillmap = 1;
		if (!$average_player_level)
			$firstmap = 1;
		$average_player_level += 30;
		//  $average_player_level *= 1.3;
		if ($skillmap)
			$sql =
				'SELECT sc_id,grow FROM sciences '.
				'WHERE sc_id>'.SC_ID_PARTICLE_MIN.' AND sc_id<'.SC_ID_PARTICLE_MAX.' '.
				'ORDER BY grow';
		else
			$sql =
				'SELECT sc_id,grow FROM sciences '.
				'WHERE (sc_id>'.SC_ID_PARTICLE_MIN.' AND sc_id<'.SC_ID_PARTICLE_MAX.') '.
				'	OR (sc_id>'.SC_ID_DNA_MIN.' AND sc_id<'.SC_ID_DNA_MAX.') '.
				'ORDER BY grow';
		$result = query($sql);
		$middle_pos = 0;
		$counter = 0;
		$artifacts = array();
		while ($row = mysql_fetch_assoc($result)) {
			if (($row['grow'] > $average_player_level) && (!$middle_pos))
				$middle_pos = $counter;
			$artifacts[] = $row['sc_id'];
			$counter++;
		}
		if ($middle_pos < 10) {
			$start_pos = 0;
			$end_pos = 20;
		}
		else if ($middle_pos > count($artifacts) - 11) {
			$start_pos = count($artifacts) - 21;
			$end_pos = count($artifacts) - 1;
		}
		else {
			$start_pos = $middle_pos -10;
			$end_pos = $middle_pos +10;
		}

		// TODO: This is old stuff!
		$sql =
			'SELECT sector_id, world_id, map_id, artifact FROM sectors '.
			"WHERE artifact=1 AND world_id=$world_id AND map_id=$map_id";
		$result = query($sql);
		$beginner_counter = 0;
		while ($row = mysql_fetch_assoc($result)) {
			if ($skillmap)
				$artifact = $artifacts[$middle_pos];
			else if (!$firstmap)
				$artifact = $artifacts[rand($start_pos, $end_pos)];
			else {
				$artifact = $BEGINNER_ARTES[$beginner_counter];
				if ($beginner_counter < count($BEGINNER_ARTES) - 1)
					$beginner_counter++;
			}

			query(
				"UPDATE sectors set artifact=$artifact ".
				"WHERE sector_id={$row['sector_id']} ".
					"AND world_id=$world_id AND map_id=$map_id"
			);
		}
	}

	/**
	 * Diese Funktion wird von insert_trees aufgerufen und setzt so viele Bäume,
	 * wie $amount angibt.
	 *
	 * @param uint $map_index
	 * @param uint $sector_id
	 * @param uint $amount
	 */
	protected function insert_trees_for_sector($map_index, $sector_id, $amount)
	{
		if ($amount <= 0)
			return;

		$map = $this->available_maps[$map_index];
		$world_id = $map['world_id'];
		$map_id = $map['map_id'];
		$template_id = $map['template_id'];

		/**
		 * Damit die Bäume nicht alle am selben Platz gesetzt werden, aber im
		 * Verhältnis zueinander immer gleich stehen (randomisiertes Aussehen
		 * bei gleicher Funktion), werden hier mehrere Fälle unterschieden und
		 * einer davon zufällig ausgewählt.
		 */
		$random_case = rand(0,3);
		switch ($random_case) {
			/**
			 * Fall 0: Alles ist wie gehabt, Bäume werden wie spezifiziert
			 * gesetzt
			 */
			case 0:
				$tree_positions = self::$tree_positions;
				break;
			/**
			 * Fall 1: x- und y-Koordinaten werden vertauscht
			 * => Spiegelung an der Diagonalen
			 */
			case 1:
				$tree_positions = array();
				$n = count(self::$tree_positions);
				for ($i = 0; $i < $n; $i++) {
					$tree_positions[$i][0] = self::$tree_positions[$i][1];
					$tree_positions[$i][1] = self::$tree_positions[$i][0];
				}
				break;
			/**
			 * Fall 2:
			 * => Spiegelung an der y-Achse
			 */
			case 2:
				$tree_positions = array();
				$n = count(self::$tree_positions);
				for ($i = 0; $i < $n; $i++) {
					$tree_positions[$i][0] = 7-self::$tree_positions[$i][0];
					$tree_positions[$i][1] = self::$tree_positions[$i][1];
				}
				break;
			/**
			 * Fall 3:
			 * => Spiegelung an der x-Achse
			 */
			default:
				$tree_positions = array();
				$n = count(self::$tree_positions);
				for ($i = 0; $i < $n; $i++) {
					$tree_positions[$i][0] = self::$tree_positions[$i][0];
					$tree_positions[$i][1] = 7-self::$tree_positions[$i][1];
				}
				break;

		}


		$sql_building =
			'INSERT INTO building (uid, time_built,finish_time, world_id, '.
				'map_id, sector_id, time_build_started, type,cur_level, ulc_x,'.
				'ulc_y) '.
			'VALUES ';
		$first = true;
		$sector_c = new WorldCoordinates($world_id, $map_id, $sector_id);
		$sector_area = Buildings::get_area_of_sector($sector_c);
		for ($i = 0; $i < $amount; $i++) {
			/**
			 * Es können sich bereits Gebäude in dem Sektor befinden. Somit kann
			 * es passieren, dass der Baum genau dort gesetzt werden soll, wo
			 * sich ein Gebäude befindet. Das geht natürlich nicht. In dem
			 * Fall wird der Baum einfach nicht gesetzt. Eine Alternative wäre,
			 * den Baum woanders zu setzen, jedoch ist das komplizierter und
			 * ggf. auch gar nicht gewünscht, denn wenn hier schon ein Gebäude
			 * steht, dann ist durch das Gebäude ja bereits Bauland vernichtet
			 * worden. Also: Das kann man dann hier nochmal prüfen und anpassen,
			 * wenn man es später komplexer haben möchte :)
			 */
			if ($sector_area[$tree_positions[$i][0]][$tree_positions[$i][1]] > 0)
				continue;

			if (!$first)
				$sql_building .= ',';
			else
				$first = false;

			$sql_building .=
				"(".HostControl::USER_HOST_ID.", ".
				Time::get_t_now().",".
				Time::get_t_now().",".
				"$world_id,".
				"$map_id,".
				"$sector_id,".
				Time::get_t_now().",".
				Buildings::NORMAL_TREE.",".
				"1, ".
				"{$tree_positions[$i][0]}, ".
				"{$tree_positions[$i][1]})";
		}
		if (!$first)
			query($sql_building);
	}

	/**
	 * Diese Funktion setzt Bäume auf die Insel, um Bauland zu begrenzen und
	 * eine schönere Optik zu gewährleisten. Bäume sind technisch Gebäude, die
	 * dem Dunkel gehören. Durch die Engine-Regel, dass Gebäude des Dunkel
	 * niemals übernommen oder zerstört werden können, noch überhaupt an Kämpfen
	 * teilnehmen, macht diese Regelung Sinn.
	 */
	protected function insert_trees($map_index) {
		$map = $this->available_maps[$map_index];
		if (!$map || !$map['template_id'] || !$map['world_id'])
			throw new Exception("", self::ERR_MAP_DOES_NOT_EXIST);
		$template_id = $map['template_id'];

		$res = query(
			"SELECT sector_id, bio, home FROM template_maps ".
			"WHERE template_id={$template_id} AND bio>0"
		);

		$sectors = null;
		$max_bio = 0;
		$bio_sum = 0;
		$sector_amount = 0;
		while ($row = mysql_fetch_assoc($res)) {
			$sectors[$row['sector_id']] = array($row['bio'], $row['home']);
			if ($row['bio'] > $max_bio)
				$max_bio = $row['bio'];
			$bio_sum += $row['bio'];
			$sector_amount++;
		}
		$bio_avrg = $bio_sum/$sector_amount;
		foreach ($sectors as $sector_id => $data) {
			$bio = $data[0];
			$is_home = $data[1];
			$tree_amount = round(($bio/$bio_avrg)+1);

			/**
			 * In den Homesector müssen noch die zwei Hauptgebäude, die passen
			 * nur rein, wenn es höchsten vier Bäume gibt nach dem hiesigen
			 * Baumpositionen. Ändert man die Positionen, gibt's an der Stelle
			 * hier ggf. Probleme!
			 */
			if ($is_home && $tree_amount > 3) {
				$tree_amount = 3;
			}
			$this->insert_trees_for_sector($map_index,$sector_id,$tree_amount);
		}
	}


	/**
	 * Gibt die Karten zurück, die in der aktuell gewählten Welt verfügbar sind.
	 *
	 * @return assoc[]
	 */
	public function get_available_maps() {
		return $this->available_maps;
	}

	/**
	 * Gibt die Karteninformation der gewählten Karte zurück
	 *
	 * @return assoc[]
	 */
	public function get_chosen_map() {
		return ($this->chosen_map_index < 0)
			? null : $this->available_maps[$this->chosen_map_index];
	}

	/**
	 * Gibt den Index der aktuell gewählten Karte zurück.
	 *
	 * @return uint
	 */
	public function get_chosen_map_index() {
		return $this->chosen_map_index;
	}

	/**
	 * Gibt die aktuell gewählte Welt zurück
	 *
	 * @return uint
	 */
	public function get_world_id() {
		return $this->world_id;
	}
}

?>