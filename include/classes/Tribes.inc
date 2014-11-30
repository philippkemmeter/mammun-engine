<?php

require_once(dirname(__FILE__).'/score/GameScore.inc');
require_once(dirname(__FILE__).'/Resource.inc');
require_once(dirname(__FILE__).'/../consts.inc');
require_once(dirname(__FILE__).'/Db.inc');
require_once(dirname(__FILE__).'/exceptions/HackattackException.inc');
//require_once(dirname(__FILE__).'/buildings/Motherfungus.inc');
require_once(dirname(__FILE__).'/../shared/classes/Tribe.inc');
require_once(dirname(__FILE__).'/../shared/classes/ITribes.inc');
require_once(dirname(__FILE__).'/../shared/classes/datastructures/UnitClothes.inc');
require_once(dirname(__FILE__).'/EventLog.inc');

/**
 * Repräsentiert einen einzelnen Tribe im Hauptgebäude.
 *
 */
//ACHTUNG! Sollte in die Tribes-Klasse verschachtelt sein, aber nested Classes wird
//von PHP nicht mehr unterstützt ebenso wenig wir Namespaces!
//ACHTUNG: !!Darf NUR von Tribes-Klasse instanziiert werden!!
class Tribes__TribeInMF extends AbstractTribe implements ITribes__TribeInMF {

	public /*int*/ $amount;	// Anzahl Einheiten des Stammes noch im Hauptgebäude
						// ACHTUNG: Berücksichtigt NICHT die noch morphenden!!!
						// Diese Anzahl ist NICHT synchron und daher private
	private /*bool*/ $dropped_once;
	public /*int*/ $add_value;
	public /*int*/ $add_stamp;
	private /*bool*/ $autodrop;
	private /*int*/ $mercenary_id;	// Verweis auf Söldnereigeneschaften, wenn Söldner
	private /*int*/ $race_level;
	private /*int[5]*/ $ability_levels;

	private /*Motherfungus*/ $mother;	// Instanz des Hauptgebäudes (nötig, um die Anzahl
								// der maximalen Einwohner zu erfahren etc)

	//private, aber Tribes-Klasse ist Freund
	//Darf NUR von Tribes-Klase aufgerufen werden, der Konstruktor

	/**
	 * Konstruktor NUR VON Tribes-Klasse heraus aufrufen
	 * (nested-Classes oder Friends werden von PHP nicht unterstützt)
	 *
	 * @param array $db_data die DB-Zeile des Tribes
	 */
	public function __construct(array $db_data) {
		$this->init_from_db($db_data);
	}
	protected function init_from_db(array $db_data=null) {
		if (!$db_data)
			return;
		parent::init_from_db($db_data);
		$this->amount = $db_data['count'];
		$this->dropped_once = $db_data['dropped_once'];
		$this->add_stamp = $db_data['add_stamp'];
		$this->add_value = $db_data['add_value'];
		$this->autodrop = $db_data['autodrop'];
		$this->mercenary_id = $db_data['mercenary_id'];
		$this->pic_url = $db_data['pic_url'];
		$this->race_level = false;	// Verzögerung, da wir hier DB-Zugriff brauchen
		$this->ability_levels = false;	// Verzögerung, wegen DB
	}

	/**
	 * Diese Funktion initialisiert race_level und ability_levels. Dazu ist
	 * ein Datenbankzugriff nötig, weswegen das hier in diese Funktion drin
	 * steht. Die Funktion wird aufgerufen, wenn beim get noch keine Daten
	 * geladen wurden (also erst on demand).
	 */
	private function init_cur_levels() {
		if ($this->is_mercenary()) {
			/*
			TODO Mercenaries
			$r = query("SELECT * FROM tribes_mercenaries WHERE id='{$this->mercenary_id}'");
			$merc_values = mysql_fetch_assoc($r);
			$this->race_level = $merc_values['race_lvl'];
			for ($i = 0; $i < Tribe::MAX_ABILITIES_PER_TRIBE; $i++)
				$this->ability_levels[$i] = $merc_values["ability_{$i}_lvl"];
				*/
		}
		else {
			$techtree = Techtree::get_instance($this->uid);
			$this->race_level = $techtree->get_science_cur_level($this->race);
			for ($i = 0; isset($this->abilities[$i]); $i++)
				$this->ability_levels[$i] = $techtree->get_science_cur_level($this->abilities[$i]);
		}
	}

