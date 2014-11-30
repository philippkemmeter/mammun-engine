-- MySQL dump 10.11
--
-- Host: localhost    Database: pvgame
-- ------------------------------------------------------
-- Server version	5.0.51a-24+lenny2

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
-- Table structure for table `template_maps_teams`
--

DROP TABLE IF EXISTS `template_maps_teams`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_maps_teams` (
  `template_id` int(10) unsigned NOT NULL,
  `sector_id` tinyint(2) unsigned NOT NULL,
  `team_size` tinyint(3) unsigned NOT NULL,
  `team_id` tinyint(1) unsigned NOT NULL,
  `fill_order` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY  (`template_id`,`sector_id`,`team_size`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_maps_teams`
--

LOCK TABLES `template_maps_teams` WRITE;
/*!40000 ALTER TABLE `template_maps_teams` DISABLE KEYS */;
INSERT INTO `template_maps_teams` VALUES (16,22,2,1,1),(16,77,2,2,1),(16,27,2,2,2),(16,72,2,1,2),(53,2,2,1,1),(53,7,2,2,1),(53,20,2,1,2),(53,29,2,2,2),(53,70,2,4,1),(53,79,2,3,1),(53,92,2,4,2),(53,97,2,3,2),(53,2,4,1,1),(53,7,4,1,2),(53,20,4,1,3),(53,29,4,1,4),(53,70,4,2,4),(53,79,4,2,3),(53,92,4,2,2),(53,97,4,2,1),(103,14,2,1,1),(103,21,2,1,2),(103,38,2,2,1),(103,56,2,2,2),(103,61,2,3,1),(103,83,2,3,2),(105,34,2,1,1),(105,43,2,2,1),(105,45,2,1,2),(105,54,2,2,2),(107,3,2,1,1),(107,30,2,1,2),(107,6,2,2,2),(107,39,2,2,1),(107,60,2,3,1),(107,93,2,3,2),(107,96,2,4,2),(107,69,2,4,1),(107,3,4,1,1),(107,6,4,1,2),(107,30,4,1,3),(107,39,4,1,4),(107,60,4,2,4),(107,69,4,2,3),(107,93,4,2,2),(107,96,4,2,1),(110,11,2,1,1),(110,17,2,1,2),(110,71,2,2,2),(110,77,2,2,1),(117,14,2,1,1),(117,18,2,2,2),(117,41,2,1,2),(117,58,2,2,1),(117,85,2,3,1),(117,81,2,3,2),(117,14,3,1,1),(117,58,3,1,2),(117,61,3,1,3),(117,18,3,2,1),(117,41,3,2,2),(117,85,3,2,3),(128,5,2,1,1),(128,49,2,2,2),(128,50,2,1,2),(128,94,2,2,1),(130,4,2,1,1),(130,40,2,1,2),(130,48,2,2,2),(130,84,2,2,1),(131,5,2,1,1),(131,40,2,1,2),(131,59,2,2,2),(131,94,2,2,1);
/*!40000 ALTER TABLE `template_maps_teams` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-04-01  6:48:18
