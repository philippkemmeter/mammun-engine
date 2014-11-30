<?php

/*
tablefileds= swapping_map, swapping_user in server
swapped in users
swapped,cleaned in maps
*/

//TODO tribes tabelle ordentlich cleanen
class DBCleaner
{
	const SWAPPING_TIME=TIME_TWO_WEEKS; //zeit nach der user rausgeswappt werden

	public function DBCleaner()
	{
		if (!(rush_hour()))
		{
			$result=query('SELECT * FROM server');
			$row=mysql_fetch_assoc($result);
			if ((!$row['swapping_map'])&&(!$row['swapping_user']))
				$this->clean_db();
		}
	}

	private function clean_finished_map($map_id)
	{
		if (!defined('CHRIS_PRIVATE_SERVER'))
			return false;

		echo 'CLEANING FINISHED MAP'.$map_id;
		query('INSERT INTO finished_sectors SELECT sector_id,template_sector_id,owner FROM sectors WHERE floor(sector_id/100)='.$map_id);

		/* Dominatoren der Karte */
		$host_data = mysql_fetch_assoc(query('SELECT * FROM worlds WHERE id='.((int)($map_id / 100))));
		$compute_id = @mysql_result(query("SELECT compute_id FROM computed_maps WHERE map_id=$map_id AND insert_finished=1 ORDER BY time_computed DESC"),0);
		if ($compute_id) {
			$res = query("SELECT t.uid, s.* FROM computed_tribes AS s, tribes AS t WHERE s.tribe_id=t.id AND s.compute_id=$compute_id ORDER BY t.uid");
			while ($row = mysql_fetch_assoc($res))
		  		$sectors[$row['sector_id']%100]['players'][$row['uid']] += $row['count'];

			$res = query("SELECT b.uid, b.sectorID, s.* FROM computed_buildings AS s, building AS b WHERE s.building_id=b.id AND s.compute_id=$compute_id ORDER BY b.uid");
			while ($row = mysql_fetch_assoc($res))
			  $sectors[$row['sectorID']%100]['players'][$row['uid']] += $row['hitpoints'];
		}
		for ($i = 0; $i < 100; $i++) {
			if (is_array($sectors[$i]['players'])) {
				$temp_count = 0;
				foreach ($sectors[$i]['players'] as $usr => $count) {
					if ($count > $temp_count) {
						$temp_count = $count;
						if ($usr >= USER_HOST_ID)
							$sector[$i]['dominator'] = USER_HOST_ID;
						else {
							if ($host_data['type'] == CLAN_MAP) {	// dann speichern wir den dominierenden Clan
								$sector[$i]['dominator'] = @mysql_result(query("SELECT c.id FROM clans AS c, users AS u WHERE u.id=$usr AND u.clan=c.id"),0);
							}
							else {	// sonst den dominierenden Spieler
								$sector[$i]['dominator'] = $usr;
							}
						}
					}
				}
			}
		}
		for ($i = 0; $i < 100; $i++) {
			if ($sector[$i]['dominator'])
				query('UPDATE finished_sectors SET owner='.$sector[$i]['dominator'].' WHERE sector_id='.($map_id*100+$i));
		}

		//now delete everything...
	}

	private function swap_user($user_id){
		if (defined('CHRIS_PRIVATE_SERVER'))
			echo 'SWAPPING USER'.$user_id;
		return 1;
	}

	private function swap_map($map_id)
	{
		if (defined('CHRIS_PRIVATE_SERVER'))
			echo 'SWAPPING MAP'.$map_id;
	}

	private function check_map($map_id)
	{
		$t_now = Time::get_t_now();
		if (mysql_result(query('SELECT MAX(u.last_login) FROM users as u, players_on_map as p where p.user_id=u.id AND p.map_id='.$map_id),0)>$t_now-self::SWAPPING_TIME)
		{
			$this->swap_map($map_id);
			return 1;
		}
		else
			return 0;
	}

	private function clean_db_in_general()
	{
		if (defined('CHRIS_PRIVATE_SERVER'))
			echo 'GENERALLY CLEANING';
	}

	private function clean_db()
	{
		$t_now = Time::get_t_now();
		$result=query('SELECT map_id FROM maps WHERE finished<'.($t_now-24*3600).' LIMIT 1');
		while ($row=mysql_fetch_assoc($result))
			$this->clean_finished_map($row['map_id']);
		$result=query('SELECT id,homesector,state FROM users WHERE id=id_ffa AND last_login<'.($t_now-self::SWAPPING_TIME).' ORDER BY last_login');
		$cleaned_one=0;
		while(($row=mysql_fetch_assoc($result))&&(!$cleaned_one))
		{
			if (defined('CHRIS_PRIVATE_SERVER'))
				echo '<br>checking user:';print_r($row);
			if ((($row['state']==2)||($row['state']==3)||($row['state']==6))&&($row['homesector']<10))
				$cleaned_one=$this->swap_user($row['id']);
			else if ($row['homesector']>99)
				$cleaned_one=$this->check_map(floor($row['homesector']/100));
			else
				errormail('Strange state/homesector check this and add a swapping cause', 'Hellmaker', 'DBCleaner, strange state/HS');
		}
		if (!$cleaned_one)
			$this->clean_db_in_general();
	}
}

?>