	/**
	 * Wirft übergebene Anzahl an Einheiten dieses Stammes auf die Karte
	 *
	 * @param int $amount 	wieviele gedroppt werden sollen
	 * @param uint $t_stamp	Wann der Drop gewesen ist (default: Jetzt)
	 * @throws HackattackException im Falle vom Versuch Scientists zu droppen
	 * @throws ExceptionWithErrormail wenn für falschen User aufgerufen wurde
	 * @return bool ob etwas gedroppt wurde (Es wird false zurückgegeben, wenn
	 *              keinerlei Einheiten zum Droppen vorhanden gewsen sind)
	 */
	public function drop($amount, $t_stamp=0) {
		$sem_id = sem_get(MUTEX_TRIBES_DB, 1, 0600, -1);
		sem_acquire($sem_id);
		if ($this->get_race() == SC_ID_SCIENTISTS) {
			sem_release($sem_id);
			throw new HackattackException("", $this->uid,
				HackattackException::CODE_SCIENTISTS_DROP);
		}
		try {
			$user = User::get_instance_by_id($this->uid);
			$tribeamount = $this->get_amount();
			$must_add = $this->get_morphing_amount();
			if ($t_stamp <= 0)
				$t_stamp = Time::get_t_now();
			if ($tribeamount > 0) {
				if ($tribeamount < $amount) {
					$amount = $tribeamount;
					$tribeamount = 0;
				}
				else
					$tribeamount = $tribeamount - $amount; //von amount wird der wert abgezogen
				$sql = "UPDATE tribes SET count='$tribeamount', dropped_once=1";
				$this->amount = $tribeamount;
				$this->dropped_once=1;
				if ($must_add != 0) {
					$sql .= "', add_value='$must_add', add_stamp='$t_stamp'";
					$this->add_value=$must_add;
					$this->add_stamp=Time::get_t_now();
				}
				$sql .= " WHERE id='{$this->id}'";
				query($sql);
				$homesector = substr($user->get_homesector(),-2);
				$world_id = $user->get_world_id();
				$map_id = $user->get_map_id();
				query(
					'INSERT INTO tribes_on_map SET '.
					"	tribe_id={$this->id}, ".
					"	t_appearance=$t_stamp, ".
					"	amount=$amount, ".
					"	world_id=$world_id, ".
					"	map_id=$map_id, ".
					"	path='$homesector'"
				);

				/**
				 * Den Sektor als dirty markieren, dass er in jedem Fall neu
				 * berechnet wird. Jetzt aber keine Berechnung ausgelöst werden,
				 * daher ist der Timeout auf unendlich.
				 */
				MapCompute::get_instance($world_id, $map_id,INF)
					->mark_sector_as_dirty($homesector);
				sem_release($sem_id);
				return true;
			}
		}
		catch(Exception $e) {
			sem_release($sem_id);
		}
		sem_release($sem_id);
		return false;
	}

