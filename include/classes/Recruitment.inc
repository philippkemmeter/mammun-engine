<?php

/*
 * Created on Apr 5, 2007 by Philipp Kemmeter
 */
 
require_once(dirname(__FILE__).'/Db.inc');

 
class Recruitment {
	private $uid;
	private $num_recruitments;	// wie viele bereits rekrutiert wurden
	
	//const HONOR_PER_RECRUITMENT = 100;
	
	function __construct($uid) {
		$this->uid = $uid;
		$this->num_recruitments = mysql_result(query("SELECT COUNT(id)/2 FROM users WHERE recruited_by=$uid"),0);
	}
	
	/**
	 * Einen User zu den Rekrutierten hinzufügen
	 * $user kann entweder die User-Id oder der Nick sein
	 * Bei der Id wird Clan mit FFa gesynct
	 */
	public function count_recruitment($user) {
		if (((int)$user) == $user) {	// es handelt sich um die id des Users
			$nick = @mysql_result(query("SELECT nick WHERE id=$user"),0);
			if (empty($nick))
				return false;
		}
		else {	// es handelt sich, um den Nick des Users
			$nick = $user;
		}
		
		query("UPDATE users SET recruited_by={$this->uid} WHERE nick='$nick'");
		$this->num_recruitments++;
		
		/*$H = new Honorer($this->uid);
		$H->add_honor(self::HONOR_PER_RECRUITMENT);
		unset($H);*/
	}
}

?>