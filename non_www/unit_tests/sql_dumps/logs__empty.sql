-- MySQL dump 10.11
--
-- Host: localhost    Database: logs_unittest
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
-- Table structure for table `game_event_log`
--

DROP TABLE IF EXISTS `game_event_log`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `game_event_log` (
  `uid` bigint(20) unsigned NOT NULL,
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `type` tinyint(3) unsigned NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  `extra` int(10) unsigned NOT NULL,
  `extra_str` varchar(60) NOT NULL default '',
  `num` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`world_id`,`map_id`,`type`,`t_stamp`,`extra`,`extra_str`),
  KEY `uid` (`uid`),
  KEY `map_id` (`world_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `game_event_log_no_map`
--

DROP TABLE IF EXISTS `game_event_log_no_map`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `game_event_log_no_map` (
  `uid` bigint(20) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  `extra` int(10) unsigned NOT NULL,
  `extra_str` varchar(40) NOT NULL default '',
  `num` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`type`,`t_stamp`,`extra`,`extra_str`),
  KEY `uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `item_buy_actions`
--

DROP TABLE IF EXISTS `item_buy_actions`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_buy_actions` (
  `type` tinyint(3) unsigned default NULL,
  `id` smallint(5) unsigned default NULL,
  `amount` tinyint(3) unsigned NOT NULL default '1',
  `price` smallint(5) unsigned default NULL,
  `timestamp` int(10) unsigned default NULL,
  `uid` bigint(20) unsigned default NULL,
  KEY `type` (`type`),
  KEY `id` (`id`),
  KEY `amount` (`amount`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `item_trade_actions`
--

DROP TABLE IF EXISTS `item_trade_actions`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_trade_actions` (
  `type` tinyint(3) unsigned NOT NULL default '0',
  `id` smallint(5) unsigned NOT NULL default '0',
  `seller_id` bigint(20) unsigned NOT NULL default '0',
  `buyer_id` bigint(20) unsigned NOT NULL default '0',
  `price` smallint(5) unsigned NOT NULL default '0',
  `timestamp` int(10) unsigned NOT NULL default '0',
  KEY `type` (`type`),
  KEY `id` (`id`),
  KEY `seller_id` (`seller_id`),
  KEY `buyer_id` (`buyer_id`),
  KEY `timestamp` (`timestamp`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `klunker_substract_actions`
--

DROP TABLE IF EXISTS `klunker_substract_actions`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `klunker_substract_actions` (
  `type` tinyint(3) unsigned NOT NULL,
  `amount` int(11) NOT NULL,
  `timestamp` int(10) unsigned NOT NULL,
  `uid` bigint(20) unsigned NOT NULL,
  KEY `type` (`type`,`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-03-28 16:32:47