	/**
	 * Erzeugt neue Einheiten (so viele, wie angegeben) dieses Stammes.
	 *
	 * @param int $amount 	wieviele erzeugt werden sollen
	 * @param string $name	wie die Einheit auf der Karte heißen soll (wird
	 * 						bei Forschern ignoriert)
	 * @param UnitClothes $clothes	Welche Kleidung er tragen soll (wird bei
	 * 						Forschern ignoriert)
	 * @param bool $female=false	Ob die Figur weiblich sein soll, sonst männl
	 *
	 * @return uint			Wieviele Klunker der User danach (noch) hat
	 *
	 * @throws HackattackException im Falle vom Versuch Söldner zu morphen
	 * @throws ExceptionWithErrormail wenn für falschen User aufgerufen wurde
	 */
	public function morph($amount, $name, UnitClothes $clothes, $female=false) {
		if ($this->get_race() == SC_ID_SCIENTISTS) {
			$name = 'Denker';
		}
		ValueChecker::int($amount, 'amount', 0);
		ValueChecker::string($name, 'name');
		ValueChecker::bool($female, 'female');

		$sem_id = sem_get(MUTEX_TRIBES_DB, 1, 0600, -1);
		sem_acquire($sem_id);

		$tribes = Tribes::get_instance($this->uid);
		if ($tribes->get_creatable_unit_amount() <= 0) {
			sem_release($sem_id);
			throw new Exception("Unit Limit reached!",
				self::ERR_UNIT_LIMIT_REACHED);
		}

		if (count($tribes->get_morphing_tribes())
			>= Tribes::TRIBE_MORPH_SIMUL_MAX)
		{
			sem_release($sem_id);
			throw new Exception("All morphing slots are full!",
				self::ERR_MOPPH_TOO_MANY_SIMUL);
		}


		if ($this->is_mercenary()) {	// Söldner dürfen nicht gemorpht werden
			sem_release($sem_id);
			throw new HackattackException("", $this->uid,
				HackattackException::CODE_MERC_MORPH);
		}
		if ($this->is_morphing()) {
			sem_release($sem_id);
			throw new ExceptionWithErrormail("Tribe is morphing atm!",
				$this->uid, 'All', 'Morph() called on morphing tribe',
				'A Tribe is morphing at the moment, and morph() has been called.'.
				'Please check the architecture and buttons. Could also be a
				hackattack.');
		}
		$other_tribes_same_race =
			Tribes::get_instance($this->uid)->get_tribes_by_race($this->race);

		foreach ($other_tribes_same_race as $tribe) {
			if ($tribe->is_morphing()) {
				sem_release($sem_id);
				throw new ExceptionWithErrorlog("An equal tribe is morphing atm!",
				$this->uid, 'All', 'Morph() called - equal tribe is morphing',
				'A Tribe is morphing at the moment, and morph() has been on '.
				'a tribe with the same race. Please check the architecture and '.
				'buttons. Could also be a hackattack.');
			}
		}

		$R = Resource::get_instance($this->uid);
		$bio = $R->get_resource(Resource::RESOURCE2);

		$costs = $amount * $this->cost;
		$tribeamount = $this->get_amount();

		if ($costs > $bio) {//er will zu viel morphen
			sem_release($sem_id);
			throw new Exception("", self::ERR_NOT_ENOUGH_BIO);
		}

		try {
			$kkr_amount = Server::get_instance(CONF_PAYMENT_SERVER_URL,
				CONF_MAMMUN_AUTH_SECRET)
					->cmd_use_items($this->uid, $clothes->to_php_array(), true);
		}
		catch (Exception $e) {
			sem_release($sem_id);
			throw new Exception($e->getMessage(), self::ERR_NOT_ENOUGH_KLUNKER);
		}

		if ($amount > 0) {
			try {
				/**
				 * Einzig möglicher Fall. Alle anderen Fälle sind nicht mehr zu
				 * beachten
				 */
				query("UPDATE tribes SET add_value='$amount', ".
					"add_stamp='".Time::get_t_now()."' WHERE id='{$this->id}'");

				$finish_time = Time::get_t_now() +
							$amount * $this->get_time_needed();

				$user = User::get_instance_by_id($this->get_uid());
				/**
				 * Alle Einheiten außer Forschern entstehen auf der Karte und
				 * werden somit direkt gedroppt. Da sie erstmal entstehen
				 * müssen, wird der Drop-Zeitpunkt in die Zukunft gesetzt.
				 */
				if ($this->get_race() != SC_ID_SCIENTISTS) {
					// FIXME: get_homesector is obsolete
					$hs = $user->get_homesector()%100;
					if ($hs < 10)
						$hs = "0$hs";

					$sex = ($female) ? 1 : 0;
					$map_id = $user->get_map_id();
					$world_id = $user->get_world_id();
					query(
						"INSERT INTO tribes_on_map ".
						"SET tribe_id={$this->id}, path='$hs', ".
							"world_id=$world_id, map_id=$map_id, amount=$amount, ".
							"t_appearance=$finish_time, name='$name', sex=$sex, ".
							"outerwear=".$clothes->outerwear.", ".
							"trousers=".$clothes->trousers.", ".
							"shoes=".$clothes->shoes.", ".
							"jewelry=".$clothes->jewelry.", ".
							"hairhat=".$clothes->hairhat.", ".
							"weapon_style=".$clothes->weapon
					);

				}

				$this->add_value = $amount;
				$this->add_stamp = Time::get_t_now();
				/**
				 * Mit der Pic-URL und dem Namen sind nun Verweise auf die
				 * aktuell morphende Einheit gemeint, nicht das generelle Bild/
				 * der generelle Name aller Einheiten dieses Typs. Daher wird
				 * das nun korrigiert.
				 */
				$pic_core = substr($this->get_pic_url(), 0,
								strrpos($this->get_pic_url(),'.')
							);
				$this->pic_url = "$pic_core/{$pic_core}_{$sex}_".
					"{$clothes->outerwear}_{$clothes->trousers}_".
					"{$clothes->shoes}_{$clothes->jewelry}_".
					"{$clothes->hairhat}_{$clothes->weapon}.png";
				$this->name = $name;

				$R->add_resource((-1)*$costs, Resource::RESOURCE2);


				/**
				 * In Event-Log eintragen (Ereignis in der Zukunft)
				 */
				$map_id = $user->get_map_id();
				$world_id = $user->get_world_id();
				$ev_log = EventLog::get_instance(
					$this->get_uid(), $world_id, $map_id
				);

				$event_type = ($this->get_race() == SC_ID_SCIENTISTS)
					? EventLog::TYPE_TRAINING_FINISHED_SCIENTIST
					: EventLog::TYPE_TRAINING_FINISHED;

				$ev_log->add_entry(
					$event_type,
					$this->race,
					"$name-{$pic_core}_{$sex}_".
							"{$clothes->outerwear}_{$clothes->trousers}_".
							"{$clothes->shoes}_{$clothes->jewelry}_".
							"{$clothes->hairhat}_{$clothes->weapon}",
					$finish_time
				);

				/**
				 * Ggf. Medallie vergeben
				 */
				$sc_medals = Medals::get_medals(IMedals::TYPE_UNITS_TRAINED);
				$sc_medals = $sc_medals[IMedals::TYPE_UNITS_TRAINED];

				$c1 = $ev_log->get_total_event_amount(
						EventLog::TYPE_TRAINING_FINISHED
					)
					+ $ev_log->get_total_event_amount(
						EventLog::TYPE_TRAINING_FINISHED_SCIENTIST
				);
				$c2 = $ev_log->get_total_event_amount($event_type, $this->race);

				$medal_id = 0;
				if (isset($sc_medals[0][$c1]))
					$medal_id = $sc_medals[0][$c1];
				else if (isset($sc_medals[$this->race][$c2]))
					$medal_id = $sc_medals[$this->race][$c2];
				if ($medal_id) {
					UserMedals::get_instance($this->uid)->add_medal(
						$medal_id, $finish_time
					);
				}
				/**
				 * Punkte durch Eventnachricht gutschreiben und Medallie
				 * vermerken
				 */
				EventMessages::get_instance($this->get_uid())
					->add_entry(
						$event_type,
						$this->race,
						"$name-{$pic_core}_{$sex}_".
							"{$clothes->outerwear}_{$clothes->trousers}_".
							"{$clothes->shoes}_{$clothes->jewelry}_".
							"{$clothes->hairhat}_{$clothes->weapon}",
						$finish_time,
						GameScore::calc_score($costs,
							GameScore::TYPE_RESSOURCE_USE),
						$medal_id
					);

				/**
				 * Userdaten neuladen (warum eigentlich? PV-Relikt?)
				 */
				$_SESSION['reload_userdata'] = true;
			}
			catch (Exception $e) {
				sem_release($sem_id);
				throw $e;
			}
		}
		sem_release($sem_id);
		return $kkr_amount;
	}

	/**
 	 * Entfernt die angegebene Anzahl an Einheiten des Stammes aus dem
	 * Hauptgebäude. Macht bei der aktuellen Konfiguration nur bei Forschern
	 * Sinn, da alle anderen direkt auf die Karte gedroppt werden.
	 *
	 * @param uint $amount
	 */
	public function unmorph($amount) {
		if ($amount < 0)
			throw new IllegalArgumentException("amount", "N+", $amount);

		if ($this->is_mercenary()) {	// Söldner dürfen nicht entmorpht werden
			throw new HackattackException("", $this->uid,
				HackattackException::CODE_MERC_MORPH);
		}
		if ($this->is_morphing()) {
			throw new ExceptionWithErrorlog("Tribe is morphing atm!",
				$this->uid,
				'A Tribe is morphing at the moment, and unmorph() has been '.
				'called. Please check the architecture and buttons. '.
				'Could also be a hackattack.');
		}
		if ($this->get_race() != SC_ID_SCIENTISTS) {
			throw new Exception("Unmorph only for Scientists",
				self::ERR_UNMORPH_SCIENTISTS_ONLY);
		}

		query("UPDATE tribes SET count=count-$amount WHERE id={$this->id}");
		$this->amount -= $amount;
	}


	/**
	 * Stoppt das Morphen
	 *
	 * @throws ExceptionWithErrormail wenn falscher User
	 */
	public function stop_morphing() {
		$tribeamount = $this->get_amount();
		$must_add = $this->get_morphing_count();
		if ($must_add) {
			$this->_set_amount($tribeamount);
			$bio = add_bio($must_add * $this->cost,$bio);
		}
	}

	/**
	 * Erzeugt alle Werte des Tribes neu auf Basis seiner Rasse und
	 * Fähigkeiten.
	 * @return bool		ob der Tribe upgedatet wurde, oder nicht
	 */
	public function recompute() {
		if ($this->is_mercenary())	// Söldner werden nie aktualisiert
			return false;
		$techtree = Techtree::get_instance($this->uid);
		$race_values = $techtree->get_science($this->get_race());

		$this->cost = $race_values['cost'];
		$this->speed = $race_values['speed'];
		$this->att = $race_values['attack'];
		$this->def = $race_values['defend'];
		$this->hp = $race_values['hitpoints'];
		$this->pic_url = $race_values['pic'];	// Ggf ändert sich das Bild

		$abilities = $this->get_abilities();
		foreach ($abilities as $ability) {
			$abil_values = $techtree->get_science($ability);
			$this->cost += $abil_values['cost'];
			$this->speed += $abil_values['speed'];
			$this->att += $abil_values['attack'];
			$this->def += $abil_values['defend'];
			$this->hp += $abil_values['hitpoints'];
		}
		if ($this->att < 1) $this->att = 1;
		if ($this->def < 1) $this->def = 1;
		if ($this->hp < 1) $this->hp = 1;
		if ($this->speed < 1) $this->speed = 1;
		if ($this->cost < 1) $this->cost = 1;

		query(
			"UPDATE tribes SET cost={$this->cost}, hp={$this->hp}, ".
			"	pic_url='{$this->pic_url}', att={$this->att}, def={$this->def}, ".
			"	speed={$this->speed} ".
			"WHERE id=".$this->get_id()
		);
		$this->pic_url = 'units/'.$this->pic_url;
		return true;
	}

	/**
	 * Diese Funktion prüft, ob der Tribe noch am Morphen ist, oder bereits
	 * fertig sein müsste. Wenn das der Fall ist, wird alles entsprechend
	 * aktualisiert.
	 */
	private function check_if_still_morphing() {
		$finish_time = $this->add_stamp +
			$this->add_value * $this->get_time_needed();
		if ($finish_time <= Time::get_t_now()) {
			if ($this->get_race() == SC_ID_SCIENTISTS) {
				$this->_set_amount($this->amount + $this->add_value);
			}
			else {
				/**
				 * Nun wird die Anzahl des Tribes im MF auf 0 gesetzt, weil sie
				 * jetzt auf der Karte erscheinen werden (wurden schon in die
				 * Zukunft gedroppt, siehe morph())
				 */
				$this->_set_amount(0);
			}
		}
	}

	/// GETTER
	/**
	 * Gibt die aktuelle Anzahl der Einheiten des Tribes im MF zurück.
	 * GEÄNDERT: Es wird während der Morphens nicht mehr aktuallisiert!
	 *
	 * @return int die aktuelle Anzahl :)
	 */
	public function get_count() {
		return floor($this->get_raw_amount());
	}

	public function get_amount() {
		return $this->get_count();
	}

	/**
	 * Gibt zurück wie lange es dauert eine Einheit davon zu morphen in Sekunden.
	 * @return float Dauer in Sekunden
	 */
	public function get_morph_duration() {
		return $this->get_time_needed();
	}
	/**
	 * Gibt zurück, wie viele Einheiten noch gemorpht werden (also wie viele
	 * noch nicht gemorpht worden sind und in der Warteschlange zum Morphen stehen)
	 *
	 * GEÄNDERT: Gibt dasselbe zurück, wie add_value
	 * @return int Anzahl der zu morphenden Einheiten
	 */
	public function get_morphing_count() {
		return $this->get_add_value();
	}

	public function get_morphing_amount() {
		return $this->get_morphing_count();
	}

	public function get_add_value() {
		$this->check_if_still_morphing();
		return $this->add_value;
	}
	public function get_add_stamp() {
		$this->check_if_still_morphing();
		return $this->add_stamp;
	}
	public function get_raw_amount() {
		$this->check_if_still_morphing();
		return $this->amount;
	}
	public function get_raw_count() {
		return $this->get_raw_amount();
	}
	public function is_morphing() {
		return ($this->get_add_value() != 0);
	}
	public function is_mercenary() {
		return ($this->mercenary_id != 0);
	}
	public function get_race_level() {
		if (!$this->race_level)
			$this->init_cur_levels();
		return $this->race_level;
	}
	/**
	 * Gibt das Level der gewählten Ability zurück
	 *
	 * @param uint $which_ability	Das Level welcher Ability
	 * @return int das entsprechende Level
	 */
	public function get_ability_level($which_ability) {
		if (!$this->ability_levels)
			$this->init_cur_levels();
		return isset($this->ability_levels[$which_ability])
			? $this->ability_levels[$which_ability] : 0;
	}

	/// SETTER

	/**
	 * ACHTUNG: NUR VON DER TRIBES-KLASSE AUFZURUFEN!
	 * Setzt die aktuelle Anzahl auf $count. Wenn gerade gemorpht wird, wird
	 * das Morphen radikal abgebrochen OHNE Gutschrift. Diese Funktion also
	 * mit Bedacht benutzen. Sie ist NICHT mit Tribes__TribeInMF::morph zu verwechseln,
	 * welche neue Einheiten zu produzieren beginnt. Hier wird direkt gehackt.
	 *
	 * @param uint $amount 	die neue Anzahl
	 * @param uint $t_stamp	Zeitpunkt, wann das setzen passiert sein soll
	 * 						(default: jetzt)
	 */
	public function _set_amount($amount, $t_stamp=0) {
		$this->amount=$amount;
		$this->add_stamp = 0;
		$this->add_value = 0;
		/*
		 * Nachdem alles gemorpht ist, werden die Einheiten
		 * 1000er-weise gedroppt. Bei jedem Drop entsteht ja
		 * ein neuer Tribe-on-Map, welcher 1000 Units intern
		 * repräsentiert.
		 * Das gilt natürlich nur für Nicht-Forscher, weil die
		 * Forscher ja niemals gedroppt werden dürfen.
		 */
		if ($this->get_race() != SC_ID_SCIENTISTS)
			for ($d = 0; $d < $amount; $d+=1000)
				$this->drop(1000,$t_stamp);	// zieht jeweils 1000 von $this->amount ab
		query("UPDATE tribes SET count={$this->amount}, add_stamp=0, ".
			"add_value=0 WHERE id='{$this->id}'");
	}

	public function to_php_assoc_array() {
		$this->init_cur_levels();

		$result = array (
			'abilities' => $this->abilities,
			'ability_levels' => $this->ability_levels,
			'add_stamp' => $this->add_stamp,
			'add_value' => $this->add_value,
			'amount' => $this->get_amount(),
			'autodrop' => $this->autodrop,
			'att' => $this->att,
			'cost' => $this->cost,
			'def' => $this->def,
			'dropped_once' => $this->dropped_once,
			'extra' => $this->extra,
			'hp' => $this->hp,
			'id' => $this->id,
			'mercenary_id' => $this->mercenary_id,
			'morph_duration' => $this->get_morph_duration(),
			'name' => $this->name,
			'pic_url' => $this->pic_url,
			'race' => $this->race,
			'race_level' => $this->race_level,
			'speed' => $this->speed,
			'state' => $this->state,
			'success' => $this->success,
			'uid' => $this->uid
		);
		for ($i = 0; $i < isset($this->abilities[$i]); $i++) {
			$result['ability_'.$i] = $this->abilities[$i];
		}
		return $result;
	}

	public function to_php_std_class() {
		$this->init_cur_levels();
		$x = new stdClass();
		$x->abilities = $this->abilities;
		$x->ability_levels = $this->ability_levels;
		$x->add_stamp = $this->add_stamp;
		$x->add_value = $this->add_value;
		$x->amount = $this->get_amount();
		$x->autodrop = $this->autodrop;
		$x->att = $this->att;
		$x->cost = $this->cost;
		$x->def = $this->def;
		$x->dropped_once = $this->dropped_once;
		$x->extra = $this->extra;
		$x->hp = $this->hp;
		$x->id = $this->id;
		$x->mercenary_id = $this->mercenary_id;
		$x->morph_duration = $this->get_morph_duration();
		$x->name = $this->name;
		$x->pic_url = $this->pic_url;
		$x->race = $this->race;
		$x->race_level = $this->race_level;
		$x->speed = $this->speed;
		$x->state = $this->state;
		$x->success = $this->success;
		$x->uid = $this->uid;
		for ($i = 0; $i < isset($this->abilities[$i]); $i++) {
			$x->{'ability_'.$i} = $this->abilities[$i];
		}
		return $x;
	}
}

//////////// ENDE VON DER KLASSE TRIBE //////////////


/**
 * Die Tribes-Klasse repräsentiert die Sammlung aller Stämme eines Users,
 * die NICHT auf der Karte (also noch im Hauptgebäude) sind.
 * Der Motherfungus hat einen Link hierauf,
 * aber es kann auch eigenständig benutzt werden (aus Effizienzgründen, da
 * nicht immer alle Informationen des MF benötigt werden, wenn die Tribes
 * gebraucht werden)
 *
 * !Singleton (nur eine Instanz pro UID)
 */
class Tribes implements ITribes {
	private $tribes = null; //all Tribes als "tribe_id => Tribe"
	private $uid = 0;
	private $db_updated = false;
	private $morph_speed = 0;

