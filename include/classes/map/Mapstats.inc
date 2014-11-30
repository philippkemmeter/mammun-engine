<?


require_once (dirname(__FILE__).'/Mapsettings.inc');

define('NEXT_STATS',6*3600); //stats alle 6h ACHTUNG AM ANFANG ÖFTER?

define('TOPIC_OVERVIEW',0);//fertig, siehe score
define('TOPIC_SECTORS',1);//fertig
define('TOPIC_BIO',2);//fertig
define('TOPIC_UNITS',3);//fertig//dropped units on map
define('TOPIC_BUILDINGS',4);//fertig
define('TOPIC_SCIENTISTS',5);//scientists at work TODO WIE AUSLESEN?,ausgabe fertig
define('TOPIC_SCORE',6); //fertig
define('TOPIC_DETAILS1',7);//fertig//            [user_units_died][enemy_units_killed]
define('TOPIC_DETAILS2',8);//fertig//            [user_buildings_destroyed][enemy_buildings_destroyed]
//rausgenommen define('TOPIC_DETAILS3',9);//            [user_bio_destroyed][enemy_bio_destroyed]
//rausgenommen define('TOPIC_DETAILS4',10);//           [user_sp_destroyed][enemy_sp_destroyed]
define('TOPIC_ALLIANCES',11);
define('TOPIC_SCIENCES',12); //fertig
define('TOPIC_TRADES',13); //muss nicht computed werden, kommt direkt aus der trade tabelle ABER sicherstellen dass aus der auch nix gelöscht wird



class Mapstats {
  private $map_id;
  private $map_finished;        // ist die map fertig?
  private $user_id;		// der user der die stats aufruft bekommt alle infos über sich selbst, wird 0 wenn nur berechnet wird
  private $time;                //zeit der berechnung

  function __construct($map_id,$user_id=0,$last_time=0) {
    $this->t_now=Time::get_t_now();
    $this->map_id=$map_id;
    $this->user_id=$user_id;
    $this->map_finished=(mysql_result(query('SELECT state FROM maps WHERE map_id='.$this->map_id),0)==2);
    if ($this->map_finished)
      $this->last_time=0;
    else
      $this->last_time=$last_time;
   }

