<?php
//if (!defined('_KI_INC_')):
//define('_KI_INC_', 1);

require_once(dirname(__FILE__).'/map/Mapsettings.inc');

/*require_once ('languages.inc');
if (!$language)
	$language = get_language();

include_language_file('KI', $language);*/

define('TRIGGER_PLAYER_ENTERS',1);
define('TRIGGER_HOST_ENTERS',2);
define('TRIGGER_FIGHT_STARTS',3);
define('TRIGGER_FIGHT_ENDS',4);
define('TRIGGER_TIMED_TRIGGER',5);
define('TRIGGER_BUILDING_BUILT',6);
define('TRIGGER_RANDOM_TRIGGER',7);
define('TRIGGER_MOTHER_TRIGGER',8);
define('TRIGGER_NO_CONDITION',0);
define('TRIGGER_CONDITION_MORE_THAN_Y',1);
define('TRIGGER_CONDITION_LESS_THAN_Y',2);
define('TRIGGER_CONDITION_TRIGGER_Y_TRIGGERED',3);
define('TRIGGER_CONDITION_TRIGGER_Y_TRIGGERED_NOT',4);
define('TRIGGER_CONDITION_YND_TRIGGERING',5);
define('TRIGGER_CONDITION_PLAYER_OWNS',6);
define('TRIGGER_CONDITION_HOST_OWNS',7);
define('ACTION_MOVE',1);
define('ACTION_MOVE_ALL_OF_TYPE',2);
define('ACTION_MOVE_ALL',3);
define('ACTION_DROP',4);
define('ACTION_MESSAGE',5);
define('ACTION_TRIGGER',6);


class KI
{
  private $trigger_secs=array();
  private $map_id;
  private $time_factor;

  function KI($mapid,$trigger_secs)
    {
      $this->map_id=$mapid;
      $this->trigger_secs=$trigger_secs;
      $mapsettings=Mapsettings::get_instance();
      $this->time_factor=$mapsettings->get_timefactor();
      //      echo 'initialized KI.';
      //      print_r($_SESSION['creaturemap_computed']['virus_count']);
      //      echo 'trigg_secs:';
      //      print_r($this->trigger_secs);
    }