	const TRIBE_MORPH_SIMUL_MAX = CIBal::TRIBE_MORPH_SIMUL_MAX;

	static private $INSTANCE = array();	// instanzüberwachung. Nur eine Instanz
										// pro UID. Konstruktor ist private.
										// Objekt statisch über get_instance($uid)
 										// anfordern! (s.u.)

 	/**
	 * Diese Statische Funtion als Konstruktor benutzen !
	 *
	 * @param uint $uid
	 * @return Tribes
	 **/
 	public static function get_instance($uid) {
 		if (!self::$INSTANCE || !self::$INSTANCE[$uid]) {
 			self::$INSTANCE[$uid] = new Tribes($uid);
 		}
 		return self::$INSTANCE[$uid];
 	}
	private function __construct($uid) {
		$this->uid = $uid;
		$this->tribes = false;	// DB-Zugriff verzögern, bis wirklich nötig
		$this->db_updated = true;	// DB hat neuere Einträge als das Objekt
	}


	/**
	 * Initialisiert die Tribes - DB-Zugriff nötig
	 * Am Ende ist $this->tribes initialisiert. Kein Rückgabewert.
	 */
	private function init_tribes() {
		if ($this->db_updated) {	// DB ist neuer
			$this->tribes = array();

			$res = query(
				"SELECT * FROM tribes WHERE uid={$this->uid} AND state=0 ".
				"ORDER BY id"
			);
			while ($tribe_data = mysql_fetch_assoc($res)) {
				$res_abil = query(
					"SELECT ability_id FROM tribes_abilities ".
					"WHERE tribe_id={$tribe_data['id']}"
				);
				$i = 0;
				while ($abil = mysql_fetch_row($res_abil)) {
					$tribe_data['ability_'.$i] = $abil[0];
					$i++;
				}

				$finish_time = 	$tribe_data['add_stamp'] +
					$tribe_data['add_value'] *
						Tribe::calculate_needed_time($tribe_data['cost']);
				if (($finish_time > Time::get_t_now())
					&& ($tribe_data['race'] != SC_ID_SCIENTISTS))
				{
					$res_tribe_on_map = query(
						"SELECT name, sex, outerwear, trousers, shoes, jewelry, ".
							"hairhat, weapon_style FROM tribes_on_map ".
						"WHERE tribe_id={$tribe_data['id']} ".
							" AND t_appearance={$finish_time}"
					);
					if (mysql_num_rows($res_tribe_on_map) == 0)
						error_log("NO TRIBE_ON_MAP TRIBE FOUND FOR ".
							"{$tribe_data['id']}. Finishtime: $finish_time");
					else {
						$tmap_d = mysql_fetch_assoc(
							$res_tribe_on_map
						);
						$tribe_data['name'] = $tmap_d['name'];

						$pic_core = substr($tribe_data['pic_url'], 0,
							strrpos($tribe_data['pic_url'],'.')
						);

						$tribe_data['pic_url'] = "$pic_core/{$pic_core}_".
							"{$tmap_d['sex']}_{$tmap_d['outerwear']}_".
							"{$tmap_d['trousers']}_{$tmap_d['shoes']}_".
							"{$tmap_d['jewelry']}_{$tmap_d['hairhat']}_".
							"{$tmap_d['weapon_style']}.png";
					}
				}
				$tribe = new Tribes__TribeInMF($tribe_data);
				$this->tribes[$tribe->get_id()] = $tribe;
			}
			$this->db_updated = false;
		}
	}

