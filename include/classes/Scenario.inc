<?


require_once(dirname(__FILE__).'/Buildings.inc');
require_once(dirname(__FILE__).'/../gen_funcs.inc');

/*define ("SC_V_ARTEFACT", 1);
define ("SC_V_DESTROY_BUILDING", 2); */

define ("SC_WORLD_ID", 8);

define ("SC_UID", 1);

define ("SC_ERR_NONE", 0);
define ("SC_ERR_NO_FREE_MAPS", 1);

class Scenario {
  public $error;
  public $which;
  public $v_cond;
  public $v_sector;
  public $map_id;
  private $t_now;

  function Scenario($which) {
    $this->error = SC_ERR_NONE;
    $this->which = $which;
    $this->t_now = time();
    $home = $this->create_map();
    if ($home === false)
      $this->error = SC_ERR_NO_FREE_MAPS;
    $this->build_scenario();
  }

  function insert_tribe($name, $att, $def, $hp, $speed, $race, $ability_1, $ability_0, $pic_url) {
    if ($ability_0) {
      $max_lev = mysql_result(query('SELECT level from sciences WHERE sc_id='.$ability_0.' ORDER BY level DESC LIMIT 1'),0);
      if ($max_lev > 1)
	$max_lev--;
      $values = Science::get_science_values($ability_0, $max_lev); // => gen_funcs.inc
      $att += $values['attack'];
      $def += $values['defend'];
      $hp  += $values['hitpoints'];
    }
    else
      $ability_0 = 0;

    if ($ability_1) {
      $max_lev = mysql_result(query('SELECT level from sciences WHERE sc_id='.$ability_1.' ORDER BY level DESC LIMIT 1'),0);
      if ($max_lev > 1)
	$max_lev--;
      $values = Science::get_science_values($ability_1, $max_lev); // => gen_funcs.inc
      $att += $values['attack'];
      $def += $values['defend'];
      $hp  += $values['hitpoints'];
    }
    else
      $ability_1 = 0;

    query("INSERT INTO tribes SET pic_url='$pic_url', name='$name', uid='".SC_UID."', speed='$speed', race='$race', cost=1, hp='$hp', att='$att', def='$def', ability_0='$ability_0', ability_1='$ability_1'");
    return mysql_insert_id();
  }

  function put_units ($tribe_id, $count, $sector) {
    $drop_time = $this->t_now + 60;
    $real_sector = ($sector < 100) ? $this->map_id*100 + $sector : $sector;

    //TODOquery('INSERT INTO sector_tribes SET time_placed='.$drop_time.', sector_id='.$real_sector.', tribe_id='.$tribe_id.', count='.$count);
    return 0;//mysql_insert_id();
  }


  function put_building($type, $sector, $level=0, $x=0, $y=0) {
    if ($sector < 100)
      $sector = $this->map_id*100 + $sector;
    if (!$x || !$y) {
      $my_b = Buildings::get_instance(SC_UID);
      $sizes = $my_b->get_b_sizes();
      $size = $sizes[$type];
      for ($x = 0; $x < 9; $x++)
        for ($y = 0; $y < 9; $y++)
	  if ($my_b->can_add($x, $y, $size, $size, $sector, true))
            break 2;	// Doppelte for-Schleife verlassen
      unset($my_b);
    }
    $t_now = Time::get_t_now();
    query("INSERT INTO building SET uid='".SC_UID."', type='$type', cur_level='$level', ulc='".$x.$y."', sectorID='$sector', time_built='$t_now', finish_time='$t_now', time_build_started='$t_now'");
    return mysql_insert_id();
  }

  function set_v_cond($v_cond, $v_sector) {
    query ("UPDATE scenarios SET v_cond='$v_cond', v_sector='$v_sector' WHERE id='{$this->id}'");
    $this->v_cond = $v_cond;
    $this->v_sector = $v_sector;
  }

