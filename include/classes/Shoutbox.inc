<?php


/*
 * Created on Dec 14, 2006 by Philipp Kemmeter
 */

/**
 * Klasse repräsentiert eine Shoutbox. Speichert die Daten in die Tabelle "chat". Über die Ids sind die
 * verschiedenen Boxen unterscheidbar.
 * Im moment gibt es daher eine Mapshoutbox (ffa und clan) sowie eine Wirtsshoutbox. Clanpageshoutboxes
 * wären auch problemlos denkbar uvm.
 */
class Shoutbox {
	private $shoutbox_id;	// Die Id der Shoutbox. z.B. die Map_id oder die World_id
	
	const ERR_MSG_TOO_LONG = -1;
	const ERR_NO_MESSAGE_ENTERTED = -2;
	const ERR_SAME_MSG_THIS_DAY = -3;

	public function __construct($shoutbox_id) {
		$this->shoutbox_id = $shoutbox_id;
	}
	
	/**
	 * Gibt die Nachrichten zurück der Shoutbox zurück.
	 * Durch Angabe von $clan_id kann nach clan_id zusätzlich gefiltert werden.
	 */
	public function get_messages($clan_id=0, $max_messages=0, $state=0) {
		$sql = 'SELECT c.*, u.nick FROM chat as c, users as u  WHERE c.poster_id=u.id AND c.map_id='.$this->shoutbox_id.' AND c.clan_id='.$clan_id;
		/* is raus, es können ruhig alle angezeigt werden		if ($state) {
			if (is_array($state)) {
				$s = array_pop($state);
				$sql .= " AND (u.state=$s OR u.waiting_state=$s";	
			
				foreach ($state as $s)
					$sql .= " OR u.state=$s OR u.waiting_state=$s";
				$sql.= ')';
			}
			else
				$sql .= " AND u.state=$state OR u.waiting_state=$state";
		}*/
		$sql .= ' ORDER BY c.post_time DESC';

		if ($max_messages > 0)
			$sql .= " LIMIT $max_messages";

		$result=query ($sql);	
		if (mysql_num_rows($result) == 0)
			return false;


	 	$return_table = array();
	 	while ($row = mysql_fetch_assoc($result))
	 		$return_table[] = $row;
		return $return_table;
	}
	
	/**
	 * Speichert einen post.
	 */
	public function post_message($poster_uid, $msg, $clan_id=0) {
		if (strlen($msg) > 2000)
			return self::ERR_MSG_TOO_LONG;
		
		if (!strlen(str_replace(array ('&#32;','&nbsp;',"\n","\r"), array ('','','',''), $msg)))
			return self::ERR_NO_MESSAGE_ENTERTED;

		/* mehrere Zeilenumbrüche in Folge zu einem verkürzen */

		$msg = str_replace("\r\n", '<br>', $msg);
		while (str_replace("<br><br>", "<br>", $msg) != $msg)
			$msg = str_replace("<br><br>", "<br>", $msg);

		$result = query('SELECT post_time FROM chat WHERE map_id=' . $this->shoutbox_id . ' AND poster_id=' . $poster_uid . ' AND clan_id=' . $clan_id . ' AND text=\'' . $msg . '\'');
		if ((mysql_num_rows($result)) && (mysql_result($result, 0) > time() -3600 * 24))
			return self::ERR_SAME_MSG_THIS_DAY;
		
		query('INSERT INTO chat SET map_id=' .	$this->shoutbox_id. ', poster_id=' . $poster_uid . ', clan_id=' . $clan_id . ', post_time=' . time() . ', text=\'' . $msg . '\'');
		return true;
	}
}


?>