	/**
	 * Gibt alle Tribes des Users zurück.
	 *
	 * @return Tribes__TribeInMF[] alle Tribes des Users als "tribe_id => Tribe"
	 */
	public function get_tribes() {
		$this->init_tribes();
		return $this->tribes;
	}

	/**
	 * Gibt den Tribe des Users zurück, welcher die übergebene ID besitzt.
	 *
	 * @return Tribes__TribeInMF der gewünschte Tribe
	 */
	public function get_tribe_by_id($id) {
		$this->init_tribes();
		return $this->tribes[$id];
	}

	/**
	 * Gibt die Tribes mit der angegebenen Rasse zurück
	 *
	 * @param int $race						Die Rassen-ID
	 * @return Tribes__TribeInMF[]/bool		Die Tribes in einem Array oder false,
	 * 										wenn kein Hit.
	 */
	public function get_tribes_by_race($race) {
		$this->init_tribes();
		$result = array();
		foreach ($this->tribes as $tribe) {
			if ($tribe->get_race() == $race)
				$result[] = $tribe;
		}
		return (count($result) > 0) ? $result : false;
	}


	/**
	 * Gibt die Tribes zurück, die gerade morphen. Wird $race mit angegeben,
	 * werden danach gefiltert nur die morphenden angegeben, welche von der
	 * Rasse $race sind.
	 *
	 * @param int $race [optional]
	 * @return Tribes__TribeInMF[] die morphenden Tribes
	 */
	public function get_morphing_tribes($race=false) {
		$this->init_tribes();
		$result = array();
		if ($race === false) {
			foreach ($this->tribes as $tribe) {
				if ($tribe->is_morphing())
					$result[] = $tribe;
			}
		}
		else {
			foreach ($this->tribes as $tribe) {
				if (($tribe->get_race() == $race) && $tribe->is_morphing())
					$result[] = $tribe;
			}
		}
		return $result;
	}