	/**
	 * Computes the current stats and adds the information to the database.
	 */
	function compute_stats($map,$force_compute=0) {
	  $mapsettings=Mapsettings::get_instance();
	  $result=query('SELECT max(time) FROM map_stats WHERE map_id='.$this->map_id);
	  if (mysql_num_rows($result)==0)
	    $last_time=0;
	  else
	    $last_time=mysql_result($result,0);
	  if ((($last_time<$this->t_now-NEXT_STATS/$mapsettings->get_timefactor())&&(!$this->map_finished))||($force_compute)) //ACHTUNG ein letztes mal sollte es aber gehn nach ende der map
	    {
	      if ($force_compute)
		  query('DELETE FROM map_stats WHERE map_id='.$this->map_id.' AND time='.$this->t_now);
	      $mapranking=compute_score($this->map_id*(-1));
	      $players=array(USER_HOST_ID+$this->map_id);
	      $result=query('SELECT user_id FROM players_on_map WHERE map_id='.$this->map_id);
	      while($row=mysql_fetch_assoc($result))
		  $players[]=$row['user_id'];
	      foreach($map->computed_map_data as $player=>$data)
		{
		  if ($player)
		    {
		      query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$player.', '.$this->t_now.', '.TOPIC_DETAILS1.', '.(($data['user_units_died'])?$data['user_units_died']:0).', '.(($data['enemy_units_killed'])?$data['enemy_units_killed']:0).')');
		      query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$player.', '.$this->t_now.', '.TOPIC_DETAILS2.', '.(($data['user_buildings_destroyed'])?$data['user_buildings_destroyed']:0).', '.(($data['enemy_buildings_destroyed'])?$data['enemy_buildings_destroyed']:0).')');
		      if($player<USER_HOST_ID)
			{
			  query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$player.', '.$this->t_now.', '.TOPIC_BIO.', '.round($data['map_bio']).',0)');
			  query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$player.', '.$this->t_now.', '.TOPIC_SCORE.', '.($mapranking[$_SESSION['players_on_map'][$player]['nick']]?$mapranking[$_SESSION['players_on_map'][$player]['nick']]:0).', 0)');
			}
		      //		  query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$player.', '.$this->t_now.', '.TOPIC_DETAILS3.', '.(($data['user_bio_destroyed'])?$data['user_bio_destroyed']:0).', '.(($data['enemy_bio_destroyed'])?$data['enemy_bio_destroyed']:0).')');
		      //		  query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$player.', '.$this->t_now.', '.TOPIC_DETAILS4.', '.(($data['user_sp_destroyed'])?$data['user_sp_destroyed']:0).', '.(($data['enemy_sp_destroyed'])?$data['enemy_sp_destroyed']:0).')');
		    }
		  //ACHTUNG ACHTUNG KAM NOCH NACH RELEASE 1.3		  else errormail('player error in mapstats!');
		}
	      foreach ($map->buildings_computed as $sector=>$buildings)
		{
		  foreach($buildings as $bla=>$data)
		      $all_buildings[$data['uid']]++;
		}
	      foreach($players as $uid)
		{
		  if ($all_buildings[$uid])
		    query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$uid.', '.$this->t_now.', '.TOPIC_BUILDINGS.', '.$all_buildings[$uid].',0)');
		  else
		    query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$uid.', '.$this->t_now.', '.TOPIC_BUILDINGS.', 0,0)');
		}
	      foreach($_SESSION['players_on_map'] as $uid=>$data)
		{
		  if ($uid!='nobody')
		    {
		      query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$uid.', '.$this->t_now.', '.TOPIC_SECTORS.', '.$data['sectors_owned'].',0)');
		      if ((!$data['dead'])&&(!($data['nick']=='Host')))
			{
			  $sciences=mysql_result(query('SELECT sum(cur_level) FROM science WHERE uid='.$uid),0);
			  if (!$sciences)
			    $sciences=0;
			  query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$uid.', '.$this->t_now.', '.TOPIC_SCIENCES.', '.$sciences.',0)');
			}
			  else if ($data['nick']!='Host')
			    query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$uid.', '.$this->t_now.', '.TOPIC_SCIENCES.', 0 ,0)');

		    }
		}
	      foreach ($map->tribes_computed as $sector=>$units)
		{
		  foreach($units as $tribe=>$data)
		    $unitcount[$data['uid']]+=$data['count'];
		}
	      /*foreach($_SESSION['players_on_map'] as $uid=>$junk)
		{
		  if ($uid!='nobody')
		    {
		      $result=query('SELECT sum(s.count) FROM sector_tribes as s,tribes as t WHERE s.tribe_id=t.id AND floor(s.sector_id/100)='.$this->map_id.' AND t.uid='.$uid.' AND s.time_placed>='.$this->t_now);
		      if (mysql_num_rows($result))
			$unitcount[$uid]+=mysql_result($result,0);
		    }
		}*/
	      foreach($players as $uid)
		{
		  if ($unitcount[$uid])
		    query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$uid.', '.$this->t_now.', '.TOPIC_UNITS.', '.$unitcount[$uid].',0)');
		  else
		    query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$uid.', '.$this->t_now.', '.TOPIC_UNITS.', 0,0)');
		}
	      foreach($map->allies as $usr=>$allies)
		{
		  foreach ($allies as $ally=>$junk)
		    {
		      if (!$all[$ally][$usr])
			{
			  query('INSERT INTO map_stats VALUES (0,'.$map->map_data['map_id'].', '.$uid.', '.$this->t_now.', '.TOPIC_ALLIANCES.', '.$usr.', '.$ally.')');
			  $all[$usr][$ally]=1;
			}
		    }
		}
	    }
	}

	/**
	 * Prints statistics for a chosen topic
	 */
	function draw($topic=TOPIC_OVERVIEW,$bg_image='mapstats_background.png') {
	  require_once (dirname(__FILE__).'/../Jpgraph.inc');

	  $colors=array('red','blue','yellow','green','magenta','purple','peru','white','lightslateblue','orchid','salmon','black','sienna','aqua','khaki','orange');

	  if ($topic==TOPIC_OVERVIEW)
	    $topic=TOPIC_SCORE;
	  switch($topic)
	    {
	    default://sectors,bio,units,buildings,sciences
	      $sql='type='.$topic;
	      break;
	    }
	  $result=query('SELECT * FROM map_stats WHERE map_id='.$this->map_id.' AND '.$sql.($this->last_time?' AND time<='.$this->last_time:'').' ORDER BY time,user_id');
	  $datax=array();
	  $maxtime=0;
	  while($row=mysql_fetch_assoc($result))
	    {
	      switch($topic)
		{
		case TOPIC_DETAILS1:
		case TOPIC_DETAILS2:
		  $datay[$row['user_id']][]=$row['value2'];
		  break;
		case TOPIC_SCIENCES:
		  if ($row['value1'])
		  $datay[$row['user_id']][]=$row['value1'];
		  else
		    $datay[$row['user_id']][]=$datay[$row['user_id']][count($datay[$row['user_id']])-1];


		  break;
		default: //sectors,bio,units,buildings,sciences
		  $datay[$row['user_id']][]=$row['value1'];
		}
	      if (!in_array($row['time'],$datax))
		{
		  $datax[]=$row['time'];
		  if ($row['time']>$maxtime)
		    $maxtime=$row['time'];
		}
	    }
	  $lowtime=mysql_result(query('SELECT started FROM maps WHERE map_id='.$this->map_id),0);
	  if ($maxtime-$lowtime<2*3600)
	    {
	      $scaling=60; //scale alle min
	      $scaletext='Minutes';
	    }
	  else	  if ($maxtime-$lowtime<48*3600)
	    {
	      $scaling=3600; //scale alle stunde
	      $scaletext='Hours';
	    }
	  else
	    {
	      $scaling=24*3600;
	      $scaletext='Days';
	    }


	  $scales=$datax;
	  //	  $maxscale=0;
	  foreach($scales as $id=>$time)
	    {
	      $scale=(($time-$lowtime)/$scaling);
	      //	      if (($scale)>$maxscale)
	      //		$maxscale=($scale);
	      $datax[$id]=$scale; //scaling in days
	    }

	  // Some data


	  // A nice graph with anti-aliasing
	  //	  $graph = new Graph(800,553,"auto");
	  $graph = new Graph(600,415,"auto");
	  $graph->img->SetMargin(60,40,40,40);
	  $graph->SetBackgroundImage(pic_path($bg_image),2);

	  $graph->SetScale("linlin");
	  //	  $graph->title->Set($topic);
	  	  $graph->SetTickDensity(TICKD_SPARSE,TICKD_NORMAL);

	  // Use built in font
	  $graph->title->SetFont(FF_FONT1,FS_BOLD);
	  $graph->xaxis->SetFont(FF_FONT1,FS_BOLD);
	  $graph->yaxis->SetFont(FF_FONT1,FS_BOLD);
	  $graph->title->SetColor('#FFCC00');
	  $graph->xaxis->SetColor('black');
	  $graph->ygrid->SetColor('black');
	  $graph->yaxis->SetColor('black');
	  	  	  $graph->xscale->SetIntScale();
	  	  $graph->xscale->SetAutoMin(0);
	  	  	  $graph->yscale->SetIntScale();
	  	  $graph->yscale->SetAutoMin(0);
	  $graph->xaxis->title->Set($scaletext);




	  // Slightly adjust the legend from it's default position in the
	  // top right corner.
	  	  $graph->legend->Pos(0.65,0.2,"right","top");

	  // Create the first line
	  $cnt=0;
	  //	  print_r($datax);
	  if ($datay)
	    {
	      foreach($datay as $userid=>$data)
		{
		  /*		  if (count($datax)>count($data))
		    {
		      $usedata=array_chunk($datax,count($data));
		      $usedata=$usedata[0];
		    }
		  else
		    $usedata=$datax;*/
		  $p[$cnt] = new LinePlot($data,$datax);
		  $p[$cnt]->SetColor($colors[$cnt]);
		  $p[$cnt]->SetWeight(3);
		  if ($userid<USER_HOST_ID)
		    $p[$cnt]->SetLegend(mysql_result(query('SELECT nick FROM users WHERE id='.$userid),0));
		  else
		    $p[$cnt]->SetLegend('Host');
		  //	  print_r($p[$cnt]->GetMax());

		  $graph->Add($p[$cnt++]);
		}


	      // Output line
	      $graph->Stroke();
	    }
	  else
	    echo 'ERROR NO DATA';
	}
}

?>