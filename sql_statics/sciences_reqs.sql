-- MySQL dump 10.11
--
-- Host: localhost    Database: pvgame
-- ------------------------------------------------------
-- Server version	5.0.76-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `sciences_reqs`
--

DROP TABLE IF EXISTS `sciences_reqs`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sciences_reqs` (
  `sc_id` int(4) unsigned NOT NULL default '0',
  `level` tinyint(1) unsigned NOT NULL default '1',
  `need_i1` mediumint(8) unsigned NOT NULL default '0',
  `need_sc1` smallint(5) unsigned NOT NULL default '0',
  `need_sc1_lev` tinyint(1) unsigned NOT NULL default '0',
  `sc1_2_op` tinyint(1) unsigned NOT NULL default '0',
  `need_sc2` smallint(5) unsigned NOT NULL default '0',
  `need_sc2_lev` tinyint(1) unsigned NOT NULL default '0',
  `sc2_3_op` tinyint(1) unsigned NOT NULL default '0',
  `need_sc3` smallint(5) unsigned NOT NULL default '0',
  `need_sc3_lev` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`sc_id`,`level`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `sciences_reqs`
--

LOCK TABLES `sciences_reqs` WRITE;
/*!40000 ALTER TABLE `sciences_reqs` DISABLE KEYS */;
INSERT INTO `sciences_reqs` VALUES (1,1,0,0,0,0,0,0,0,0,0),(47,1,0,5,1,0,0,0,0,0,0),(10,1,0,4,1,0,0,0,0,0,0),(1001,1,0,43,1,1,39,1,0,0,0),(1002,1,0,1001,1,1,8,1,0,0,0),(1004,1,0,3012,1,1,1002,1,1,43,1),(1007,1,0,3017,1,0,0,0,0,0,0),(12,1,0,4,1,0,0,0,0,0,0),(13,1,0,4,1,0,0,0,0,0,0),(14,1,0,5,1,0,0,0,0,0,0),(16,1,0,5,1,0,0,0,0,0,0),(17,1,0,5,1,0,0,0,0,0,0),(18,1,0,5,1,0,0,0,0,0,0),(49,1,0,39,1,0,0,0,0,0,0),(22,1,0,13,1,0,0,0,0,0,0),(23,1,0,54,1,1,22,1,0,0,0),(24,1,0,3,1,0,0,0,0,0,0),(25,1,0,7,1,0,0,0,0,0,0),(26,1,0,25,1,1,9,1,1,16,1),(3020,1,0,42,1,1,3017,1,0,0,0),(27,1,0,18,1,0,0,0,0,0,0),(29,1,0,16,1,2,17,1,2,18,1),(46,1,0,5,1,0,0,0,0,0,0),(51,1,0,33,1,1,18,1,1,50,1),(3,1,0,5,1,0,0,0,0,0,0),(30,1,0,29,1,0,0,0,0,0,0),(3001,1,0,3009,1,1,50,1,0,0,0),(3008,1,0,42,1,1,3001,1,0,0,0),(3007,1,0,53,1,0,0,0,0,0,0),(3002,1,0,42,1,1,3007,1,0,0,0),(3009,1,0,12,1,0,0,0,0,0,0),(3010,1,0,52,1,0,0,0,0,0,0),(3011,1,0,0,0,0,0,0,0,0,0),(3012,1,0,42,1,0,0,0,0,0,0),(3013,1,0,36,1,1,3012,1,0,0,0),(3014,1,0,42,1,1,3011,1,0,0,0),(3016,1,0,36,1,1,3014,1,0,0,0),(3017,1,0,3009,1,0,0,0,0,0,0),(3019,1,0,3012,1,1,3010,1,0,0,0),(3021,1,0,36,1,1,3002,1,0,0,0),(3022,1,0,36,1,1,3008,1,0,0,0),(31,1,0,29,1,1,40,1,0,0,0),(32,1,0,30,1,1,31,1,0,0,0),(33,1,0,30,1,1,31,1,0,0,0),(34,1,0,23,1,2,25,1,2,24,1),(35,1,0,32,1,0,0,0,0,0,0),(36,1,0,42,1,1,4,1,0,0,0),(39,1,0,5,1,0,0,0,0,0,0),(48,1,0,39,1,0,0,0,0,0,0),(50,1,0,14,1,0,0,0,0,0,0),(2,1,0,0,0,0,0,0,0,0,0),(54,1,0,41,1,0,0,0,0,0,0),(4,1,0,0,0,0,0,0,0,0,0),(40,1,0,3009,1,1,41,1,0,0,0),(4001,1,0,3,1,1,14,1,0,0,0),(4002,1,0,43,1,0,0,0,0,0,0),(4003,1,0,3001,1,0,0,0,0,0,0),(4004,1,0,4,1,0,0,0,0,0,0),(4005,1,0,5,1,0,0,0,0,0,0),(4007,1,0,3014,1,0,0,0,0,0,0),(4008,1,0,3007,1,0,0,0,0,0,0),(4009,1,0,4008,1,0,0,0,0,0,0),(4010,1,0,4008,1,0,0,0,0,0,0),(4011,1,0,54,1,1,3001,1,0,0,0),(4012,1,0,3001,1,0,0,0,0,0,0),(4013,1,0,4012,1,0,0,0,0,0,0),(4014,1,0,4012,1,0,0,0,0,0,0),(4015,1,0,5,1,0,0,0,0,0,0),(4016,1,0,30,1,0,0,0,0,0,0),(41,1,0,3001,1,2,3011,1,0,0,0),(42,1,0,10,1,1,34,1,1,29,1),(43,1,0,3014,1,0,0,0,0,0,0),(44,1,0,1007,1,1,14,1,0,0,0),(45,1,0,41,1,1,22,1,0,0,0),(5,1,0,0,0,0,0,0,0,0,0),(1009,1,0,0,0,0,0,0,0,0,0),(52,1,0,3009,1,1,51,1,0,0,0),(4006,1,0,5,1,2,4,1,2,41,1),(53,1,0,52,1,0,0,0,0,0,0),(1006,1,0,31,1,0,0,0,0,0,0),(55,1,0,3016,1,0,0,0,0,0,0),(7,1,0,3,1,0,0,0,0,0,0),(8,1,0,3,1,0,0,0,0,0,0),(9,1,0,3,1,0,0,0,0,0,0);
/*!40000 ALTER TABLE `sciences_reqs` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-11-29 11:05:43
