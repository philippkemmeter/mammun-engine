<?php
/**
 * Diese Klasse verwaltet die Ausgabe der Karteninformationen, welche für die
 * Anzeige benötigt werden.
 *
 * Alle Daten sind user-abhängig, so dass jeder User hier eine andere Ausgabe
 * erhält. Es ist auch möglich alle Daten in einer "allwissenden" Art zu
 * erhalten, also aus der kombinierten Sicht aller User auf der Karte. Hierfür
 * kann MapView::UID_ALL genutzt werden. Ebenso ist es möglich, die Sicht auf
 * die Karte zu erhalten als komplett außenstehender. Hierfür kann entweder eine
 * UID übergeben werden, welche nicht auf der Karte befindlich ist, oder die
 * Konstante MapView::UID_NONE.
 * Sensible Daten wie IDs werden immer verschlüsselt mithilfe des globalen
 * $cryptObj zurückgeliefert.
 *
 * Diese Klasse ist ein kompletter Rewrite der Mapprint-Klasse, und stellt NICHT
 * das Pendant zur Mapview-Klasse von PV dar. Das Pendant zur Mapview-Klasse von
 * PV ist die Klasse MapCompute und alle Unterklassen.
 *
 * @author Philipp Kemmeter
 */
require_once(dirname(__FILE__).'/../host/HostControl.inc');

class MapView {
	const UID_ALL = 0;				// UID-Code für "allwissend"
	const UID_NONE = HostControl::USER_HOST_ID;
									// UID-Code für "Nichtswissend"

	private $uid;
	private $map_id;
	private $world_id;

	private static $INSTANCES = array();

	/**
	 * Gibt die entsprechende Instanz zurück. Die Karte wird über $map_id
	 * spezifiziert, die Sicht über $uid. Wird $uid = MapView::UID_ALL benutzt,
	 * sieht man aus der Sich aller User auf der Karte, bei MapView::UID_NONE
	 * ist die Sicht eines außenstehenden gegeben.
	 * @param uint $map_id
	 * @param uint $uid
	 * @return MapView
	 */
	public static function get_instance($world_id, $map_id, $uid) {
		if ($world_id < 0) {
			throw new IllegalArgumentException("world_id", "N0", $world_id);
		}
		$world_id = (int)$world_id;
		if ($map_id < 0) {
			throw new IllegalArgumentException("map_id", "N0", $map_id);
		}
		$map_id = (int)$map_id;
		if ($uid < 0) {
			throw new IllegalArgumentException("uid", "N", $uid);
		}
		$uid = (int)$uid;
		if (!isset(self::$INSTANCES[$world_id][$map_id][$uid]))
			self::$INSTANCES[$world_id][$map_id][$uid] = new self($world_id,
				$map_id,$uid);
		return self::$INSTANCES[$world_id][$map_id][$uid];
	}

	/**
	 * Konstruktor private.
	 * @see MapView::get_instance
	 * @param uint $world_id
	 * @param uint $map_id
	 * @param uint $uid
	 * @return MapView
	 */
	private function __construct($world_id, $map_id, $uid) {
		$this->world_id = $world_id;
		$this->map_id = $map_id;
		$this->uid = $uid;
	}

