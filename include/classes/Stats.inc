<?


require_once(dirname(__FILE__).'/Science.inc');
require_once(dirname(__FILE__).'/Buildings.inc');
require_once(dirname(__FILE__).'/../consts.inc');
require_once(dirname(__FILE__).'/../gen_funcs.inc');
/**
 * Stats-Class berechnet die Statistiken eines bestimmten Tribes in einem Sektor oder
 * aller Tribes eines Sektors eines Users
 */

require_once(dirname(__FILE__).'/../languages.inc');
if (!$language)
	$language = get_language();

include_language_file('Stats', $language);


class Stats {
	public $sector_id;
	public $sObj;
	public $bObj;
	public $uid;

	public $error;

	const ERR_NO_ERROR = 0;
	const ERR_BUILDING_ALREADY_DESTROYED = 1;
	const ERR_TRIBE_ALREADY_DEAD = 2;
	const ERR_OWNER_MISSMATCH = 3;

	const STAT_STYLE_NORMAL = 0;
	const STAT_STYLE_VERTICAL = 1;
	const STAT_STYLE_TABLE = 2;

	public function __construct($uid, $sector_id) {
		$this->sector_id = $sector_id % 100;
		$this->uid = $uid;
		$this->sObj = Science::get_instance();
		$this->bObj = Buildings::get_instance($uid);
		$this->error = self::ERR_NO_ERROR;
	}

	public function __destruct() {}

	public function get_stats_of_tribe($tribe_id) {
	  global $t_now;
		if ($tribe_id > BUILDING_TRIBE_ID)
			$creaturemap = new mapview(31, TIME_SLOW_MAPVIEW);

		if (($tribe_id > BUILDING_TRIBE_ID) && (!isset ($_SESSION['creaturemap_computed']['buildings_computed'][$this->sector_id][$tribe_id -BUILDING_TRIBE_ID]))) {
			$error = self::ERR_BUILDING_ALREADY_DESTROYED;
			return false;
		}
		if (($tribe_id < BUILDING_TRIBE_ID) && (!isset ($_SESSION['creaturemap_computed']['tribes_computed'][$this->sector_id][$tribe_id]))) {
			$error = self::ERR_TRIBE_ALREADY_DEAD;
			return false;
		}
		if ((($tribe_id > BUILDING_TRIBE_ID) && ($_SESSION['creaturemap_computed']['buildings_computed'][$this->sector_id][$tribe_id -BUILDING_TRIBE_ID]['uid'] != $this->uid)) || (($tribe_id < BUILDING_TRIBE_ID) && ($_SESSION['creaturemap_computed']['tribes_computed'][$this->sector_id][$tribe_id]['uid'] != $this->uid))) {
			$error = self::ERR_OWNER_MISSMATCH;
			return false;
		}
		$virus_count = get_allied_virus_count_in_sec($this->sector_id) + get_friendly_virus_count_in_sec($this->sector_id);
		$enemy_count = get_enemy_virus_count_in_sec($this->sector_id);

		$boons = '';
		$attplus = 0;
		$defplus = 0;
		if (is_array($_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid])) {
			$data = $_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid];
			$attplus += round($data['att'] * 100) / 100;
			$defplus += round($data['def'] * 100) / 100;
			$boons .= $this->sObj->science_name($data['type']) . ' [';
			if ($data['uid'] != $this->uid)
				$boons .= LNG_BY.' ' . $_SESSION['players_on_map'][$data['uid']]['nick'] . ', ';
			$boons .= ceil($data['percent'] * 100) . '%]: ';

