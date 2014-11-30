-- MySQL dump 10.11
--
-- Host: localhost    Database: score
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
-- Table structure for table `league_score`
--

DROP TABLE IF EXISTS `league_score`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `league_score` (
  `uid` int(10) unsigned NOT NULL,
  `league_id` int(10) unsigned NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `score` float unsigned NOT NULL,
  `creation_t_stamp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`league_id`,`type`),
  KEY `league_id` (`league_id`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `league_score`
--

LOCK TABLES `league_score` WRITE;
/*!40000 ALTER TABLE `league_score` DISABLE KEYS */;
/*!40000 ALTER TABLE `league_score` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `league_score_log`
--

DROP TABLE IF EXISTS `league_score_log`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `league_score_log` (
  `uid` int(10) unsigned NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `score` float unsigned NOT NULL,
  `league_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`t_stamp`,`type`),
  KEY `league_id` (`league_id`),
  KEY `uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `league_score_log`
--

LOCK TABLES `league_score_log` WRITE;
/*!40000 ALTER TABLE `league_score_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `league_score_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `map_score`
--

DROP TABLE IF EXISTS `map_score`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `map_score` (
  `uid` int(10) unsigned NOT NULL,
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `type` tinyint(3) unsigned NOT NULL,
  `score` float unsigned NOT NULL,
  `creation_t_stamp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`world_id`,`map_id`,`type`),
  KEY `map_id` (`world_id`),
  KEY `uid` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `map_score`
--

LOCK TABLES `map_score` WRITE;
/*!40000 ALTER TABLE `map_score` DISABLE KEYS */;
/*!40000 ALTER TABLE `map_score` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `map_score_log`
--

DROP TABLE IF EXISTS `map_score_log`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `map_score_log` (
  `uid` int(10) unsigned NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `score` float unsigned NOT NULL,
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`t_stamp`,`type`),
  KEY `uid` (`uid`),
  KEY `world_id` (`world_id`,`map_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `map_score_log`
--

LOCK TABLES `map_score_log` WRITE;
/*!40000 ALTER TABLE `map_score_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `map_score_log` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-02-24 17:54:50
