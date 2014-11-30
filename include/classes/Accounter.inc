<?php


/*
 * Created on Dec 11, 2006 by Philipp Kemmeter
 */
require_once (dirname(__FILE__).'/../shared/main_funcs.inc');
 
/**
 * Die Accounter Classe verwaltet Accounttypen (normnal, premium, deluxe ...).
 * 
 * Jede änderung bezieht sich auf den aktuellen Wert von Accounter::username.
 */
 
class Accounter {
	private $username;
	
	/**
	 * Wenn $user ein Int ist, dann ist es die ID, ansonsten der Username
	 */
	function __construct($user) {
		if (is_anint($user)) {	// es handelt sich um die id des Users
			$nick = @mysql_result(query("SELECT nick FROM users WHERE id=$user"),0);
			if (empty($nick))
				return false;
		}
		else {	// es handelt sich, um den Nick des Users
			$nick = $user;
		}
		$this->username = $nick;
	}
	
	/**
	 * Ändert einen Account in $type mit der Länge $duration. $duration ist in MONATEN anzugeben.
	 * Ist $duration 0, so ist endlos gemeint.
	 * Wenn $add_duration==true, dann wird die duration der aktuellen Zeit hinzugefügt, wenn der Account
	 * type bereits gleich ist.
	 */
	public function change_account_type($type, $duration, $add_duration=true) {
		$endtime = ($duration > 5) ? time() + (floor(TIME_ONE_MONTH*12*$duration))/12 : time() + floor(TIME_ONE_MONTH*$duration);	
		if ($this->username === 'all') {
			// alle Verlängern, die schon haben
			$duration_add = floor(TIME_ONE_MONTH * $duration);
			query("UPDATE users SET account_ending_time=account_ending_time+$duration_add WHERE account_type>0 AND account_ending_time>0");

			// allen anderen geben
			query("UPDATE users SET account_type=$type, account_ending_time='$endtime' WHERE account_type=0");
			
			// Forentitel werden hier keine vergeben (zu aufwendig)
		}
		else {
			if ($add_duration) {
				$current_type = mysql_result(query("SELECT account_type FROM users WHERE nick='{$this->username}'"), 0);
				if ($current_type == $type) {
					$this->increase_duration($duration*31);
					return;
				}
			}
			query("UPDATE users SET account_type=$type, account_ending_time='$endtime' WHERE nick='{$this->username}'");
			if (!defined('NO_COMMUNITY')) {
				$com_db = new Db(Db::COM_HOST, Db::COM_NAME, Db::COM_PW, Db::COM_SELECT_DB, false);
		    	$specialTitle = @mysql_result($com_db->query("SELECT specialTitle FROM fwc_registry WHERE username='{$this->username}'"), 0);
			    if ($type > ACCOUNT_NORMAL) {
		    	    if (strpos(strtolower($specialTitle), 'premium') === false) {
		    	    	if (!empty($specialTitle)) {
			    	    	$specialTitle = str_replace(' &', ',', $specialTitle);
			    	    	$specialTitle = str_replace('&', ',', $specialTitle);
			    	    	$specialTitle .= '<br/><img src=\'/forum/extra-title/premium.png\' />';
		    	    	}
		    	    	else 
		    	    		$specialTitle = '<img src=\'/forum/extra-title/premium.png\' />';
		    	    	$com_db->query("UPDATE fwc_registry SET specialTitle=\"$specialTitle\" WHERE username='{$this->username}'");
		    	    }
			    }
			    else {	// normaler Account - Premium User muss weggenommen werden
			    	// es gibt 5 Möglichkeiten, wie Premium User vorkommen kann im Titel
			    	$specialTitle = str_replace(' & Premium User', '', $specialTitle);		// ganz alte Anhängeart
			    	$specialTitle = str_replace(' &<br> Premium User', '', $specialTitle);	// alte Anhängeart
			    	$specialTitle = str_replace('Premium User', '', $specialTitle);			// alle anderen alten Fälle
			    	$specialTitle = str_replace('<br/><img src=\'/forum/extra-title/premium.png\' />', '', $specialTitle);	// Bild angehängt
			    	$specialTitle = str_replace('<img src=\'/forum/extra-title/premium.png\' />', '', $specialTitle);			// alle anderen Fälle mit Bild
			    	$com_db->query("UPDATE fwc_registry SET specialTitle=\"$specialTitle\" WHERE username='{$this->username}'");
			    }
				unset($com_db);
			}
			
			if (User::get_current_user()->get_nick() === $this->username)	{		// wenn man selbst seinen eigenen Account ändert, dann muss
		    	$_SESSION['userdata']['account_type'] = $type;	// die Session angepasst werden (z.B. Gutschein einlösen)
		    	User::get_current_user()->set_account_type(ACCOUNT_NORMAL);
	  			User::get_current_user()->set_account_ending_time(0);	// unendlich
			}
		}
	}
	
	/**
	 * Erhöht die Dauer des Account von $this->username um $duration TAGE. Wird ein negativer Wert angegeben,
	 * so verringert sich die Dauer entsprechend.
	 */
	public function increase_duration($duration) {	
		$duration_add = 60*60*24*($duration);
  		if ($this->username === 'all')
  			query("UPDATE users SET account_ending_time=account_ending_time+$duration_add WHERE account_type>0 AND account_ending_time>0");
  		else
  			query("UPDATE users SET account_ending_time=account_ending_time+$duration_add WHERE nick='{$this->username}' AND account_ending_time>0");
	}
	
	/**
	 * Gibt allen Usern den Account Typ $type (welcher sinnvollerweise mindestens Premium sein sollte) für die
	 * Dauer von $duration TAGEN.
	 * Hier sei nochmal betont, dass nur ein Upgrade der Accounts vorgenommen wird, niemals ein Downgrade.
	 * (kann auch statisch aufgerufen werden)
	 */
	function give_all_account_type($type, $duration) {	
  		$duration_add = 60*60*24*($duration);
  		query("UPDATE users SET account_ending_time=account_ending_time+$duration_add WHERE account_type>=$type AND account_ending_time>0");	// allen premium usern die tage gutschreiben
  		$endtime = time() + $duration_add;
  		query("UPDATE users SET account_ending_time=$endtime, account_type=$type WHERE account_type=0");	// allen anderen die besagte Zeit premium geben
	}
	
	
	/**
	 * Gibt den Namen des Accounts zurück (kann statisch und nicht-statisch aufgerufen werden).
	 * 
	 * @return string
	 */
	function get_account_name($account_type) {
		switch ($account_type) {
			case ACCOUNT_PREMIUM:
				return 'Premium';
			case ACCOUNT_DELUXE:
				return 'Deluxe';
			default:
				return 'Normal';
		}
	}
	 
}

?>