	/**
	 * Gibt die Einheiten im Sektor zurück. Wird kein Sektor angegeben, wird
	 * ein Array aller 100 Sektoren mit deren Daten zurückgeliefert.
	 * Die Einheiten werden dabei pro Sektor wie folgt codiert:
	 *
	 * $result[USER][POSITION] = TribeData;
	 *
	 * @param [0,...,99] $sector_id
	 * @global $cryptObj [nur lesen]
	 * @return StdClass[][] / StdClass[100][][]
	 */
	public function get_units($sector_id = false) {
		global $cryptObj;
		if (!$cryptObj)
			$cryptObj = new PV_encryption();

		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_units($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}
		$world_id = $this->world_id;
		$map_id = $this->map_id;
		$uid = $this->uid;

		$mc = MapCompute::get_instance($world_id, $map_id);
		$users = MapPlayers::get_instance($world_id, $map_id)
			->get_users_on_map();
		$sec = Sector::get_instance(
			new WorldCoordinates($world_id, $map_id, $sector_id)
		);
		$attacker_pos = array();// Positionen der Angreifer. s.u.
		$defender = array();	// verteidigende Tribes im Sektor. s.u.

		$result = array();
		$users[] = User::get_host_user();
		foreach ($users as $user) {
			$tribes = $mc->get_tribes_in_sector($sector_id, $user->get_id());
			$my_tribe_array = array();
			for ($i = 0; $i < count($tribes); $i++) {
				// Getarnte, fremde Einheiten nicht anzeigen
				if (($tribes[$i]->get_uid() != $uid)
					&& $tribes[$i]->is_camouflaged())
				{
					continue;
				}
				$tribe_data = $tribes[$i]->to_php_std_class();

				// Die Information der UID ist rekursiv und kann gelöscht werden
				// (Zudem wollen wir nirgends DB-IDs stehen haben)
				unset($tribe_data->uid);
				// ID in jedem Fall verschlüsseln
				$tribe_data->id = $cryptObj->do_encryption(
					array('id0' => $tribe_data->id), false
				);
				/**
				 * DETAILINFORMATIONEN GGF. NICHT MITSCHICKEN?
				 */
				if ($tribes[$i]->get_uid() != $uid)
				{	// Wenn nicht meine Einheit, dann "geheime" Infos löschen
					unset($tribe_data->att);
					unset($tribe_data->def);
					unset($tribe_data->hp);
					unset($tribe_data->speed);
					unset($tribe_data->abilities);
					unset($tribe_data->path);
				}
				else {
					if ($tribes[$i]->is_camouflaged())
						$tribe_data->camouflaged = 1;
				}

				/**
				 * Jetzt werden die Einheiten an die entsprechende Seiten
				 * positioniert, abhängig davon, ob gekämpft wird, oder nicht.
				 */

				if (!$sec->get_fight()) {
					/**
					 * 1. Fall, es ist ruhig. Es sollen die a) Grenzen zu
					 * gegnerischem Gebiet bewacht werden, alle anderen sind
					 * unwichtig. b) Gibt es keine Grenze zum Gegner, dann wird
					 * zufällig verteilt.
					 */
					$neighbors = $sec->get_neighbor_dominator_ids();
					// Unten und oben tauschen, weil Karte andersrum aussieht
					$tmp = $neighbors[2];
					$neighbors[2] = $neighbors[3];
					$neighbors[3] = $tmp;
					$enemy_edges = array();
					for ($ni = 0; $ni < count($neighbors); $ni++)
						if (($neighbors[$ni] > 0) &&
							($neighbors[$ni] != $sec->get_dominator_id()))
							$enemy_edges[] = $ni;

					$e_i = 0;	// edge index

					// Aufteilen der Kisten auf alle Ecken
					if (!empty($enemy_edges)) {
						$min = INF;
						foreach ($enemy_edges as $ne) {
							if (!isset($my_tribe_array[$ne])
								|| !is_array($my_tribe_array[$ne]))
							{
								$c = 0;
							}
							else
								$c = count($my_tribe_array[$ne]);

							if ($c < $min) {
								$e_i = $ne^1;
								$min = $c;
							}
						}
					}
					// Gibt es keine, dann auf alle aufteilen
					else {
						$min = INF;
						for ($ni = 0; $ni < count($neighbors); $ni++) {
							if (!isset($my_tribe_array[$ni])
								|| !is_array($my_tribe_array[$ni]))
							{
								$c = 0;
							}
							else
								$c = count($my_tribe_array[$ni]);

							if ($c < $min) {
								$e_i = $ni;
								$min = $c;
							}
						}
					}
					$my_tribe_array[$e_i][] = $tribe_data;
				}
				else {
					/**
					 * Zweiter Fall: Kampf.
					 * a) Der Angreifer soll dort positioniert werden, woher er
					 * kam. Ist er von unten eingefallen, soll er auch unterhalb
					 * des Sektors angezeigt werden. Das entspricht genau seinem
					 * heading.
					 * b) Der Verteidiger soll dann andenselben Kanten
					 * positioniert werden. Dadurch kommt es zu einer
					 * Gegenüberstellung der Parteien.
					 * Dazu wird er hier nur zwischengespeichert. Die
					 * Verteidiger werden dann am Ende zugewiesen (nach der
					 * ganzen Schleife).
					 */

					// Attacker
					if ($sec->get_dominator_id() != $tribes[$i]->get_uid()) {
						$h = $tribes[$i]->get_heading(true);
						// Unten und oben tauschen, weil Karte andersrum aussieht
						if ($h > 1)
							$h = $h ^ 1;
						$my_tribe_array[$h][] = $tribe_data;

						// Die Position merken für Verteidigerpositionierungen
						if (!in_array($h, $attacker_pos))
							$attacker_pos[] = $h;
					}
					// Defender
					else {
						$defender[] = array(
							"nick" => $user->get_nick(),
							"data" =>$tribe_data
						);
					}

				}
			}
			if (!empty($my_tribe_array))
				$result[$user->get_nick()] = $my_tribe_array;
		}
		/**
		 * Jetzt können die Defender positioniert werden. Dazu werden zunächst
		 * alle Attackerpositionen bestimmt und die Defender auf diese
		 * gleichmäßig Verteilt.
		 */
		if (!empty($defender)) {
			$modul = count($attacker_pos);
			$n = count($defender);
			for($i = 0; $i < $n; $i++) {
				$j = ($modul == 0) ? 0 : $attacker_pos[$i%$modul];
				$result[$defender[$i]['nick']][$j][] = $defender[$i]['data'];
			}
		}
		return $result;
	}