  private function trigger_trigger($trigger_id,$triggerer,$sector_id,$debugtrig=0)
    { //ACHTUNG die movenden tribes werden auf der map erstmal sowohl als stehend als auch movend angezeigt - ändert sich dann aber.
      global $t_now,$userdata;
      if ($debugtrig)
            echo '<br>TRIGGERED TRIGGER '.$trigger_id;
      $result=query('SELECT * FROM template_actions WHERE template_trigger_id='.$trigger_id.' ORDER BY id');
      while($action=mysql_fetch_assoc($result))
	{
if ($debugtrig)
	  	  echo 'action:'.$action['type'];
	  //	  echo '<br><br>';
	  if (!$action['movetime'])
	    $action['movetime']=rand(TRIGGER_MOVE_TIME_MIN, TRIGGER_MOVE_TIME_MAX);
	  else
	    $action['movetime']=$action['movetime']*60;
	  $action['movetime']=$action['movetime']/$this->time_factor;
	  if ($action['y']==0)
	    foreach($_SESSION['host_tribes'] as $num=>$tribe)
	    {
	      if ($num>8)
		break;
	      $action_units[$num]=$tribe;
	    }
	  else if ($action['y']<100)
	    $action_units=array($_SESSION['host_tribes'][$action['y']-1]);
	  else
	    {
	      $cntx=0;
	      $sqladd='';
	      foreach($_SESSION['players_on_map'] as $plid=>$jnk)
		{
		  if (($plid!='nobody')&&($plid!=USER_HOST_ID))
		    {
		      if ($cntx++<1)
			$sqladd.=' AND (t.uid='.$plid;
		      else
			$sqladd.=' OR t.uid='.$plid;
		    }
		}
	      $result2=query('SELECT t.id FROM tribes as t, tribes_mercenaries as tm, template_mercenaries as m WHERE t.mercenary_id=tm.id'.$sqladd.') AND tm.template_mercenary_id=m.id AND m.id='.($action['y']-100).' ORDER BY t.id DESC');
	      if (mysql_num_rows($result2))
		{
		  $action_units=array(mysql_result($result2,0));
		}
	    }
	  if (!is_array($action_units))
	    $action_units=array();
	  //	  	  echo 'action:';
if ($debugtrig)
	  	  	  print_r($action_units);
/* if ($userdata['id']==2879)
   {
     echo 'debug:';
     print_r($_SESSION['host_tribes']);
     echo '/';
     print_r($action_units);
   }*/

	  switch($action['type'])
	    {
	    case ACTION_MOVE:
	      $cnt=0;
	      $movesec=$this->map_id*100+$action['sector_1'];
	      $targetsec=$this->map_id*100+$action['sector_2'];
	      if ($_SESSION['creaturemap_computed']['tribes_computed'][$action['sector_1']])
		{
		  foreach ($_SESSION['creaturemap_computed']['tribes_computed'][$action['sector_1']] as $trid => $tribe) {
		    //		    if (($tribe['uid'] == USER_HOST_ID)&&(in_array($trid,$action_units))) {
		    if (in_array($trid,$action_units)) {
		      if ($tribe['count']>$action['x']-$cnt)
			$tribe['count']=$action['x']-$cnt;
		      if ($tribe['count']>0)
			{
			  $cnt+=$tribe['count'];
/*			  if (!mysql_num_rows(query('SELECT * FROM sector_tribes WHERE tribe_id='.$trid.' AND count=-'.$tribe['count'].' AND time_placed>'.($t_now-USE_COMPUTATION_DELAY).' AND time_started>'.($t_now-USE_COMPUTATION_DELAY).' AND sector_id='.$movesec)))
			    {
			      query('INSERT INTO sector_tribes set count=-'.$tribe['count'].', time_placed='.$t_now.', time_started='.$t_now.', sector_id='.$movesec.', tribe_id='.$trid);
			      query('INSERT INTO sector_tribes set called_back=2, count='.round($tribe['count']*get_protector_factor($targetsec)).', time_placed='. ($t_now + $action['movetime']).', time_started='.$t_now.', sector_id='.$targetsec.', tribe_id='.$trid.', path=\''. (($action['sector_1'] < 10) ? ('0'.$action['sector_1']) : $action['sector_1']). (($action['sector_2'] < 10) ? ('0'.$action['sector_2']) : $action['sector_2']).'\'');
			      //			      $moving_units[$movesec%100][$trid]['uid']=USER_HOST_ID;
			      //			      $moving_units[$movesec%100][$trid]['count']-=$tribe['count'];
			    }*/
			}
		    }
		  }
		}
	      break;
	    case ACTION_MOVE_ALL_OF_TYPE:
	      if ($debugtrig)
		echo 'MOVE_ALL_OF_TYPE';
	      $movesec=$this->map_id*100+$action['sector_1'];
	      $targetsec=$this->map_id*100+$action['sector_2'];
	      if ($_SESSION['creaturemap_computed']['tribes_computed'][$action['sector_1']])
		{
		  foreach ($_SESSION['creaturemap_computed']['tribes_computed'][$action['sector_1']] as $trid => $tribe) {
		    //		    if (($tribe['uid'] == USER_HOST_ID)&&(in_array($trid,$action_units))) {
		    if (in_array($trid,$action_units)) {
		      $tribe['count']-=$action['x'];
		      if ($tribe['count']>0)
			{
			  /*if (!mysql_num_rows(query('SELECT * FROM sector_tribes WHERE tribe_id='.$trid.' AND count=-'.$tribe['count'].' AND time_placed>'.($t_now-USE_COMPUTATION_DELAY).' AND time_started>'.($t_now-USE_COMPUTATION_DELAY).' AND sector_id='.$movesec)))
			    {
			      query('INSERT INTO sector_tribes set count=-'.$tribe['count'].', time_placed='.$t_now.', time_started='.$t_now.', sector_id='.$movesec.', tribe_id='.$trid);
			      query('INSERT INTO sector_tribes set called_back=2, count='.round($tribe['count']*get_protector_factor($targetsec)).', time_placed='. ($t_now + $action['movetime']).', time_started='.$t_now.', sector_id='.$targetsec.', tribe_id='.$trid.', path=\''. (($action['sector_1'] < 10) ? ('0'.$action['sector_1']) : $action['sector_1']). (($action['sector_2'] < 10) ? ('0'.$action['sector_2']) : $action['sector_2']).'\'');
			      //			      $moving_units[$movesec%100][$trid]['uid']=USER_HOST_ID;
			      //			      $moving_units[$movesec%100][$trid]['count']-=$tribe['count'];
			    }*/
			}
		    }
		  }
		}
	      break;
	    case ACTION_MOVE_ALL:
	      $movesec=$this->map_id*100+$action['sector_1'];
	      $targetsec=$this->map_id*100+$action['sector_2'];
	      if ($_SESSION['creaturemap_computed']['tribes_computed'][$action['sector_1']])
		{
		  foreach ($_SESSION['creaturemap_computed']['tribes_computed'][$action['sector_1']] as $trid => $tribe) {
		    if ($tribe['uid'] == USER_HOST_ID) {
		      if(in_array($trid,$action_units))
			$tribe['count']-=$action['x'];
		      if ($tribe['count']>0)
			{
			  //			  if (!mysql_num_rows(query('SELECT * FROM sector_tribes WHERE tribe_id='.$trid.' AND time_started='.$t_now.' AND sector_id='.$movesec)))
			  /*if (!mysql_num_rows(query('SELECT * FROM sector_tribes WHERE tribe_id='.$trid.' AND count=-'.$tribe['count'].' AND time_placed>'.($t_now-USE_COMPUTATION_DELAY).' AND time_started>'.($t_now-USE_COMPUTATION_DELAY).' AND sector_id='.$movesec)))
			    {
			      query('INSERT INTO sector_tribes set count=-'.$tribe['count'].', time_placed='.$t_now.', time_started='.$t_now.', sector_id='.$movesec.', tribe_id='.$trid);
			      query('INSERT INTO sector_tribes set called_back=2, count='.round($tribe['count']*get_protector_factor($targetsec)).', time_placed='. ($t_now + $action['movetime']).', time_started='.$t_now.', sector_id='.$targetsec.', tribe_id='.$trid.', path=\''. (($action['sector_1'] < 10) ? ('0'.$action['sector_1']) : $action['sector_1']). (($action['sector_2'] < 10) ? ('0'.$action['sector_2']) : $action['sector_2']).'\'');
			      //			      $moving_units[$movesec%100][$trid]['uid']=USER_HOST_ID;
			      //			      $moving_units[$movesec%100][$trid]['count']-=$tribe['count'];
			    }*/
			}
		    }
		  }
		}
	      break;
	    case ACTION_DROP:
	      $targetsec=$this->map_id*100+$action['sector_1'];
//	      drop_host_units($this->map_id,$action['sector_1'],0,1,$action['x']*get_protector_factor($targetsec),1,(($action['y'])?$action_units:0));
		break;
	  	case ACTION_TRIGGER:
		  	$this->trigger_trigger($action['x'],$triggerer,$sector_id);
	      break;

	    case ACTION_MESSAGE:
		$mdata=query('SELECT * FROM custom_messages WHERE mid='.$action['x']);
	      if (mysql_num_rows($mdata))
		{
		  $m=mysql_fetch_assoc($mdata);
		  switch ($m['replace_extra'])
		    {
		    case 2:
		      $replace=$triggerer;
		      break;
		    case 5:
		      $replace=$sector_id;
		      break;
		    case 1: //not implemented ist aber in der Messages schon drin
		    case 3://not implemented ist aber in der Messages schon drin
		    default:
		      $replace=0;
		    }
		  if (($action['y']==1)||($triggerer<=0)) //send to all players ACHTUNG wurde jetzt auch für die situationen eingeführt in denen es keinen triggerer gibt
		    foreach ($_SESSION['players_on_map'] as $plid => $jnk)
		    {
		      if (($plid!='nobody')&&($plid!=USER_HOST_ID))
			eventmsg_map($action['x'],$t_now,$plid,$replace,0);
		    }
		  else if ($action['y']==2) //send to all players but triggerer
		    foreach ($_SESSION['players_on_map'] as $plid => $jnk)
		    {
		      if (($plid!='nobody')&&($plid!=USER_HOST_ID)&&($plid!=$triggerer))
			eventmsg_map($action['x'],$t_now,$plid,$replace,0);
		    }
		  else //send to triggerer only
		    eventmsg_map($action['x'],$t_now,$triggerer,$replace,0);
		  break;
		}
	    }
	}

      query('UPDATE triggers SET triggered=triggered+1,last_triggered='.$t_now.' WHERE map_id='.$this->map_id.' AND template_trigger_id='.$trigger_id);

      //jetzt random triggers suchen die diesen als mother haben
      $result=query('SELECT * FROM template_triggers WHERE type='.TRIGGER_RANDOM_TRIGGER.' AND y='.$trigger_id.' ORDER BY id');
      while($row=mysql_fetch_assoc($result))
	{
	  $sure_trigger=$this->check_trigger($row,$triggerer,$sector_id);
	  if ($sure_trigger>0) //p=1 trigger nachher wird einer davon gewählt
	    $sure_triggers[]=$sure_trigger;
	}
      if (is_array($sure_triggers))
	  $this->trigger_trigger(random_value($sure_triggers),$triggerer,$sector_id);
//jetzt child trigger triggern
	      $result=query('SELECT * FROM template_triggers WHERE type='.TRIGGER_MOTHER_TRIGGER.' AND x='.$trigger_id.' ORDER BY id');
      while($row=mysql_fetch_assoc($result))
	  $this->check_trigger($row,$triggerer,$sector_id);
    }


