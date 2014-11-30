-- MySQL dump 10.10
--
-- Host: localhost    Database: pvgame
-- ------------------------------------------------------
-- Server version	5.0.26-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES latin1 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `race_abilities`
--

DROP TABLE IF EXISTS `race_abilities`;
CREATE TABLE `race_abilities` (
  `race_sc_id` int(11) unsigned NOT NULL,
  `ability_type` int(11) unsigned NOT NULL,
  `amount` tinyint unsigned NOT NULL default '1',
  PRIMARY KEY  (`race_sc_id`, `ability_type`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;

--
-- Dumping data for table `race_abilities`
--

LOCK TABLES `race_abilities` WRITE;
/*!40000 ALTER TABLE `race_abilities` DISABLE KEYS */;
INSERT INTO `race_abilities` VALUES
(3001,	 100,	0),
(3001,	 101,	0),
(3001,	 200,	0),
(3002,	 100,	0),
(3002,	 103,	0),
(3003,	 100,	0),
(3004,	 100,	0),
(3005,	 100,	0),
(3006,	 100,	0),
(3007,	 100,	0),
(3007,	 103,	0),
(3008,	 100,	0),
(3008,	 101,	0),
(3008,	 200,	0),
(3009,	 100,	0),
(3011,	 100,	0),
(3011,	 102,	0),
(3014,	 100,	0),
(3014,	 102,	0),
(3014,	 202,	0),
(3015,	 100,	0),
(3015,	 102,	0),
(3015,	 202,	0),
(3016,	 100,	0),
(3016,	 102,	0),
(3016,	 202,	0),
(3021,	 100,	0),
(3021,	 103,	0),
(3022,	 100,	0),
(3022,	 200,	0),
(3022,	 101,	0);

/*!40000 ALTER TABLE `race_abilities` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-02-03 16:24:01