	/**
	 * Fügt einen neuen Tribe zu den produzierbaren Tribes im MF ein.
	 * Wird eine Fähigkeit mehrfach im Array angegeben, wird diese Redundanz
	 * entfernt. Man kann also einem Tribe nicht 2x dieselbe Fähigkeit geben.
	 * Dieser Fall wirft keinen Fehler.
	 *
	 * @param int $race			Grundtyp/Rasse
	 * @param string $name		Name, den der Tribe bekommen soll
	 * @param int[] $abilities	Fähigkeiten, die der Tribe haben soll
	 * @return Tribes__TribeInMF 	Den hinzugefügten Tribe als Objekt
	 */
	public function insert_new_tribe($race, $name, $abilities=array()) {
		$techtree = Techtree::get_instance($this->uid);
		if (!$techtree->get_science_cur_level($race)) {
			throw new HackattackException("Race has not been researched!",
				$this->uid, HackattackException::CODE_MORPH_UNRESEARCHED);
		}

		foreach ($abilities as $ability) {
			if (!$techtree->get_science_cur_level($ability))
				throw new HackattackException("Ability has not been researched!",
					$this->uid, HackattackException::CODE_MORPH_UNRESEARCHED);
		}
		$tribes = $this->get_tribes();
		$name2 = str_replace(' ', '', $name);
		if (empty($name2))
			throw new Exception("", self::E_NAME_EMPTY);

		foreach ($tribes as $tribe) {
			// 2x gleicher Name ist verboten
			/*if ($tribe->get_name() == $name) {
				throw new Exception("", self::E_SAME_NAME_TWICE);
			}*/
			// Test, ob 2x die gleiche Konfiguration unter unterschiedlichem
			// Namen versucht wird
			if ($tribe->get_race() == $race) {
				$abils = $tribe->get_abilities();
				if (count($abils) == count($abilities)) {
					$abils_twice_count = 0;
					foreach ($abilities as $abil)
						if (in_array($abil, $abils))
							$abils_twice_count++;
					if ($abils_twice_count == count($abils))
						throw new Exception($tribe->get_id(),
								self::E_SAME_TRIBE_CONFIG_TWICE);
				}
			}
		}
		//checken, ob der Typ die gewählte Fähigkeit überhaupt erhalten darf
		$allowed_abilities = $techtree->get_allowed_abilities_of_race($race);
		$abil_types = array();
		foreach ($abilities as $abil) {
			$abilitiy_values = $techtree->get_science($abil);
			$abil_types[$abilitiy_values['type']]++;
			if (!isset($allowed_abilities[$abilitiy_values['type']])
				|| (($allowed_abilities[$abilitiy_values['type']] != 0)
					&& ($allowed_abilities[$abilitiy_values['type']] <
						$abil_types[$abilitiy_values['type']])) )
			{
				throw new HackattackException("Race may not be equipped with ".
												"this ability!",
					$this->uid, HackattackException::CODE_ILLEGAL_ABIL_FOR_RACE);
			}
		}

		// alle Checks erledig, Tribe kann hinzugefügt werden
		$tribe_data['name'] = $name;
		$tribe_data['uid'] = $this->uid;
		$tribe_data['race'] = $race;

		query(
			'INSERT INTO tribes '.
			'SET race='.$tribe_data['race'].', name="'.$tribe_data['name'].'", '.
			'	uid='.$tribe_data['uid']
		);
		// WARUM AUCH IMMER: mysql_insert_id() funktionierte nicht mehr...
		$tribe_data['id'] = mysql_result(query(
			"SELECT id FROM tribes WHERE race={$tribe_data['race']} AND ".
				"name='{$tribe_data['name']}' AND uid={$tribe_data['uid']} AND ".
				"cost=0 AND att=0 AND def=0"
		),0);
		$i = 0;
		foreach ($abilities as $abil) {
			// Fähigkeiten einfügen (Dopplungen ignorieren)
			query(
				"INSERT IGNORE INTO tribes_abilities ".
					"SET tribe_id={$tribe_data['id']}, ability_id=$abil"
			);
			$tribe_data['ability_'.$i++] = $abil;
		}

		$tribe_data['att'] = $tribe_data['def'] = $tribe_data['cost']
			= $tribe_data['speed'] = $tribe_data['state'] = $tribe_data['extra']
			= $tribe_data['add_value'] = $tribe_data['add_stamp']
			= $tribe_data['autodrop'] = $tribe_data['dropped_once']
			= $tribe_data['mercenary_id'] = 0;
		$tribe_data['pic_url'] = '';

		$tribe = new Tribes__TribeInMF($tribe_data);
		$this->init_tribes();
		$this->tribes[$tribe->get_id()] = $tribe;
		$tribe->recompute();	// setzt dann alle Attributswerte etc

		return $tribe;
	}