	/**
	 * Gibt die sich bewegenden Einheiten im Sektor zurück. Wird kein Sektor
	 * angegeben, wird ein Array aller 100 Sektoren mit deren Daten
	 * zurückgeliefert.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return StdClass[] / StdClass[100][]
	 */
	public function get_moving_units($sector_id=false) {
		global $cryptObj;
		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_moving_units($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}

		$m = MapCompute::get_instance($this->world_id, $this->map_id);
		$moving_units = $m->get_moving_units($sector_id);
		$result = null;
		foreach ($moving_units as $tribe) {
			$nick = User::get_instance_by_id($tribe->get_uid())->get_nick();
			$tribe_data = $tribe->to_php_std_class();
			/**
			 * DETAILINFORMATIONEN GGF. NICHT MITSCHICKEN?
			 */
			if ($tribe->get_uid() != $this->uid)
			{	// Wenn nicht meine Einheit, dann "geheime" Infos löschen
				unset($tribe_data->att);
				unset($tribe_data->def);
				unset($tribe_data->hp);
				unset($tribe_data->speed);
				unset($tribe_data->abilities);
				unset($tribe_data->path);
			}
			$tribe_data->user_nick = $nick;
			// Die Information der UID ist rekursiv und kann gelöscht werden
			// (Zudem wollen wir nirgends DB-IDs stehen haben)
			unset($tribe_data->uid);
			unset($tribe_data->heading);
			// ID in jedem Fall verschlüsseln
			$tribe_data->id = $cryptObj->do_encryption(
				array('id0' => $tribe_data->id), false
			);
			$tribe_data->time_per_sector =
				($tribe->get_arrival_time() - $tribe->get_start_time())/
				(count($tribe->get_path())-1);

			$result[$tribe->get_heading(true)][] = $tribe_data;
		}
		return $result;
	}

	/**
	 * Gibt die Ressourcen im Sektor als Array zurück. Wird kein Sektor
	 * angegeben, wird ein Array aller 100 Sektoren mit deren Ressourcen
	 * zurückgegeben.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return ufloat[3] / ufloat[100][3]
	 */
	public function get_bio($sector_id=false) {
		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_bio($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}

		$resources = Sector::get_instance(
			new WorldCoordinates($this->world_id, $this->map_id, $sector_id)
		)->get_max_resources();

		return ($resources[0] || $resources[1]) ?
			$resources : null;
	}

	/**
	 * Gibt die Wirtsaktivität im Sektor zurück. Wird kein Sektor
	 * angegeben, wird ein Array aller 100 Sektoren mit deren Aktivität
	 * zurückgegeben.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return uint / uint[100]
	 */
	public function get_host_activity($sector_id=false) {
		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_host_activity($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}

		return Sector::get_instance(
			new WorldCoordinates($this->world_id, $this->map_id, $sector_id)
		)->get_host_activity();
	}

	/**
	 * Gibt zurück, ob der Sektor ein Domination Point ist. Wird kein Sektor
	 * angegeben, wird ein Array aller 100 Sektoren mit der Auskunft
	 * zurückgegeben.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return bool / bool[100]
	 */
	public function get_dom_point($sector_id=false) {
		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_dom_point($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}

		return Sector::get_instance(
			new WorldCoordinates($this->world_id, $this->map_id, $sector_id)
		)->is_domination_point();
	}