			if ($tribe_id > BUILDING_TRIBE_ID) {
				if ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['att'])
					$boons .= (($data['att']) ? 'Att+: ' . (round($data['att'] * 100) / 100) . ' ' : '') . (($data['def']) ? 'Def+: ' . (round($data['def'] * 100) / 100) : '') . '<br>';
				else {
					$attplus = 0;
					$boons .= (($data['att']) ? 'Att+: NA ' : '') . (($data['def']) ? 'Def+: ' . (round($data['def'] * 100) / 100) : '') . '<br>';
				}
			} else
				$boons .= (($data['att']) ? 'Att+: ' . (round($data['att'] * 100) / 100) . ' ' : '') . (($data['def']) ? 'Def+: ' . (round($data['def'] * 100) / 100) : '') . '<br>';

		}
		if ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['boon'] == 1) {
			$swarm = (($this->sObj->get_science_level(15) * 0.0001 + 0.0002) * $virus_count);
			if ($swarm > ($this->sObj->get_science_level(15) + 1) * 2)
				$swarm = ($this->sObj->get_science_level(15) + 1) * 2;
			$multiply = 0.4 + $virus_count / ($virus_count + $enemy_count) * 0.5 + $this->sObj->get_science_level(15) * 0.1;
			$multiply = sqrt($multiply);

			$before = $_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['att'];
			$after = $before;
			if ($multiply > 1) {
				$after += $swarm;
				$after = $after * $multiply;
			} else {
				$swarm = $swarm * $multiply;
				$after += $swarm;
			}

			$boons .= LNG_SWARM_ATTACK . ': Att+: ' . (round(($after - $before) * 100) / 100) . '<br>';
			$attplus += (round(($after - $before) * 100) / 100);
		}
		elseif ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['boon'] == 2) {
			$boons .= LNG_SLIME_FUSION . ': ';
			if ($_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid]) {
				$attplus += (round($_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid]['att'] * ($this->sObj->get_science_level(16) + 1.5) * 100 / 3) / 100);
				$defplus += (round($_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid]['def'] * ($this->sObj->get_science_level(16) + 1.5) * 100 / 3) / 100);
				$boons .= (($_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid]['att'])
					? 'Att+: ' . (round($_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid]['att'] * ($this->sObj->get_science_level(16) + 1.5) * 100 / 3) / 100) . ', '
					: '');
				$boons .= (($_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid]['def'])
					? 'Def+: ' . (round($_SESSION['creaturemap_computed']['actual_slime'][$this->sector_id][$this->uid]['def'] * ($this->sObj->get_science_level(16) + 1.5) * 100 / 3) / 100)
					: '') . '<br>';
			} else
				$boons .= LNG_NO_SLIME . '<br>';
		}
		elseif (($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['att']) && ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['boon'] == 3)) {
			$boons .= LNG_CLOUD_OF_SPORES . ': ';
			if ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['type'] == 2) //small fungus
				$att = pow($enemy_count, 0.4 + $this->sObj->get_science_level(17) * 0.015) / 3;
			elseif ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['type'] == 3) //advanced fungus
				$att = pow($enemy_count, 0.4 + $this->sObj->get_science_level(17) * 0.02) / 3;
			elseif ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['type'] == 4) //big fungus
				$att = pow($enemy_count, 0.4 + $this->sObj->get_science_level(17) * 0.025) / 3;
			elseif ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['type'] == 5) //mother fungus
				$att = pow($enemy_count, 0.4 + $this->sObj->get_science_level(17) * 0.03) / 3;
			$attplus += round($att * 100) / 100;
			$boons .= 'Att+: ' . (round($att * 100) / 100) . ' (' . round($enemy_count) . ' ' . LNG_ENEMIES_IN_SECTOR . ')<br>';
		}
		if ($tribe_id > BUILDING_TRIBE_ID) {
			$current_stats = array (
				died => $_SESSION['creaturemap_computed']['buildings_computed'][$this->sector_id][$tribe_id - BUILDING_TRIBE_ID]['died'],
				killed => $_SESSION['creaturemap_computed']['buildings_computed'][$this->sector_id][$tribe_id - BUILDING_TRIBE_ID]['killed'],
				eaten => $_SESSION['creaturemap_computed']['buildings_computed'][$this->sector_id][$tribe_id - BUILDING_TRIBE_ID]['eaten'],
				absorbed => 0
			);
			//$hp = ceil($_SESSION['creaturemap_computed']['buildings_computed'][$this->sector_id][$tribe_id - BUILDING_TRIBE_ID]['hitpoints']);
			//$is_building = true;
		}
		elseif ($tribe_id < BUILDING_TRIBE_ID) {
			//$is_building = false;
			$current_stats = array (
				died => $_SESSION['creaturemap_computed']['tribes_computed'][$this->sector_id][$tribe_id]['died'],
				killed => $_SESSION['creaturemap_computed']['tribes_computed'][$this->sector_id][$tribe_id]['killed'],
				eaten => $_SESSION['creaturemap_computed']['tribes_computed'][$this->sector_id][$tribe_id]['eaten'],
				absorbed => (($_SESSION['creaturemap_computed'][tribe_field][$tribe_id]['race'] == 3016	) ? 0 : $_SESSION['creaturemap_computed']['tribes_computed'][$this->sector_id][$tribe_id]['absorbed']));
			/*$hp = $_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['hp'];
			$alive = $_SESSION['creaturemap_computed']['tribes_computed'][$this->sector_id][$tribe_id]['count'];*/
		}
		/*$att = $_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['att'] + $attplus;
		$def = $_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['def'] + $defplus;*/

		if ($tribe_id > BUILDING_TRIBE_ID) {
			if (($this->sObj->get_science_level(20)) && ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['att']))
				$boons .= LNG_CONTAMINATED_SPORES_BOON . '<br>';
			$spezi=mysql_fetch_assoc(query('SELECT spezi,spezi_finished FROM building WHERE id='.($tribe_id-BUILDING_TRIBE_ID)));
			if (($spezi['spezi_finished'])&&($spezi['spezi_finished']<$t_now))
			  {
			    switch($spezi['spezi'])
			      {
			      case 1:
				$boons.=LNG_V_ATT.'<br>';break;
			      case 2:
				$boons.=LNG_V_DEF.'<br>';break;
			      case 3:
				$boons.=LNG_V_HUNT.'<br>';break;
			      case 4:
				$boons.=LNG_CLOAK.'<br>';break;
			      case 5:
				$boons.=LNG_BC.'<br>';break;
			      case 6:
				$boons.=LNG_DO.'<br>';break;
			      }
			  }
			$boons.='<br>';
		}
		elseif ($tribe_id < BUILDING_TRIBE_ID) {
			$boons.='<br>';
			if (($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3005) || ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3006) || ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3021))
				$boons .= LNG_DEFENDER_BOON .'<br>';
			elseif (($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3003) || ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3004) || ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3020))
				$boons .= LNG_ATTACKER_BOON .'<br>';
			elseif (($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3064) || ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3065) || ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3066))
				$boons .= LNG_HUNTER_BOON .'<br>';
			if ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3055)
				$boons .= LNG_CRUSHER_BOON .'<br>';
			if ($this->sObj->get_science_level(21))
				$boons .= LNG_CONTAMINATED_HULL_BOON .'<br>';
			for ($i = 0; $i < 5; $i++) {
				if ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['ability_' . $i] == 3060)
					$boons .= LNG_POISONOUS_TENTACLES_BOON .
					'<br>';
				if ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['ability_' . $i] > ITEM_SCIENCE_ID) { //copy paste aus artifact name
					$sec_id = mysql_result(query('SELECT sector_id FROM artifacts WHERE artifact_id=' . $_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['ability_' . $i] . ' LIMIT 1'), 0);
					$data = mysql_fetch_assoc(query('SELECT name,uid FROM tribes WHERE id=' . ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['ability_' . $i] - ITEM_SCIENCE_ID)));
					if ($data['uid'] == USER_HOST_ID)
						$data['nick'] = LNG_HOST;
					else
						$data['nick'] = mysql_result(query('SELECT nick FROM users WHERE id=' .
						$data['uid']), 0);
					$map_name = mysql_result(query('SELECT name from worlds where id=' . floor(($sec_id % NUM_HOSTS) / 100)), 0);
					$boons .= LNG_GENOM_BOON . ' ' . $data['name'] . ' [' . $data['nick'] . ', ' . $map_name . ' ' . LNG_MAP . ' ' . ($sec_id % 100) . ']<br>';
				}
			}
			if ($_SESSION['creaturemap_computed']['tribe_field'][$tribe_id]['race'] == 3019)
				$boons .= LNG_BODYGUARD_BOON .'<br>';
			elseif (($enemy_count > 0) && ($_SESSION['creaturemap_computed']['tribes_computed'][$this->sector_id][$tribe_id]['count'] / $enemy_count < 0.01))
				$boons .= '<font color=red>' .	LNG_STAMPEDING . '</font><br>';
		}
		return array (
			'current_stats'	=> $current_stats,
			'attplus' 		=> $attplus,
			'defplus' 		=> $defplus,
			'boons'			=> $boons,
			);
	}

	/*****************************************************************************
		printed die stats (killed, died eaten, absorbed) aus
		data ist ein array wo alles drin ist
	*****************************************************************************/
	static public function print_stats($data, $stat_style=self::STAT_STYLE_NORMAL,$tribe_id=0,$sector_id=0) {
	  global $t_now,$userdata;
		if ((!$data['died'])&&(!$data['killed'])) {
			echo '<font color=red>'.LNG_NO_ACTIVITY_UP_TO_NOW.'</font>';
			if ($stat_style == self::STAT_STYLE_VERTICAL || $stat_style == self::STAT_STYLE_TABLE) {
				echo'<br><br><br>';
			}
		}
		else {
		  if ($tribe_id)
		    {
		      $last_computes=query('SELECT compute_id,time_computed FROM computed_maps WHERE map_id='.$userdata['map_id'].' ORDER BY time_computed DESC LIMIT 2');
		      while ($row=mysql_fetch_assoc($last_computes))
			  {
			    if ($tribe_id<BUILDING_TRIBE_ID)
			    $kd=mysql_fetch_assoc(query('SELECT killed,died FROM computed_tribes WHERE compute_id='.$row['id'].' AND tribe_id='.$tribe_id.' AND sector_id='.$sector_id));
else
			    $kd=mysql_fetch_assoc(query('SELECT killed,died FROM computed_buildings WHERE compute_id='.$row['id'].' AND building_id='.($tribe_id-BUILDING_TRIBE_ID)));
			    $computes[]=array('time_computed'=>$row['time_computed'],'killed'=>$kd['killed'],'died'=>$kd['died']);
			  }
			     $killed_diff=$computes[0]['killed']-$computes[1]['killed'];
			     $died_diff=$computes[0]['died']-$computes[1]['died'];

		      if (($killed_diff<0)||($died_diff<0))
			$tribe_id=0;

		    }
			$floor_died=floor($data['died']);
			$floor_killed=floor($data['killed']);
			switch ($stat_style) {
				case self::STAT_STYLE_VERTICAL:
					echo '['.LNG_UNITS_DIED.': <font color=red>'.($data['died']?($floor_died?$floor_died:'<1'):'0').'</font>]<br>';
					echo '['.LNG_ENEMIES_KILLED.': <font color=red>'.($data['killed']?($floor_killed?$floor_killed:'<1'):'0').'</font>]<br>';
					echo '['.LNG_SUCCESS.': <font color=red>'.successmeter($data['killed'],$data['died']).'</font>]<br>';
					break;
				case self::STAT_STYLE_TABLE:
					echo '<table border=0 cellpadding=0 cellspacing=0>
					<tr>
					 <td>['.LNG_UNITS_DIED.': </td><td align=right><font color=red>'.($data['died']?($floor_died?$floor_died:'<1'):'0').'</font>]</td>
					</tr><tr>
					 <td>['.LNG_ENEMIES_KILLED.': </td><td align=right><font color=red>'.($data['killed']?($floor_killed?$floor_killed:'<1'):'0').'</font>]</td>
					</tr><tr>
					 <td>['.LNG_SUCCESS.': </td><td align=right><font color=red>'.successmeter($data['killed'],$data['died']).'</font>]</td>
					</tr>
					</table>';
					break;
				default:
				  //					echo '['.LNG_UNITS_DIED.': <font color=red>'.($data['died']?($floor_died?$floor_died:'<1'):'0').'</font>] ['.LNG_ENEMIES_KILLED.': <font color=red>'.($data['killed']?($floor_killed?$floor_killed:'<1'):'0').'</font>]<br>['.LNG_SUCCESS.': <font color=red>'.successmeter($data['killed'],$data['died']).'</font>]'.((($tribe_id)&&($killed_diff>0))?' ['.LNG_CURRENT_STATS.' '.LNG_LAST.' '.secInTime($t_now-$computes[1]['time_computed']).': <font color=red>'.successmeter($killed_diff,$died_diff).'</font>]':'').'<br>';
				  echo '['.LNG_UNITS_DIED.': <font color=red>'.($data['died']?($floor_died?$floor_died:'<1'):'0').'</font>] ['.LNG_ENEMIES_KILLED.': <font color=red>'.($data['killed']?($floor_killed?$floor_killed:'<1'):'0').'</font>]<br>['.LNG_AVG_SUCCESS.': <font color=red>'.successmeter($data['killed'],$data['died']).'</font>]'.((($tribe_id)&&($killed_diff>0))?' ['.LNG_CURRENT_STATS.': <font color=red>'.successmeter($killed_diff,$died_diff).'</font>]':'').'<br>';
			}
		}
	}
}


?>