<?


require_once(dirname(__FILE__).'/map/Sector.inc');
require_once(dirname(__FILE__).'/map/MapCompute/MapCompute.inc');
require_once(dirname(__FILE__).'/host/HostControl.inc');

/**
 * Verwaltet die Rohstoffe eines Spielers.
 * In jedem Sektor wird bei dominanz eine Ressource namens Bio abgebaut. Diese
 * ist nur intern und kommt dem Spieler nie zu Gesicht. Das Bio wird direkt in
 * Resourcen umgewandelt, so dass die verschiedenen Resourcen, die aus Spielersicht
 * in einem Sektor zu sammeln sind, gänzlich voneinander abhängig sind.
 */


class Resource {	//TODO Viele Funktionen funktionieren nur bei aktuellem User!
					//Sollte weiter ausgebaut werden (Sessionverwaltung hier)
	private $resources = array();
	private $uid;
	private $user_resource = array();

	const ERR_NOT_ENOUGH_RESOURCES　= 1;


	const NUM_RESOURCES = 2;
	const RESOURCE1 = 0;
	const RESOURCE2 = 1;

	const RESOURCE1_RATE = 0.02;// Soviel Anteil des Bio wird direkt in Resource1 umgewandelt
	const RESOURCE2_RATE = 0.2;	// Soviel Anteil des Bio wird direkt in Resource2 umgewandelt

	const RESOURCE1_NORMAL_GAIN = 1;	// Soviel Resource1 wird ohne Zusatzgebäude
										// immer gewonnen (falls Sektormaximum nicht überschritten)
	const RESOURCE2_NORMAL_GAIN = 15;	// Soviel Resource2 wird ohne Zusatzgebäude
										// immer gewonnen (falls Sektormaximum nicht überschritten)

	static private $INSTANCE = array();	// instanzüberwachung. Nur eine Instanz pro UID innerhalb von PV
 										// Konstruktor ist private. Objekt statisch über get_instance($uid)
 										// anfordern! (s.u.)

 	/**
	 * Diese Statische Funtion als Konstruktor benutzen !
	 *
	 * @param uint $uid
	 * @return Resource
	 */
 	public static function get_instance($uid) {
 		if (!isset(self::$INSTANCE[$uid])) {
 			self::$INSTANCE[$uid] = new Resource($uid);
 		}
 		return self::$INSTANCE[$uid];
 	}

 	private function __construct($uid) {
 		$this->uid = $uid;
 	}

 	private function init_resources() {
 		/**
 		 * Der Host soll nix haben an Ressourcen. In der DB wird sich auch
 		 * kein Eintrag finden.
 		 */
 		if ($this->uid == HostControl::USER_HOST_ID) {
 			$this->resources = array(0,0,0);
	 		$this->user_resource['resource0'] = 0;
	 		$this->user_resource['resource0_tstamp'] = 0;
	 		$this->user_resource['resource1'] = 0;
	 		$this->user_resource['resource1_tstamp'] = 0;
	 		$this->user_resource['resource2'] = 0;
	 		$this->user_resource['resource2_tstamp'] = 0;
	 		return;
 		}
 		$user = User::get_instance_by_id($this->uid);

 		$world_id = $user->get_world_id();
 		$map_id = $user->get_map_id();
		$res = query(
			"SELECT * FROM resource WHERE uid={$this->uid}"
		);
		if (mysql_num_rows($res) == 0) {
			query(
				"INSERT IGNORE INTO resource SET uid={$this->uid}"
			);
		}
		$this->user_resource = mysql_fetch_assoc($res);

		/**
		 * Es wird versucht die Karteneinnahmen mitzuberücksichtigen. Das
		 * schlägt i.d.R. dann fehl, wenn die Karte nicht läuft.
		 */
		try {
			$map_compute = MapCompute::get_instance($world_id, $map_id);
			for ($r = 0; $r < self::NUM_RESOURCES; $r++) {
				$map_age = (Time::get_t_now()
					- $map_compute->get_last_compute_time());
				if ($map_age > 0)
				{//map is schon etwas älter
					$resource = $this->get_resource_gain($r)*$map_age/3600;
				}
				else {
					$resource = 0;
				}

				for ($sector = 0; $sector < 100; $sector++) {
					$resource += $map_compute->get_sector_resource($sector, $r,
						$this->uid);
				}
				$resource += $this->get_user_resource($r);
				$this->resources[$r] = round($this->check_resource_capacity(
					$resource, $r));
			}
		}
		/**
		 * Bei Fehlschlag nur die User-Ressourcen anzeigen.
		 */
		catch (Exception $e) {
			for ($r = 0; $r < self::NUM_RESOURCES; $r++) {
				$this->resources[$r] = $this->get_user_resource($r);
			}
		}
 	}

