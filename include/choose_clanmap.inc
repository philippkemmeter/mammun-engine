<?php
//ACHTUNG: beim wechsel/verlassen eines clans muss aus waiting players gelöscht werden
require_once(dirname(__FILE__).'/languages.inc');
require_once(dirname(__FILE__).'/shared/classes/PV_encryption.inc');
require_once(dirname(__FILE__).'/classes/Choosemap.inc');
if ($cryptObj)
	$CryptObj = $cryptObj;
else
	$CryptObj=new PV_encryption();

$CMap=new Choosemap(CLAN_MAP);
$language = get_language();
include_language_file('choose_clanmap', $language);
$expected_data=array('pause'=>'bool',
		     'play_handicapped'=>'bool',
		     'accept_map'=>'bool',
		     'deny_map'=>'bool',
		     'play_normal'=>'bool',
		     'uid'=>'eint',
		     'handicap'=>'int');

	$GPmap = $cryptObj->load_GET_POST($expected_data);

if (($GPmap['accept_map'])||($GPmap['deny_map'])||($GPmap['pause'])||($GPmap['play_handicapped'])||($GPmap['play_normal']))
{
  if (!$CMap->clan_maps_locked())
    {
      if ($GPmap['accept_map'])
	{
	  if ($show_leader)
	    query('UPDATE players_waiting SET accepted=1 WHERE clan_id='.$userdata['clan']);
	  else
	    query('UPDATE players_waiting SET accepted=1 WHERE player_id='.$userdata['id']);
	}
      if ($GPmap['deny_map'])
	{
	  if ($show_leader)
	    query('UPDATE players_waiting SET accepted=0 WHERE clan_id='.$userdata['clan']);
	  else
	    query('UPDATE players_waiting SET accepted=0 WHERE player_id='.$userdata['id']);
	}
      if (($GPmap['play_handicapped'])&&($GPmap['uid']))
	{
	  if ((!$GPmap['handicap'])||($GPmap['handicap']<0))
	    $GPmap['handicap']=0;
	  if ($GPmap['handicap']>usr_level(mysql_result(query('SELECT last_score FROM users WHERE id='.$GPmap['uid']),0)))
	    $GPmap['play_normal']=1;
	  else
	    {
	      query('DELETE FROM players_waiting WHERE player_id='.$GPmap['uid']);
	      query('INSERT INTO players_waiting SET clan_id='.$userdata['clan'].', player_id='.$GPmap['uid'].', handicap='.$GPmap['handicap']);
	    }
	}
      if (($GPmap['pause'])&&($GPmap['uid']))
	{
	  query('DELETE FROM players_waiting WHERE player_id='.$GPmap['uid']);
	}
      if (($GPmap['play_normal'])&&($GPmap['uid']))
	{
	  query('DELETE FROM players_waiting WHERE player_id='.$GPmap['uid']);
	  query('INSERT INTO players_waiting SET clan_id='.$userdata['clan'].', player_id='.$GPmap['uid']);
	}
      if ($CMap->check_clanmap_possible())
	{
	officer(3,LNG_MAP_STARTED);
	die();
	}
      //      else
      //	officer(3,LNG_NOT_YET);
   }
 }


function echoplayer($data)
{
  if ($data['accepted'])
  $fontcol='#00FF00';
  else $fontcol='white';
  echo '<font color='.$fontcol.'>';
  if ($data['handicapped'])
  echo $data['nick'].' (<font color=red>'.$data['level'].'</font><font color='.$fontcol.'>)<br>';
  else
  echo $data['nick'].' ('.$data['level'].')<br>';
}
function echo_constellations()
{
  global $map_data,$CMap;
$players=$CMap->get_clanmap_waiting_players();
  if (is_array($players))
    {
      echo '<center><table cellpadding=5><tr>';
      foreach($players as $clan=>$members)
	{
	  if (count($members)>=$map_data['team_players'])
	    {
	      echo '<td align=center valign=top><b>'.$clan.'</b><br>';
	      $avg[$clan]=0;
	      $counts[$clan]=count($members);
	      $cnt=0;
	      foreach($members as $member)
		{
		  if ($cnt==$map_data['team_players'])
		    {
		      echo '<br>';
		    }
		  echoplayer($member);
		  if ($cnt++<$map_data['team_players'])
		    $avg[$clan]+=$member['level'];
		}
	      echo '</td>';
	    }
	}
      echo '</tr><tr>';
      foreach($players as $clan=>$members)
	{
	  if (count($members)>=$map_data['team_players'])
	    {
	      if ($counts[$clan]>$map_data['team_players'])
		$counts[$clan]=$map_data['team_players'];
	      echo '<td align=center>'.LNG_AVG_LVL.': '.round($avg[$clan]/$counts[$clan]).'</td>';
	    }
	}
      echo '</tr></table><br></center>';
    }
}

$result=query('SELECT id,nick,last_score FROM users WHERE id=id_clanmode AND state=8 AND clan='.$userdata['clan']);
while($row=mysql_fetch_assoc($result))
{
  $waiting_players[$row['id']]['nick']=$row['nick'];
  $waiting_players[$row['id']]['level']=usr_level($row['last_score']);
}
  
$result=query('SELECT * FROM players_waiting WHERE clan_id='.$userdata['clan'].' ORDER BY accepted DESC,id');
while($row=mysql_fetch_assoc($result))
{
  if ($row['handicap']>-1)
    {
      $row['handicapped']=1;
      $waiting_players[$row['player_id']]['level']=$row['handicap'];
    }
  $row['nick']=$waiting_players[$row['player_id']]['nick'];
  $row['level']=$waiting_players[$row['player_id']]['level'];
  if ($row['nick'])
    $selected_players[$row['player_id']]=$row;
  unset($waiting_players[$row['player_id']]);
}

$map_data=$CMap->get_next_clanmap_data();
  $clanmap_info=LNG_NEXT_CLANMAP.': '.$map_data['num_players'].' '.LNG_PLAYERS.', '.$map_data['teams'].' '.LNG_TEAMS.' (';
  for ($i=0;$i<$map_data['teams'];$i++)
    {
      $clanmap_info.=$map_data['team_players'];
      if ($i<$map_data['teams']-1)
	$clanmap_info.='vs';
    }
	$clanmap_info.=')';
$clanmap_info.=', '.LNG_CLANMAP_CHANGE.': '.secInTime(CLANMAP_CHANGE_CYCLE-($t_now-$map_data['last_mapchange'])).'.';
include ('../game/templates/choose_clanmap.tmpl.inc');
?>
