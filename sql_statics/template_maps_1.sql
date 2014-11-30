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
-- Table structure for table `template_maps_1`
--

DROP TABLE IF EXISTS `template_maps_1`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_maps_1` (
  `template_id` int(10) unsigned NOT NULL default '0',
  `user` varchar(100) NOT NULL default '',
  `num_players` tinyint(2) unsigned NOT NULL default '0',
  `possible_teams` int(10) unsigned NOT NULL default '0',
  `state` tinyint(1) unsigned NOT NULL default '1',
  `allow_host_move` tinyint(1) unsigned NOT NULL default '1',
  `type` tinyint(3) unsigned NOT NULL default '1',
  `min_level` smallint(5) unsigned NOT NULL default '0',
  `max_level` smallint(5) unsigned NOT NULL default '0',
  `difficulty` tinyint(3) unsigned NOT NULL default '1',
  `name` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`template_id`),
  KEY `template_id` (`template_id`),
  KEY `num_players` (`num_players`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_maps_1`
--

LOCK TABLES `template_maps_1` WRITE;
/*!40000 ALTER TABLE `template_maps_1` DISABLE KEYS */;
INSERT INTO `template_maps_1` VALUES (53,'cospike',8,24,2,1,2,0,0,1,'Krake'),(103,'Nappo',6,3,2,1,5,0,0,2,'Watch your Bio'),(105,'Nappo',4,2,2,1,5,0,0,2,'Enter the Middel'),(107,'Nappo',8,24,2,1,7,0,0,1,'4 Achsen des Boesen'),(108,'Nappo',6,2,2,1,2,0,0,1,'Ein lausiger Kaefer'),(110,'Nappo',4,2,2,1,7,0,0,1,'Just another one'),(117,'Nappo',6,23,2,1,5,0,0,2,'6x(3x3)+6'),(128,'Nappo',4,2,2,1,7,0,0,1,'Diagonale Front'),(130,'Nappo',4,2,2,1,5,0,0,1,'Karte fuer die mir kein Name einfaellt'),(131,'Nappo',4,2,2,1,7,0,0,1,'Shortcut'),(140,'Nappo',8,24,2,1,2,0,0,1,'CornerFight'),(0,'',0,0,1,1,1,0,0,1,'Dummy-Template');
/*!40000 ALTER TABLE `template_maps_1` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-03-14 16:14:00