  function get_v_cond() {
    if ($this->v_cond && $this->v_sector)
      return array(0 => $this->v_cond, 'v_cond' => $this->v_cond, 1 => $this->v_sector, 'v_sector' => $this->v_sector);
    else {
      $result = mysql_fetch_array(query("SELECT v_cond, v_sector FROM scenarios WHERE id='{$this->id}'"));
      $this->v_cond = $result['v_cond'];
      $this->v_sector = $result['v_sector'];
      return $result;
    }
  }

  function build_scenario() {
    $res = query("SELECT * FROM scenario_buildings WHERE sc_type='{$this->which}'");
    while ($sc = mysql_fetch_assoc($res))
      $this->put_building($sc['type'], $sc['sector'], $sc['level'], $sc['x'], $sc['y']);

    $off_abils = array(3031,3029,3037); 	//block, rage, strength
    $def_abils = array(3034,3037,3029,3037); 	//hull, def slime, block, rage
    $spc_abils=array(3015,3017); 		//timewarp,freeze


    $res = query("SELECT * FROM scenario_units WHERE sc_type='{$this->which}'");

    $added_tribes = array();		// dass man die DB nicht unnötig flooded, wird hier gespeichert
    					// welche Tribes schon hinzugefügt wurden für das Szenario
    while ($sc = mysql_fetch_assoc($res)) {
      if ($sc['race'] == 3003) {	//Attacker
        $abil1 = $off_abils[rand(0, count($off_abils)-1)];
        $pic_url = 'attacker.gif';
      }
      else if ($sc['race'] == 3005) {	// Defender
        $abil1 = $def_abils[rand(0, count($def_abils)-1)];
        $pic_url = 'defender.gif';
      }
      else if ($sc['race'] == 3008) {
        $abil0 = $spc_abils[rand(0, count($spc_abils)-1)];
        $pic_url = 'specialist.gif';
      }
      if (!in_array($sc['name'], $added_tribes)) {
        $tribe_id = $this->insert_tribe($sc['name'], $sc['att'], $sc['def'], $sc['hp'], $sc['speed'], $sc['race'], $abil1, $abil0);
	$added_tribes[] = $sc['name'];
      }
      else
        $tribe_id = mysql_result(query("SELECT id FROM tribes WHERE name='{$sc['name']}'"), 0);

      $this->put_units($tribe_id, $sc['count'], $sc['sector']);
    }
  }

  function create_map(){
    // ob noch genug platz auf einem Host
    $result = query('SELECT free_maps FROM worlds WHERE id='.SC_WORLD_ID);
    $free_maps=mysql_result($result,0);
    if ($free_maps < 2)
      return false;

    $t_id = mysql_result(query('SELECT tmpl_id FROM scenarios WHERE id='.$this->which), 0);

    $this->map_id = SC_WORLD_ID*100 + 100 - $free_maps;

    query('INSERT INTO maps SET map_id='.$this->map_id.', type=2, state=1, template_id='.$t_id.', inserted='.$this->t_now);
    $result = query('SELECT * FROM template_maps WHERE status=0 AND template_id='.$t_id);
    while($row=mysql_fetch_assoc($result)) {
      if ($row['home'])  // Beim Scenario gibts ja nur einen HS
	$home = $row['sector_id'];
      $artifact = ($row['artifact']) ? 1 : 0;
      $sector_id = $row['sector_id'];
      query('INSERT INTO sectors SET sector_id='.$sector_id.', map_id='.$this->map_id.'status=0, artifact='.$artifact.', host_activity='.$row['host_activity'].', bio='.$row['bio'].', domination_point='.$row['domination_point']);
    }
//    $random_home = random_value($homes) + $this->map_id*100;
    $home += $this->map_id*100;
    $sql='UPDATE worlds SET free_maps=free_maps-1, wait_maps=wait_maps+1 WHERE id='.SC_WORLD_ID;
    query($sql);
    return($home);
  }
}


?>