-- MySQL dump 10.11
--
-- Host: localhost    Database: payment
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
-- Table structure for table `item_available`
--

DROP TABLE IF EXISTS `item_available`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_available` (
  `type` tinyint(3) unsigned NOT NULL default '0',
  `id` smallint(5) unsigned NOT NULL default '0',
  `start_tstamp` int(10) unsigned NOT NULL default '0',
  `lasts_until_tstamp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`type`,`id`,`start_tstamp`),
  KEY `lasts_until_tstamp` (`lasts_until_tstamp`),
  KEY `start_tstamp` (`start_tstamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `item_available`
--

LOCK TABLES `item_available` WRITE;
/*!40000 ALTER TABLE `item_available` DISABLE KEYS */;
INSERT INTO `item_available` VALUES (1,105,1277700000,1277960453),(10,2601,1287972000,1288620000),(10,2701,1287972000,1288620000),(10,2801,1287972000,1288620000),(10,3301,1291356000,1291658400),(10,3401,1291528800,1291831200),(10,3501,1292133600,1292436000),(10,2901,1291701600,1293429599),(10,3001,1290924000,1293429599),(10,3101,1291960800,1293429599),(10,3201,1291615200,1293429599),(10,3601,1291096800,1293429599),(1,1,0,4294967295),(10,601,0,4294967295);
/*!40000 ALTER TABLE `item_available` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_discount_duration`
--

DROP TABLE IF EXISTS `item_discount_duration`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_discount_duration` (
  `type` tinyint(3) unsigned NOT NULL default '0',
  `id` smallint(5) unsigned NOT NULL default '0',
  `start_tstamp` int(10) unsigned NOT NULL default '0',
  `discount` tinyint(3) unsigned NOT NULL default '0',
  `lasts_until_tstamp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`type`,`id`),
  KEY `lasts_until_tstamp` (`lasts_until_tstamp`),
  KEY `start_tstamp` (`start_tstamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `item_discount_duration`
--

LOCK TABLES `item_discount_duration` WRITE;
/*!40000 ALTER TABLE `item_discount_duration` DISABLE KEYS */;
/*!40000 ALTER TABLE `item_discount_duration` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_discount_quantity`
--

DROP TABLE IF EXISTS `item_discount_quantity`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_discount_quantity` (
  `type` tinyint(3) unsigned NOT NULL default '0',
  `id` smallint(5) unsigned NOT NULL default '0',
  `amount` tinyint(3) unsigned NOT NULL default '0',
  `discount` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`type`,`id`,`amount`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `item_discount_quantity`
--

LOCK TABLES `item_discount_quantity` WRITE;
/*!40000 ALTER TABLE `item_discount_quantity` DISABLE KEYS */;
INSERT INTO `item_discount_quantity` VALUES (1,1,1,0),(1,2,1,0),(1,3,1,0),(1,5,1,0),(1,6,1,0),(1,10,1,0),(1,101,1,0),(1,102,1,0),(1,103,1,0),(1,105,1,0),(1,106,1,0),(1,110,1,0),(1,202,1,0),(2,1,1,0),(2,2,1,0),(2,3,1,0),(2,4,1,0),(2,5,1,0),(2,8,1,0),(2,9,1,0),(5,1,1,0),(5,2,1,0),(5,3,1,0),(5,4,1,0),(5,5,1,0),(5,6,1,0),(5,8,1,0),(5,10,1,0),(5,101,1,0),(10,601,1,0),(10,2601,1,0),(10,2701,1,0),(10,2801,1,0),(10,2901,1,0),(10,3001,1,0),(10,3101,1,0),(10,3201,1,0),(10,3301,1,0),(10,3401,1,0),(10,3501,1,0),(10,3601,1,0);
/*!40000 ALTER TABLE `item_discount_quantity` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_not_available`
--

DROP TABLE IF EXISTS `item_not_available`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_not_available` (
  `type` tinyint(3) unsigned NOT NULL default '0',
  `id` smallint(5) unsigned NOT NULL default '0',
  `start_tstamp` int(10) unsigned NOT NULL default '0',
  `lasts_until_tstamp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`type`,`id`),
  KEY `lasts_until_tstamp` (`lasts_until_tstamp`),
  KEY `start_tstamp` (`start_tstamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `item_not_available`
--

LOCK TABLES `item_not_available` WRITE;
/*!40000 ALTER TABLE `item_not_available` DISABLE KEYS */;
INSERT INTO `item_not_available` VALUES (1,2,0,4294967295),(1,3,0,4294967295),(1,5,0,4294967295),(1,6,0,4294967295),(1,10,0,4294967295),(1,101,0,4294967295),(1,102,0,4294967295),(1,103,0,4294967295),(1,105,1277960453,4294967295),(1,106,0,4294967295),(1,110,0,4294967295),(1,202,0,4294967295),(2,1,0,4294967295),(2,2,0,4294967295),(2,3,0,4294967295),(2,4,0,4294967295),(2,5,0,4294967295),(2,8,0,4294967295),(2,9,0,4294967295),(5,1,0,4294967295),(5,2,0,4294967295),(5,3,0,4294967295),(5,4,0,4294967295),(5,5,0,4294967295),(5,6,0,4294967295),(5,8,0,4294967295),(5,10,0,4294967295),(5,101,0,4294967295),(10,1101,0,4294967295),(10,1201,0,4294967295),(10,1301,0,4294967295),(10,1401,0,4294967295),(10,1501,0,4294967295),(10,1601,0,4294967295),(10,1701,0,4294967295),(10,1801,0,4294967295),(10,1901,0,4294967295),(10,2001,0,4294967295),(10,2101,0,4294967295),(10,2201,0,4294967295),(10,2301,0,4294967295),(10,2401,0,4294967295),(10,2501,0,4294967295),(10,2601,1288620000,4294967295),(10,2701,1288620000,4294967295),(10,2801,1288620000,4294967295),(10,2901,1294380000,4294967295),(10,3001,1294380000,4294967295),(10,3101,1294380000,4294967295),(10,3201,1294380000,4294967295),(10,3301,1294380000,4294967295),(10,3401,1294380000,4294967295),(10,3501,1294380000,4294967295),(10,3601,1294380000,4294967295);
/*!40000 ALTER TABLE `item_not_available` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_trade_offers`
--

DROP TABLE IF EXISTS `item_trade_offers`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_trade_offers` (
  `seller_id` bigint(20) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `id` smallint(5) unsigned NOT NULL,
  `buyer_id` bigint(20) unsigned NOT NULL,
  `price` smallint(5) unsigned NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`seller_id`,`type`,`id`,`buyer_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `item_trade_offers`
--

LOCK TABLES `item_trade_offers` WRITE;
/*!40000 ALTER TABLE `item_trade_offers` DISABLE KEYS */;
/*!40000 ALTER TABLE `item_trade_offers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `items`
--

DROP TABLE IF EXISTS `items`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `items` (
  `type` tinyint(3) unsigned NOT NULL,
  `id` smallint(5) unsigned NOT NULL,
  `shop_id` tinyint(3) unsigned NOT NULL,
  `cost` smallint(5) unsigned NOT NULL,
  `days_in_shop` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`type`,`id`),
  KEY `type` USING HASH (`type`),
  KEY `shop_id` USING HASH (`shop_id`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `items`
--

LOCK TABLES `items` WRITE;
/*!40000 ALTER TABLE `items` DISABLE KEYS */;
INSERT INTO `items` VALUES (1,1,1,15,0),(1,2,2,40,0),(1,3,2,40,0),(1,5,2,40,0),(1,6,2,40,0),(1,10,2,40,0),(1,101,1,15,0),(1,102,2,25,0),(1,103,2,25,0),(1,105,2,45,3),(1,106,2,25,0),(1,110,2,25,0),(1,202,3,60,3),(2,1,1,5,0),(2,2,2,40,0),(2,3,2,40,0),(2,4,2,40,0),(2,5,2,40,0),(2,8,2,40,0),(2,9,2,40,0),(5,1,2,30,0),(5,2,2,40,1),(5,3,2,40,0),(5,4,2,60,0),(5,5,2,40,0),(5,6,2,40,0),(5,8,2,40,0),(5,10,2,40,0),(5,101,3,60,0),(10,601,10,5,0),(10,1101,10,10000,0),(10,1201,10,10000,0),(10,1301,10,10000,0),(10,1401,10,10000,0),(10,1501,10,10000,0),(10,1601,10,10000,0),(10,1701,10,10000,0),(10,1801,10,10000,0),(10,1901,10,10000,0),(10,2001,10,10000,0),(10,2101,10,10000,0),(10,2201,10,10000,0),(10,2301,10,10000,0),(10,2401,10,10000,0),(10,2501,10,10000,0),(10,2601,10,5,7),(10,2701,10,10,7),(10,2801,10,66,7),(10,2901,10,22,3),(10,3001,10,55,29),(10,3101,10,55,3),(10,3201,10,33,20),(10,3301,10,33,3),(10,3401,10,99,3),(10,3501,10,99,3),(10,3601,10,55,27);
/*!40000 ALTER TABLE `items` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `klunker`
--

DROP TABLE IF EXISTS `klunker`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `klunker` (
  `uid` bigint(20) unsigned NOT NULL,
  `amount` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `klunker`
--

LOCK TABLES `klunker` WRITE;
/*!40000 ALTER TABLE `klunker` DISABLE KEYS */;
/*!40000 ALTER TABLE `klunker` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sms_payment_prices`
--

DROP TABLE IF EXISTS `sms_payment_prices`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_payment_prices` (
  `country` varchar(2) NOT NULL,
  `reference_price` smallint(5) unsigned NOT NULL,
  `price` smallint(5) unsigned NOT NULL,
  `currency` varchar(3) NOT NULL,
  PRIMARY KEY  (`country`,`reference_price`),
  KEY `country` (`country`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `sms_payment_prices`
--

LOCK TABLES `sms_payment_prices` WRITE;
/*!40000 ALTER TABLE `sms_payment_prices` DISABLE KEYS */;
/*!40000 ALTER TABLE `sms_payment_prices` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sms_payment_prices_tstamp`
--

DROP TABLE IF EXISTS `sms_payment_prices_tstamp`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_payment_prices_tstamp` (
  `country` varchar(2) NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`country`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `sms_payment_prices_tstamp`
--

LOCK TABLES `sms_payment_prices_tstamp` WRITE;
/*!40000 ALTER TABLE `sms_payment_prices_tstamp` DISABLE KEYS */;
/*!40000 ALTER TABLE `sms_payment_prices_tstamp` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `transaction_ids`
--

DROP TABLE IF EXISTS `transaction_ids`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `transaction_ids` (
  `uid` bigint(20) unsigned NOT NULL,
  `transaction_id` mediumint(8) unsigned NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`transaction_id`),
  KEY `uid` (`uid`),
  KEY `t_stamp` (`t_stamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `transaction_ids`
--

LOCK TABLES `transaction_ids` WRITE;
/*!40000 ALTER TABLE `transaction_ids` DISABLE KEYS */;
/*!40000 ALTER TABLE `transaction_ids` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `unique_ids`
--

DROP TABLE IF EXISTS `unique_ids`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `unique_ids` (
  `uid` bigint(20) unsigned NOT NULL,
  `heidelpay` varchar(32) default NULL,
  PRIMARY KEY  (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `unique_ids`
--

LOCK TABLES `unique_ids` WRITE;
/*!40000 ALTER TABLE `unique_ids` DISABLE KEYS */;
/*!40000 ALTER TABLE `unique_ids` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_chest_visits`
--

DROP TABLE IF EXISTS `user_chest_visits`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user_chest_visits` (
  `uid` bigint(20) unsigned NOT NULL default '0',
  `visitor` bigint(20) unsigned NOT NULL default '0',
  `t_stamp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`visitor`),
  KEY `t_stamp` USING BTREE (`t_stamp`),
  KEY `visitor` USING HASH (`visitor`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `user_chest_visits`
--

LOCK TABLES `user_chest_visits` WRITE;
/*!40000 ALTER TABLE `user_chest_visits` DISABLE KEYS */;
/*!40000 ALTER TABLE `user_chest_visits` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_items`
--

DROP TABLE IF EXISTS `user_items`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user_items` (
  `uid` bigint(20) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `id` smallint(5) unsigned NOT NULL,
  `amount` smallint(5) unsigned NOT NULL default '0',
  `price` smallint(5) unsigned NOT NULL default '0',
  `amount_mp` smallint(5) unsigned NOT NULL default '0',
  `exclude_direct` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`type`,`id`),
  KEY `type` (`type`),
  KEY `price` (`price`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `user_items`
--

LOCK TABLES `user_items` WRITE;
/*!40000 ALTER TABLE `user_items` DISABLE KEYS */;
/*!40000 ALTER TABLE `user_items` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_items_locks`
--

DROP TABLE IF EXISTS `user_items_locks`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user_items_locks` (
  `locker_uid` bigint(20) unsigned NOT NULL default '0',
  `type` tinyint(3) unsigned NOT NULL default '0',
  `id` smallint(5) unsigned NOT NULL default '0',
  `uid` bigint(20) unsigned NOT NULL default '0',
  `amount` smallint(5) unsigned NOT NULL default '0',
  `t_stamp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`locker_uid`,`type`,`id`),
  KEY `t_stamp` USING BTREE (`t_stamp`),
  KEY `locker_uid` USING HASH (`locker_uid`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `user_items_locks`
--

LOCK TABLES `user_items_locks` WRITE;
/*!40000 ALTER TABLE `user_items_locks` DISABLE KEYS */;
/*!40000 ALTER TABLE `user_items_locks` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-02-26 15:19:25