 	/**
 	 * Liefert die gewählte Ressource aus dem resource-Array zurück.
 	 *
 	 * @param int $which
 	 * @param bool $force_recompute ob die Ressourcen mit aktuellen $t_now
 	 * 								neu berechnet werden sollen oder nicht
 	 * 								[normalerweise unnötig!]
 	 */
 	public function get_resource($which=0, $force_recompute=false) {
 		if ($which < 0 || $which >= self::NUM_RESOURCES)
 			throw new ExceptionWithErrormail("Resource-Error", $this->uid, 'All',
 				"Trying to access invalid ressources",
 				"\$which = $which in Resource::get_resource");

 		if (empty($this->resources) || $force_recompute)
 			$this->init_resources();

		return $this->resources[$which];
 	}

 	/**
 	 * Prüft das aktuelle Bio-Grow (Also das Wachstum des ersten Rohstoffs)
 	 * Hiermit ist nur das einfache Wachstum gemeint ohne
 	 */
	public function check_bio_grow() {
		//$addbio = $this->mother->get_level()*100;
		$this->set_bio_grow(0/*300+$addbio*/);
	}

	public function check_resource_capacity($computed_resource, $which=0) {
		if ($which < 0 || $which >= self::NUM_RESOURCES)
 			throw new ExceptionWithErrormail("Ressource-Error", $this->uid, 'All',
 				"Trying to access invalid ressources",
 				"\$which = $which in Resource::get_resource");

 		// TODO: Hier noch keine Funktionalität. Die von PV war verbuggt.
		return $computed_resource;
	}

	/**
	 * Fügt dem User die angegebene Anzahl Rohstoff vom übergebenen Typ hinzu.
	 * Wird ein negativer Wert für $amount übergeben, dann wird der Rohstoff
	 * dem User abgezogen.
	 * Gibt zurück, wie viel der User dann hat, nach Abzug/Hinzufügen.
	 *
	 * @param float $amount		Wieviel hinzugefügt/abgezogen werden soll
	 * @param int $which		Welcher Rohstoff betroffen ist
	 * @return ufloat
	 * @throws Exception		Wenn User nicht genug hat zum Abziehen
	 */
	public function add_resource($amount, $which=0) {
		if ($which < 0 || $which >= self::NUM_RESOURCES)
 			throw new ExceptionWithErrormail("Ressource-Error", $this->uid, 'All',
 				"Trying to access invalid ressources",
 				"\$which = $which in Resource::get_resource");

		// MUTEX: Keine zwei ADDS gleichzeitig
		$sem_id = sem_get(MUTEX_USER_OFFSET + $this->uid, 1, 0600, -1);
		sem_acquire($sem_id);

		query("START TRANSACTION");
		if (($this->get_resource($which, true) + $amount) < 0) {
			query("ROLLBACK");
			sem_release($sem_id);
			throw new Exception("", self::ERR_NOT_ENOUGH_RESOURCES);
		}

		$this->user_resource['resource'.$which] += $amount;
		$this->user_resource['resource'.$which.'_tstamp'] = Time::get_t_now();

		query(
			"UPDATE resource SET ".
				"resource{$which}={$this->user_resource['resource'.$which]},".
				"resource{$which}_tstamp=".Time::get_t_now()." ".
			"WHERE uid={$this->uid}"
		);
		query("COMMIT");

		sem_release($sem_id);
	}