	/**
	 * Gibt zurück, wessen Heimatsektor der Sektor ist. Wird kein Sektor
	 * angegeben, wird ein Array aller 100 Sektoren mit der Auskunft
	 * zurückgegeben.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return uint / uint[100]
	 */
	public function get_home($sector_id=false) {
		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_home($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}

		return Sector::get_instance(
			new WorldCoordinates($this->world_id, $this->map_id, $sector_id)
		)->get_homesector_uid();
	}

	/**
	 * Gibt zurück, wer den Sektor dominiert. Wird kein Sektor
	 * angegeben, wird ein Array aller 100 Sektoren mit der Auskunft
	 * zurückgegeben.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return string / string[100]
	 */
	public function get_dominator($sector_id=false) {
		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_dominator($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}

		return Sector::get_instance(
			new WorldCoordinates($this->world_id, $this->map_id, $sector_id)
		)->get_dominator_nick();
	}

	/**
	 * Gibt zurück, ob im Sektor geklämpft wird. Wird kein Sektor
	 * angegeben, wird ein Array aller 100 Sektoren mit der Auskunft
	 * zurückgegeben.
	 *
	 * @param [0,1,..99] $sector_id
	 * @return bool / bool[100]
	 */
	public function get_fight($sector_id=false) {
		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_fight($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}

		return Sector::get_instance(
			new WorldCoordinates($this->world_id, $this->map_id, $sector_id)
		)->get_fight();
	}

	/**
	 * Gibt die Gebäude im Sektor zurück. Wird kein Sektor
	 * angegeben, wird ein Array aller 100 Sektoren mit der Auskunft
	 * zurückgegeben.
	 *
	 * @param [0,1,..99] $sector_id
	 * @global $cryptObj [nur lesen]
	 * @return mixed[][] / mixed[][][100]
	 */
	public function get_buildings($sector_id=false) {
		global $cryptObj;
		if (!$cryptObj)
			$cryptObj = new PV_encryption();

		if ($sector_id === false) {
			$result = array();
			for ($i = 0; $i < 100; $i++) {
				$tmp = $this->get_buildings($i);
				if ($tmp)
					$result[$i] = $tmp;
			}
			return $result;
		}
		$sector = Sector::get_instance(
			new WorldCoordinates($this->world_id, $this->map_id, $sector_id)
		);
		$buildings = $sector->get_visible_buildings();

		$sort = array();	// Wird zum Sortieren benutzt (s.u.)
		$b_array = array(); $j=0;

		foreach ($buildings as $building) {
			$b_array[$j] = new stdClass();
			$b_array[$j]->type = $building->get_type();
			$b_array[$j]->color = $building->get_color();
			$b_array[$j]->const_key = $building->get_const_key();
			$ulc = $building->get_ulc();
			$b_array[$j]->ulc = array($ulc->x, $ulc->y);
			if ($building->get_color() > 0) {
				$b_array[$j]->pic_url =
					str_replace('.png', $building->get_color().'.png',
						$building->get_pic()
					);
			}
			else {
				$b_array[$j]->pic_url = $building->get_pic();
			}
			$b_array[$j]->size = $building->get_size();
			$b_array[$j]->height = $building->get_height();
			if ($building->is_upgrading()) {
				$b_array[$j]->progress = $building->get_progress();
				$b_array[$j]->grow = $building->get_grow();
				$b_array[$j]->t_stamp = $building->get_t_stamp();
			}
			$b_array[$j]->neutral =
				(($building->get_type() >= IBuildings::NEUTRAL_BUILDINGS_BEGIN)
				&& ($building->get_type() <=  IBuildings::NEUTRAL_BUILDINGS_END));

			$b_array[$j]->custom =
				(($building->get_type() >= IBuildings::CUSTOM_BUILDINGS_BEGIN)
				&& ($building->get_type() <=  IBuildings::CUSTOM_BUILDINGS_END));

			$b_array[$j]->xp_level = $building->get_xp_level();

			$sort[$j] = $ulc->x.$ulc->y;	// Sortierindex sind die Koordinaten
			$j++;
		}
		if (!empty($b_array)) {
			/**
			 * Hier wird sortiert. Am Ende sind die Gebäude zuerst im
			 * Array, die bei der 2D-Projektion hinten sind. Die Gebäude
			 * sind also von hinten nach vorne sortiert.
			 */
			array_multisort($sort, SORT_ASC, $b_array);
			return $b_array;
		}
		else {
			return null;
		}
	}
}
?>