	/**
	 * Setzt die Tribes auf den Stand (zurück), den der Spieler zu Beginn
	 * des Spiel hat - es werden also alle Tribes gelöscht bis auf die
	 * absolut notwendigen.
	 * ACHTUNG! Das ist IRREVERSIBEL!
	 */
	public function clear() {
		$sem_id = sem_get(MUTEX_TRIBES_DB, 1, 0600, -1);
		sem_acquire($sem_id);
		query("DELETE FROM tribes WHERE uid={$this->uid}");
		query(
			"INSERT INTO tribes SET name='Scientists', race='2', ".
				"cost='1.5', att='1', def='1', hp='1', speed='1', ".
				"pic_url='scientist.gif', uid={$this->uid}, count='4000'");
		unset($this->tribes);
		$this->db_updated = true;
		sem_release($sem_id);
	}

	/**
	 * Setzt das ganz
	 */
	public function clear_map_end() {
		$sem_id = sem_get(MUTEX_TRIBES_DB, 1, 0600, -1);
		sem_acquire($sem_id);
		query(
			"UPDATE tribes SET state=1 ".
			"WHERE mercenary_id=0 AND uid={$this->uid} AND race!=2"
		);
		unset($this->tribes);
		$this->db_updated = true;
		sem_release($sem_id);
	}

