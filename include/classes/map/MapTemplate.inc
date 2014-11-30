<?php


require_once(dirname(__FILE__).'/../Db.inc');
require_once (dirname(__FILE__).'/../../shared/classes/exceptions/IllegalArgumentException.inc');

/**
 * Diese Klasse repräsentiert das Template einer Karte (also die
 * templatespezifischen Eigenschaften). Es entspricht somit einer
 * uninitialisierten Karte (im Gegensatz zur MapDescription, welche die
 * kartenspezifischen Eigenschaften enthält - z.B. wo gekämpft wird etc).
 *
 * @author Philipp Kemmeter
 */
class MapTemplate {
	const ERR_TEMPLATE_DOES_NOT_EXIST = 1;
	const ERR_DB_INCONSISTENT = 2;

	const HOME_OF_USER = 1;
	const HOME_OF_HOST = 2;

	const TYPE_SUITABLE_FOR_FFA = 1;
	const TYPE_SUITABLE_FOR_CLAN = 2;
	const TYPE_SUITABLE_FOR_TAG_TEAM = 4;
	const TYPE_SUITABLE_FOR_SP = 8;

	private /*int*/ $template_id;
	private /*int*/ $num_players;
	private /*int*/ $allow_host_move;
	private /*int*/ $type;
	private /*string*/ $name;
	private /*mixed[100]*/ $sector_data = array();
	private /*int[]*/ $homesectors = array();
	private /*int*/ $homesector_of_host = false;
	private /*int[]*/ $artifact_sectors = array();

	private static $INSTANCES = array();

	/**
	 *
	 * @param uint $template_id
	 * @throws IllegalArgumentException
	 * @return MapTemplate
	 */
	public static function get_instance($template_id) {
		$template_id_old = $template_id;
		$template_id = (int)$template_id;
		if ($template_id < 0)
			throw new IllegalArgumentException("template_id",
				'N0', $template_id_old);
		if (!isset(self::$INSTANCES[$template_id]))
			self::$INSTANCES[$template_id] = new MapTemplate($template_id);
		return self::$INSTANCES[$template_id];
	}

	/**
	 * @param int $template_id Die ID des Templates in der Datenbank
	 */
	private function __construct($template_id) {
		$res = query(
			"SELECT * FROM template_maps_1 WHERE template_id=$template_id"
		);
		if (mysql_num_rows($res) == 0) {
			error_log(print_r(debug_backtrace(false),true));
			throw new ExceptionWithErrorlog("", 0,
				"Template(template_maps_1) $template_id does not exist",
				self::ERR_TEMPLATE_DOES_NOT_EXIST
			);
		}

		$data = mysql_fetch_assoc($res);
		$this->template_id = $data['template_id'];
		$this->num_players = $data['num_players'];
		$this->allow_host_move = $data['allow_host_move'];
		$this->type = $data['type'];
		$this->name = $data['name'];
		$res = query (
			"SELECT * FROM template_maps WHERE template_id=$template_id"
		);
		$current_user_id = (User::get_current_user())
			? User::get_current_user()->get_id() : 0;
		//Konsistenzcheck (Sektoranzahl)
		if (mysql_num_rows($res) != 100)
			throw new ExceptionWithErrormail("Template inconsistent!",
				$current_user_id, 'All',
				"Template $this->template_id inconsistent!",
				"Check template_maps-Table! ".mysql_num_rows($res). " rows for".
				" Template $template_id found.");
		while ($row = mysql_fetch_assoc($res)) {
			$row['bio'] *= Mapsettings::get_instance()->get_timefactor();
			$row['artifact'] = (bool)$row['artifact'];
			$row['domination_point'] = (bool)$row['domination_point'];
			$sector_id = $row['sector_id'];
			if ($row['home'] == self::HOME_OF_USER)
				$this->homesectors[] = $sector_id;
			else if ($row['home'] == self::HOME_OF_HOST)
				$this->homesector_of_host = $sector_id;
			if ($row['artifact'])
				$this->artifact_sectors[] = $sector_id;

			unset($row['sector_id']);
				$this->sector_data[$sector_id] = $row;
		}
		//Konsistenzcheck (Homesectors)
		$all_hs = count($this->homesectors);
		if ($this->homesector_of_host)
			$all_hs++;
		if ($all_hs != $this->num_players)
			throw new ExceptionWithErrormail("Template inconsistent!",
				$current_user_id, 'All',
				"Template $this->template_id inconsistent!",
				"Check template_maps and template_maps_1 tables! ".
				$all_hs." Homesectors in total in template_maps found,".
				"but num_players={$this->num_players} in template_maps_1. ".
				"Those values must be equal.");

	}

	public function get_id() {
		return $this->template_id;
	}

	public function get_name() {
		return $this->name;
	}

	public function get_homesectors() {
		return $this->homesectors;
	}

	public function get_artifact_sectors() {
		return $this->artifact_sectors;
	}

	public function get_num_players() {
		return $this->num_players;
	}

	/**
	 * Gibt zurück, welcher Sektor der Heimatsektor des Hosts ist, oder false,
	 * falls er keinen hat.
	 *
	 * @return int/bool
	 */
	public function get_homesector_of_host() {
		return $this->homesector_of_host;
	}

	/**
	 * Gibt zurück, ob der Sektor ein Domination-Point ist.
	 *
	 * @param [0,1,...99] $sector_id
	 * @return bool
	 * @throws IllegalArgumentException
	 */
	public function get_sector_is_domination_point($sector_id) {
		if ($sector_id < 0 || $sector_id > 99)
			throw new IllegalArgumentException();
		return $this->sector_data[$sector_id]['domination_point'];
	}

	/**
	 * Gibt zurück, ob der Sektor ein Homesector ist.
	 *
	 * @param [0,1,...99] $sector_id
	 * @return bool
	 * @throws IllegalArgumentException
	 */
	public function get_sector_is_homesector($sector_id) {
		if ($sector_id < 0 || $sector_id > 99)
			throw new IllegalArgumentException();
		return (in_array($sector_id, $this->homesectors) ||
			($sector_id == $this->homesector_of_host));
	}
}

?>