<?php


/*
 * Created on Jun 12, 2007 by Philipp Kemmeter
 */
require_once(dirname(__FILE__).'/../consts.inc');
require_once(dirname(__FILE__).'/../assert.inc');

/** T
 * his class represents a voting system. Using this system, you can give your vote
 * to seperate topics or show the results etc.
 */
class Voting {
	private $uid;
	private $topics_voted;
	
	static private $INSTANCE;

	const DEBUG = 0;
	const MAX_CHOICES = 10;		// höchstens 10 verschiedene Möglichkeit per Topic. DB ist nicht auf mehr ausgerichtet
	
	/** Diese statische Funktion als Konstruktor benutzen */
	public static function get_instance($uid) {
 		if (self::DEBUG) {
			echo "Voting: User id@get_instance: $uid<br>\n";
 		}
 		if (!self::$INSTANCE || !self::$INSTANCE[$uid]) {
 			self::$INSTANCE[$uid] = new Voting($uid);
 		}
 		return self::$INSTANCE[$uid];
 	}

	private function __construct($uid) {
		$this->uid = $uid;
		$this->topics_voted = 0;
	}
	
	/** Die Wahl $choice treffen. Wird $topic angegeben, dann wird die Wahl diesem
	 * Topic zugeordnet. Ist $topic===false, dann wird es dem aktuellen topic zugeordnet.
	 * Das aktuelle Topic ist das, das die höchste ID hat.
	 * Eine alte Wahl also zu reaktivieren geht nur, wenn man dann beim Wählen auch das
	 * entsprechende Topic angibt!
	 * 
	 * Wenn man den Topic schon weiß, sollte man ihn auch übergeben. Spart einen DB-Zugriff.
	 */
	public function vote($choice, $topic=false) {
		if (!$topic)
			$topic = @mysql_result(query('SELECT MAX(id) FROM voting'),0);
		$topic = (int)$topic;	
		pv_assert($topic > 0, '$topic="'.$topic.'", needs to be >0. Function Voting::vote()');
		$topic_flag = constant('FLAG_'.$topic);
		pv_assert(is_anint($topic_flag) && $topic_flag > 0, '$topic_flag="'.$topic_flag.'" in function Voting::vote()');
		if ($this->voted_already($topic_flag))
			return false;	// er hat bereits gewählt
			
		query("UPDATE voting SET num_choice_{$choice}=num_choice_{$choice}+1 WHERE id=$topic");
		query("UPDATE users SET voted_for_topics=(voted_for_topics | $topic_flag) WHERE id_ffa={$this->uid} OR id_clanmode={$this->uid}");
		return true;
	}
	