	/**
	 * ACHTUNG!Setzt die Tribes aller User auf den initialstand zurück.
	 * Irreversibel!
	 */
	public static function clear_of_all_users() {
		$sem_id = sem_get(MUTEX_TRIBES_DB, 1, 0600, -1);
		sem_acquire($sem_id);

		self::$INSTANCE = array();
		query("DELETE FROM tribes");
		query(
			"INSERT IGNORE INTO tribes (name, race, cost, att, def, hp, speed, ".
				"pic_url, uid, count) ".
			"SELECT 'Scientists', 2, 1.5, 1, 1, 1, 1, 'scientist.gif', id, ".
				"4000 FROM users"
		);
		query("DELETE FROM tribes_abilities");
		sem_release($sem_id);
	}

	/**
	 * Aktualisiert alle Attribute der Tribes, die die übergebene Ability besitzen.
	 * Wird z.B. beim Erstellen eines Tribes oder wenn eine Ability höher geforscht
	 * wurde angewandt.
	 * @param int $ability	Die sich gerade geänderte Fähigkeit
	 */
	public function update_all_tribes_with_ability($ability) {
		foreach ($this->tribes as $tribe) {
			foreach ($tribe->get_abilities() as $abil)
				if ($abil == $ability)
					$tribe->recompute();
		}
	}

	/**
	 * Aktualisiert alle Attribute der Tribes, die auf der übergebenen Rasse basieren.
	 * Wird z.B. beim Erstellen eines Tribes oder wenn eine Rasse höher geforscht
	 * wurde angewandt.
	 * @param int $race		Die sich gerade geänderte Rasse
	 */
	public function update_all_tribes_with_race($race) {
		foreach ($this->tribes as $tribe) {
			if ($tribe->get_race() == $race)
				$tribe->recompute();
		}
	}

	/**
	 * Gibt das Einheitenlimit zurück
	 *
	 * @return uint
	 */
	public function get_unit_limit() {
		$user = User::get_instance_by_id($this->uid);
		$map_id = $user->get_map_id();
		$world_id = $user->get_world_id();
		$dwellings = BuildingOnMap
			::get_all_of_user($this->uid, Buildings::DWELLING, $world_id,
				$map_id);
		$tents = BuildingOnMap
			::get_all_of_user($this->uid, Buildings::TENT, $world_id, $map_id);
		return count($dwellings)*2 + count($tents)*1 + 12;
	}

	/**
	 * Gibt zurück, wie viele Einheiten bereits produziert wurden,
	 * die vom Einheitenlimit abgezogen werden müssen.
	 *
	 * @return uint
	 */
	public function get_unit_amount_for_limit() {
		// 1. Einheiten auf der Karte
		$user = User::get_instance_by_id($this->uid);
		$world_id = $user->get_world_id();
		$map_id = $user->get_map_id();

		$tribes = TribeOnMap::get_all_of_user($world_id, $map_id, $this->uid);
		$unit_amount = count($tribes);
		// 2. All Forscher
		$scs_tribe = $this->get_tribes_by_race(SC_ID_SCIENTISTS);
		$scs_tribe = $scs_tribe[0];
		$unit_amount += floor($scs_tribe->get_amount()/1000);
		// 3. Alle Forscher, die gerade produziert werden
		$unit_amount += count($this->get_morphing_tribes(SC_ID_SCIENTISTS));
		return $unit_amount;
	}

	/**
	 * Gibt zurück, wie viele Einheiten noch produzierbar sind, bis das
	 * Einheitenlimit erreicht ist (also die Differenz von Einheiten auf der
	 * Karte und Einheitenlimit).
	 *
	 * @return uint
	 */
	public function get_creatable_unit_amount() {
		return $this->get_unit_limit() - $this->get_unit_amount_for_limit();
	}

	/**
	 * Gibt die Geschwindigkeit zurück, wie schnell trainiert werden kann.
	 *
	 * Aus PV-Zeiten heißt Trainieren noch Morphen.
	 */
	public function get_morph_speed() {
		if (!$this->morph_speed) {
			/**
			 * Nur solange puffern, wie der Zeitfaktor der Karte sich nicht
			 * geändert hat.
			 */
			if ((empty($_SESSION['Tribes']['morph_speed']))
				|| ($_SESSION['Tribes']['morph_speed_time_fac']
					!= Mapsettings::get_instance()->get_timefactor())
				)
			{
				$data = Techtree::get_instance($this->uid)
							->get_science(SC_ID_MORPHING);

				$time_fac = Mapsettings::get_instance()->get_timefactor();
				$_SESSION['Tribes']['morph_speed']
					= $data['f_extra1'] * $time_fac;

				$_SESSION['Tribes']['morph_speed_time_fac'] = $time_fac;
			}
			$this->morph_speed = $_SESSION['Tribes']['morph_speed'];
		}
		return $this->morph_speed;
	}
}
?>