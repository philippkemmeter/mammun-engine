<?php
require_once('Invitation.inc');

/**
 * Diese Klasse ist zuständig für das Sauberhalten der Datenbank. Dabei ist
 * vieles nicht nur performance-relevant, sondern essentiell wichtig für
 * saubere Übergänge vom User von z.B. "auf Karte" zu "nicht mehr auf Karte",
 * dass sich da nichts vermischt.
 *
 * Die DBCleanAndClear-Klasse hat Zugriff auf die gesamte DB.
 * TODO: Diesen Direktzugriff auf die DB über die entsprechenden Klassen kapseln
 */

class DBCleanAndClear {
	private $uid = 0;

	private static $INSTANCES = array();

	/**
	 * @return DBCleanAndClear
	 */
	public static function get_instance($uid) {
		if (!self::$INSTANCES[$uid])
			self::$INSTANCES[$uid] = new DBCleanAndClear($uid);
		return self::$INSTANCES[$uid];
	}

	private function __construct($uid) {
		$this->uid = $uid;

	}

	/**
	 * Dinge, die sowohl beim Mapstart, als auch beim Beenden gemacht werden
	 * müssen, sind hier ausgelagert.
	 */
	private function clear_mapstartendgeneral() {
		query("UPDATE science SET count='0', l_stamp='0', finish_time='3000000000' WHERE uid='" . $this->uid . '\'');
	}
	public function clear_mapstart($world_id, $map_id) {
		$this->clear_mapstartendgeneral();

		query (
			"UPDATE tribes SET add_stamp=0, add_value=0, count=5000 ".
				"WHERE uid={$this->uid} AND race=".SC_ID_SCIENTISTS
		);

		$mapstart = mysql_result(
			query(
				"SELECT started FROM maps ".
				"WHERE world_id=$world_id AND map_id=$map_id"
			), 0
		);
		if (MapPlayers::get_instance($world_id, $map_id, true)
			->get_handicapped($this->uid))
		{
			$mlev = World::get_instance($world_id)->get_maximum_level();
			Science::get_instance($this->uid)->clear_current_map(
				User::get_instance_by_id($this->uid)->get_cur_level() - $mlev
			);
		}
		else {
			Science::get_instance($this->uid)->clear_current_map();
		}
		User::get_instance_by_id($this->uid)->set_last_score_to_cur_score();


		$r = Resource::get_instance($this->uid);
		$r->set_user_resource(200, Resource::RESOURCE1, $mapstart);
		$r->set_user_resource(3500, Resource::RESOURCE2, $mapstart);
	}

	public function clear_mapend($world_id, $map_id) {
		$this->clear_mapstartendgeneral();
		Tribes::get_instance($this->uid)->clear_map_end();
		Invitation::get_instance($this->uid)->clear_spam_protect($world_id,
			$map_id);
	}
}

?>