	/**
	 * Setzt die Ressource des Users unabhängig vom Mapgain.
	 * D.h., dass hiermit die Startressourcen gesetzt (auch rückwirkend
	 * theoretisch), sowie Ressource-Korrekturen des Users vorgenommen werden
	 * können (wobei Korrekturen eher über add_resource gemacht werden würden).
	 *
	 * Negative Angaben für $amount sind möglich. Das macht dann Sinn, wenn
	 * der User beispielsweise mit 0 gestartet ist, durch Mapgain 3000 verdient
	 * hat und diese jetzt ausgeben möchte => -3000. Aber wie gesagt, solche
	 * Korrekturen eher über add_resource vornehmen.
	 *
	 * @param int $amount
	 * @param uint $which
	 * @param uint $t_stamp
	 */
	public function set_user_resource($amount, $which=0, $t_stamp=0) {
		if ($which < 0 || $which >= self::NUM_RESOURCES)
 			throw new ExceptionWithErrormail("Ressource-Error", $this->uid, 'All',
 				"Trying to access invalid ressources",
 				"\$which = $which in Resource::get_resource");

		if (empty($this->user_resource))
			$this->init_resources();

		$t_stamp = (int)$t_stamp;
		if (!$t_stamp)
			$t_stamp = Time::get_t_now();

		$this->user_resource['resource'.$which] = $amount;
		$this->user_resource['resource'.$which.'_tstamp'] = $t_stamp;

		query(
			"UPDATE resource SET ".
				"resource{$which}={$amount},".
				"resource{$which}_tstamp=$t_stamp ".
			"WHERE uid={$this->uid}"
		);
	}


	/**
	 * Setzt den aktuellen Wert in der Session, mit dem das Bio wächst.
	 * @todo NUR FÜR AKTUELLEN USER!
	 */
	private function set_bio_grow($b) {
		if (User::get_current_user()->get_id() != $this->uid)	// Funktion nur für aktuellen User
			return;
		$mapsettings = Mapsettings::get_instance();
		$time_fac=$mapsettings->get_timefactor();
		$_SESSION['bio_grow'] = $time_fac* $b;
		unset($mapsettings);
	}

	/**
	 * Gibt zurück, wieviel Rohstoff vom übergebenen Typ der User unabhängig
	 * von der gesammelten Rohstoff von der Karte besitzt. Ausgaben werden
	 * mit diesem Teil der Rohstoffsumme verrechnet. Negative user-resource
	 * ist also nicht unüblich.
	 *
	 * @param int $which	welcher Rohstoff
	 * @return float 		der aktuelle User-Rohstoff-Anteil [d.h. karten unabh.]
	 */
	public function get_user_resource($which=0){
		if ($which < 0 || $which >= self::NUM_RESOURCES)
 			throw new ExceptionWithErrormail("Ressource-Error", $this->uid, 'All',
 				"Trying to access invalid ressources",
 				"\$which = $which in Resource::get_user_resource");
 		if (empty($this->user_resource))
 			$this->init_resources();

 		return $this->user_resource['resource'.$which]
 			+ ($this->user_resource['resource'.$which.'_tstamp']
 				* $this->get_resource_grow($which)/3600);
	}

	public function get_resource_grow($which=0) {
		/*if (!$_SESSION['bio_grow'])
			$this->check_bio_grow();*/
		return 0/*$_SESSION['bio_grow']*/;
	}