  function check_trigger($trigger_data,$triggerer,$sector_id)
    {
      global $t_now;
      //                          if ($trigger_data['id']==191)
      //                      	$debugtrig=1;
      if ($debugtrig)
	echo 'CHECKING TRIGGER '.$trigger_data['id'].' UID'.$triggerer;
 //           print_r($trigger_data);
      $result=query('SELECT * FROM triggers WHERE map_id='.$this->map_id.' AND template_trigger_id='.$trigger_data['id']);
      if (!mysql_num_rows($result)) //trigger wurde noch nicht eingefügt -> nachholen
	{
	  query('INSERT INTO triggers SET map_id='.$this->map_id.', template_trigger_id='.$trigger_data['id']);
	  $result=query('SELECT * FROM triggers WHERE map_id='.$this->map_id.' AND template_trigger_id='.$trigger_data['id']);
	}
      $trigger=mysql_fetch_assoc($result);
      if (($trigger_data['max_triggers'])&&($trigger['triggered']>=$trigger_data['max_triggers']))
	return 0;
      $ok=0;
      switch($trigger_data['trigger_condition'])
	{
	case TRIGGER_NO_CONDITION:
      if ($debugtrig)
	  	  echo 'nocond';
	  $ok=1;
	  break;
	case TRIGGER_CONDITION_MORE_THAN_Y:
	  if ($debugtrig){
	  echo 'condmore'.$trigger_data['y'];
	  echo 'sec'.$trigger_data['x'].':';
	  print_r($_SESSION['creaturemap_computed']['virus_count'][$sector_id]);
	  }
	  if ($_SESSION['creaturemap_computed']['virus_count'][$sector_id][$triggerer]>=$trigger_data['y'])
	    $ok=1;
	  break;
	case TRIGGER_CONDITION_LESS_THAN_Y:
	  //	  echo 'condless';
	  if ($_SESSION['creaturemap_computed']['virus_count'][$sector_id][$triggerer]<=$trigger_data['y'])
	    $ok=1;
	  break;
	case TRIGGER_CONDITION_TRIGGER_Y_TRIGGERED:
	        if ($debugtrig)
	  echo 'condtrigger_riggerd';
	  if (@mysql_result(query('SELECT triggered FROM triggers WHERE map_id='.$this->map_id.' AND template_trigger_id='.$trigger_data['y']),0))
	    $ok=1;
	  break;
	case TRIGGER_CONDITION_TRIGGER_Y_TRIGGERED_NOT:
	        if ($debugtrig)
	  echo'condtriggertriggerednot';
	  if (@mysql_result(query('SELECT triggered FROM triggers WHERE map_id='.$this->map_id.' AND template_trigger_id='.$trigger_data['y']),0)==0)
	    $ok=1;
	  break;
	case TRIGGER_CONDITION_YND_TRIGGERING:
	  if ($debugtrig)
	    echo'condynd';
	  if ($trigger['triggered']>=$trigger_data['y']-1)
	    $ok=1;
	  else
	    $increasetrigger=1;
	  break;
	case TRIGGER_CONDITION_PLAYER_OWNS:
	  if ($debugtrig)
	    echo'condplayer';
	  if ($_SESSION['sector_data'][$sector_id]['dominator']!=USER_HOST_ID)
	    $ok=1;
	  break;
	case TRIGGER_CONDITION_HOST_OWNS:
	  if ($debugtrig)
	    echo'condhost';
	  if ($_SESSION['sector_data'][$sector_id]['dominator']==USER_HOST_ID)
	    $ok=1;
	  break;
	}
            if ($debugtrig)
     echo 'AFTER CONDITION:'.$ok.'/increase:'.$increasetrigger;
      switch($trigger_data['type'])
	{
	case TRIGGER_PLAYER_ENTERS:
	        if ($debugtrig)
	  echo 'player'.$triggerer;
	  if (!(($triggerer>0)&&($triggerer!=USER_HOST_ID)))
	    {
	      $ok=0;
	      $increasetrigger=0;
	    }
	  break;

	case TRIGGER_HOST_ENTERS:
	        if ($debugtrig)
	  echo 'host';
	  if ($triggerer!=USER_HOST_ID)
	    {
	      $ok=0;
	      $increasetrigger=0;
	    }

	  break;
	case TRIGGER_FIGHT_STARTS:
	        if ($debugtrig)
	  echo 'fight';
	  if ($triggerer!=-1)
	    {
	      $ok=0;
	      $increasetrigger=0;
	    }
	  break;
	case TRIGGER_FIGHT_ENDS:
	        if ($debugtrig)
	  echo 'fighte';
	  if ($triggerer!=-2)
	    $ok=0;
	  break;
	case TRIGGER_TIMED_TRIGGER:
	        if ($debugtrig)
	  echo 'timed';
	  if ($trigger['last_triggered']>$t_now-$trigger_data['x']*60/$this->time_factor)
	    {
	      $ok=0;
	      $increasetrigger=0;
	    }
	  break;
	case TRIGGER_BUILDING_BUILT:
	        if ($debugtrig)
	  echo 	  'CHECK BUILDING TRIGGER before:'.$ok;
	  if ($triggerer!=-3)
	    {
	      $ok=0;
	      $increasetrigger=0;
	    }
	  break;
	case TRIGGER_RANDOM_TRIGGER:
	  if ($trigger_data['x']>=100)
	    return $trigger_data['id'];
	  $randval=rand(1,100);
	  if ($randval>$trigger_data['x'])
	    {
	      $ok=0;
	      $increasetrigger=0;
	    }
	  break;
	case TRIGGER_MOTHER_TRIGGER:
//always trigger
	  break;
	default:
	        if ($debugtrig)
	  echo 'default';
	  $ok=0;
	  $increasetrigger=0;
	}
            if ($debugtrig)
          echo 'aftertrif:'.$ok;
      if ($ok)
	$this->trigger_trigger($trigger_data['id'],$triggerer,$sector_id,$debugtrig);
      else if ($increasetrigger)
	query('UPDATE triggers SET triggered=triggered+1,last_triggered='.$t_now.' WHERE id='.$trigger['id']);
      return 0;
    }

  function check_triggers() //trigger_secs is 1 wenn user reinkommt u. 2 wenn host reinkommt
    {
      global $t_now;
      $result=query('SELECT t.*,s.sector_id FROM maps AS m, template_triggers AS t, template_maps AS s WHERE t.template_sector_id=s.id AND s.template_id=m.template_id AND m.map_id='.$this->map_id.' ORDER BY t.id');


      while($row=mysql_fetch_assoc($result))
	{
	  if ($row['type']==TRIGGER_TIMED_TRIGGER)
	    $this->check_trigger($row,0,$row['sector_id']);
	  else if ($this->trigger_secs[$row['sector_id']%100])
	    { //trigger this sector
	      //	      echo 'FOUND TRIGGER';//print_r($this->trigger_secs[$row['sector_id']%100]);
	      foreach ($this->trigger_secs[$row['sector_id']%100] as $triggerer)
		$this->check_trigger($row,$triggerer,$row['sector_id']);

	    }
	}
    }
}

//
?>