<?

/***********************************************************************************
speichert Namen und Beschreibungen, sowie die Anzahl der sciences in globalen arrays

Wenn $also_db_value auf false gesetzt wird, werden nur die Sprachgeschichten neu
geladen.
***********************************************************************************/
function get_general_science_data($also_db_values=true)
{
	$SCIENCE_TEMP['sc_name']=array(
		SC_ID_MORPHING				=>	'LNG_MORPHING',
	   	SC_ID_SCIENTISTS			=>	'LNG_SCIENTISTS',
		SC_ID_TREES					=>	'LNG_TREES',
		SC_ID_STONES				=>	'LNG_STONES',
		SC_ID_PLANTS				=>	'LNG_PLANTS',
		
		SC_ID_CLAM					=>	'LNG_CLAM',
		SC_ID_BIRCH					=>	'LNG_BIRCH',
		SC_ID_LUMBER				=>	'LNG_LUMBER',
		SC_ID_BARK					=>	'LNG_BARK',
		SC_ID_BASALT				=>	'LNG_BASALT',
		SC_ID_PERMSANDSTONE			=>	'LNG_PERMSANDSTONE',
		SC_ID_FLINTSTONE			=>	'LNG_FLINTSTONE',
		SC_ID_PYRIT					=>	'LNG_PYRIT',
		SC_ID_HEDGE					=>	'LNG_HEDGE',
		SC_ID_GROUND_NESTER			=>	'LNG_GROUND_NESTER',
		SC_ID_BAST					=>	'LNG_BAST',
		SC_ID_FLAX					=>	'LNG_FLAX',
		SC_ID_HEMP					=>	'LNG_HEMP',
		SC_ID_POLE_WOOD				=>	'LNG_POLE_WOOD',
		SC_ID_FISHING				=>	'LNG_FISHING',
		SC_ID_ADV_FISHING			=>	'LNG_ADV_FISHING',
		SC_ID_FIRE					=>	'LNG_FIRE',
		SC_ID_GELATINE				=>	'LNG_GELATINE',
		SC_ID_RESIN					=>	'LNG_RESIN',
		SC_ID_BIRCH_PITCH			=>	'LNG_BIRCH_PITCH',
		SC_ID_CONTAINERS			=>	'LNG_CONTAINERS',
		SC_ID_JOINT					=>	'LNG_JOINT',
		SC_ID_CLOTHES				=>	'LNG_CLOTHES',
		SC_ID_FIBRE					=>	'LNG_FIBRE',
		SC_ID_CORD					=>	'LNG_CORD',
		SC_ID_KNOT					=>	'LNG_KNOT',
		SC_ID_NETS					=>	'LNG_NETS',
		SC_ID_SLING					=>	'LNG_SLING',
		SC_ID_GLUE					=>	'LNG_GLUE',
		SC_ID_CATCH_PARTRIDGE		=>	'LNG_CATCH_PARTRIDGE',
		SC_ID_SHARPEN				=>	'LNG_SHARPEN',
		SC_ID_AGRICULTIRE			=>	'LNG_AGRICULTURE',
		SC_ID_ADV_AGRICULTURE		=>	'LNG_ADV_AGRICULTURE',
		SC_ID_REED					=>	'LNG_REED',
		SC_ID_SKINNING				=>	'LNG_SKINNING',
		SC_ID_HUNTING				=>	'LNG_HUNTING',
		SC_ID_WEAPON_REVOLUTION		=>	'LNG_WEAPON_REVOLUTION',
		SC_ID_CHOP_DOWN_TREES		=>	'LNG_CHOP_DOWN_TREES',
		SC_ID_CREATE_HEDGE			=>	'LNG_CREATE_HEDGE',
		SC_ID_COOK_MEAT				=>	'LNG_COOK_MEAT',
	
	//Waffen & Werkzeuge
		SC_ID_JAVELIN				=>	'LNG_JAVELIN',
		SC_ID_ADV_JAVELIN			=>	'LNG_ADV_JAVELIN',
		SC_ID_CLUB					=>	'LNG_CLUB',
		SC_ID_HARPOON				=>	'LNG_HARPOON',
		SC_ID_ADV_HARPOON			=>	'LNG_ADV_HARPOON',
		SC_ID_BOW					=>	'LNG_BOW',
		SC_ID_ADV_BOW				=>	'LNG_ADV_BOW',
		SC_ID_BLADE					=>	'LNG_BLADE',
		SC_ID_DRILL					=>	'LNG_DRILL',
		SC_ID_ADZE_BEATEN			=>	'LNG_ADZE_BEATEN',
		SC_ID_ADZE_SHARPENED		=>	'LNG_ADZE_SHARPENED',
		SC_ID_AXE_BEATEN			=>	'LNG_AXE_BEATEN',
		SC_ID_AXE_BEATEN_SHARPENED	=>	'LNG_AXE_BEATEN_SHARPENED',
		SC_ID_AXE_SHARPENED			=>	'LNG_AXE_SHARPENED',
		SC_ID_DIGWOOD				=>	'LNG_DIGWOOD',
		SC_ID_WHEEL					=>	'LNG_WHEEL',
		SC_ID_WALZENBEIL			=>	'LNG_WALZENBEIL',
		SC_ID_ADV_BOW_SHARPENED		=>	'LNG_ADV_BOW_SHARPENED',
		SC_ID_ADV_JAVELIN_SHARPENED	=>	'LNG_ADV_JAVELIN_SHARPENED',
	
	// Fähigkeiten (Training) und Zusatzwaffen
		SC_ID_CAMOUFLAGE			=>	'LNG_CAMOUFLAGE',
		SC_ID_MOVE_TREES			=>	'LNG_MOVE_TREES',
		SC_ID_STICK_PARRY			=>	'LNG_STICK_PARRY',
		SC_ID_TRAIN_ABDOMINALS		=>	'LNG_TRAIN_ABDOMINALS',
		SC_ID_PUSH_UP				=>	'LNG_PUSH_UP',
		SC_ID_AXE_BLOCK				=>	'LNG_AXE_BLOCK',
		SC_ID_TWO_AXES				=>	'LNG_TWO_AXES',
		SC_ID_BOW_AIMING			=>	'LNG_BOW_AIMING',
		SC_ID_BOW_DIRECT_SHOT		=>	'LNG_BOW_DIRECT_SHOT',
		SC_ID_BOW_MOVING_TARGETS	=>	'LNG_BOW_MOVING_TARGETS',
		SC_ID_ATLATL				=>	'LNG_ATLATL',
		SC_ID_JAVELIN_AIM			=>	'LNG_JAVELIN_AIM',
		SC_ID_JAVELIN_DIRECT_THROW	=>	'LNG_JAVELIN_DIRECT_THROW',
		SC_ID_JAVELIN_MOVING_TARGETS => 'LNG_JAVELIN_MOVING_TARGETS',
		SC_ID_HERBS					=>	'LNG_HERBS',
		SC_ID_ROPE_SKIPPING			=>	'LNG_ROPE_SKIPPING',
		
	//Gebäude
		SC_ID_STATICS				=>	'LNG_STATICS',
		SC_ID_REED_ROOF				=>	'LNG_REED_ROOF',
		SC_ID_HUT					=>	'LNG_HUT',
		SC_ID_ADV_HUT				=>	'LNG_ADV_HUT',
		SC_ID_LUMBERJACKS_HUT		=>	'LNG_LUMBERJACKS_HUT',
		SC_ID_ADV_LUMBERJACKS_HUT	=>	'LNG_ADV_LUMBERJACKS_HUT',
		SC_ID_ANIMAL_SKIN_TENT		=>	'LNG_ANIMAL_SKIN_TENT',
		SC_ID_FARM					=>	'LNG_FARM',
		SC_ID_ADV_FARM				=>	'LNG_ADV_FARM',
		SC_ID_SHED					=>	'LNG_SHED',
		
	// Dark
		SC_ID_DARK_UNIT				=>	'LNG_DARK_UNIT'
	);


	$SCIENCE_TEMP['sc_desc']=array(
		SC_ID_MORPHING				=>	'LNG_MORPHING_DESC',
	   	SC_ID_SCIENTISTS			=>	'LNG_SCIENTISTS_DESC',
		SC_ID_TREES					=>	'LNG_TREES_DESC',
		SC_ID_STONES				=>	'LNG_STONES_DESC',
		SC_ID_PLANTS				=>	'LNG_PLANTS_DESC',
		
		SC_ID_CLAM					=>	'LNG_CLAM_DESC',
		SC_ID_BIRCH					=>	'LNG_BIRCH_DESC',
		SC_ID_LUMBER				=>	'LNG_LUMBER_DESC',
		SC_ID_BARK					=>	'LNG_BARK_DESC',
		SC_ID_BASALT				=>	'LNG_BASALT_DESC',
		SC_ID_PERMSANDSTONE			=>	'LNG_PERMSANDSTONE_DESC',
		SC_ID_FLINTSTONE			=>	'LNG_FLINTSTONE_DESC',
		SC_ID_PYRIT					=>	'LNG_PYRIT_DESC',
		SC_ID_HEDGE					=>	'LNG_HEDGE_DESC',
		SC_ID_GROUND_NESTER			=>	'LNG_GROUND_NESTER_DESC',
		SC_ID_BAST					=>	'LNG_BAST_DESC',
		SC_ID_FLAX					=>	'LNG_FLAX_DESC',
		SC_ID_HEMP					=>	'LNG_HEMP_DESC',
		SC_ID_POLE_WOOD				=>	'LNG_POLE_WOOD_DESC',
		SC_ID_FISHING				=>	'LNG_FISHING_DESC',
		SC_ID_ADV_FISHING			=>	'LNG_ADV_FISHING_DESC',
		SC_ID_FIRE					=>	'LNG_FIRE_DESC',
		SC_ID_GELATINE				=>	'LNG_GELATINE_DESC',
		SC_ID_RESIN					=>	'LNG_RESIN_DESC',
		SC_ID_BIRCH_PITCH			=>	'LNG_BIRCH_PITCH_DESC',
		SC_ID_CONTAINERS			=>	'LNG_CONTAINERS_DESC',
		SC_ID_JOINT					=>	'LNG_JOINT_DESC',
		SC_ID_CLOTHES				=>	'LNG_CLOTHES_DESC',
		SC_ID_FIBRE					=>	'LNG_FIBRE_DESC',
		SC_ID_CORD					=>	'LNG_CORD_DESC',
		SC_ID_KNOT					=>	'LNG_KNOT_DESC',
		SC_ID_NETS					=>	'LNG_NETS_DESC',
		SC_ID_SLING					=>	'LNG_SLING_DESC',
		SC_ID_GLUE					=>	'LNG_GLUE_DESC',
		SC_ID_CATCH_PARTRIDGE		=>	'LNG_CATCH_PARTRIDGE_DESC',
		SC_ID_SHARPEN				=>	'LNG_SHARPEN_DESC',
		SC_ID_AGRICULTIRE			=>	'LNG_AGRICULTURE_DESC',
		SC_ID_ADV_AGRICULTURE		=>	'LNG_ADV_AGRICULTURE_DESC',
		SC_ID_REED					=>	'LNG_REED_DESC',
		SC_ID_SKINNING				=>	'LNG_SKINNING_DESC',
		SC_ID_HUNTING				=>	'LNG_HUNTING_DESC',
		SC_ID_WEAPON_REVOLUTION		=>	'LNG_WEAPON_REVOLUTION_DESC',
		SC_ID_CHOP_DOWN_TREES		=>	'LNG_CHOP_DOWN_TREES_DESC',
		SC_ID_CREATE_HEDGE			=>	'LNG_CREATE_HEDGE_DESC',
		SC_ID_COOK_MEAT				=>	'LNG_COOK_MEAT_DESC',
	
	//Waffen & Werkzeuge
		SC_ID_JAVELIN				=>	'LNG_JAVELIN_DESC',
		SC_ID_ADV_JAVELIN			=>	'LNG_ADV_JAVELIN_DESC',
		SC_ID_CLUB					=>	'LNG_CLUB_DESC',
		SC_ID_ATLATL				=>	'LNG_ATLATL_DESC',
		SC_ID_HARPOON				=>	'LNG_HARPOON_DESC',
		SC_ID_ADV_HARPOON			=>	'LNG_ADV_HARPOON_DESC',
		SC_ID_BOW					=>	'LNG_BOW_DESC',
		SC_ID_ADV_BOW				=>	'LNG_ADV_BOW_DESC',
		SC_ID_BLADE					=>	'LNG_BLADE_DESC',
		SC_ID_DRILL					=>	'LNG_DRILL_DESC',
		SC_ID_ADZE_BEATEN			=>	'LNG_ADZE_BEATEN_DESC',
		SC_ID_ADZE_SHARPENED		=>	'LNG_ADZE_SHARPENED_DESC',
		SC_ID_AXE_BEATEN			=>	'LNG_AXE_BEATEN_DESC',
		SC_ID_AXE_BEATEN_SHARPENED	=>	'LNG_AXE_BEATEN_SHARPENED_DESC',
		SC_ID_AXE_SHARPENED			=>	'LNG_AXE_SHARPENED_DESC',
		SC_ID_DIGWOOD				=>	'LNG_DIGWOOD_DESC',
		SC_ID_WHEEL					=>	'LNG_WHEEL_DESC',
		SC_ID_WALZENBEIL			=>	'LNG_WALZENBEIL_DESC',
		SC_ID_ADV_BOW_SHARPENED		=>	'LNG_ADV_BOW_SHARPENED_DESC',
		SC_ID_ADV_JAVELIN_SHARPENED	=>	'LNG_ADV_JAVELIN_SHARPENED_DESC',
	
	// Fähigkeiten (Training)
		SC_ID_CAMOUFLAGE			=>	'LNG_CAMOUFLAGE_DESC',
		SC_ID_MOVE_TREES			=>	'LNG_MOVE_TREES_DESC',
		SC_ID_STICK_PARRY			=>	'LNG_STICK_PARRY_DESC',
		SC_ID_TRAIN_ABDOMINALS		=>	'LNG_TRAIN_ABDOMINALS_DESC',
		SC_ID_PUSH_UP				=>	'LNG_PUSH_UP_DESC',
		SC_ID_AXE_BLOCK				=>	'LNG_AXE_BLOCK_DESC',
		SC_ID_TWO_AXES				=>	'LNG_TWO_AXES_DESC',
		
	//Gebäude
		SC_ID_STATICS				=>	'LNG_STATICS_DESC',
		SC_ID_REED_ROOF				=>	'LNG_REED_ROOF_DESC',
		SC_ID_HUT					=>	'LNG_HUT_DESC',
		SC_ID_ADV_HUT				=>	'LNG_ADV_HUT_DESC',
		SC_ID_LUMBERJACKS_HUT		=>	'LNG_LUMBERJACKS_HUT_DESC',
		SC_ID_ADV_LUMBERJACKS_HUT	=>	'LNG_ADV_LUMBERJACKS_HUT_DESC',
		SC_ID_ANIMAL_SKIN_TENT		=>	'LNG_ANIMAL_SKIN_TENT_DESC',
		SC_ID_FARM					=>	'LNG_FARM_DESC',
		SC_ID_ADV_FARM				=>	'LNG_ADV_FARM_DESC',
		SC_ID_SHED					=>	'LNG_SHED_DESC',
		
	// Dark
		SC_ID_DARK_UNIT				=>	'LNG_DARK_UNIT_DESC'
	);
      
	$SCIENCE_TEMP['sc_type_name'] = array (
      	SC_TYPE_GENERAL_ABILS1		=> 'LNG_GENERAL_ABILS1',
      	SC_TYPE_GENERAL_ABILS2		=> 'LNG_GENERAL_ABILS2',
      	SC_TYPE_GENERAL_ABILS3		=> 'LNG_GENERAL_ABILS3',
      	SC_TYPE_OFFENSIVE_ABILS1	=> 'LNG_OFFENSIVE_ABILS1',
      	SC_TYPE_OFFENSIVE_ABILS2	=> 'LNG_OFFENSIVE_ABILS2',
      	SC_TYPE_OFFENSIVE_ABILS3	=> 'LNG_OFFENSIVE_ABILS3',
      	SC_TYPE_DEFENSIVE_ABILS1	=> 'LNG_DEFENSIVE_ABILS1',
      	SC_TYPE_DEFENSIVE_ABILS2	=> 'LNG_DEFENSIVE_ABILS2',
      	SC_TYPE_DEFENSIVE_ABILS3	=> 'LNG_DEFENSIVE_ABILS3',
      	SC_TYPE_SPECIAL_ABILS1		=> 'LNG_SPECIAL_ABILS1',
      	SC_TYPE_SPECIAL_ABILS2		=> 'LNG_SPECIAL_ABILS2',
      	SC_TYPE_SPECIAL_ABILS3		=> 'LNG_SPECIAL_ABILS3',
      	SC_TYPE_COUNTER_ABILS1		=> 'LNG_COUNTER_ABILS1',
	);
      
	$SCIENCE_TEMP['sc_type_desc'] = array (
      	SC_TYPE_GENERAL_ABILS1		=> 'LNG_GENERAL_ABILS1_DESC',
      	SC_TYPE_GENERAL_ABILS2		=> 'LNG_GENERAL_ABILS2_DESC',
      	SC_TYPE_GENERAL_ABILS3		=> 'LNG_GENERAL_ABILS3_DESC',
      	SC_TYPE_OFFENSIVE_ABILS1	=> 'LNG_OFFENSIVE_ABILS1_DESC',
      	SC_TYPE_OFFENSIVE_ABILS2	=> 'LNG_OFFENSIVE_ABILS2_DESC',
      	SC_TYPE_OFFENSIVE_ABILS3	=> 'LNG_OFFENSIVE_ABILS3_DESC',
      	SC_TYPE_DEFENSIVE_ABILS1	=> 'LNG_DEFENSIVE_ABILS1_DESC',
      	SC_TYPE_DEFENSIVE_ABILS2	=> 'LNG_DEFENSIVE_ABILS2_DESC',
      	SC_TYPE_DEFENSIVE_ABILS3	=> 'LNG_DEFENSIVE_ABILS3_DESC',
      	SC_TYPE_SPECIAL_ABILS1		=> 'LNG_SPECIAL_ABILS1_DESC',
      	SC_TYPE_SPECIAL_ABILS2		=> 'LNG_SPECIAL_ABILS2_DESC',
      	SC_TYPE_SPECIAL_ABILS3		=> 'LNG_SPECIAL_ABILS3_DESC',
      	SC_TYPE_COUNTER_ABILS1		=> 'LNG_COUNTER_ABILS1_DESC',
	);


	if ($also_db_values) {
		$SCIENCE_TEMP['all_sciences']=array();
	//  $sql="SELECT DISTINCT sc_id FROM sciences ORDER BY sc_id"; //unique;
	   /*	  $sql="SELECT DISTINCT sc_id FROM sciences"; //unique;
	  $result=query($sql);
	  while ($row=mysql_fetch_row($result))
	      $all_sci[$row[0]]=$SCIENCE_TEMP['sc_name'][$row[0]];*/

		$result=query('SELECT sc_id, max(LEVEL) as lev,pic,type FROM sciences GROUP BY sc_id');
		while($row=mysql_fetch_assoc($result))
		{
			$SCIENCE_TEMP['max_levels'][$row['sc_id']]=$row['lev'];
			$SCIENCE_TEMP['pics'][$row['sc_id']]=$row['pic'];
			$all_sci[$row['sc_id']]=$SCIENCE_TEMP['sc_name'][$row['sc_id']];
			if ($row['type']==30)
				$SCIENCE_TEMP['all_races'][]=$row['sc_id'];
			else if ($row['type']==31)
				$SCIENCE_TEMP['all_tools'][]=$row['sc_id'];
			
		}
		asort($all_sci);
		
		foreach($all_sci as $sccid=>$nname)
			$SCIENCE_TEMP['all_sciences'][]=$sccid;

		$result_str = '';
		$cnt=0;
		$result_str .= '<?
class ScienceConstants {	// Namespace!
	public static $ALL_SCIENCES = array(';
		foreach($SCIENCE_TEMP['all_sciences'] as $sc_id) {
			if ($cnt++)
				$result_str .= ','.$sc_id;
			else
				$result_str .= $sc_id;
		}
		$result_str .=  ');';

		$cnt=0;
		$result_str .= '

	public static $ALL_RACES = array(';
		foreach($SCIENCE_TEMP['all_races'] as $sc_id) {
			if ($cnt++)
				$result_str .= ','.$sc_id;
			else
				$result_str .= $sc_id;
		}
		$result_str .= ');';

		$cnt=0;
		$result_str .= '
	public static $ALL_TOOLS = array(';
		foreach($SCIENCE_TEMP['all_tools'] as $sc_id) {
			if ($cnt++)
				$result_str .= ','.$sc_id;
			else
				$result_str .= $sc_id;
		}
		$result_str .= ');';

		$cnt=0;
		$result_str .=  '
	
	public static $MAX_LEVELS = array(';
		foreach($SCIENCE_TEMP['max_levels'] as $sc_id=>$max_lev) {
			if ($cnt++)
				$result_str .= ','.$sc_id.'=>'.$max_lev;
			else
				$result_str .= $sc_id.'=>'.$max_lev;
		}
		$result_str .= ');';

	$cnt=0;
	$result_str .= '

	public static $PICS = array(';
		foreach($SCIENCE_TEMP['pics'] as $sc_id=>$pic) {
			if ($cnt++)
				$result_str .= ',
		'.$sc_id.' => \''.$pic.'\'';
			else
				$result_str .= '
		'.$sc_id.' => \''.$pic.'\'';
		}
		$result_str .= '
	);';
  
		$cnt=0;
		$result_str .= '
	
	public static $SC_NAME = array(';
		foreach($SCIENCE_TEMP['sc_name'] as $sc_id=>$name) {
			if ($cnt++)
				$result_str .= ',
		'.$sc_id.' => '.$name;
			else
				$result_str .= '
		'.$sc_id.' => '.$name;
		}
		$result_str .= '
	);';

		$cnt=0;
		$result_str .= '

	public static $SC_DESC = array (';

		foreach($SCIENCE_TEMP['sc_desc'] as $sc_id=>$desc) {
			if ($cnt++)
				$result_str .= ',
		'.$sc_id.' => '.$desc;
			else
				$result_str .= '
		'.$sc_id.' => '.$desc;
		}
		$result_str .= '
	);';

		$cnt=0;
		$result_str .= '

	public static $SC_TYPE_NAME = array (';

		foreach($SCIENCE_TEMP['sc_type_name'] as $sc_id=>$desc) {
			if ($cnt++)
				$result_str .= ',
		'.$sc_id.' => '.$desc;
			else
				$result_str .= '
		'.$sc_id.' => '.$desc;
		}
		$result_str .= '
	);';

		$cnt=0;
		$result_str .= '

	public static $SC_TYPE_DESC = array (';

		foreach($SCIENCE_TEMP['sc_type_desc'] as $sc_id=>$desc) {
			if ($cnt++)
				$result_str .= ',
		'.$sc_id.' => '.$desc;
			else
				$result_str .= '
		'.$sc_id.' => '.$desc;
		}
		$result_str .= '
	);
	private function __construct() {} //all static => no construct
}
?>';
	}
	
	return $result_str;
}
?>