	/**

	public function get_bio_gain($which=false) {
		if ($which === false)
			return $_SESSION['creaturemap_computed']['map_data']['user_bio_gain'];
		if ($which < 0 || $which >= self::NUM_RESOURCES)
 			throw new ExceptionWithErrormail("Ressource-Error", $this->uid, 'All',
 				"Trying to access invalid ressources",
 				"\$which = $which in Resource::get_bio_gain");
		return $_SESSION['creaturemap_computed']['map_data']['user_bio_gain'] * self::get_bio_to_resource_rate($which);
	}*/

	/**
	 * Gibt zurück, wieviel Resource des übergebenen Typs der User vorrausichtlich
	 * für den übergebnen Sektor pro Stunde bekommen wird. Wird 'all' als Sektor
	 * angegeben, so wird die Summe über alle Sektoren zurückgegeben.
	 *
	 * @param [0,1,...99] $sector	Resourcen aus dem Sektor ('all' für alle Sektoren)
	 * @param uint $which			Welche Resource gewünscht ist
	 * @param bool $split_resource_types	Wenn TRUE wird ein Array
	 * 								zurückgegeben, wobei:
	 *							$return[0]: das vom Sektormax beschränkte Gain
	 *							$return[1]: das unbeschränkte Gain
	 *							$return[2]: das Forschungsgain
	 * 							(Sprich: Jagdhaus vs. Bauernhof vs. Bodenbrüter)
	 * @param bool $dont_check_if_dominating
	 * 		WORKAROUND-Shit. Dominatortest nicht machen
	 *
	 * @return float/float[2]
	 */
	public function get_resource_gain($which=0, $sector='all_sum',
		$split_resource_types=false, $dont_check_if_dominating=false)
	{
		//TODO Summe über alle Sektorresourcen cachen
		if ($which < 0 || $which >= self::NUM_RESOURCES)
 			throw new IllegalArgumentException("\$which has to be in [0,..,".
 				(self::NUM_RESOURCES-1)."].", $which);

		if (($sector === 'all_sum') || ($sector === 'all')) {
			$resource = ($split_resource_types) ? array(0,0,0) : 0;
			for ($sector = 0; $sector < 100; $sector++) {
				$r_temp = $this->get_resource_gain(
					$which, $sector,
					$split_resource_types, $dont_check_if_dominating
				);
				if ($split_resource_types) {
					$resource[0] += $r_temp[0];
					$resource[1] += $r_temp[1];
					$resource[2] += $r_temp[2];
				}
				else
					$resource += $r_temp;
			}
			return $resource;
		}

		if ($sector === 'all_array') {
			$result = array();
			for ($sector = 0; $sector < 100; $sector++) {
				$result[$sector] = array(0,0,0);
				$r_temp = $this->get_resource_gain(
					$which, $sector,
					$split_resource_types, $dont_check_if_dominating
				);

				if ($split_resource_types) {
					for ($r = 0; $r < self::NUM_RESOURCES; $r++) {
						$result[$sector][0] = $r_temp[0];
						$result[$sector][1] = $r_temp[1];
						$result[$sector][2] = $r_temp[2];
					}
				}
				else {
					$result[$sector] = $r_temp;
				}
			}
			return $result;
		}
		$world_id = User::get_instance_by_id($this->uid)->get_world_id();
		$map_id = User::get_instance_by_id($this->uid)->get_map_id();
		$sector_c = new WorldCoordinates($world_id, $map_id, $sector);
		$this_sector = Sector::get_instance($sector_c);
		if (!$dont_check_if_dominating
			&& (!$this_sector || ($this_sector->get_dominator_id() != $this->uid)))
		{
//			echo $this_sector->get_dominator_id();
//			echo "--";
//			echo $this->uid;
			return ($split_resource_types) ? array(0,0,0) : 0;
		}

		/**
		 * Jetzt kommt der Bonus, der durch Gebäude im Sektor
		 * ausgelöst wird
		 */
 		$max_resource = $this_sector->get_max_resources($which);
 		$buildings_in_sector = $this_sector->get_visible_buildings($this->uid);

 		$resource = ($split_resource_types) ? array(0,0,0) : 0;
 		if ($which == 0) {
 			$building_resource = 0;
 			foreach ($buildings_in_sector as $building) {
 				/*// Wenn das Gebäude noch nicht fertiggestellt, nicht einrechnen
 				if ($building->get_level() < 1)
 					continue;*/
				switch ($building->get_type()) {
					//TODO klevere Formeln überlegen
					case Buildings::MOTHER_FUNGUS:
						$building_resource += 10;
						break;
					case Buildings::LUMBERJACKS_HUT:
						$building_resource += 10;
						break;
				}
	 		}
	 		$r_temp = self::RESOURCE1_NORMAL_GAIN + $building_resource;
	 		if ($r_temp > $max_resource)
	 			$r_temp = $max_resource;

	 		if ($split_resource_types) {
	 			$resource[0] = $r_temp;
	 			$resource[1] = 0;
	 		}
	 		else {
	 			$resource = $r_temp;
	 		}
 		}
 		else if ($which == 1) {
 			$building_resource_no_max = 0;
 			$building_resource = 0;
 			foreach ($buildings_in_sector as $building) {
 				/*// Wenn das Gebäude noch nicht fertiggestellt, nicht einrechnen
 				if ($building->get_level() < 1)
 					continue;*/
				switch ($building->get_type()) {
					//TODO klevere Formeln überlegen
					case Buildings::MOTHER_FUNGUS:
						$building_resource += 10;
						break;
 					case Buildings::HUNTING_LODGE:
						$building_resource += 10;
						break;
					case Buildings::FARM:
						$building_resource_no_max += 5;
						break;
				}
 			}

 			$r_temp = self::RESOURCE2_NORMAL_GAIN  + $building_resource;
 			if ($r_temp > $max_resource)
 				$r_temp = $max_resource;

 			if ($split_resource_types) {
 				$resource[0] = $r_temp;
 				$resource[1] = $building_resource_no_max;
 			}
 			else {
 				$resource = $r_temp + $building_resource_no_max;
 			}
 		}

 		/**
 		 * Jetzt kommt der Bonus durch Forschung, der sich auf jeden
 		 * Sektor auswirkt
 		 */
 		$tr = Techtree::get_instance($this->uid)->get_cur_resource_increases();
 		if ($split_resource_types) {
 			$resource[2] = $tr[$which];
 		}
 		else{
	 		$resource += $tr[$which];
 		}

 		return $resource;
	}

	/**
	 * Gibt zurück, zu welcher Rate Bio in Resource1 oder in Resource2
	 * umgewandelt wird (abhängig von $which). Die Rate ist ein Faktor, dass
	 * Bio/h*Resource-Rate=Resource/h ergibt.
	 *
	 * @param uint $which
	 * @return ufloat
	 */
	public static function get_bio_to_resource_rate($which = 0) {
		if ($which < 0 || $which >= self::NUM_RESOURCES)
 			throw new ExceptionWithErrormail("Ressource-Error",
 				User::get_current_user()->get_id(), 'All',
 				"Trying to access invalid ressources",
 				"\$which = $which in Resource::get_resource_modifier");
 		if ($which == 0)
 			return self::RESOURCE1_RATE;
 		else
 			return self::RESOURCE2_RATE;
	}


	/**
	 * !!WARNUNG: Setzt alle (User)-Ressource-Daten aller User zurück!
	 * Irreversibel.
	 */
	public static function clear_of_all_users() {
		/**
		 * Resource 2 (Klunker) nicht löschen! NIE!
		 */
		query(
			"UPDATE resource SET resource0=0, resource1=0, resource0_tstamp=0,".
				"resource1_tstamp=0"
		);
	}
}


?>