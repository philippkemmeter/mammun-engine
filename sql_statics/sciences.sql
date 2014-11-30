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
-- Table structure for table `sciences`
--

DROP TABLE IF EXISTS `sciences`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sciences` (
  `sc_id` smallint(5) unsigned NOT NULL default '0',
  `level` tinyint(3) unsigned NOT NULL default '1',
  `type` tinyint(1) unsigned NOT NULL default '1',
  `grow` float unsigned NOT NULL default '1',
  `cost` float unsigned NOT NULL default '0',
  `resource1` float unsigned NOT NULL default '0',
  `resource2` float unsigned NOT NULL default '0',
  `hitpoints` smallint(5) NOT NULL default '0',
  `attack` smallint(4) NOT NULL default '0',
  `defend` smallint(4) NOT NULL default '0',
  `speed` smallint(4) NOT NULL default '0',
  `f_extra1` float unsigned NOT NULL default '0',
  `important` tinyint(1) NOT NULL default '0',
  `pic` varchar(25) NOT NULL default 'spacer.gif',
  `const_key` varchar(40) default NULL,
  PRIMARY KEY  (`sc_id`,`level`),
  KEY `sc_id` USING HASH (`sc_id`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `sciences`
--

LOCK TABLES `sciences` WRITE;
/*!40000 ALTER TABLE `sciences` DISABLE KEYS */;
INSERT INTO `sciences` VALUES (1,1,1,1,0,0,0,0,0,0,0,400,0,'pattern/spacer.gif','morphing'),(47,1,1,5,0,0,1,0,0,0,0,0,0,'pattern/spacer.gif','chicken_eggs'),(10,1,1,5,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','basalt'),(1001,1,1,26,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','reed_roof'),(1002,1,1,65,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','hut'),(1004,1,1,70,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','lumberjacks_hut'),(1007,1,1,35,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','farm'),(12,1,1,5,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','flintstone'),(13,1,1,7,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','pyrit'),(14,1,1,20,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','hedge'),(16,1,1,5,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','bast'),(17,1,1,6,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','flax'),(18,1,1,10,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','hemp'),(49,1,1,7,0,0,1,0,0,0,0,0,0,'pattern/spacer.gif','goose_eggs'),(22,1,1,12,0,0,0,0,0,0,0,0,1,'spacer.gif','fire'),(23,1,1,15,0,0,0,0,1,0,0,0,0,'pattern/spacer.gif','gelatine'),(24,1,1,5,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','resin'),(25,1,1,5,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','birch_pitch'),(26,1,1,35,0,0,1,0,0,0,0,0,0,'pattern/spacer.gif','containers'),(3020,1,31,36,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','walzenbeil'),(27,1,1,26,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','joint'),(29,1,1,4,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','fibre'),(46,1,1,5,0,0,1,0,0,0,0,0,0,'pattern/spacer.gif','quail_eggs'),(51,1,1,20,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','bowstring'),(3,1,1,4,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','trees'),(30,1,1,15,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','cord'),(3001,1,30,16,3.3,0,3.3,100,10,10,100,0,0,'javelin.png','javelin'),(3008,1,30,29,4,0,4,100,17,10,100,0,0,'javelin_stone.png','adv_javelin'),(3007,1,30,45,4.4,0,4.4,100,14,12,100,0,0,'bow.png','bow'),(3002,1,30,60,5,0,5,100,19,13,100,0,0,'bow_stone.png','adv_bow'),(3009,1,30,15,3.1,0,3.1,100,5,14,120,0,1,'blade.png','blade'),(3010,1,31,20,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','drill'),(3011,1,30,0.1,3,0,3,100,10,8,100,0,0,'club.png','club'),(3012,1,31,25,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','adze_beaten'),(3013,1,31,35,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','adze_sharpened'),(3014,1,30,35,3.5,0,3.5,100,16,8,100,0,0,'axe.png','axe_beaten'),(3016,1,30,54,3.6,0,3.6,100,19,8,100,0,0,'axe_sharpened.png','axe_sharpened'),(3017,1,31,16,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','digwood'),(3019,1,31,100,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','wheel'),(3021,1,30,56,5.1,0,5.1,100,25,13,100,0,0,'bow_sharpened.png','adv_bow_sharpened'),(3022,1,30,46,4.1,0,4.1,100,20,10,100,0,0,'javelin_sharpened.png','adv_javelin_sharpened'),(31,1,1,8,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','knot'),(32,1,1,30,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','nets'),(33,1,1,27,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','sling'),(34,1,1,36,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','glue'),(35,1,1,45,0,0,1,0,0,0,0,0,0,'pattern/spacer.gif','catch_partridge'),(36,1,1,72,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','sharpen'),(39,1,1,6,0,0,0,0,0,0,0,0,0,'spacer.gif','reed'),(48,1,1,7,0,0,1,0,0,0,0,0,0,'pattern/spacer.gif','duck_eggs'),(50,1,1,13,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','cornus'),(2,1,1,1,2,0,2,70,0,1,0,2,0,'scientist.gif','scientists'),(54,1,1,4,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','bones'),(4,1,1,0.0111,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','stones'),(40,1,1,1.5,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','skinning'),(4001,1,100,21,0.5,0,0.5,0,0,0,0,0,0,'camouflage.png','camouflage'),(4002,1,100,20,0.3,0,0.3,0,3,0,0,0,0,'move_trees.png','move_trees'),(4003,1,101,23,0.5,0,0.5,0,0,3,0,0,0,'stick_parry.png','stick_parry'),(4004,1,100,16,0.25,0,0.25,0,0,2,0,0,0,'train_abdominals.png','train_abdominals'),(4005,1,100,10,0.2,0,0.2,0,1,1,0,0,0,'push_up.png','push_up'),(4007,1,202,50,0.4,0,0.4,0,5,-1,0,0,0,'two_axes.png','two_axes'),(4008,1,103,45,0.3,0,0.3,0,4,1,0,0,0,'bow_aiming.png','bow_aiming'),(4009,1,103,39,0.5,0,0.4,0,3,2,0,0,0,'bow_direct_shot.png','bow_direct_shot'),(4010,1,103,42,0.25,0,0.25,0,3,1,0,0,0,'bow_moving_target.png','bow_moving_targets'),(4011,1,200,20,0.5,0,0.5,0,5,0,0,0,0,'atlatl.png','atlatl'),(4012,1,101,35,0.45,0,0.45,0,4,0,0,0,0,'javelin_aiming.png','javelin_aim'),(4013,1,101,38,0.4,0,0.4,0,2,1,0,0,0,'javelin_direct_shot.png','javelin_direct_throw'),(4014,1,101,32,0.2,0,0.2,0,2,0,0,0,0,'javelin_moving_target.png','javelin_moving_targets'),(4015,1,100,56,0.5,0,0.5,10,0,0,0,0,0,'herbs.png','herbs'),(4016,1,100,26,0.3,0,0.3,0,0,1,10,0,0,'rope_skipping.png','rope_skipping'),(41,1,1,0.0111,0,0,3,0,0,0,0,0,1,'pattern/spacer.gif','hunting'),(42,1,1,0.5,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','weapon_revolution'),(43,1,1,53,0,5,0,0,0,0,0,0,0,'pattern/spacer.gif','chop_down_trees'),(44,1,1,30,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','create_hedge'),(45,1,1,36,0,0,2,0,0,0,0,0,0,'pattern/spacer.gif','cook_meat'),(5,1,1,0.0111,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','plants'),(1009,1,1,5,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','shed'),(52,1,1,30,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','bow_pure'),(4006,1,102,24,0.45,0,0.45,0,0,3,0,0,0,'block.png','axe_block'),(53,1,1,25,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','arrows'),(1006,1,1,23,0,0,0,0,0,0,0,0,1,'pattern/spacer.gif','animal_skin_tent'),(55,1,1,53,0,3,0,0,0,0,0,0,0,'pattern/spacer.gif','chop_down_trees2'),(7,1,1,11,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','birch'),(8,1,1,20,0,1,0,0,0,0,0,0,0,'pattern/spacer.gif','lumber'),(9,1,1,9,0,0,0,0,0,0,0,0,0,'pattern/spacer.gif','bark');
/*!40000 ALTER TABLE `sciences` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-11-29 11:05:49