	/** Neue Abstimmung erstellen. Ist statisch aufzurufen (kein Objekt nötig)
	 * $topic_array:	$topic_array[0] = DEUTSCHER_TITEL; $topic_array[1] = ENGLISH_TITLE
	 * $choices_array:	$choices_array[0] = array(DEUTSCHE_WAHL_1, DEUTSCHE_WAHL_2,...);
	 * 					$choices_array[1] = array(ENGLISH_CHOICE_1, ENGLISCH_CHOICE_2, ...);
	 * Höchstens 10 Möglichkeiten (choices) möglich!
	 */
	static public function create_new_vote($topic_array, $choices_array) {
		if (!is_array($topic_array) || count($topic_array) != 2) {
			echo "<b>PV Error:</b> \$topic_array needs an array with exact 2 entries. The first one is the german topic, the last one the english one.<br>\n";
			return false;
		}
		if (!is_array($choices_array) || count($choices_array) != 2 || !is_array($choices_array[0]) || !is_array($choices_array[1]) || count($choices_array[0]) != count($choices_array[1])) {
			echo "<b>PV Error:</b> \$choices_array needs an array with exact 2 entries. The first must be an array with the choices in german, the last one the ones in english. Both array, \$choices_array[0] and \$choices_array[1] must have the same size.<br>\n";
			return false;
		}
		if (count($choices_array[0]) < 2 || count($choices_array[0]) > self::MAX_CHOICES) {
			echo "<b>PV Error:</b> \$choices_array[i] must have the length from 2 to 10";
			return false;
		}
		global $t_now;
		
		$sql = "INSERT INTO voting SET title_de='{$topic_array[0]}', title_en='{$topic_array[1]}'";
		$n = count($choices_array[0]);
		for ($i = 0; $i < $n; $i++) {
			if (empty($choices_array[0][$i]))
				continue;
			$sql .= ", choice_{$i}_de='{$choices_array[0][$i]}', choice_{$i}_en='{$choices_array[1][$i]}', num_choice_{$i}=0";
		}
		$sql .= ", creation_time=$t_now, last_modified_date=0";
		query($sql);
		return true;
	}
	
	
	/**
	 * Gibt das angegebene Voting aus in der angegebenen Sprache des angegebenen Topics. Ist $topic===false, wird das
	 * zuletzt in die DB gespeicherte angenommen.
	 * Ist $topic bekannt, sollte es unbedingt mitübergeben werden, um DB-Zugriffe zu sparen.
	 */
	public function draw($language, $topic=false) {
		if (!$topic)
			$topic = @mysql_result(query('SELECT MAX(id) FROM voting'),0);
		if (!$topic) {
			echo "<b>Warning:</b>No Voting in DB</b><br>\n";
			return;
		}
		$topic = (int)$topic;	
		pv_assert($topic > 0, '$topic="'.$topic.'", needs to be >0. Function Voting::draw()');
		pv_assert($language && (($language === 'de') || ($language === 'en')), '$language="'.$language.'", needs to be either de or en. Function Voting::draw()');
		
	
		$voting = mysql_fetch_assoc(query("SELECT * FROM voting WHERE id=$topic"));
		$choices = array();
		$choices_num = array();
		$choices_procent = array();
		$total_choices_num = 0;
		for ($i = 0; $i < self::MAX_CHOICES; $i++) {
			if (empty($voting['choice_'.$i.'_'.$language]))	// natürlich so befüllt, dass das erste leere, das ende anzeigt
				break;
			
			$choices[] = $voting['choice_'.$i.'_'.$language];
			$choices_num[] = $voting['num_choice_'.$i];
			$total_choices_num += $voting['num_choice_'.$i];
		}
		if (!$total_choices_num) {
			for ($i = 0; $i < count($choices); $i++) {
				$choices_procent[] = 0;
			}
		}
		else {
			for ($i = 0; $i < count($choices); $i++) {
				$choices_procent[] = ($choices_num[$i] / $total_choices_num)*100;
			}
		}
			
		$topic_flag = constant('FLAG_'.$topic);
		pv_assert(is_anint($topic_flag) && $topic_flag > 0, '$topic_flag="'.$topic_flag.'" in function Voting::vote()');
		$user_voted_already = $this->voted_already($topic_flag);

		$title = $voting['title_'.$language];
		include ('templates/Voting.tmpl.inc');
	}
	
	/**
	 * Um DB-Zugriffe zu sparen, wird folgender Trick verwendet.
	 * Wenn abgefragt wird, ob ein User bereits gewählt habe, wird bei der ersten
	 * Abfrage die Antwort aus der DB gelesen und in $this->topics_voted der Wert zwischengespeichert.
	 * Später, wenn sie aufgerufen wird, kann bei im Falle, dass er schon gewählt hatte (was ja gespeichert ist)
	 * direkt true zurückgegeben werden, da eine Wahl nicht rückgängig gemacht werden kann.
	 * Dies macht also höchstens einen DB-Zugriff für diese Info und keinen, wenn man die Info nicht braucht. 
	 */
	private function voted_already($topic_flag) {
		if ($this->topics_voted == ($this->topics_voted | $topic_flag))	// Er hat schonmal gewählt. Gewählt zu haben ist irreversibel
			return true;
	
		$this->topics_voted = @mysql_result(query("SELECT voted_for_topics FROM users WHERE id={$this->uid}"),0);
		pv_assert(is_anint($this->topics_voted), '$voted_for_topic="'.$this->topics_voted.'" in function Voting::vote()');
		return ($this->topics_voted == ($this->topics_voted | $topic_flag));
	}
	
	/** Die öffentliche Schnittstelle für die voted_already-funktion */
	public function has_voted($topic = false) {
		if (!$topic)
			$topic = @mysql_result(query('SELECT MAX(id) FROM voting'),0);
		$topic = (int)$topic;	
		pv_assert($topic > 0, '$topic="'.$topic.'", needs to be >0. Function Voting::has_voted()');
		
		$topic_flag = constant('FLAG_'.$topic);
		pv_assert(is_anint($topic_flag) && $topic_flag > 0, '$topic_flag="'.$topic_flag.'" in function Voting::vote()');
		return $this->voted_already($topic_flag);
	}
}


?>