<?php


require_once(dirname(__FILE__).'/Db.inc');

/**
 * Zählt und verwaltet alle Refs, sprich: Jemand surft PV mit ref_id=XY an, dann
 * wird das Ding hier angeschossen, um diesen Zugriff genau zu loggen
 * (ref_id, Datum, Uhrzeit, IP)
 * 
 * Diese Klasse ist rein statisch und als Namespace zu verstehn 
 */
class RefCounter {
	const MYSQL_RESSOURCE = 1;
	const ASSOC_ARRAY = 2;
	const INDEXED_ARRAY = 3;
	private function __construct() {}
	
 	public static function log_ref_id($id) {
 		 $ip = $_SERVER['REMOTE_ADDR'];
 		 $t_stamp = time();
 		 query("INSERT INTO ref_links SET ref_id='$id', t_stamp='$t_stamp', ip='$ip'");
 	}
 	
 	public static function get_ref_log($limit=false, $id=false, $return_type=self::MYSQL_RESSOURCE) {
 		$sql = 'SELECT ref_id, t_stamp, ip FROM ref_links';
 		if ($id !== false) {
 			$sql .= " WHERE ref_id='$id'";
 		}
 		$sql .= ' ORDER BY t_stamp DESC';
 		if ($limit) {
 			$sql .= " LIMIT $limit";
 		}
 		
 		$res = query($sql);
 		if ($return_type == self::MYSQL_RESSOURCE)
 			return $res;
 		else if ($return_type == self::ASSOC_ARRAY) {
 			$result = array();
 			while ($row = mysql_fetch_assoc($res)) {
 				$result[] = $row;
 			}
 			return $result;
 		}
 		else if ($return_type == self::INDEXED_ARRAY) {
 		 	$result = array();
 			while ($row = mysql_fetch_assoc($res)) {
 				$result[] = array($row['ref_id'], $row['t_stamp'], $row['ip']);
 			}
 			return $result;
 		}
 		else
 			return false;
 	}
 	
 	public static function get_ref_overview($limit=false, $id=false, $return_type=self::MYSQL_RESSOURCE) {
 		$sql = 'SELECT ref_id, COUNT(*) AS count FROM ref_links';
 		if ($id)
 			$sql .= " WHERE ref_id=$id";
 		if ($limit)
 			$sql .= " LIMIT $limit";
 		$sql .= ' GROUP BY ref_id';
 		$res = query($sql);
 		if ($return_type == self::MYSQL_RESSOURCE) {
 			return $res;
 		}
 		else if ($return_type == self::ASSOC_ARRAY) {
 			$result = array();
 			while ($row = mysql_fetch_assoc($res)) {
 				$result[] = $row;
 			}
 			return $result;
 		}
 		else if ($return_type == self::INDEXED_ARRAY) {
 			$result = array();
 			while ($row = mysql_fetch_assoc($res)) {
 				$result[$row['ref_id']] = $row['count'];
 			}
 			return $result;
 		}
 	}
 	
 	public static function get_ref_ids($limit=false, $return_type=self::MYSQL_RESSOURCE) {
 		$sql = 'SELECT DISTINCT ref_id FROM ref_links ORDER BY ref_id';
 		if ($limit) {
 			$sql .= " LIMIT $limit";
 		}
 		$res = query($sql);
 		if ($return_type == self::MYSQL_RESSOURCE) {
 			return $res;
 		}
 		else if ($return_type == self::ASSOC_ARRAY) {
 			$result = array();
 			while ($row = mysql_fetch_assoc($res)) {
 				$result[] = $row;
 			}
 			return $result;
 		}
 		else if ($return_type == self::INDEXED_ARRAY) {
 			$result = array();
 			while ($row = mysql_fetch_assoc($res)) {
 				$result[] = $row['ref_id'];
 			}
 			return $result;
 		}
 		else
 			return false;
 	}
}

?>