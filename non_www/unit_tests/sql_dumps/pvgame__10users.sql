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
-- Table structure for table `artifacts`
--

DROP TABLE IF EXISTS `artifacts`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `artifacts` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `artifact_id` smallint(5) unsigned NOT NULL default '0',
  `user_id` int(10) unsigned NOT NULL default '0',
  `sector_id` int(10) unsigned NOT NULL default '0',
  `identified` tinyint(1) unsigned NOT NULL default '0',
  `state` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `artifact_id` (`artifact_id`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `artifacts`
--

LOCK TABLES `artifacts` WRITE;
/*!40000 ALTER TABLE `artifacts` DISABLE KEYS */;
/*!40000 ALTER TABLE `artifacts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `bugreports`
--

DROP TABLE IF EXISTS `bugreports`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `bugreports` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `user_ID` int(10) unsigned NOT NULL,
  `browser` varchar(50) default NULL,
  `os` varchar(20) default NULL,
  `resolution` varchar(13) default NULL,
  `window_size` varchar(13) default NULL,
  `bug_source` varchar(50) default NULL,
  `repro_desc` text character set utf8,
  `comment` text character set utf8,
  `datum` datetime default NULL,
  `status` tinyint(1) unsigned NOT NULL default '1',
  `cat_id` tinyint(3) unsigned NOT NULL default '0',
  `priority` tinyint(1) unsigned NOT NULL default '0',
  `fixcomment` text character set utf8,
  `fixdate` datetime default NULL,
  `joint_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=591 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `bugreports`
--

LOCK TABLES `bugreports` WRITE;
/*!40000 ALTER TABLE `bugreports` DISABLE KEYS */;
/*!40000 ALTER TABLE `bugreports` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `building`
--

DROP TABLE IF EXISTS `building`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `building` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `sector_id` tinyint(3) unsigned NOT NULL default '0',
  `ulc_x` tinyint(4) NOT NULL default '0',
  `ulc_y` tinyint(4) NOT NULL default '0',
  `uid` int(10) unsigned NOT NULL default '0',
  `to_build` tinyint(1) NOT NULL default '0',
  `finish_time` int(10) unsigned NOT NULL default '0',
  `progress` float(10,2) NOT NULL default '0.00',
  `t_stamp` int(10) unsigned NOT NULL default '0',
  `type` smallint(5) unsigned NOT NULL default '0',
  `color` tinyint(3) unsigned NOT NULL default '0',
  `painted_by_uid` int(10) unsigned NOT NULL default '0',
  `cur_level` tinyint(3) unsigned NOT NULL default '1',
  `time_built` int(10) unsigned NOT NULL default '0',
  `time_build_started` int(10) unsigned NOT NULL default '0',
  `extra` bigint(20) unsigned NOT NULL default '0',
  `spezi` int(10) unsigned NOT NULL default '0',
  `spezi_finished` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`sector_id`,`ulc_x`,`ulc_y`),
  KEY `uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `building`
--

LOCK TABLES `building` WRITE;
/*!40000 ALTER TABLE `building` DISABLE KEYS */;
/*!40000 ALTER TABLE `building` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `buildings`
--

DROP TABLE IF EXISTS `buildings`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `buildings` (
  `type` smallint(5) unsigned NOT NULL default '0',
  `level` tinyint(3) unsigned NOT NULL default '0',
  `cost` int(10) unsigned NOT NULL default '0',
  `hitpoints` int(10) unsigned NOT NULL default '0',
  `def` tinyint(3) unsigned NOT NULL default '0',
  `att` tinyint(3) unsigned NOT NULL default '0',
  `req_sc_id` int(10) unsigned NOT NULL default '0',
  `req_level` tinyint(2) unsigned NOT NULL default '0',
  `grow` smallint(4) NOT NULL default '1',
  `base_size` tinyint(3) unsigned NOT NULL default '0',
  `height_factor` float unsigned NOT NULL default '0',
  `pic` varchar(100) default NULL,
  `const_key` varchar(40) default NULL,
  PRIMARY KEY  (`type`,`level`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `buildings`
--

LOCK TABLES `buildings` WRITE;
/*!40000 ALTER TABLE `buildings` DISABLE KEYS */;
INSERT INTO `buildings` VALUES (1,0,2500,1000,23,0,1000,1,130,3,1,'techcenter.png','TECHCENTER'),(1,1,2500,1000,23,0,1000,1,130,3,1,'techcenter.png','TECHCENTER'),(1,2,2500,1450,24,0,1000,1,340,3,1,'techcenter.png','TECHCENTER'),(1,3,2300,1650,25,0,1000,1,410,3,1,'techcenter.png','TECHCENTER'),(1,4,2100,1800,26,0,1000,2,470,3,1,'techcenter.png','TECHCENTER'),(1,5,2050,2000,27,0,1000,3,540,3,1,'techcenter.png','TECHCENTER'),(1,6,2010,2200,29,0,1000,4,610,3,1,'techcenter.png','TECHCENTER'),(1,7,1995,2500,33,0,1000,5,680,3,1,'techcenter.png','TECHCENTER'),(2,0,200,50,3,0,1006,1,30,3,1,'tent.png','TENT'),(2,1,200,50,3,0,1006,1,30,3,1,'tent.png','TENT'),(3,0,1000,200,10,0,1002,1,170,3,1.32593,'dwelling.png','DWELLING'),(3,1,1000,200,10,0,1002,1,170,3,1.32593,'dwelling.png','DWELLING'),(4,0,800,500,16,0,1003,1,330,3,1,'adv_dwelling.png','ADV_DWELLING'),(4,1,800,500,16,0,1003,1,330,3,1,'adv_dwelling.png','ADV_DWELLING'),(5,0,700,200,15,0,9999,1,200,3,1,'hunting_lodge.png','HUNTING_LODGE'),(5,1,700,200,15,0,9999,1,200,3,1,'hunting_lodge.png','HUNTING_LODGE'),(6,0,1200,400,17,0,9999,1,570,3,1,'adv_hunting_lodge.png','ADV_HUNTING_LODGE'),(6,1,1200,400,17,0,9999,1,570,3,1,'adv_hunting_lodge.png','ADV_HUNTING_LODGE'),(7,0,100,150,15,0,1007,1,200,5,1,'farm.png','FARM'),(7,1,100,150,15,0,1007,1,200,5,1,'farm.png','FARM'),(8,0,600,200,15,0,9999,1,210,3,1,'lumberjacks_hut.png','LUMBERJACKS_HUT'),(8,1,600,200,15,0,9999,1,210,3,1,'lumberjacks_hut.png','LUMBERJACKS_HUT'),(9,0,1100,400,18,0,1005,1,350,3,1,'adv_lumberjacks_hut.png','ADV_LUMBERJACKS_HUT'),(9,1,1100,400,18,0,1005,1,350,3,1,'adv_lumberjacks_hut.png','ADV_LUMBERJACKS_HUT'),(10,0,5,30,1,0,1009,1,10,2,1,'shed.png','SHED'),(10,1,5,30,1,0,1009,1,10,2,1,'shed.png','SHED'),(100,0,3000,3000,40,0,1000,1,110,3,1,'mother.png','MOTHER_FUNGUS'),(100,1,3000,3000,40,0,1000,1,110,3,1,'mother.png','MOTHER_FUNGUS'),(100,2,2750,3450,42,0,1000,1,310,3,1,'mother.png','MOTHER_FUNGUS'),(100,3,2550,3675,43,0,1000,2,400,3,1,'mother.png','MOTHER_FUNGUS'),(100,4,2450,3900,45,0,1000,3,490,3,1,'mother.png','MOTHER_FUNGUS'),(100,5,2400,4125,46,0,1000,4,580,3,1,'mother.png','MOTHER_FUNGUS'),(100,6,2360,4375,50,0,1000,5,670,3,1,'mother.png','MOTHER_FUNGUS'),(100,7,2345,4500,54,0,1000,6,750,3,1,'mother.png','MOTHER_FUNGUS'),(100,8,2315,4800,59,0,1000,7,840,3,1,'mother.png','MOTHER_FUNGUS'),(100,9,2290,5200,65,0,1000,8,930,3,1,'mother.png','MOTHER_FUNGUS'),(200,0,20,50,1,0,9999,1,1,1,1,'christmas_tree.png','CHRISTMAS_TREE'),(200,1,20,50,1,0,9999,1,1,1,1,'christmas_tree.png','CHRISTMAS_TREE'),(201,0,20,50,1,0,9999,1,1,1,1.66667,'normal_tree.png','NORMAL_TREE'),(201,1,20,50,1,0,9999,1,1,1,1.66667,'normal_tree.png','NORMAL_TREE'),(301,0,50,1000,1,0,9999,1,1,2,1,'stone_big1.png','C_STONE_BIG1'),(301,1,50,1000,1,0,9999,1,1,2,1,'stone_big1.png','C_STONE_BIG1'),(401,1,50,1000,1,0,9999,1,1,2,1,'stone_big2.png','C_STONE_BIG2'),(401,0,50,1000,1,0,9999,1,1,2,1,'stone_big2.png','C_STONE_BIG2'),(501,0,50,1000,1,0,9999,1,1,2,1,'stone_big3.png','C_STONE_BIG3'),(501,1,50,1000,1,0,9999,1,1,2,1,'stone_big3.png','C_STONE_BIG3'),(601,0,0,1000,1,0,9999,1,1,1,1,'stone_small1.png','C_STONE_SMALL1'),(601,1,0,1000,1,0,9999,1,1,1,1,'stone_small1.png','C_STONE_SMALL1'),(701,0,50,1000,1,0,9999,1,1,1,1,'stone_small2.png','C_STONE_SMALL2'),(701,1,50,1000,1,0,9999,1,1,1,1,'stone_small2.png','C_STONE_SMALL2'),(801,0,50,1000,1,0,9999,1,1,1,1.66667,'oak_tree1.png','C_OAK_TREE1'),(801,1,50,1000,1,0,9999,1,1,1,1.66667,'oak_tree1.png','C_OAK_TREE1'),(901,0,0,1000,1,0,9999,1,1,1,1.66667,'fir_tree1.png','C_FIR_TREE1'),(901,1,0,1000,1,0,9999,1,1,1,1.66667,'fir_tree1.png','C_FIR_TREE1'),(1001,1,0,1000,1,0,9999,1,1,1,1.66667,'palm_tree1.png','C_PALM_TREE1'),(1001,0,0,1000,1,0,9999,1,1,1,1.66667,'palm_tree1.png','C_PALM_TREE1'),(1101,0,0,1000,1,0,9999,1,1,5,1.15556,'obilisk1.png','C_OBILISK1'),(1101,1,0,1000,1,0,9999,1,1,5,1.15556,'obilisk1.png','C_OBILISK1'),(1201,1,0,1000,1,0,9999,1,1,5,1,'pyramide1.png','C_PYRAMIDE1'),(1201,0,0,1000,1,0,9999,1,1,5,1,'pyramide1.png','C_PYRAMIDE1'),(1301,0,0,1000,1,0,9999,1,1,2,1,'letter_type1.png','C_LETTER_TYPE1'),(1301,1,0,1000,1,0,9999,1,1,2,1,'letter_type1.png','C_LETTER_TYPE1'),(1401,1,0,1000,1,0,9999,1,1,1,1.66667,'stela1.png','C_STELA1'),(1401,0,0,1000,1,0,9999,1,1,1,1.66667,'stela1.png','C_STELA1'),(1601,0,0,1000,1,0,9999,1,1,2,1.17778,'double_stela1.png','C_DOUBLE_STELA1'),(1601,1,0,1000,1,0,9999,1,1,2,1.17778,'double_stela1.png','C_DOUBLE_STELA1'),(1701,0,0,1000,1,0,9999,1,1,1,1.48889,'stone_pillar1.png','C_STONE_PILLAR1'),(1701,1,0,1000,1,0,9999,1,1,1,1.48889,'stone_pillar1.png','C_STONE_PILLAR1'),(1501,1,0,1000,1,0,9999,1,1,2,1,'stonehenge1.png','C_STONEHENGE1'),(1501,0,0,1000,1,0,9999,1,1,2,1,'stonehenge1.png','C_STONEHENGE1'),(1801,0,0,1000,1,0,9999,1,1,4,1.48148,'pharos.png','C_PHAROS'),(1801,1,0,1000,1,0,9999,1,1,4,1.48148,'pharos.png','C_PHAROS'),(1901,0,0,1000,1,0,9999,1,1,5,1.1378,'triumphal_arch.png','C_TRIUMPHAL_ARCH'),(1901,1,0,1000,1,0,9999,1,1,5,1.1378,'triumphal_arch.png','C_TRIUMPHAL_ARCH'),(2001,1,0,1000,1,0,9999,1,1,5,1,'maya_pyramide1.png','C_MAYA_PYRAMIDE1'),(2001,0,0,1000,1,0,9999,1,1,5,1,'maya_pyramide1.png','C_MAYA_PYRAMIDE1'),(2101,0,0,1000,1,0,9999,1,1,3,1,'pile_of_stones.png','C_PILE_OF_STONES'),(2101,1,0,1000,1,0,9999,1,1,3,1,'pile_of_stones.png','C_PILE_OF_STONES'),(2201,1,0,1000,1,0,9999,1,1,4,1,'sundial.png','C_SUNDIAL'),(2201,0,0,1000,1,0,9999,1,1,4,1,'sundial.png','C_SUNDIAL'),(2301,0,0,1000,1,0,9999,1,1,3,1.48148,'geyser.png','C_GEYSER'),(2301,1,0,1000,1,0,9999,1,1,3,1.48148,'geyser.png','C_GEYSER'),(2401,0,0,1000,1,0,9999,1,1,5,1,'waterfall.png','C_WATERFALL'),(2401,1,0,1000,1,0,9999,1,1,5,1,'waterfall.png','C_WATERFALL'),(2501,0,0,1000,1,0,9999,1,1,5,1.88889,'eiffel_tower.png','C_EIFFEL_TOWER'),(2501,1,0,1000,1,0,9999,1,1,5,1.88889,'eiffel_tower.png','C_EIFFEL_TOWER'),(2701,1,0,1000,1,0,9999,1,1,1,1.22222,'halloween_scarecrow.png','C_HALLOWEEN_SCARECROW'),(2701,0,0,1000,1,0,9999,1,1,1,1.22222,'halloween_scarecrow.png','C_HALLOWEEN_SCARECROW'),(2601,0,0,1000,1,0,9999,1,1,1,1,'halloween_pumpkin.png','C_HALLOWEEN_PUMPKIN'),(2601,1,0,1000,1,0,9999,1,1,1,1,'halloween_pumpkin.png','C_HALLOWEEN_PUMPKIN'),(2801,0,0,1000,1,0,9999,1,1,5,1.06667,'halloween_boneyard.png','C_HALLOWEEN_BONEYARD'),(2801,1,0,1000,1,0,9999,1,1,5,1.06667,'halloween_boneyard.png','C_HALLOWEEN_BONEYARD'),(2901,0,0,1000,1,0,9999,1,1,1,1.33333,'christmas_scarecrow.png','C_CHRISTMAS_SCARECROW'),(2901,1,0,1000,1,0,9999,1,1,1,1.33333,'christmas_scarecrow.png','C_CHRISTMAS_SCARECROW'),(3101,0,0,1000,1,0,9999,1,1,3,1,'christmas_tree_devasted.png','C_CHRISTMAS_TREE_DEVASTED'),(3101,1,0,1000,1,0,9999,1,1,3,1,'christmas_tree_devasted.png','C_CHRISTMAS_TREE_DEVASTED'),(3201,0,0,1000,1,0,9999,1,1,2,1,'christmas_presents.png','C_CHRISTMAS_PRESENTS'),(3201,1,0,1000,1,0,9999,1,1,2,1,'christmas_presents.png','C_CHRISTMAS_PRESENTS'),(3401,1,0,1000,1,0,9999,1,1,5,1,'christmas_slide.png','C_CHRISTMAS_SLIDE'),(3401,0,0,1000,1,0,9999,1,1,5,1,'christmas_slide.png','C_CHRISTMAS_SLIDE'),(3501,0,0,1000,1,0,9999,1,1,5,1,'christmas_slide_devasted.png','C_CHRISTMAS_SLIDE_DEVASTED'),(3601,0,0,1000,1,0,9999,1,1,2,1,'christmas_snowman.png','C_CHRISTMAS_SNOWMAN'),(3601,1,0,1000,1,0,9999,1,1,2,1,'christmas_snowman.png','C_CHRISTMAS_SNOWMAN'),(3001,0,0,1000,1,0,9999,1,1,3,1.18519,'christmas_tree.png','C_CHRISTMAS_TREE'),(3001,1,0,1000,1,0,9999,1,1,3,1.18519,'christmas_tree.png','C_CHRISTMAS_TREE'),(3301,0,0,1000,1,0,9999,1,1,2,1,'christmas_presents_bag.png','C_CHRISTMAS_PRESENTS_SACK'),(3301,1,0,1000,1,0,9999,1,1,2,1,'christmas_presents_bag.png','C_CHRISTMAS_PRESENTS_SACK'),(3501,1,0,1000,1,0,9999,1,1,5,1,'christmas_slide_devasted.png','C_CHRISTMAS_SLIDE_DEVASTED');
/*!40000 ALTER TABLE `buildings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `computed_bio`
--

DROP TABLE IF EXISTS `computed_bio`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `computed_bio` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `compute_id` int(10) unsigned NOT NULL,
  `user_id` int(10) unsigned NOT NULL default '0',
  `user_units_died` float unsigned NOT NULL default '0',
  `user_buildings_destroyed` float unsigned NOT NULL default '0',
  `user_bio_destroyed` float unsigned NOT NULL default '0',
  `user_sp_destroyed` float unsigned NOT NULL default '0',
  `enemy_units_killed` float unsigned NOT NULL default '0',
  `enemy_buildings_destroyed` float unsigned NOT NULL default '0',
  `enemy_bio_destroyed` float unsigned NOT NULL default '0',
  `enemy_sp_destroyed` float unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`compute_id`,`user_id`),
  KEY `compute_id` (`compute_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `computed_bio`
--

LOCK TABLES `computed_bio` WRITE;
/*!40000 ALTER TABLE `computed_bio` DISABLE KEYS */;
/*!40000 ALTER TABLE `computed_bio` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `computed_buildings`
--

DROP TABLE IF EXISTS `computed_buildings`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `computed_buildings` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `sector_id` tinyint(3) unsigned NOT NULL default '0',
  `ulc_x` tinyint(3) unsigned NOT NULL default '0',
  `ulc_y` tinyint(3) unsigned NOT NULL default '0',
  `compute_id` int(10) unsigned NOT NULL,
  `hitpoints` float unsigned NOT NULL default '0',
  `killed` float unsigned NOT NULL default '0',
  `died` float unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`sector_id`,`ulc_x`,`ulc_y`,`compute_id`),
  KEY `compute_id` (`compute_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `computed_buildings`
--

LOCK TABLES `computed_buildings` WRITE;
/*!40000 ALTER TABLE `computed_buildings` DISABLE KEYS */;
/*!40000 ALTER TABLE `computed_buildings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `computed_extra`
--

DROP TABLE IF EXISTS `computed_extra`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `computed_extra` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `compute_id` int(10) unsigned NOT NULL,
  `sector_id` tinyint(3) unsigned NOT NULL,
  `user_id` int(10) unsigned NOT NULL default '0',
  `sector_resource1` float NOT NULL default '0',
  `sector_resource2` float NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`compute_id`,`sector_id`,`user_id`),
  KEY `compute_id` (`compute_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `computed_extra`
--

LOCK TABLES `computed_extra` WRITE;
/*!40000 ALTER TABLE `computed_extra` DISABLE KEYS */;
/*!40000 ALTER TABLE `computed_extra` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `computed_maps`
--

DROP TABLE IF EXISTS `computed_maps`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `computed_maps` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `compute_id` int(10) unsigned NOT NULL auto_increment,
  `time_computed` int(10) unsigned NOT NULL default '0',
  `insert_finished` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`compute_id`),
  KEY `map_id` (`world_id`)
) ENGINE=MyISAM AUTO_INCREMENT=102238 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `computed_maps`
--

LOCK TABLES `computed_maps` WRITE;
/*!40000 ALTER TABLE `computed_maps` DISABLE KEYS */;
/*!40000 ALTER TABLE `computed_maps` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `computed_map_data`
--
DROP TABLE IF EXISTS `computed_map_data`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `computed_map_data` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `compute_id` int(10) unsigned NOT NULL,
  `user_id` int(10) unsigned NOT NULL default '0',
  `user_units_died` float unsigned NOT NULL default '0',
  `user_buildings_destroyed` float unsigned NOT NULL default '0',
  `user_bio_destroyed` float unsigned NOT NULL default '0',
  `user_sp_destroyed` float unsigned NOT NULL default '0',
  `enemy_units_killed` float unsigned NOT NULL default '0',
  `enemy_buildings_destroyed` float unsigned NOT NULL default '0',
  `enemy_bio_destroyed` float unsigned NOT NULL default '0',
  `enemy_sp_destroyed` float unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`compute_id`,`user_id`),
  KEY `compute_id` (`compute_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

LOCK TABLES `computed_map_data` WRITE;
/*!40000 ALTER TABLE `computed_map_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `computed_map_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `computed_sector_data`
--
DROP TABLE IF EXISTS `computed_sector_data`;
SET @saved_cs_client	= @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `computed_sector_data` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `sector_id` tinyint(3) unsigned NOT NULL default '0',
  `compute_id` bigint(20) unsigned NOT NULL default '0',
  `owner` bigint(20) unsigned NOT NULL default '0',
  `status` tinyint(4) NOT NULL default '0',
  `t_stamp_start` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`sector_id`,`compute_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

LOCK TABLES `computed_sector_data` WRITE;
/*!40000 ALTER TABLE `computed_sector_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `computed_sector_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `computed_tribes`
--

DROP TABLE IF EXISTS `computed_tribes`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `computed_tribes` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `compute_id` int(10) unsigned NOT NULL,
  `tribe_id` int(10) unsigned NOT NULL default '0',
  `sector_id` tinyint(3) unsigned NOT NULL,
  `amount` float NOT NULL default '0',
  `killed` float unsigned NOT NULL default '0',
  `died` float unsigned NOT NULL default '0',
  `absorbed` int(10) unsigned NOT NULL default '0',
  `eaten` int(10) unsigned NOT NULL default '0',
  `camouflaged` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`compute_id`,`tribe_id`),
  KEY `compute_id` (`compute_id`),
  KEY `tribe_id_k` (`tribe_id`),
  KEY `sector_id` (`sector_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `computed_tribes`
--

LOCK TABLES `computed_tribes` WRITE;
/*!40000 ALTER TABLE `computed_tribes` DISABLE KEYS */;
/*!40000 ALTER TABLE `computed_tribes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hacks`
--

DROP TABLE IF EXISTS `hacks`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `hacks` (
  `uid` int(10) unsigned NOT NULL default '0',
  `attack_type` tinyint(3) unsigned NOT NULL default '0',
  `attack_count` int(10) unsigned NOT NULL default '0',
  `last_attack` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`uid`,`attack_type`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `hacks`
--

LOCK TABLES `hacks` WRITE;
/*!40000 ALTER TABLE `hacks` DISABLE KEYS */;
/*!40000 ALTER TABLE `hacks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `invitation_map_email`
--

DROP TABLE IF EXISTS `invitation_map_email`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `invitation_map_email` (
  `uid` bigint(20) unsigned NOT NULL,
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `email` varchar(100) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `invitation_map_email`
--

LOCK TABLES `invitation_map_email` WRITE;
/*!40000 ALTER TABLE `invitation_map_email` DISABLE KEYS */;
/*!40000 ALTER TABLE `invitation_map_email` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `invitation_map_fb`
--

DROP TABLE IF EXISTS `invitation_map_fb`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `invitation_map_fb` (
  `uid` bigint(20) unsigned NOT NULL,
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `fb_id` bigint(20) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`world_id`,`map_id`,`fb_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `invitation_map_fb`
--

LOCK TABLES `invitation_map_fb` WRITE;
/*!40000 ALTER TABLE `invitation_map_fb` DISABLE KEYS */;
/*!40000 ALTER TABLE `invitation_map_fb` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `invitation_map_ingame`
--

DROP TABLE IF EXISTS `invitation_map_ingame`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `invitation_map_ingame` (
  `uid` bigint(20) unsigned NOT NULL,
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `uid2` bigint(20) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`world_id`,`map_id`,`uid2`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `invitation_map_ingame`
--

LOCK TABLES `invitation_map_ingame` WRITE;
/*!40000 ALTER TABLE `invitation_map_ingame` DISABLE KEYS */;
/*!40000 ALTER TABLE `invitation_map_ingame` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `invitation_mentor_email`
--

DROP TABLE IF EXISTS `invitation_mentor_email`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `invitation_mentor_email` (
  `uid` bigint(20) unsigned NOT NULL,
  `email` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`uid`,`email`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `invitation_mentor_email`
--

LOCK TABLES `invitation_mentor_email` WRITE;
/*!40000 ALTER TABLE `invitation_mentor_email` DISABLE KEYS */;
/*!40000 ALTER TABLE `invitation_mentor_email` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `invitation_mentor_fb`
--

DROP TABLE IF EXISTS `invitation_mentor_fb`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `invitation_mentor_fb` (
  `uid` bigint(20) unsigned NOT NULL,
  `fb_id` bigint(20) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`fb_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `invitation_mentor_fb`
--

LOCK TABLES `invitation_mentor_fb` WRITE;
/*!40000 ALTER TABLE `invitation_mentor_fb` DISABLE KEYS */;
/*!40000 ALTER TABLE `invitation_mentor_fb` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ips`
--

DROP TABLE IF EXISTS `ips`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `ips` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `uid` int(10) unsigned NOT NULL default '0',
  `ip` varchar(15) NOT NULL default '',
  `t_stamp` int(10) unsigned NOT NULL default '0',
  `neverdrop` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `uid` (`uid`,`neverdrop`)
) ENGINE=MyISAM AUTO_INCREMENT=3144 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `ips`
--

LOCK TABLES `ips` WRITE;
/*!40000 ALTER TABLE `ips` DISABLE KEYS */;
/*!40000 ALTER TABLE `ips` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_restrictions`
--

DROP TABLE IF EXISTS `item_restrictions`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_restrictions` (
  `type` tinyint(3) unsigned NOT NULL,
  `id` smallint(5) unsigned NOT NULL,
  `brezeln` smallint(5) unsigned NOT NULL,
  `usr_level` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`type`,`id`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `item_restrictions`
--

LOCK TABLES `item_restrictions` WRITE;
/*!40000 ALTER TABLE `item_restrictions` DISABLE KEYS */;
/*!40000 ALTER TABLE `item_restrictions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_statistics_cache`
--

DROP TABLE IF EXISTS `item_statistics_cache`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_statistics_cache` (
  `type` tinyint(3) unsigned NOT NULL default '0',
  `id` smallint(5) unsigned NOT NULL default '0',
  `rareness_factor` tinyint(3) unsigned default NULL,
  PRIMARY KEY  (`type`,`id`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `item_statistics_cache`
--

LOCK TABLES `item_statistics_cache` WRITE;
/*!40000 ALTER TABLE `item_statistics_cache` DISABLE KEYS */;
/*!40000 ALTER TABLE `item_statistics_cache` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_statistics_cache_1`
--

DROP TABLE IF EXISTS `item_statistics_cache_1`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `item_statistics_cache_1` (
  `cache_id` tinyint(3) unsigned NOT NULL default '0',
  `last_update` int(10) unsigned default NULL,
  PRIMARY KEY  (`cache_id`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `item_statistics_cache_1`
--

LOCK TABLES `item_statistics_cache_1` WRITE;
/*!40000 ALTER TABLE `item_statistics_cache_1` DISABLE KEYS */;
/*!40000 ALTER TABLE `item_statistics_cache_1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `level_up_presents`
--

DROP TABLE IF EXISTS `level_up_presents`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `level_up_presents` (
  `level` smallint(5) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `id` smallint(5) unsigned NOT NULL,
  `blueprint` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`level`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `level_up_presents`
--

LOCK TABLES `level_up_presents` WRITE;
/*!40000 ALTER TABLE `level_up_presents` DISABLE KEYS */;
/*!40000 ALTER TABLE `level_up_presents` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `m_event`
--

DROP TABLE IF EXISTS `m_event`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `m_event` (
  `uid` bigint(20) unsigned NOT NULL default '0',
  `num` smallint(5) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL default '0',
  `t_stamp` int(10) unsigned NOT NULL,
  `extra` int(10) unsigned NOT NULL,
  `extra_str` varchar(60) NOT NULL default '',
  `score` smallint(5) unsigned NOT NULL default '0',
  `flags` tinyint(3) unsigned NOT NULL default '0',
  `medal_id` smallint(6) NOT NULL default '0',
  PRIMARY KEY  (`uid`,`num`),
  KEY `num` (`num`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 PACK_KEYS=1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `m_event`
--

LOCK TABLES `m_event` WRITE;
/*!40000 ALTER TABLE `m_event` DISABLE KEYS */;
/*!40000 ALTER TABLE `m_event` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `m_event_filter`
--

DROP TABLE IF EXISTS `m_event_filter`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `m_event_filter` (
  `uid` bigint(20) unsigned NOT NULL,
  `filter_engine` bigint(20) unsigned NOT NULL default '0',
  `filter_payment` smallint(5) unsigned NOT NULL default '0',
  `filter_special` smallint(5) unsigned NOT NULL default '0',
  `filter_special_popups` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `m_event_filter`
--

LOCK TABLES `m_event_filter` WRITE;
/*!40000 ALTER TABLE `m_event_filter` DISABLE KEYS */;
/*!40000 ALTER TABLE `m_event_filter` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `m_events_random`
--

DROP TABLE IF EXISTS `m_events_random`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `m_events_random` (
  `type` tinyint(3) unsigned NOT NULL,
  `times_to_show` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`type`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `m_events_random`
--

LOCK TABLES `m_events_random` WRITE;
/*!40000 ALTER TABLE `m_events_random` DISABLE KEYS */;
/*!40000 ALTER TABLE `m_events_random` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `map_reserved_maps`
--

DROP TABLE IF EXISTS `map_reserved_maps`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `map_reserved_maps` (
  `world_id` int(10) unsigned NOT NULL,
  `map_id` tinyint(2) unsigned NOT NULL,
  `auth_key` varchar(6) default NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`world_id`,`map_id`),
  KEY `auth_key` (`auth_key`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `map_reserved_maps`
--

LOCK TABLES `map_reserved_maps` WRITE;
/*!40000 ALTER TABLE `map_reserved_maps` DISABLE KEYS */;
/*!40000 ALTER TABLE `map_reserved_maps` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `map_reserved_slots`
--

DROP TABLE IF EXISTS `map_reserved_slots`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `map_reserved_slots` (
  `res_by_uid` bigint(20) unsigned NOT NULL,
  `auth_key` varchar(32) NOT NULL,
  `world_id` int(10) unsigned NOT NULL,
  `map_id` tinyint(3) unsigned NOT NULL,
  `team_id` tinyint(1) unsigned NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`res_by_uid`),
  KEY `auth_key` (`auth_key`),
  KEY `world_id` (`world_id`,`map_id`,`team_id`),
  KEY `t_stamp` (`t_stamp`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `map_reserved_slots`
--

LOCK TABLES `map_reserved_slots` WRITE;
/*!40000 ALTER TABLE `map_reserved_slots` DISABLE KEYS */;
/*!40000 ALTER TABLE `map_reserved_slots` ENABLE KEYS */;
UNLOCK TABLES;
--
-- Table structure for table `maps`
--

DROP TABLE IF EXISTS `maps`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `maps` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `template_id` tinyint(3) unsigned NOT NULL default '0',
  `template_variation` int(10) unsigned NOT NULL default '0',
  `type` smallint(2) unsigned NOT NULL default '0',
  `state` tinyint(3) unsigned NOT NULL default '0',
  `inserted` int(10) unsigned NOT NULL default '0',
  `started` int(10) unsigned NOT NULL default '0',
  `finished` int(10) unsigned NOT NULL default '0',
  `winner` int(10) unsigned NOT NULL default '0',
  `winner_homesector` int(10) unsigned NOT NULL default '0',
  `avg_player_level` float unsigned NOT NULL default '0',
  `mail_sent` tinyint(1) unsigned NOT NULL default '0',
  `timefactor` float unsigned NOT NULL default '1',
  `max_unit_limit` smallint(5) unsigned NOT NULL default '0',
  `mentor` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `maps`
--

LOCK TABLES `maps` WRITE;
/*!40000 ALTER TABLE `maps` DISABLE KEYS */;
INSERT INTO `maps` VALUES (0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0);
/*!40000 ALTER TABLE `maps` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `medals`
--

DROP TABLE IF EXISTS `medals`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `medals` (
  `id` smallint(5) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `subtype` smallint(5) unsigned NOT NULL default '0',
  `amount` smallint(5) unsigned NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `type` (`type`,`subtype`),
  KEY `id` (`id`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `medals`
--

LOCK TABLES `medals` WRITE;
/*!40000 ALTER TABLE `medals` DISABLE KEYS */;
INSERT INTO `medals` VALUES (1,2,0,5),(12,2,3,1),(13,2,0,25),(14,2,0,50),(15,2,0,100),(2,1,3009,1),(3,1,1006,1),(20,3,10,1),(21,3,0,3),(22,3,0,7),(23,3,2,1),(24,3,7,1),(25,3,3,1),(26,3,0,50),(27,3,0,100),(40,4,1,2),(41,4,1,5),(42,4,1,50),(43,4,1,100),(44,4,1,200),(50,4,2,1),(51,4,2,5),(16,2,0,200),(52,4,2,50),(53,4,2,100),(60,4,3,1),(61,4,3,5),(17,2,0,500),(18,2,0,1000),(28,3,0,200),(29,3,0,500),(30,3,0,1000),(45,4,1,500),(46,4,1,1000),(54,4,2,200),(55,4,2,500),(56,4,2,1000),(62,4,3,50),(63,4,3,100),(64,4,3,200),(65,4,3,500),(66,4,3,1000),(70,6,3011,1),(71,6,3011,50),(72,6,3009,1),(73,6,3009,50),(74,6,3001,1),(75,6,3001,50),(76,6,3007,1),(77,6,3007,50),(78,6,3014,1),(79,6,3014,50),(80,6,3008,1),(81,6,3008,50),(82,6,3002,1),(83,6,3002,50),(84,6,3016,1),(85,6,3016,50),(86,6,3022,1),(87,6,3022,50),(88,6,3021,1),(89,6,3021,50),(90,6,2,1),(91,6,2,50),(100,7,0,1),(101,7,0,5),(102,7,0,10),(103,7,0,20),(104,7,0,50),(105,7,0,100),(110,8,0,1),(112,8,0,50),(113,8,0,150),(114,8,0,250),(115,8,0,500),(116,8,0,1000),(111,8,0,10);
/*!40000 ALTER TABLE `medals` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `notes`
--

DROP TABLE IF EXISTS `notes`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `notes` (
  `uid` int(10) unsigned NOT NULL,
  `note` text character set utf8,
  `t_stamp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `notes`
--

LOCK TABLES `notes` WRITE;
/*!40000 ALTER TABLE `notes` DISABLE KEYS */;
/*!40000 ALTER TABLE `notes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `players_on_map`
--

DROP TABLE IF EXISTS `players_on_map`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `players_on_map` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `user_id` int(10) unsigned NOT NULL default '0',
  `handicapped` tinyint(3) unsigned NOT NULL default '0',
  `last_map_score` mediumint(9) NOT NULL default '0',
  `killer_id` int(10) unsigned NOT NULL default '0',
  `kill_time` int(10) unsigned NOT NULL default '0',
  `start_time` int(10) unsigned NOT NULL default '0',
  `color` enum('black','blue','green','orange','pink','purple','red','white','yellow') NOT NULL default 'black',
  `homesector` tinyint(2) unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`user_id`),
  KEY `user_id` (`user_id`),
  KEY `map_id` (`world_id`,`killer_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `players_on_map`
--

LOCK TABLES `players_on_map` WRITE;
/*!40000 ALTER TABLE `players_on_map` DISABLE KEYS */;
/*!40000 ALTER TABLE `players_on_map` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `players_waiting`
--

DROP TABLE IF EXISTS `players_waiting`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `players_waiting` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `player_id` int(10) unsigned NOT NULL default '0',
  `clan_id` mediumint(8) unsigned NOT NULL default '0',
  `handicap` smallint(5) NOT NULL default '-1',
  `accepted` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `player_id` (`player_id`),
  KEY `clan_id` (`clan_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `players_waiting`
--

LOCK TABLES `players_waiting` WRITE;
/*!40000 ALTER TABLE `players_waiting` DISABLE KEYS */;
/*!40000 ALTER TABLE `players_waiting` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `race_abilities`
--

DROP TABLE IF EXISTS `race_abilities`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `race_abilities` (
  `race_sc_id` int(11) unsigned NOT NULL,
  `ability_type` int(11) unsigned NOT NULL,
  `amount` tinyint(3) unsigned NOT NULL default '1',
  PRIMARY KEY  (`race_sc_id`,`ability_type`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `race_abilities`
--

LOCK TABLES `race_abilities` WRITE;
/*!40000 ALTER TABLE `race_abilities` DISABLE KEYS */;
INSERT INTO `race_abilities` VALUES (3001,100,0),(3001,101,0),(3001,200,0),(3002,100,0),(3002,103,0),(3003,100,0),(3004,100,0),(3005,100,0),(3006,100,0),(3007,100,0),(3007,103,0),(3008,100,0),(3008,101,0),(3008,200,0),(3009,100,0),(3011,100,0),(3011,102,0),(3014,100,0),(3014,102,0),(3014,202,0),(3015,100,0),(3015,102,0),(3015,202,0),(3016,100,0),(3016,102,0),(3016,202,0),(3021,100,0),(3021,103,0),(3022,100,0),(3022,200,0),(3022,101,0);
/*!40000 ALTER TABLE `race_abilities` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `resource`
--

DROP TABLE IF EXISTS `resource`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `resource` (
  `uid` int(10) unsigned NOT NULL,
  `resource0` float NOT NULL default '0',
  `resource0_tstamp` int(10) unsigned NOT NULL default '0',
  `resource1` float NOT NULL default '0',
  `resource1_tstamp` int(10) unsigned NOT NULL default '0',
  `resource2` float NOT NULL default '0',
  `resource2_tstamp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `resource`
--

LOCK TABLES `resource` WRITE;
/*!40000 ALTER TABLE `resource` DISABLE KEYS */;
/*!40000 ALTER TABLE `resource` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `science`
--

DROP TABLE IF EXISTS `science`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `science` (
  `uid` int(10) unsigned NOT NULL default '0',
  `sc_id` mediumint(4) unsigned NOT NULL default '0',
  `count` mediumint(8) unsigned NOT NULL default '0',
  `l_stamp` int(10) unsigned NOT NULL default '0',
  `l_progress` float(14,12) unsigned NOT NULL default '0.000000000000',
  `finish_time` int(10) unsigned NOT NULL default '3000000000',
  `cur_level` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`sc_id`),
  KEY `uid` (`uid`,`sc_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `science`
--

LOCK TABLES `science` WRITE;
/*!40000 ALTER TABLE `science` DISABLE KEYS */;
INSERT INTO `science` VALUES (3760,2,0,0,0.000000000000,3000000000,1),(3760,1,0,0,0.000000000000,3000000000,1),(3760,3011,0,0,0.000000000000,3000000000,1),(3760,1009,0,0,0.000000000000,3000000000,1),(3762,2,0,0,0.000000000000,3000000000,1),(3762,1,0,0,0.000000000000,3000000000,1),(3762,3011,0,0,0.000000000000,3000000000,1),(3762,1009,0,0,0.000000000000,3000000000,1),(3764,2,0,0,0.000000000000,3000000000,1),(3764,1,0,0,0.000000000000,3000000000,1),(3764,3011,0,0,0.000000000000,3000000000,1),(3764,1009,0,0,0.000000000000,3000000000,1),(3766,2,0,0,0.000000000000,3000000000,1),(3766,1,0,0,0.000000000000,3000000000,1),(3766,3011,0,0,0.000000000000,3000000000,1),(3766,1009,0,0,0.000000000000,3000000000,1),(3768,2,0,0,0.000000000000,3000000000,1),(3768,1,0,0,0.000000000000,3000000000,1),(3768,3011,0,0,0.000000000000,3000000000,1),(3768,1009,0,0,0.000000000000,3000000000,1),(3770,2,0,0,0.000000000000,3000000000,1),(3770,1,0,0,0.000000000000,3000000000,1),(3770,3011,0,0,0.000000000000,3000000000,1),(3770,1009,0,0,0.000000000000,3000000000,1),(3772,2,0,0,0.000000000000,3000000000,1),(3772,1,0,0,0.000000000000,3000000000,1),(3772,3011,0,0,0.000000000000,3000000000,1),(3772,1009,0,0,0.000000000000,3000000000,1),(3774,2,0,0,0.000000000000,3000000000,1),(3774,1,0,0,0.000000000000,3000000000,1),(3774,3011,0,0,0.000000000000,3000000000,1),(3774,1009,0,0,0.000000000000,3000000000,1),(3776,2,0,0,0.000000000000,3000000000,1),(3776,1,0,0,0.000000000000,3000000000,1),(3776,3011,0,0,0.000000000000,3000000000,1),(3776,1009,0,0,0.000000000000,3000000000,1);
/*!40000 ALTER TABLE `science` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `science_keeping`
--

DROP TABLE IF EXISTS `science_keeping`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `science_keeping` (
  `uid` int(10) unsigned NOT NULL default '0',
  `sc_id` mediumint(4) unsigned NOT NULL default '0',
  `cur_level` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`sc_id`),
  KEY `uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `science_keeping`
--

LOCK TABLES `science_keeping` WRITE;
/*!40000 ALTER TABLE `science_keeping` DISABLE KEYS */;
/*!40000 ALTER TABLE `science_keeping` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `science_kept`
--

DROP TABLE IF EXISTS `science_kept`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `science_kept` (
  `uid` int(10) unsigned NOT NULL default '0',
  `sc_id` mediumint(4) unsigned NOT NULL default '0',
  `cur_level` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`sc_id`),
  KEY `uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `science_kept`
--

LOCK TABLES `science_kept` WRITE;
/*!40000 ALTER TABLE `science_kept` DISABLE KEYS */;
INSERT INTO `science_kept` VALUES (3760,2,0),(3760,1,0),(3760,3011,0),(3760,1009,0),(3762,2,0),(3762,1,0),(3762,3011,0),(3762,1009,0),(3764,2,0),(3764,1,0),(3764,3011,0),(3764,1009,0),(3766,2,0),(3766,1,0),(3766,3011,0),(3766,1009,0),(3768,2,0),(3768,1,0),(3768,3011,0),(3768,1009,0),(3770,2,0),(3770,1,0),(3770,3011,0),(3770,1009,0),(3772,2,0),(3772,1,0),(3772,3011,0),(3772,1009,0),(3774,2,0),(3774,1,0),(3774,3011,0),(3774,1009,0),(3776,2,0),(3776,1,0),(3776,3011,0),(3776,1009,0);
/*!40000 ALTER TABLE `science_kept` ENABLE KEYS */;
UNLOCK TABLES;

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

--
-- Table structure for table `sectors`
--

DROP TABLE IF EXISTS `sectors`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sectors` (
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `sector_id` int(10) unsigned NOT NULL default '0',
  `template_sector_id` int(10) unsigned NOT NULL default '0',
  `status` tinyint(4) unsigned NOT NULL default '0',
  `owner` int(11) unsigned NOT NULL default '0',
  `fight` tinyint(1) unsigned NOT NULL default '0',
  `artifact` mediumint(8) unsigned NOT NULL default '0',
  `landscape` tinyint(4) unsigned NOT NULL default '0',
  `host_activity` tinyint(4) unsigned NOT NULL default '0',
  `domination_point` tinyint(1) unsigned NOT NULL default '0',
  `bio` mediumint(8) unsigned NOT NULL default '0',
  `last_stats_start` int(10) unsigned NOT NULL default '0',
  `last_fight_length` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`world_id`,`map_id`,`sector_id`),
  KEY `sector_id` (`sector_id`),
  KEY `owner` (`owner`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `sectors`
--

LOCK TABLES `sectors` WRITE;
/*!40000 ALTER TABLE `sectors` DISABLE KEYS */;
/*!40000 ALTER TABLE `sectors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `techtree`
--

DROP TABLE IF EXISTS `techtree`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `techtree` (
  `uid` int(10) unsigned NOT NULL default '0',
  `sc_id` mediumint(4) unsigned NOT NULL default '0',
  `max_level` tinyint(3) unsigned NOT NULL default '0',
  `oracle_add_one` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`sc_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `techtree`
--

LOCK TABLES `techtree` WRITE;
/*!40000 ALTER TABLE `techtree` DISABLE KEYS */;
INSERT INTO `techtree` VALUES (3760,2,1,0),(3760,1,1,0),(3760,3011,1,0),(3760,1009,1,0),(3762,2,1,0),(3762,1,1,0),(3762,3011,1,0),(3762,1009,1,0),(3764,2,1,0),(3764,1,1,0),(3764,3011,1,0),(3764,1009,1,0),(3766,2,1,0),(3766,1,1,0),(3766,3011,1,0),(3766,1009,1,0),(3768,2,1,0),(3768,1,1,0),(3768,3011,1,0),(3768,1009,1,0),(3770,2,1,0),(3770,1,1,0),(3770,3011,1,0),(3770,1009,1,0),(3772,2,1,0),(3772,1,1,0),(3772,3011,1,0),(3772,1009,1,0),(3774,2,1,0),(3774,1,1,0),(3774,3011,1,0),(3774,1009,1,0),(3776,2,1,0),(3776,1,1,0),(3776,3011,1,0),(3776,1009,1,0);
/*!40000 ALTER TABLE `techtree` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `template_actions`
--

DROP TABLE IF EXISTS `template_actions`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_actions` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `template_trigger_id` int(10) unsigned NOT NULL default '0',
  `type` smallint(5) unsigned NOT NULL default '0',
  `x` int(10) unsigned NOT NULL default '0',
  `y` int(10) unsigned NOT NULL default '0',
  `sector_1` int(10) unsigned NOT NULL default '0',
  `sector_2` int(10) unsigned NOT NULL default '0',
  `movetime` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `template_trigger_id` (`template_trigger_id`)
) ENGINE=MyISAM AUTO_INCREMENT=339 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_actions`
--

LOCK TABLES `template_actions` WRITE;
/*!40000 ALTER TABLE `template_actions` DISABLE KEYS */;
/*!40000 ALTER TABLE `template_actions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `template_additional_sciences`
--

DROP TABLE IF EXISTS `template_additional_sciences`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_additional_sciences` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `template_id` int(10) unsigned NOT NULL default '0',
  `science_id` int(11) NOT NULL default '0',
  `level` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `template_id` (`template_id`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_additional_sciences`
--

LOCK TABLES `template_additional_sciences` WRITE;
/*!40000 ALTER TABLE `template_additional_sciences` DISABLE KEYS */;
/*!40000 ALTER TABLE `template_additional_sciences` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `template_buildings`
--

DROP TABLE IF EXISTS `template_buildings`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_buildings` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `template_sector_id` int(10) unsigned NOT NULL default '0',
  `type` smallint(5) unsigned NOT NULL default '0',
  `ulc` char(2) NOT NULL default '00',
  `level` tinyint(3) unsigned NOT NULL default '0',
  `owner` int(10) unsigned NOT NULL default '0',
  `special` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `template_sector_id` (`template_sector_id`)
) ENGINE=MyISAM AUTO_INCREMENT=210 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_buildings`
--

LOCK TABLES `template_buildings` WRITE;
/*!40000 ALTER TABLE `template_buildings` DISABLE KEYS */;
/*!40000 ALTER TABLE `template_buildings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `template_maps`
--

DROP TABLE IF EXISTS `template_maps`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_maps` (
  `template_id` int(10) unsigned NOT NULL default '0',
  `sector_id` tinyint(3) unsigned NOT NULL default '0',
  `status` tinyint(4) NOT NULL default '0',
  `home` tinyint(1) unsigned NOT NULL default '0',
  `domination_point` tinyint(1) unsigned NOT NULL default '0',
  `bio` mediumint(8) unsigned NOT NULL default '0',
  `host_activity` tinyint(3) unsigned NOT NULL default '0',
  `artifact` int(10) unsigned NOT NULL default '0',
  `additional_host_units` int(10) unsigned NOT NULL default '0',
  `additional_sp_workers` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`template_id`,`sector_id`),
  KEY `home` (`home`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_maps`
--

LOCK TABLES `template_maps` WRITE;
/*!40000 ALTER TABLE `template_maps` DISABLE KEYS */;
INSERT INTO `template_maps` VALUES (53,0,1,0,0,0,0,0,0,0),(53,1,1,0,0,0,0,0,0,0),(53,2,0,1,0,80,0,0,0,0),(53,3,1,0,0,0,0,0,0,0),(53,4,0,0,0,50,0,0,0,0),(53,5,0,0,0,20,30,1,0,0),(53,6,1,0,0,0,0,0,0,0),(53,7,0,1,0,80,0,0,0,0),(53,8,1,0,0,0,0,0,0,0),(53,9,1,0,0,0,0,0,0,0),(53,10,1,0,0,0,0,0,0,0),(53,11,1,0,0,0,0,0,0,0),(53,12,0,0,0,40,0,0,0,0),(53,13,1,0,0,0,0,0,0,0),(53,14,0,0,0,40,10,0,0,0),(53,15,0,0,0,50,0,0,0,0),(53,16,1,0,0,0,0,0,0,0),(53,17,0,0,0,40,0,0,0,0),(53,18,1,0,0,0,0,0,0,0),(53,19,1,0,0,0,0,0,0,0),(53,20,0,1,0,80,0,0,0,0),(53,21,0,0,0,40,0,0,0,0),(53,22,0,0,0,40,0,0,0,0),(53,23,1,0,0,0,0,0,0,0),(53,24,0,0,0,40,10,0,0,0),(53,25,0,0,0,40,10,0,0,0),(53,26,0,0,0,40,0,0,0,0),(53,27,0,0,0,40,0,0,0,0),(53,28,0,0,0,40,0,0,0,0),(53,29,0,1,0,80,0,0,0,0),(53,30,1,0,0,0,0,0,0,0),(53,31,1,0,0,0,0,0,0,0),(53,32,0,0,0,40,0,0,0,0),(53,33,1,0,0,0,0,0,0,0),(53,34,0,0,0,10,80,1,0,0),(53,35,0,0,0,20,10,0,0,0),(53,36,1,0,0,0,0,0,0,0),(53,37,1,0,0,0,0,0,0,0),(53,38,1,0,0,0,0,0,0,0),(53,39,1,0,0,0,0,0,0,0),(53,40,0,0,0,20,30,1,0,0),(53,41,0,0,0,50,0,0,0,0),(53,42,0,0,0,40,10,0,0,0),(53,43,0,0,0,20,10,0,0,0),(53,44,0,0,0,10,50,1,0,0),(53,45,0,0,0,10,50,1,0,0),(53,46,0,0,0,10,80,1,0,0),(53,47,0,0,0,40,10,0,0,0),(53,48,0,0,0,40,10,0,0,0),(53,49,0,0,0,50,0,0,0,0),(53,50,0,0,0,50,0,0,0,0),(53,51,0,0,0,40,10,0,0,0),(53,52,0,0,0,40,10,0,0,0),(53,53,0,0,0,10,80,1,0,0),(53,54,0,0,0,10,50,1,0,0),(53,55,0,0,0,10,50,1,0,0),(53,56,0,0,0,20,10,0,0,0),(53,57,0,0,0,40,10,0,0,0),(53,58,0,0,0,50,0,0,0,0),(53,59,0,0,0,20,30,1,0,0),(53,60,1,0,0,0,0,0,0,0),(53,61,1,0,0,0,0,0,0,0),(53,62,1,0,0,0,0,0,0,0),(53,63,1,0,0,0,0,0,0,0),(53,64,0,0,0,20,10,0,0,0),(53,65,0,0,0,10,80,1,0,0),(53,66,1,0,0,0,0,0,0,0),(53,67,0,0,0,40,0,0,0,0),(53,68,1,0,0,0,0,0,0,0),(53,69,1,0,0,0,0,0,0,0),(53,70,0,1,0,80,0,0,0,0),(53,71,0,0,0,40,0,0,0,0),(53,72,0,0,0,40,0,0,0,0),(53,73,0,0,0,40,0,0,0,0),(53,74,0,0,0,40,10,0,0,0),(53,75,0,0,0,40,10,0,0,0),(53,76,1,0,0,0,0,0,0,0),(53,77,0,0,0,40,0,0,0,0),(53,78,0,0,0,40,0,0,0,0),(53,79,0,1,0,80,0,0,0,0),(53,80,1,0,0,0,0,0,0,0),(53,81,1,0,0,0,0,0,0,0),(53,82,0,0,0,40,0,0,0,0),(53,83,1,0,0,0,0,0,0,0),(53,84,0,0,0,50,0,0,0,0),(53,85,0,0,0,40,10,0,0,0),(53,86,1,0,0,0,0,0,0,0),(53,87,0,0,0,40,0,0,0,0),(53,88,1,0,0,0,0,0,0,0),(53,89,1,0,0,0,0,0,0,0),(53,90,1,0,0,0,0,0,0,0),(53,91,1,0,0,0,0,0,0,0),(53,92,0,1,0,80,0,0,0,0),(53,93,1,0,0,0,0,0,0,0),(53,94,0,0,0,20,30,1,0,0),(53,95,0,0,0,50,0,0,0,0),(53,96,1,0,0,0,0,0,0,0),(53,97,0,1,0,80,0,0,0,0),(53,98,1,0,0,0,0,0,0,0),(53,99,1,0,0,0,0,0,0,0),(99,0,1,0,0,0,0,0,0,0),(98,0,0,0,0,1,0,0,0,0),(103,0,1,0,0,0,0,0,0,0),(103,1,0,0,0,30,0,0,0,0),(103,2,0,0,0,20,30,1,0,0),(103,3,0,0,0,30,0,0,0,0),(103,4,0,0,0,50,0,0,0,0),(103,5,0,0,0,50,25,1,0,0),(103,6,1,0,0,0,0,0,0,0),(103,7,1,0,0,0,0,0,0,0),(103,8,1,0,0,0,0,0,0,0),(103,9,1,0,0,0,0,0,0,0),(103,10,0,0,0,50,25,1,0,0),(103,11,0,0,0,50,0,0,0,0),(103,12,1,0,0,0,0,0,0,0),(103,13,1,0,0,0,0,0,0,0),(103,14,0,1,0,60,0,0,0,0),(103,15,0,0,0,50,0,0,0,0),(103,16,0,0,0,40,0,0,0,0),(103,17,0,0,0,80,70,1,500,0),(103,18,0,0,0,40,0,0,0,0),(103,19,1,0,0,0,0,0,0,0),(103,20,0,0,0,50,0,0,0,0),(103,21,0,1,0,60,0,0,0,0),(103,22,0,0,0,50,0,0,0,0),(103,23,0,0,0,20,20,1,0,0),(103,24,0,0,0,50,0,0,0,0),(103,25,1,0,0,0,0,0,0,0),(103,26,1,0,0,0,0,0,0,0),(103,27,1,0,0,0,0,0,0,0),(103,28,0,0,0,50,0,0,0,0),(103,29,0,0,0,50,25,1,0,0),(103,30,0,0,0,40,0,0,0,0),(103,31,1,0,0,0,0,0,0,0),(103,32,1,0,0,0,0,0,0,0),(103,33,0,0,0,40,50,0,0,0),(103,34,1,0,0,0,0,0,0,0),(103,35,0,0,0,40,50,0,0,0),(103,36,0,0,0,20,20,1,0,0),(103,37,0,0,0,50,0,0,0,0),(103,38,0,1,0,60,0,0,0,0),(103,39,0,0,0,50,0,0,0,0),(103,40,0,0,0,80,70,1,500,0),(103,41,1,0,0,0,0,0,0,0),(103,42,1,0,0,0,0,0,0,0),(103,43,1,0,0,0,0,0,0,0),(103,44,1,0,0,0,0,0,0,0),(103,45,1,0,0,0,0,0,0,0),(103,46,0,0,0,50,0,0,0,0),(103,47,1,0,0,0,0,0,0,0),(103,48,1,0,0,0,0,0,0,0),(103,49,0,0,0,30,0,0,0,0),(103,50,0,0,0,40,0,0,0,0),(103,51,1,0,0,0,0,0,0,0),(103,52,1,0,0,0,0,0,0,0),(103,53,0,0,0,40,50,0,0,0),(103,54,1,0,0,0,0,0,0,0),(103,55,1,0,0,0,0,0,0,0),(103,56,0,1,0,60,0,0,0,0),(103,57,0,0,0,50,0,0,0,0),(103,58,0,0,0,30,0,0,0,0),(103,59,0,0,0,20,30,1,0,0),(103,60,0,0,0,50,0,0,0,0),(103,61,0,1,0,60,0,0,0,0),(103,62,0,0,0,50,0,0,0,0),(103,63,0,0,0,20,20,1,0,0),(103,64,1,0,0,0,0,0,0,0),(103,65,0,0,0,40,0,0,0,0),(103,66,0,0,0,50,0,0,0,0),(103,67,0,0,0,50,25,1,0,0),(103,68,1,0,0,0,0,0,0,0),(103,69,1,0,0,0,0,0,0,0),(103,70,0,0,0,50,25,1,0,0),(103,71,0,0,0,50,0,0,0,0),(103,72,1,0,0,0,0,0,0,0),(103,73,0,0,0,50,0,0,0,0),(103,74,1,0,0,0,0,0,0,0),(103,75,0,0,0,80,70,1,500,0),(103,76,1,0,0,0,0,0,0,0),(103,77,1,0,0,0,0,0,0,0),(103,78,1,0,0,0,0,0,0,0),(103,79,1,0,0,0,0,0,0,0),(103,80,1,0,0,0,0,0,0,0),(103,81,0,0,0,30,0,0,0,0),(103,82,1,0,0,0,0,0,0,0),(103,83,0,1,0,60,0,0,0,0),(103,84,0,0,0,50,0,0,0,0),(103,85,0,0,0,40,0,0,0,0),(103,86,1,0,0,0,0,0,0,0),(103,87,1,0,0,0,0,0,0,0),(103,88,1,0,0,0,0,0,0,0),(103,89,1,0,0,0,0,0,0,0),(103,90,1,0,0,0,0,0,0,0),(103,91,0,0,0,20,30,1,0,0),(103,92,0,0,0,30,0,0,0,0),(103,93,0,0,0,50,0,0,0,0),(103,94,0,0,0,50,25,1,0,0),(103,95,1,0,0,0,0,0,0,0),(103,96,1,0,0,0,0,0,0,0),(103,97,1,0,0,0,0,0,0,0),(103,98,1,0,0,0,0,0,0,0),(103,99,1,0,0,0,0,0,0,0),(105,0,1,0,0,0,0,0,0,0),(105,1,0,0,0,20,20,0,0,0),(105,2,0,0,0,20,0,0,0,0),(105,3,0,0,0,20,0,0,0,0),(105,4,0,0,0,50,20,0,0,0),(105,5,0,0,0,20,0,0,0,0),(105,6,0,0,0,20,0,0,0,0),(105,7,0,0,0,20,20,0,0,0),(105,8,1,0,0,0,0,0,0,0),(105,9,1,0,0,0,0,0,0,0),(105,10,0,0,0,20,20,0,0,0),(105,11,0,0,0,20,40,0,0,0),(105,12,1,0,0,0,0,0,0,0),(105,13,0,0,0,30,0,0,0,0),(105,14,0,0,0,10,20,1,0,0),(105,15,0,0,0,30,0,0,0,0),(105,16,1,0,0,0,0,0,0,0),(105,17,0,0,0,20,40,0,0,0),(105,18,0,0,0,20,20,0,0,0),(105,19,1,0,0,0,0,0,0,0),(105,20,0,0,0,20,0,0,0,0),(105,21,1,0,0,0,0,0,0,0),(105,22,0,0,0,10,40,1,0,0),(105,23,0,0,0,40,10,0,0,0),(105,24,0,0,0,50,0,0,0,0),(105,25,0,0,0,40,10,0,0,0),(105,26,0,0,0,10,40,1,0,0),(105,27,1,0,0,0,0,0,0,0),(105,28,0,0,0,20,0,0,0,0),(105,29,1,0,0,0,0,0,0,0),(105,30,0,0,0,20,0,0,0,0),(105,31,0,0,0,30,0,0,0,0),(105,32,0,0,0,40,10,0,0,0),(105,33,1,0,0,0,0,0,0,0),(105,34,0,1,0,80,0,0,0,0),(105,35,1,0,0,0,0,0,0,0),(105,36,0,0,0,40,10,0,0,0),(105,37,0,0,0,30,0,0,0,0),(105,38,0,0,0,20,0,0,0,0),(105,39,1,0,0,0,0,0,0,0),(105,40,0,0,0,50,20,0,0,0),(105,41,0,0,0,10,20,1,0,0),(105,42,0,0,0,50,0,0,0,0),(105,43,0,1,0,80,0,0,0,0),(105,44,0,0,0,100,100,1,3000,0),(105,45,0,1,0,80,0,0,0,0),(105,46,0,0,0,50,0,0,0,0),(105,47,0,0,0,10,20,1,0,0),(105,48,0,0,0,50,20,0,0,0),(105,49,1,0,0,0,0,0,0,0),(105,50,0,0,0,20,0,0,0,0),(105,51,0,0,0,30,0,0,0,0),(105,52,0,0,0,40,10,0,0,0),(105,53,1,0,0,0,0,0,0,0),(105,54,0,1,0,80,0,0,0,0),(105,55,1,0,0,0,0,0,0,0),(105,56,0,0,0,40,10,0,0,0),(105,57,0,0,0,30,0,0,0,0),(105,58,0,0,0,20,0,0,0,0),(105,59,1,0,0,0,0,0,0,0),(105,60,0,0,0,20,0,0,0,0),(105,61,1,0,0,0,0,0,0,0),(105,62,0,0,0,10,40,1,0,0),(105,63,0,0,0,40,10,0,0,0),(105,64,0,0,0,50,0,0,0,0),(105,65,0,0,0,40,10,0,0,0),(105,66,0,0,0,10,40,1,0,0),(105,67,1,0,0,0,0,0,0,0),(105,68,0,0,0,20,0,0,0,0),(105,69,1,0,0,0,0,0,0,0),(105,70,0,0,0,20,20,0,0,0),(105,71,0,0,0,20,40,0,0,0),(105,72,1,0,0,0,0,0,0,0),(105,73,0,0,0,30,0,0,0,0),(105,74,0,0,0,10,20,1,0,0),(105,75,0,0,0,30,0,0,0,0),(105,76,1,0,0,0,0,0,0,0),(105,77,0,0,0,20,40,0,0,0),(105,78,0,0,0,20,20,0,0,0),(105,79,1,0,0,0,0,0,0,0),(105,80,1,0,0,0,0,0,0,0),(105,81,0,0,0,20,20,0,0,0),(105,82,0,0,0,20,0,0,0,0),(105,83,0,0,0,20,0,0,0,0),(105,84,0,0,0,50,20,0,0,0),(105,85,0,0,0,20,0,0,0,0),(105,86,0,0,0,20,0,0,0,0),(105,87,0,0,0,20,20,0,0,0),(105,88,1,0,0,0,0,0,0,0),(105,89,1,0,0,0,0,0,0,0),(105,90,1,0,0,0,0,0,0,0),(105,91,1,0,0,0,0,0,0,0),(105,92,1,0,0,0,0,0,0,0),(105,93,1,0,0,0,0,0,0,0),(105,94,1,0,0,0,0,0,0,0),(105,95,1,0,0,0,0,0,0,0),(105,96,1,0,0,0,0,0,0,0),(105,97,1,0,0,0,0,0,0,0),(105,98,1,0,0,0,0,0,0,0),(105,99,1,0,0,0,0,0,0,0),(107,0,0,0,0,10,80,0,0,0),(107,1,0,0,0,30,70,1,0,0),(107,2,0,0,0,70,0,0,0,0),(107,3,0,1,0,130,0,0,0,0),(107,4,1,0,0,0,0,0,0,0),(107,5,1,0,0,0,0,0,0,0),(107,6,0,1,0,130,0,0,0,0),(107,7,0,0,0,70,0,0,0,0),(107,8,0,0,0,30,70,1,0,0),(107,9,0,0,0,10,80,0,0,0),(107,10,0,0,0,30,70,1,0,0),(107,11,1,0,0,0,0,0,0,0),(107,12,0,0,0,100,30,0,0,0),(107,13,0,0,0,70,0,0,0,0),(107,14,1,0,0,0,0,0,0,0),(107,15,1,0,0,0,0,0,0,0),(107,16,0,0,0,70,0,0,0,0),(107,17,0,0,0,100,30,0,0,0),(107,18,1,0,0,0,0,0,0,0),(107,19,0,0,0,30,70,1,0,0),(107,20,0,0,0,70,0,0,0,0),(107,21,0,0,0,100,30,0,0,0),(107,22,1,0,0,0,0,0,0,0),(107,23,0,0,0,30,0,0,0,0),(107,24,1,0,0,0,0,0,0,0),(107,25,1,0,0,0,0,0,0,0),(107,26,0,0,0,30,0,0,0,0),(107,27,1,0,0,0,0,0,0,0),(107,28,0,0,0,100,30,0,0,0),(107,29,0,0,0,70,0,0,0,0),(107,30,0,1,0,130,0,0,0,0),(107,31,0,0,0,70,0,0,0,0),(107,32,0,0,0,30,0,0,0,0),(107,33,0,0,0,10,70,1,0,0),(107,34,0,0,0,10,5,0,0,0),(107,35,0,0,0,10,5,0,0,0),(107,36,0,0,0,10,70,1,0,0),(107,37,0,0,0,30,0,0,0,0),(107,38,0,0,0,70,0,0,0,0),(107,39,0,1,0,130,0,0,0,0),(107,40,1,0,0,0,0,0,0,0),(107,41,1,0,0,0,0,0,0,0),(107,42,1,0,0,0,0,0,0,0),(107,43,0,0,0,10,5,0,0,0),(107,44,0,0,0,15,65,1,0,0),(107,45,0,0,0,15,65,1,0,0),(107,46,0,0,0,10,5,0,0,0),(107,47,1,0,0,0,0,0,0,0),(107,48,1,0,0,0,0,0,0,0),(107,49,1,0,0,0,0,0,0,0),(107,50,1,0,0,0,0,0,0,0),(107,51,1,0,0,0,0,0,0,0),(107,52,1,0,0,0,0,0,0,0),(107,53,0,0,0,10,5,0,0,0),(107,54,0,0,0,15,65,1,0,0),(107,55,0,0,0,15,65,1,0,0),(107,56,0,0,0,10,5,0,0,0),(107,57,1,0,0,0,0,0,0,0),(107,58,1,0,0,0,0,0,0,0),(107,59,1,0,0,0,0,0,0,0),(107,60,0,1,0,130,0,0,0,0),(107,61,0,0,0,70,0,0,0,0),(107,62,0,0,0,30,0,0,0,0),(107,63,0,0,0,10,70,1,0,0),(107,64,0,0,0,10,5,0,0,0),(107,65,0,0,0,10,5,0,0,0),(107,66,0,0,0,10,70,1,0,0),(107,67,0,0,0,30,0,0,0,0),(107,68,0,0,0,70,0,0,0,0),(107,69,0,1,0,130,0,0,0,0),(107,70,0,0,0,70,0,0,0,0),(107,71,0,0,0,100,30,0,0,0),(107,72,1,0,0,0,0,0,0,0),(107,73,0,0,0,30,0,0,0,0),(107,74,1,0,0,0,0,0,0,0),(107,75,1,0,0,0,0,0,0,0),(107,76,0,0,0,30,0,0,0,0),(107,77,1,0,0,0,0,0,0,0),(107,78,0,0,0,100,30,0,0,0),(107,79,0,0,0,70,0,0,0,0),(107,80,0,0,0,30,70,1,0,0),(107,81,1,0,0,0,0,0,0,0),(107,82,0,0,0,100,30,0,0,0),(107,83,0,0,0,70,0,0,0,0),(107,84,1,0,0,0,0,0,0,0),(107,85,1,0,0,0,0,0,0,0),(107,86,0,0,0,70,0,0,0,0),(107,87,0,0,0,100,30,0,0,0),(107,88,1,0,0,0,0,0,0,0),(107,89,0,0,0,30,70,1,0,0),(107,90,0,0,0,10,80,0,0,0),(107,91,0,0,0,30,70,1,0,0),(107,92,0,0,0,70,0,0,0,0),(107,93,0,1,0,130,0,0,0,0),(107,94,1,0,0,0,0,0,0,0),(107,95,1,0,0,0,0,0,0,0),(107,96,0,1,0,130,0,0,0,0),(107,97,0,0,0,70,0,0,0,0),(107,98,0,0,0,30,70,1,0,0),(107,99,0,0,0,10,80,0,0,0),(108,0,1,0,0,0,0,0,0,0),(108,1,1,0,0,0,0,0,0,0),(108,2,1,0,0,0,0,0,0,0),(108,3,0,1,0,150,0,0,0,0),(108,4,0,0,0,55,0,0,0,0),(108,5,0,0,0,40,0,0,0,0),(108,6,0,0,0,45,60,1,0,0),(108,7,1,0,0,0,0,0,0,0),(108,8,1,0,0,0,0,0,0,0),(108,9,0,1,0,150,0,0,0,0),(108,10,0,1,0,150,0,0,0,0),(108,11,1,0,0,0,0,0,0,0),(108,12,1,0,0,0,0,0,0,0),(108,13,1,0,0,0,0,0,0,0),(108,14,1,0,0,0,0,0,0,0),(108,15,0,0,0,30,0,0,0,0),(108,16,1,0,0,0,0,0,0,0),(108,17,1,0,0,0,0,0,0,0),(108,18,1,0,0,0,0,0,0,0),(108,19,0,0,0,55,0,0,0,0),(108,20,0,0,0,55,0,0,0,0),(108,21,0,0,0,40,0,0,0,0),(108,22,0,0,0,30,0,0,0,0),(108,23,0,0,0,40,0,0,0,0),(108,24,1,0,0,0,0,0,0,0),(108,25,0,0,0,40,0,0,0,0),(108,26,1,0,0,0,0,0,0,0),(108,27,0,0,0,40,0,0,0,0),(108,28,0,0,0,30,0,0,0,0),(108,29,0,0,0,40,0,0,0,0),(108,30,1,0,0,0,0,0,0,0),(108,31,0,0,0,45,60,1,0,0),(108,32,1,0,0,0,0,0,0,0),(108,33,0,0,0,30,0,0,0,0),(108,34,1,0,0,0,0,0,0,0),(108,35,0,0,0,30,0,0,0,0),(108,36,1,0,0,0,0,0,0,0),(108,37,0,0,0,30,0,0,0,0),(108,38,1,0,0,0,0,0,0,0),(108,39,0,0,0,45,60,1,0,0),(108,40,1,0,0,0,0,0,0,0),(108,41,1,0,0,0,0,0,0,0),(108,42,1,0,0,0,0,0,0,0),(108,43,0,0,0,10,65,1,0,0),(108,44,0,0,0,5,99,1,0,0),(108,45,0,0,0,30,55,1,0,0),(108,46,0,0,0,5,99,1,0,0),(108,47,0,0,0,10,65,1,0,0),(108,48,1,0,0,0,0,0,0,0),(108,49,1,0,0,0,0,0,0,0),(108,50,1,0,0,0,0,0,0,0),(108,51,0,0,0,45,60,1,0,0),(108,52,1,0,0,0,0,0,0,0),(108,53,0,0,0,30,0,0,0,0),(108,54,1,0,0,0,0,0,0,0),(108,55,0,0,0,30,0,0,0,0),(108,56,1,0,0,0,0,0,0,0),(108,57,0,0,0,30,0,0,0,0),(108,58,1,0,0,0,0,0,0,0),(108,59,0,0,0,45,60,1,0,0),(108,60,0,0,0,55,0,0,0,0),(108,61,0,0,0,40,0,0,0,0),(108,62,0,0,0,30,0,0,0,0),(108,63,0,0,0,40,0,0,0,0),(108,64,1,0,0,0,0,0,0,0),(108,65,0,0,0,40,0,0,0,0),(108,66,1,0,0,0,0,0,0,0),(108,67,0,0,0,40,0,0,0,0),(108,68,0,0,0,30,0,0,0,0),(108,69,0,0,0,40,0,0,0,0),(108,70,0,1,0,150,0,0,0,0),(108,71,1,0,0,0,0,0,0,0),(108,72,1,0,0,0,0,0,0,0),(108,73,1,0,0,0,0,0,0,0),(108,74,1,0,0,0,0,0,0,0),(108,75,0,0,0,30,0,0,0,0),(108,76,1,0,0,0,0,0,0,0),(108,77,1,0,0,0,0,0,0,0),(108,78,1,0,0,0,0,0,0,0),(108,79,0,0,0,55,0,0,0,0),(108,80,1,0,0,0,0,0,0,0),(108,81,1,0,0,0,0,0,0,0),(108,82,1,0,0,0,0,0,0,0),(108,83,0,1,0,150,0,0,0,0),(108,84,0,0,0,55,0,0,0,0),(108,85,0,0,0,40,0,0,0,0),(108,86,0,0,0,45,60,1,0,0),(108,87,1,0,0,0,0,0,0,0),(108,88,1,0,0,0,0,0,0,0),(108,89,0,1,0,150,0,0,0,0),(108,90,1,0,0,0,0,0,0,0),(108,91,1,0,0,0,0,0,0,0),(108,92,1,0,0,0,0,0,0,0),(108,93,1,0,0,0,0,0,0,0),(108,94,1,0,0,0,0,0,0,0),(108,95,1,0,0,0,0,0,0,0),(108,96,1,0,0,0,0,0,0,0),(108,97,1,0,0,0,0,0,0,0),(108,98,1,0,0,0,0,0,0,0),(108,99,1,0,0,0,0,0,0,0),(110,0,1,0,0,0,0,0,0,0),(110,1,0,0,0,40,0,0,0,0),(110,2,0,0,0,25,0,0,0,0),(110,3,1,0,0,0,0,0,0,0),(110,4,0,0,0,20,15,0,0,0),(110,5,1,0,0,0,0,0,0,0),(110,6,0,0,0,25,0,0,0,0),(110,7,0,0,0,40,0,0,0,0),(110,8,1,0,0,0,0,0,0,0),(110,9,1,0,0,0,0,0,0,0),(110,10,0,0,0,40,0,0,0,0),(110,11,0,1,0,60,0,0,0,0),(110,12,0,0,0,35,0,0,0,0),(110,13,0,0,0,20,20,0,0,0),(110,14,0,0,0,10,35,1,0,0),(110,15,0,0,0,20,20,0,0,0),(110,16,0,0,0,35,0,0,0,0),(110,17,0,1,0,60,0,0,0,0),(110,18,0,0,0,40,0,0,0,0),(110,19,1,0,0,0,0,0,0,0),(110,20,0,0,0,25,0,0,0,0),(110,21,0,0,0,35,0,0,0,0),(110,22,0,0,0,15,15,1,0,0),(110,23,1,0,0,0,0,0,0,0),(110,24,0,0,0,5,25,0,0,0),(110,25,1,0,0,0,0,0,0,0),(110,26,0,0,0,15,15,1,0,0),(110,27,0,0,0,35,0,0,0,0),(110,28,0,0,0,25,0,0,0,0),(110,29,1,0,0,0,0,0,0,0),(110,30,1,0,0,0,0,0,0,0),(110,31,0,0,0,20,20,0,0,0),(110,32,1,0,0,0,0,0,0,0),(110,33,0,0,0,10,10,0,0,0),(110,34,0,0,0,10,10,0,0,0),(110,35,0,0,0,10,10,0,0,0),(110,36,1,0,0,0,0,0,0,0),(110,37,0,0,0,20,20,0,0,0),(110,38,1,0,0,0,0,0,0,0),(110,39,1,0,0,0,0,0,0,0),(110,40,0,0,0,20,15,0,0,0),(110,41,0,0,0,10,35,1,0,0),(110,42,0,0,0,5,25,0,0,0),(110,43,0,0,0,10,10,0,0,0),(110,44,0,0,0,1,80,1,0,0),(110,45,0,0,0,10,10,0,0,0),(110,46,0,0,0,5,25,0,0,0),(110,47,0,0,0,10,35,1,0,0),(110,48,0,0,0,20,15,0,0,0),(110,49,1,0,0,0,0,0,0,0),(110,50,1,0,0,0,0,0,0,0),(110,51,0,0,0,20,20,0,0,0),(110,52,1,0,0,0,0,0,0,0),(110,53,0,0,0,10,10,0,0,0),(110,54,0,0,0,10,10,0,0,0),(110,55,0,0,0,10,10,0,0,0),(110,56,1,0,0,0,0,0,0,0),(110,57,0,0,0,20,20,0,0,0),(110,58,1,0,0,0,0,0,0,0),(110,59,1,0,0,0,0,0,0,0),(110,60,0,0,0,25,0,0,0,0),(110,61,0,0,0,35,0,0,0,0),(110,62,0,0,0,15,15,1,0,0),(110,63,1,0,0,0,0,0,0,0),(110,64,0,0,0,5,25,0,0,0),(110,65,1,0,0,0,0,0,0,0),(110,66,0,0,0,15,15,1,0,0),(110,67,0,0,0,35,0,0,0,0),(110,68,0,0,0,25,0,0,0,0),(110,69,1,0,0,0,0,0,0,0),(110,70,0,0,0,40,0,0,0,0),(110,71,0,1,0,60,0,0,0,0),(110,72,0,0,0,35,0,0,0,0),(110,73,0,0,0,20,20,0,0,0),(110,74,0,0,0,10,35,1,0,0),(110,75,0,0,0,20,20,0,0,0),(110,76,0,0,0,35,0,0,0,0),(110,77,0,1,0,60,0,0,0,0),(110,78,0,0,0,40,0,0,0,0),(110,79,1,0,0,0,0,0,0,0),(110,80,1,0,0,0,0,0,0,0),(110,81,0,0,0,40,0,0,0,0),(110,82,0,0,0,25,0,0,0,0),(110,83,1,0,0,0,0,0,0,0),(110,84,0,0,0,20,15,0,0,0),(110,85,1,0,0,0,0,0,0,0),(110,86,0,0,0,25,0,0,0,0),(110,87,0,0,0,40,0,0,0,0),(110,88,1,0,0,0,0,0,0,0),(110,89,1,0,0,0,0,0,0,0),(110,90,1,0,0,0,0,0,0,0),(110,91,1,0,0,0,0,0,0,0),(110,92,1,0,0,0,0,0,0,0),(110,93,1,0,0,0,0,0,0,0),(110,94,1,0,0,0,0,0,0,0),(110,95,1,0,0,0,0,0,0,0),(110,96,1,0,0,0,0,0,0,0),(110,97,1,0,0,0,0,0,0,0),(110,98,1,0,0,0,0,0,0,0),(110,99,1,0,0,0,0,0,0,0),(117,0,1,0,0,0,0,0,0,0),(117,1,1,0,0,0,0,0,0,0),(117,2,1,0,0,0,0,0,0,0),(117,3,0,0,0,25,10,0,0,0),(117,4,0,0,0,50,5,0,0,0),(117,5,0,0,0,25,10,0,0,0),(117,6,0,0,0,70,80,1,500,0),(117,7,0,0,0,25,10,0,0,0),(117,8,0,0,0,50,5,0,0,0),(117,9,0,0,0,25,10,0,0,0),(117,10,1,0,0,0,0,0,0,0),(117,11,1,0,0,0,0,0,0,0),(117,12,1,0,0,0,0,0,0,0),(117,13,0,0,0,50,5,0,0,0),(117,14,0,1,0,135,0,0,0,0),(117,15,0,0,0,50,5,0,0,0),(117,16,1,0,0,0,0,0,0,0),(117,17,0,0,0,50,5,0,0,0),(117,18,0,1,0,135,0,0,0,0),(117,19,0,0,0,50,5,0,0,0),(117,20,1,0,0,0,0,0,0,0),(117,21,1,0,0,0,0,0,0,0),(117,22,1,0,0,0,0,0,0,0),(117,23,0,0,0,25,10,0,0,0),(117,24,0,0,0,50,5,0,0,0),(117,25,0,0,0,60,30,1,0,0),(117,26,1,0,0,0,0,0,0,0),(117,27,0,0,0,60,30,1,0,0),(117,28,0,0,0,50,5,0,0,0),(117,29,0,0,0,25,10,0,0,0),(117,30,0,0,0,25,10,0,0,0),(117,31,0,0,0,50,5,0,0,0),(117,32,0,0,0,25,10,0,0,0),(117,33,0,0,0,70,40,1,500,0),(117,34,1,0,0,0,0,0,0,0),(117,35,1,0,0,0,0,0,0,0),(117,36,1,0,0,0,0,0,0,0),(117,37,1,0,0,0,0,0,0,0),(117,38,1,0,0,0,0,0,0,0),(117,39,0,0,0,70,40,1,500,0),(117,40,0,0,0,50,5,0,0,0),(117,41,0,1,0,135,0,0,0,0),(117,42,0,0,0,50,5,0,0,0),(117,43,1,0,0,0,0,0,0,0),(117,44,1,0,0,0,0,0,0,0),(117,45,1,0,0,0,0,0,0,0),(117,46,1,0,0,0,0,0,0,0),(117,47,0,0,0,60,30,1,0,0),(117,48,0,0,0,50,5,0,0,0),(117,49,0,0,0,25,10,0,0,0),(117,50,0,0,0,25,10,0,0,0),(117,51,0,0,0,50,5,0,0,0),(117,52,0,0,0,60,30,1,0,0),(117,53,1,0,0,0,0,0,0,0),(117,54,1,0,0,0,0,0,0,0),(117,55,1,0,0,0,0,0,0,0),(117,56,1,0,0,0,0,0,0,0),(117,57,0,0,0,50,5,0,0,0),(117,58,0,1,0,135,0,0,0,0),(117,59,0,0,0,50,5,0,0,0),(117,60,0,0,0,70,80,1,500,0),(117,61,1,0,0,0,0,0,0,0),(117,62,1,0,0,0,0,0,0,0),(117,63,1,0,0,0,0,0,0,0),(117,64,1,0,0,0,0,0,0,0),(117,65,1,0,0,0,0,0,0,0),(117,66,0,0,0,70,80,1,500,0),(117,67,0,0,0,25,10,0,0,0),(117,68,0,0,0,50,5,0,0,0),(117,69,0,0,0,25,10,0,0,0),(117,70,0,0,0,25,10,0,0,0),(117,71,0,0,0,50,5,0,0,0),(117,72,0,0,0,60,30,1,0,0),(117,73,1,0,0,0,0,0,0,0),(117,74,0,0,0,60,30,1,0,0),(117,75,0,0,0,50,5,0,0,0),(117,76,0,0,0,25,10,0,0,0),(117,77,1,0,0,0,0,0,0,0),(117,78,1,0,0,0,0,0,0,0),(117,79,1,0,0,0,0,0,0,0),(117,80,0,0,0,50,5,0,0,0),(117,81,0,1,0,135,0,0,0,0),(117,82,0,0,0,50,5,0,0,0),(117,83,1,0,0,0,0,0,0,0),(117,84,0,0,0,50,5,0,0,0),(117,85,0,1,0,135,0,0,0,0),(117,86,0,0,0,50,5,0,0,0),(117,87,1,0,0,0,0,0,0,0),(117,88,1,0,0,0,0,0,0,0),(117,89,1,0,0,0,0,0,0,0),(117,90,0,0,0,25,10,0,0,0),(117,91,0,0,0,50,5,0,0,0),(117,92,0,0,0,25,10,0,0,0),(117,93,0,0,0,70,40,1,500,0),(117,94,0,0,0,25,10,0,0,0),(117,95,0,0,0,50,5,0,0,0),(117,96,0,0,0,25,10,0,0,0),(117,97,1,0,0,0,0,0,0,0),(117,98,1,0,0,0,0,0,0,0),(117,99,1,0,0,0,0,0,0,0),(126,0,1,0,0,0,0,0,0,0),(126,1,1,0,0,0,0,0,0,0),(126,2,0,0,0,15,0,0,0,0),(126,3,0,0,0,5,10,0,0,0),(126,4,0,0,0,1,20,0,0,0),(126,5,0,0,0,5,10,0,0,0),(126,6,0,0,0,15,0,0,0,0),(126,7,1,0,0,0,0,0,0,0),(126,8,1,0,0,0,0,0,0,0),(126,9,1,0,0,0,0,0,0,0),(126,10,1,0,0,0,0,0,0,0),(126,11,0,0,0,25,25,1,0,0),(126,12,0,0,0,40,0,0,0,0),(126,13,1,0,0,0,0,0,0,0),(126,14,0,0,0,42,50,0,0,0),(126,15,1,0,0,0,0,0,0,0),(126,16,0,0,0,40,0,0,0,0),(126,17,0,0,0,25,25,1,0,0),(126,18,1,0,0,0,0,0,0,0),(126,19,1,0,0,0,0,0,0,0),(126,20,0,0,0,15,0,0,0,0),(126,21,0,0,0,40,0,0,0,0),(126,22,0,1,0,100,0,0,0,0),(126,23,0,0,0,45,5,0,0,0),(126,24,1,0,0,0,0,0,0,0),(126,25,0,0,0,45,5,0,0,0),(126,26,0,1,0,100,0,0,0,0),(126,27,0,0,0,40,0,0,0,0),(126,28,0,0,0,15,0,0,0,0),(126,29,1,0,0,0,0,0,0,0),(126,30,0,0,0,5,10,0,0,0),(126,31,1,0,0,0,0,0,0,0),(126,32,0,0,0,45,5,0,0,0),(126,33,0,0,0,25,15,0,0,0),(126,34,1,0,0,0,0,0,0,0),(126,35,0,0,0,25,15,0,0,0),(126,36,0,0,0,45,5,0,0,0),(126,37,1,0,0,0,0,0,0,0),(126,38,0,0,0,5,10,0,0,0),(126,39,1,0,0,0,0,0,0,0),(126,40,0,0,0,1,20,0,0,0),(126,41,0,0,0,42,50,0,0,0),(126,42,1,0,0,0,0,0,0,0),(126,43,0,0,0,2,30,1,0,0),(126,44,0,0,0,4,60,0,0,0),(126,45,0,0,0,2,30,1,0,0),(126,46,1,0,0,0,0,0,0,0),(126,47,0,0,0,42,50,0,0,0),(126,48,0,0,0,1,20,0,0,0),(126,49,1,0,0,0,0,0,0,0),(126,50,0,0,0,5,10,0,0,0),(126,51,1,0,0,0,0,0,0,0),(126,52,0,0,0,45,5,0,0,0),(126,53,0,0,0,25,15,0,0,0),(126,54,1,0,0,0,0,0,0,0),(126,55,0,0,0,25,15,0,0,0),(126,56,0,0,0,45,5,0,0,0),(126,57,1,0,0,0,0,0,0,0),(126,58,0,0,0,5,10,0,0,0),(126,59,1,0,0,0,0,0,0,0),(126,60,0,0,0,15,0,0,0,0),(128,0,0,0,0,1,40,1,0,0),(128,1,0,0,0,5,10,1,0,0),(128,2,0,0,0,10,5,0,0,0),(128,3,0,0,0,20,1,0,0,0),(128,4,0,0,0,40,0,0,0,0),(128,5,0,1,0,80,0,0,0,0),(128,6,0,0,0,40,0,0,0,0),(128,7,0,0,0,20,1,0,0,0),(128,8,0,0,0,10,5,1,0,0),(128,9,0,0,0,5,10,1,0,0),(128,10,0,0,0,5,10,1,0,0),(128,11,0,0,0,1,20,1,0,0),(128,12,1,0,0,0,0,0,0,0),(128,13,0,0,0,10,5,1,0,0),(128,14,1,0,0,0,0,0,0,0),(128,15,0,0,0,40,0,0,0,0),(128,16,0,0,0,20,20,0,0,0),(128,17,1,0,0,0,0,0,0,0),(128,18,0,0,0,5,10,1,0,0),(128,19,0,0,0,10,5,1,0,0),(128,20,0,0,0,10,5,0,0,0),(128,21,1,0,0,0,0,0,0,0),(128,22,0,0,0,1,20,0,0,0),(128,23,0,0,0,5,10,0,0,0),(128,24,1,0,0,0,0,0,0,0),(128,25,0,0,0,20,1,1,0,0),(128,26,1,0,0,0,0,0,0,0),(128,27,1,0,0,0,0,0,0,0),(128,28,1,0,0,0,0,0,0,0),(128,29,0,0,0,20,1,0,0,0),(128,30,0,0,0,20,1,0,0,0),(128,31,0,0,0,10,5,1,0,0),(128,32,0,0,0,5,10,0,0,0),(128,33,0,0,0,1,60,0,0,0),(128,34,0,0,0,5,10,0,0,0),(128,35,0,0,0,10,5,0,0,0),(128,36,0,0,0,5,10,0,0,0),(128,37,1,0,0,0,0,0,0,0),(128,38,0,0,0,20,20,0,0,0),(128,39,0,0,0,40,0,0,0,0),(128,40,0,0,0,40,0,0,0,0),(128,41,1,0,0,0,0,0,0,0),(128,42,1,0,0,0,0,0,0,0),(128,43,0,0,0,5,10,0,0,0),(128,44,1,0,0,0,0,0,0,0),(128,45,0,0,0,5,10,0,0,0),(128,46,0,0,0,10,5,0,0,0),(128,47,0,0,0,20,1,1,0,0),(128,48,0,0,0,40,0,0,0,0),(128,49,0,1,0,80,0,0,0,0),(128,50,0,1,0,80,0,0,0,0),(128,51,0,0,0,40,0,0,0,0),(128,52,0,0,0,20,1,1,0,0),(128,53,0,0,0,10,5,0,0,0),(128,54,0,0,0,5,10,0,0,0),(128,55,1,0,0,0,0,0,0,0),(128,56,0,0,0,5,10,0,0,0),(128,57,1,0,0,0,0,0,0,0),(128,58,1,0,0,0,0,0,0,0),(128,59,0,0,0,40,0,0,0,0),(128,60,0,0,0,40,0,0,0,0),(128,61,0,0,0,20,20,0,0,0),(128,62,1,0,0,0,0,0,0,0),(128,63,0,0,0,5,10,0,0,0),(128,64,0,0,0,10,5,0,0,0),(128,65,0,0,0,5,10,0,0,0),(128,66,0,0,0,1,60,0,0,0),(128,67,0,0,0,5,10,0,0,0),(128,68,0,0,0,10,5,1,0,0),(128,69,0,0,0,20,1,0,0,0),(128,70,0,0,0,20,1,0,0,0),(128,71,1,0,0,0,0,0,0,0),(128,72,1,0,0,0,0,0,0,0),(128,73,1,0,0,0,0,0,0,0),(128,74,0,0,0,20,1,1,0,0),(128,75,1,0,0,0,0,0,0,0),(128,76,0,0,0,5,10,0,0,0),(128,77,0,0,0,1,20,0,0,0),(128,78,1,0,0,0,0,0,0,0),(128,79,0,0,0,10,5,0,0,0),(128,80,0,0,0,10,5,1,0,0),(128,81,0,0,0,5,10,1,0,0),(128,82,1,0,0,0,0,0,0,0),(128,83,0,0,0,20,20,0,0,0),(128,84,0,0,0,40,0,0,0,0),(128,85,1,0,0,0,0,0,0,0),(128,86,0,0,0,10,5,1,0,0),(128,87,1,0,0,0,0,0,0,0),(128,88,0,0,0,1,20,1,0,0),(128,89,0,0,0,5,10,1,0,0),(128,90,0,0,0,5,10,1,0,0),(128,91,0,0,0,10,5,1,0,0),(128,92,0,0,0,20,1,0,0,0),(128,93,0,0,0,40,0,0,0,0),(128,94,0,1,0,80,0,0,0,0),(128,95,0,0,0,40,0,0,0,0),(128,96,0,0,0,20,1,0,0,0),(128,97,0,0,0,10,5,0,0,0),(128,98,0,0,0,5,10,1,0,0),(128,99,0,0,0,1,40,1,0,0),(130,0,0,0,0,6,40,0,0,0),(130,1,0,0,0,11,20,0,0,0),(130,2,0,0,0,21,10,1,0,0),(130,3,0,0,0,59,0,0,0,0),(130,4,0,1,0,121,0,0,0,0),(130,5,0,0,0,67,0,0,0,0),(130,6,0,0,0,37,5,0,0,0),(130,7,0,0,0,13,10,1,0,0),(130,8,0,0,0,7,20,0,0,0),(130,9,0,0,0,1,30,0,0,0),(130,10,0,0,0,11,20,0,0,0),(130,11,0,0,0,2,85,0,0,0),(130,12,0,0,0,11,20,0,0,0),(130,13,1,0,0,0,0,0,0,0),(130,14,0,0,0,63,0,0,0,0),(130,15,1,0,0,0,0,0,0,0),(130,16,1,0,0,0,0,0,0,0),(130,17,0,0,0,7,20,0,0,0),(130,18,1,0,0,0,0,0,0,0),(130,19,0,0,0,7,20,0,0,0),(130,20,0,0,0,21,10,1,0,0),(130,21,0,0,0,11,20,0,0,0),(130,22,0,0,0,6,40,0,0,0),(130,23,1,0,0,0,0,0,0,0),(130,24,0,0,0,47,5,0,0,0),(130,25,1,0,0,0,0,0,0,0),(130,26,0,0,0,8,55,1,0,0),(130,27,0,0,0,1,30,0,0,0),(130,28,0,0,0,7,20,0,0,0),(130,29,0,0,0,13,10,1,0,0),(130,30,0,0,0,59,0,0,0,0),(130,31,1,0,0,0,0,0,0,0),(130,32,1,0,0,0,0,0,0,0),(130,33,1,0,0,0,0,0,0,0),(130,34,0,0,0,23,25,1,0,0),(130,35,1,0,0,0,0,0,0,0),(130,36,1,0,0,0,0,0,0,0),(130,37,1,0,0,0,0,0,0,0),(130,38,1,0,0,0,0,0,0,0),(130,39,0,0,0,37,5,0,0,0),(130,40,0,1,0,121,0,0,0,0),(130,41,0,0,0,63,0,0,0,0),(130,42,0,0,0,47,5,0,0,0),(130,43,0,0,0,23,25,1,0,0),(130,44,0,0,0,4,100,0,0,0),(130,45,0,0,0,23,25,1,0,0),(130,46,0,0,0,47,5,0,0,0),(130,47,0,0,0,63,0,0,0,0),(130,48,0,1,0,121,0,0,0,0),(130,49,0,0,0,67,0,0,0,0),(130,50,0,0,0,67,0,0,0,0),(130,51,1,0,0,0,0,0,0,0),(130,52,1,0,0,0,0,0,0,0),(130,53,1,0,0,0,0,0,0,0),(130,54,0,0,0,23,25,1,0,0),(130,55,1,0,0,0,0,0,0,0),(130,56,1,0,0,0,0,0,0,0),(130,57,1,0,0,0,0,0,0,0),(130,58,0,0,0,59,0,0,0,0),(130,59,1,0,0,0,0,0,0,0),(130,60,0,0,0,37,5,0,0,0),(130,61,1,0,0,0,0,0,0,0),(130,62,0,0,0,8,55,1,0,0),(130,63,1,0,0,0,0,0,0,0),(130,64,0,0,0,47,5,0,0,0),(130,65,1,0,0,0,0,0,0,0),(130,66,0,0,0,6,40,0,0,0),(130,67,0,0,0,11,20,0,0,0),(130,68,0,0,0,21,10,1,0,0),(130,69,1,0,0,0,0,0,0,0),(130,70,0,0,0,13,10,1,0,0),(130,71,0,0,0,7,20,0,0,0),(130,72,0,0,0,1,30,0,0,0),(130,73,1,0,0,0,0,0,0,0),(130,74,0,0,0,63,0,0,0,0),(130,75,1,0,0,0,0,0,0,0),(130,76,0,0,0,11,20,0,0,0),(130,77,0,0,0,2,85,0,0,0),(130,78,0,0,0,11,20,0,0,0),(130,79,1,0,0,0,0,0,0,0),(130,80,0,0,0,7,20,0,0,0),(130,81,1,0,0,0,0,0,0,0),(130,82,0,0,0,7,20,0,0,0),(130,83,1,0,0,0,0,0,0,0),(130,84,0,1,0,121,0,0,0,0),(130,85,0,0,0,59,0,0,0,0),(130,86,0,0,0,21,10,1,0,0),(130,87,0,0,0,11,20,0,0,0),(130,88,0,0,0,6,40,0,0,0),(130,89,1,0,0,0,0,0,0,0),(130,90,0,0,0,1,30,0,0,0),(130,91,0,0,0,7,20,0,0,0),(130,92,0,0,0,13,10,1,0,0),(130,93,0,0,0,37,5,0,0,0),(130,94,0,0,0,67,0,0,0,0),(130,95,1,0,0,0,0,0,0,0),(130,96,1,0,0,0,0,0,0,0),(130,97,1,0,0,0,0,0,0,0),(130,98,1,0,0,0,0,0,0,0),(130,99,1,0,0,0,0,0,0,0),(131,0,0,0,0,25,0,0,0,0),(131,1,0,0,0,20,0,0,0,0),(131,2,0,0,0,15,5,0,0,0),(131,3,1,0,0,0,0,0,0,0),(131,4,0,0,0,50,0,0,0,0),(131,5,0,1,0,100,0,0,0,0),(131,6,0,0,0,50,0,0,0,0),(131,7,0,0,0,35,0,0,0,0),(131,8,0,0,0,30,0,0,0,0),(131,9,0,0,0,25,0,0,0,0),(131,10,0,0,0,30,0,0,0,0),(131,11,1,0,0,0,0,0,0,0),(131,12,0,0,0,10,10,0,0,0),(131,13,0,0,0,20,25,1,0,0),(131,14,0,0,0,30,5,0,0,0),(131,15,0,0,0,50,0,0,0,0),(131,16,0,0,0,30,5,0,0,0),(131,17,1,0,0,0,0,0,0,0),(131,18,1,0,0,0,0,0,0,0),(131,19,0,0,0,20,0,0,0,0),(131,20,0,0,0,35,0,0,0,0),(131,21,1,0,0,0,0,0,0,0),(131,22,0,0,0,10,15,0,0,0),(131,23,0,0,0,10,20,0,0,0),(131,24,1,0,0,0,0,0,0,0),(131,25,1,0,0,0,0,0,0,0),(131,26,0,0,0,1,80,1,0,0),(131,27,0,0,0,10,15,0,0,0),(131,28,0,0,0,10,10,0,0,0),(131,29,0,0,0,15,5,0,0,0),(131,30,0,0,0,50,0,0,0,0),(131,31,0,0,0,30,5,0,0,0),(131,32,0,0,0,1,80,1,0,0),(131,33,0,0,0,8,25,0,0,0),(131,34,1,0,0,0,0,0,0,0),(131,35,0,0,0,4,30,0,0,0),(131,36,0,0,0,8,25,0,0,0),(131,37,0,0,0,10,20,0,0,0),(131,38,0,0,0,20,25,1,0,0),(131,39,1,0,0,0,0,0,0,0),(131,40,0,1,0,100,0,0,0,0),(131,41,0,0,0,50,0,0,0,0),(131,42,1,0,0,0,0,0,0,0),(131,43,0,0,0,4,30,0,0,0),(131,44,0,0,0,2,40,1,0,0),(131,45,0,0,0,2,40,1,0,0),(131,46,1,0,0,0,0,0,0,0),(131,47,1,0,0,0,0,0,0,0),(131,48,0,0,0,30,5,0,0,0),(131,49,0,0,0,50,0,0,0,0),(131,50,0,0,0,50,0,0,0,0),(131,51,0,0,0,30,5,0,0,0),(131,52,1,0,0,0,0,0,0,0),(131,53,1,0,0,0,0,0,0,0),(131,54,0,0,0,2,40,1,0,0),(131,55,0,0,0,2,40,1,0,0),(131,56,0,0,0,4,30,0,0,0),(131,57,1,0,0,0,0,0,0,0),(131,58,0,0,0,50,0,0,0,0),(131,59,0,1,0,100,0,0,0,0),(131,60,1,0,0,0,0,0,0,0),(131,61,0,0,0,20,25,1,0,0),(131,62,0,0,0,10,20,0,0,0),(131,63,0,0,0,8,25,0,0,0),(131,64,0,0,0,4,30,0,0,0),(131,65,1,0,0,0,0,0,0,0),(131,66,0,0,0,8,25,0,0,0),(131,67,0,0,0,1,85,1,0,0),(131,68,0,0,0,30,5,0,0,0),(131,69,0,0,0,50,0,0,0,0),(131,70,0,0,0,15,5,0,0,0),(131,71,0,0,0,10,10,0,0,0),(131,72,0,0,0,10,15,0,0,0),(131,73,0,0,0,1,85,1,0,0),(131,74,1,0,0,0,0,0,0,0),(131,75,1,0,0,0,0,0,0,0),(131,76,0,0,0,10,20,0,0,0),(131,77,0,0,0,10,15,0,0,0),(131,78,1,0,0,0,0,0,0,0),(131,79,0,0,0,35,0,0,0,0),(131,80,0,0,0,20,0,0,0,0),(131,81,1,0,0,0,0,0,0,0),(131,82,1,0,0,0,0,0,0,0),(131,83,0,0,0,30,5,0,0,0),(131,84,0,0,0,50,0,0,0,0),(131,85,0,0,0,30,5,0,0,0),(131,86,0,0,0,20,25,1,0,0),(131,87,0,0,0,10,10,0,0,0),(131,88,1,0,0,0,0,0,0,0),(131,89,0,0,0,20,0,0,0,0),(131,90,0,0,0,25,0,0,0,0),(131,91,0,0,0,30,0,0,0,0),(131,92,0,0,0,35,0,0,0,0),(131,93,0,0,0,50,0,0,0,0),(131,94,0,1,0,100,0,0,0,0),(131,95,0,0,0,50,0,0,0,0),(131,96,1,0,0,0,0,0,0,0),(131,97,0,0,0,15,5,0,0,0),(131,98,0,0,0,20,0,0,0,0),(131,99,0,0,0,25,0,0,0,0),(136,0,0,0,0,15,20,1,0,0),(136,1,0,0,0,40,0,0,0,0),(136,2,0,0,0,25,0,0,0,0),(136,3,0,0,0,15,0,0,0,0),(136,4,0,0,0,10,40,0,0,0),(136,5,0,0,0,10,40,0,0,0),(136,6,0,0,0,15,0,0,0,0),(136,7,0,0,0,25,0,0,0,0),(136,8,0,0,0,40,0,0,0,0),(136,9,0,0,0,15,20,1,0,0),(136,10,0,0,0,30,50,0,0,0),(136,11,1,0,0,0,0,0,0,0),(136,12,1,0,0,0,0,0,0,0),(136,13,0,0,0,8,20,1,0,0),(136,14,1,0,0,0,0,0,0,0),(136,15,1,0,0,0,0,0,0,0),(136,16,0,0,0,8,20,1,0,0),(136,17,1,0,0,0,0,0,0,0),(136,18,1,0,0,0,0,0,0,0),(136,19,0,0,0,30,50,0,0,0),(136,20,0,0,0,40,20,0,0,0),(136,21,0,0,0,35,0,0,0,0),(136,22,0,0,0,45,0,0,0,0),(136,23,1,0,0,0,0,0,0,0),(136,24,1,0,0,0,0,0,0,0),(136,25,1,0,0,0,0,0,0,0),(136,26,1,0,0,0,0,0,0,0),(136,27,0,0,0,45,0,0,0,0),(136,28,0,0,0,35,0,0,0,0),(136,29,0,0,0,40,20,0,0,0),(136,30,0,0,0,20,0,0,0,0),(136,31,1,0,0,0,0,0,0,0),(136,32,0,0,0,35,0,0,0,0),(136,33,0,0,0,45,0,0,0,0),(136,34,0,1,0,80,0,0,0,0),(136,35,1,0,0,0,0,0,0,0),(136,36,0,0,0,45,0,0,0,0),(136,37,0,0,0,35,0,0,0,0),(136,38,1,0,0,0,0,0,0,0),(136,39,0,0,0,20,0,0,0,0),(136,40,0,0,0,10,0,0,0,0),(136,41,1,0,0,0,0,0,0,0),(136,42,1,0,0,0,0,0,0,0),(136,43,1,0,0,0,0,0,0,0),(136,44,0,0,0,5,50,0,0,0),(136,45,0,0,0,5,50,0,0,0),(136,46,0,1,0,80,0,0,0,0),(136,47,1,0,0,0,0,0,0,0),(136,48,1,0,0,0,0,0,0,0),(136,49,0,0,0,10,0,0,0,0),(136,50,0,0,0,10,0,0,0,0),(136,51,1,0,0,0,0,0,0,0),(136,52,1,0,0,0,0,0,0,0),(136,53,0,1,0,80,0,0,0,0),(136,54,0,0,0,5,50,0,0,0),(136,55,0,0,0,5,50,0,0,0),(136,56,1,0,0,0,0,0,0,0),(136,57,1,0,0,0,0,0,0,0),(136,58,1,0,0,0,0,0,0,0),(136,59,0,0,0,10,0,0,0,0),(136,60,0,0,0,20,0,0,0,0),(136,61,1,0,0,0,0,0,0,0),(136,62,0,0,0,35,0,0,0,0),(136,63,0,0,0,45,0,0,0,0),(136,64,1,0,0,0,0,0,0,0),(136,65,0,1,0,80,0,0,0,0),(136,66,0,0,0,45,0,0,0,0),(136,67,0,0,0,35,0,0,0,0),(136,68,1,0,0,0,0,0,0,0),(136,69,0,0,0,20,0,0,0,0),(136,70,0,0,0,40,20,0,0,0),(136,71,0,0,0,35,0,0,0,0),(136,72,0,0,0,45,0,0,0,0),(136,73,1,0,0,0,0,0,0,0),(136,74,1,0,0,0,0,0,0,0),(136,75,1,0,0,0,0,0,0,0),(136,76,1,0,0,0,0,0,0,0),(136,77,0,0,0,45,0,0,0,0),(136,78,0,0,0,35,0,0,0,0),(136,79,0,0,0,40,0,0,0,0),(136,80,0,0,0,30,50,0,0,0),(136,81,1,0,0,0,0,0,0,0),(136,82,1,0,0,0,0,0,0,0),(136,83,0,0,0,8,20,1,0,0),(136,84,1,0,0,0,0,0,0,0),(136,85,1,0,0,0,0,0,0,0),(136,86,0,0,0,8,20,1,0,0),(136,87,1,0,0,0,0,0,0,0),(136,88,1,0,0,0,0,0,0,0),(136,89,0,0,0,30,50,0,0,0),(136,90,0,0,0,15,20,1,0,0),(136,91,0,0,0,40,0,0,0,0),(136,92,0,0,0,25,0,0,0,0),(136,93,0,0,0,15,0,0,0,0),(136,94,0,0,0,10,40,0,0,0),(136,95,0,0,0,10,40,0,0,0),(136,96,0,0,0,15,0,0,0,0),(136,97,0,0,0,25,0,0,0,0),(137,0,0,0,0,15,20,1,0,0),(137,1,0,0,0,40,0,0,0,0),(137,2,0,0,0,25,0,0,0,0),(137,3,0,0,0,15,0,0,0,0),(137,4,0,0,0,10,40,0,0,0),(137,5,0,0,0,10,40,0,0,0),(137,6,0,0,0,15,0,0,0,0),(137,7,0,0,0,25,0,0,0,0),(137,8,0,0,0,40,0,0,0,0),(137,9,0,0,0,15,20,1,0,0),(137,10,0,0,0,30,50,0,0,0),(137,11,1,0,0,0,0,0,0,0),(137,12,1,0,0,0,0,0,0,0),(137,13,0,0,0,8,20,1,0,0),(137,14,1,0,0,0,0,0,0,0),(137,15,1,0,0,0,0,0,0,0),(137,16,0,0,0,8,20,1,0,0),(137,17,1,0,0,0,0,0,0,0),(137,18,1,0,0,0,0,0,0,0),(137,19,0,0,0,30,50,0,0,0),(137,20,0,0,0,40,20,0,0,0),(137,21,0,0,0,35,0,0,0,0),(137,22,0,0,0,45,0,0,0,0),(137,23,1,0,0,0,0,0,0,0),(137,24,1,0,0,0,0,0,0,0),(137,25,1,0,0,0,0,0,0,0),(137,26,1,0,0,0,0,0,0,0),(137,27,0,0,0,45,0,0,0,0),(137,28,0,0,0,35,0,0,0,0),(137,29,0,0,0,40,20,0,0,0),(137,30,0,0,0,20,0,0,0,0),(137,31,1,0,0,0,0,0,0,0),(137,32,0,0,0,35,0,0,0,0),(137,33,0,0,0,45,0,0,0,0),(137,34,0,1,0,80,0,0,0,0),(137,35,1,0,0,0,0,0,0,0),(137,36,0,0,0,45,0,0,0,0),(137,37,0,0,0,35,0,0,0,0),(137,38,1,0,0,0,0,0,0,0),(137,39,0,0,0,20,0,0,0,0),(137,40,0,0,0,10,0,0,0,0),(137,41,1,0,0,0,0,0,0,0),(137,42,1,0,0,0,0,0,0,0),(137,43,1,0,0,0,0,0,0,0),(137,44,0,0,0,5,50,0,0,0),(137,45,0,0,0,5,50,0,0,0),(137,46,0,1,0,80,0,0,0,0),(137,47,1,0,0,0,0,0,0,0),(137,48,1,0,0,0,0,0,0,0),(137,49,0,0,0,10,0,0,0,0),(137,50,0,0,0,10,0,0,0,0),(137,51,1,0,0,0,0,0,0,0),(137,52,1,0,0,0,0,0,0,0),(137,53,0,1,0,80,0,0,0,0),(137,54,0,0,0,5,50,0,0,0),(137,55,0,0,0,5,50,0,0,0),(137,56,1,0,0,0,0,0,0,0),(137,57,1,0,0,0,0,0,0,0),(137,58,1,0,0,0,0,0,0,0),(137,59,0,0,0,10,0,0,0,0),(137,60,0,0,0,20,0,0,0,0),(137,61,1,0,0,0,0,0,0,0),(137,62,0,0,0,35,0,0,0,0),(137,63,0,0,0,45,0,0,0,0),(137,64,1,0,0,0,0,0,0,0),(137,65,0,1,0,80,0,0,0,0),(137,66,0,0,0,45,0,0,0,0),(137,67,0,0,0,35,0,0,0,0),(137,68,1,0,0,0,0,0,0,0),(137,69,0,0,0,20,0,0,0,0),(137,70,0,0,0,40,20,0,0,0),(137,71,0,0,0,35,0,0,0,0),(137,72,0,0,0,45,0,0,0,0),(137,73,1,0,0,0,0,0,0,0),(137,74,1,0,0,0,0,0,0,0),(140,0,1,0,0,0,0,0,0,0),(140,1,1,0,0,0,0,0,0,0),(140,2,1,0,0,0,0,0,0,0),(140,3,1,0,0,0,0,0,0,0),(140,4,1,0,0,0,0,0,0,0),(140,5,1,0,0,0,0,0,0,0),(140,6,1,0,0,0,0,0,0,0),(140,7,1,0,0,0,0,0,0,0),(140,8,1,0,0,0,0,0,0,0),(140,9,1,0,0,0,0,0,0,0),(140,10,1,0,0,0,0,0,0,0),(140,11,0,0,0,5,50,1,0,0),(140,12,0,0,0,25,35,1,0,0),(140,13,0,0,0,30,15,0,0,0),(140,14,0,0,0,20,20,0,0,0),(140,15,0,0,0,20,20,0,0,0),(140,16,0,0,0,35,15,0,0,0),(140,17,0,0,0,25,35,1,0,0),(140,18,0,0,0,5,50,1,0,0),(140,19,1,0,0,0,0,0,0,0),(140,20,1,0,0,0,0,0,0,0),(140,21,0,0,0,25,35,1,0,0),(140,22,1,0,0,0,0,0,0,0),(140,23,0,0,0,60,20,0,0,0),(140,24,0,0,0,50,0,0,0,0),(140,25,0,0,0,50,0,0,0,0),(140,26,0,0,0,60,20,0,0,0),(140,27,1,0,0,0,0,0,0,0),(140,28,0,0,0,25,35,1,0,0),(140,29,1,0,0,0,0,0,0,0),(140,30,1,0,0,0,0,0,0,0),(140,31,0,0,0,30,15,0,0,0),(140,32,0,0,0,60,20,0,0,0),(140,33,1,0,0,0,0,0,0,0),(140,34,0,1,0,150,0,0,0,0),(140,35,0,1,0,150,0,0,0,0),(140,36,1,0,0,0,0,0,0,0),(140,37,0,0,0,60,20,0,0,0),(140,38,0,0,0,30,15,0,0,0),(140,39,1,0,0,0,0,0,0,0),(140,40,1,0,0,0,0,0,0,0),(140,41,0,0,0,20,20,0,0,0),(140,42,0,0,0,50,0,0,0,0),(140,43,0,1,0,150,0,0,0,0),(140,44,1,0,0,0,0,0,0,0),(140,45,1,0,0,0,0,0,0,0),(140,46,0,1,0,150,0,0,0,0),(140,47,0,0,0,50,0,0,0,0),(140,48,0,0,0,20,20,0,0,0),(140,49,1,0,0,0,0,0,0,0),(140,50,1,0,0,0,0,0,0,0),(140,51,0,0,0,20,20,0,0,0),(140,52,0,0,0,50,0,0,0,0),(140,53,0,1,0,150,0,0,0,0),(140,54,1,0,0,0,0,0,0,0),(140,55,1,0,0,0,0,0,0,0),(140,56,0,1,0,150,0,0,0,0),(140,57,0,0,0,50,0,0,0,0),(140,58,0,0,0,20,20,0,0,0),(140,59,1,0,0,0,0,0,0,0),(140,60,1,0,0,0,0,0,0,0),(140,61,0,0,0,30,15,0,0,0),(140,62,0,0,0,60,20,0,0,0),(140,63,1,0,0,0,0,0,0,0),(140,64,0,1,0,150,0,0,0,0),(140,65,0,1,0,150,0,0,0,0),(140,66,1,0,0,0,0,0,0,0),(140,67,0,0,0,60,20,0,0,0),(140,68,0,0,0,30,15,0,0,0),(140,69,1,0,0,0,0,0,0,0),(140,70,1,0,0,0,0,0,0,0),(140,71,0,0,0,25,35,1,0,0),(140,72,1,0,0,0,0,0,0,0),(140,73,0,0,0,60,20,0,0,0),(140,74,0,0,0,50,0,0,0,0),(140,75,0,0,0,50,0,0,0,0),(140,76,0,0,0,60,20,0,0,0),(140,77,1,0,0,0,0,0,0,0),(140,78,0,0,0,25,35,1,0,0),(140,79,1,0,0,0,0,0,0,0),(140,80,1,0,0,0,0,0,0,0),(140,81,0,0,0,5,50,1,0,0),(140,82,0,0,0,25,35,1,0,0),(140,83,0,0,0,30,15,0,0,0),(140,84,0,0,0,20,20,0,0,0),(140,85,0,0,0,20,20,0,0,0),(140,86,0,0,0,35,15,0,0,0),(140,87,0,0,0,25,35,1,0,0),(140,88,0,0,0,5,50,1,0,0),(140,89,1,0,0,0,0,0,0,0),(140,90,1,0,0,0,0,0,0,0),(140,91,1,0,0,0,0,0,0,0),(140,92,1,0,0,0,0,0,0,0),(140,93,1,0,0,0,0,0,0,0),(140,94,1,0,0,0,0,0,0,0),(140,95,1,0,0,0,0,0,0,0),(140,96,1,0,0,0,0,0,0,0),(140,97,1,0,0,0,0,0,0,0),(140,98,1,0,0,0,0,0,0,0),(140,99,1,0,0,0,0,0,0,0),(146,0,1,0,0,0,0,0,0,0),(153,0,1,0,0,0,0,0,0,0),(153,1,1,0,0,0,0,0,0,0),(153,2,0,0,0,50,50,1,0,0),(153,3,0,0,0,50,50,1,0,0),(153,4,0,0,0,50,50,1,0,0),(153,5,0,0,0,50,50,1,0,0),(153,6,1,0,0,0,0,0,0,0),(153,7,1,0,0,0,0,0,0,0),(153,8,1,0,0,0,0,0,0,0),(153,9,1,0,0,0,0,0,0,0),(153,10,0,0,0,1,20,0,0,0),(153,11,0,0,0,1,10,0,0,0),(153,12,0,1,0,150,0,0,0,0),(153,13,0,1,0,150,0,0,0,0),(153,14,0,1,0,150,0,0,0,0),(153,15,0,1,0,150,0,0,0,0),(153,16,0,0,0,1,10,0,0,0),(153,17,0,0,0,1,20,0,0,0),(153,18,1,0,0,0,0,0,0,0),(153,19,1,0,0,0,0,0,0,0),(153,20,0,0,0,1,40,0,0,0),(153,21,1,0,0,0,0,0,0,0),(153,22,0,0,0,100,0,0,0,0),(153,23,0,0,0,100,0,0,0,0),(153,24,0,0,0,100,0,0,0,0),(153,25,0,0,0,100,0,0,0,0),(153,26,1,0,0,0,0,0,0,0),(153,27,0,0,0,1,40,0,0,0),(153,28,1,0,0,0,0,0,0,0),(153,29,1,0,0,0,0,0,0,0),(153,30,0,0,0,1,60,0,0,0),(0,0,1,0,0,0,0,0,0,0),(0,1,1,0,0,0,0,0,0,0),(0,2,1,0,0,0,0,0,0,0),(0,3,1,0,0,0,0,0,0,0),(0,4,1,0,0,0,0,0,0,0),(0,5,1,0,0,0,0,0,0,0),(0,6,1,0,0,0,0,0,0,0),(0,7,1,0,0,0,0,0,0,0),(0,8,1,0,0,0,0,0,0,0),(0,9,1,0,0,0,0,0,0,0),(0,10,1,0,0,0,0,0,0,0),(0,11,1,0,0,0,0,0,0,0),(0,12,1,0,0,0,0,0,0,0),(0,13,1,0,0,0,0,0,0,0),(0,14,1,0,0,0,0,0,0,0),(0,15,1,0,0,0,0,0,0,0),(0,16,1,0,0,0,0,0,0,0),(0,17,1,0,0,0,0,0,0,0),(0,18,1,0,0,0,0,0,0,0),(0,19,1,0,0,0,0,0,0,0),(0,20,1,0,0,0,0,0,0,0),(0,21,1,0,0,0,0,0,0,0),(0,22,1,0,0,0,0,0,0,0),(0,23,1,0,0,0,0,0,0,0),(0,24,1,0,0,0,0,0,0,0),(0,25,1,0,0,0,0,0,0,0),(0,26,1,0,0,0,0,0,0,0),(0,27,1,0,0,0,0,0,0,0),(0,28,1,0,0,0,0,0,0,0),(0,29,1,0,0,0,0,0,0,0),(0,30,1,0,0,0,0,0,0,0),(0,31,1,0,0,0,0,0,0,0),(0,32,1,0,0,0,0,0,0,0),(0,33,1,0,0,0,0,0,0,0),(0,34,1,0,0,0,0,0,0,0),(0,35,1,0,0,0,0,0,0,0),(0,36,1,0,0,0,0,0,0,0),(0,37,1,0,0,0,0,0,0,0),(0,38,1,0,0,0,0,0,0,0),(0,39,1,0,0,0,0,0,0,0),(0,40,1,0,0,0,0,0,0,0),(0,41,1,0,0,0,0,0,0,0),(0,42,1,0,0,0,0,0,0,0),(0,43,1,0,0,0,0,0,0,0),(0,44,1,0,0,0,0,0,0,0),(0,45,1,0,0,0,0,0,0,0),(0,46,1,0,0,0,0,0,0,0),(0,47,1,0,0,0,0,0,0,0),(0,48,1,0,0,0,0,0,0,0),(0,49,1,0,0,0,0,0,0,0),(0,50,1,0,0,0,0,0,0,0),(0,51,1,0,0,0,0,0,0,0),(0,52,1,0,0,0,0,0,0,0),(0,53,1,0,0,0,0,0,0,0),(0,54,1,0,0,0,0,0,0,0),(0,55,1,0,0,0,0,0,0,0),(0,56,1,0,0,0,0,0,0,0),(0,57,1,0,0,0,0,0,0,0),(0,58,1,0,0,0,0,0,0,0),(0,59,1,0,0,0,0,0,0,0),(0,60,1,0,0,0,0,0,0,0),(0,61,1,0,0,0,0,0,0,0),(0,62,1,0,0,0,0,0,0,0),(0,63,1,0,0,0,0,0,0,0),(0,64,1,0,0,0,0,0,0,0),(0,65,1,0,0,0,0,0,0,0),(0,66,1,0,0,0,0,0,0,0),(0,67,1,0,0,0,0,0,0,0),(0,68,1,0,0,0,0,0,0,0),(0,69,1,0,0,0,0,0,0,0),(0,70,1,0,0,0,0,0,0,0),(0,71,1,0,0,0,0,0,0,0),(0,72,1,0,0,0,0,0,0,0),(0,73,1,0,0,0,0,0,0,0),(0,74,1,0,0,0,0,0,0,0),(0,75,1,0,0,0,0,0,0,0),(0,76,1,0,0,0,0,0,0,0),(0,77,1,0,0,0,0,0,0,0),(0,78,1,0,0,0,0,0,0,0),(0,79,1,0,0,0,0,0,0,0),(0,80,1,0,0,0,0,0,0,0),(0,81,1,0,0,0,0,0,0,0),(0,82,1,0,0,0,0,0,0,0),(0,83,1,0,0,0,0,0,0,0),(0,84,1,0,0,0,0,0,0,0),(0,85,1,0,0,0,0,0,0,0),(0,86,1,0,0,0,0,0,0,0),(0,87,1,0,0,0,0,0,0,0),(0,88,1,0,0,0,0,0,0,0),(0,89,1,0,0,0,0,0,0,0),(0,90,1,0,0,0,0,0,0,0),(0,91,1,0,0,0,0,0,0,0),(0,92,1,0,0,0,0,0,0,0),(0,93,1,0,0,0,0,0,0,0),(0,94,1,0,0,0,0,0,0,0),(0,95,1,0,0,0,0,0,0,0),(0,96,1,0,0,0,0,0,0,0),(0,97,1,0,0,0,0,0,0,0),(0,98,1,0,0,0,0,0,0,0),(0,99,1,0,0,0,0,0,0,0);
/*!40000 ALTER TABLE `template_maps` ENABLE KEYS */;
UNLOCK TABLES;

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
/*!40000 ALTER TABLE `template_maps_teams` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `template_mercenaries`
--

DROP TABLE IF EXISTS `template_mercenaries`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_mercenaries` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `template_id` int(10) unsigned NOT NULL default '0',
  `owner` int(10) unsigned NOT NULL default '0',
  `name` varchar(50) NOT NULL default '',
  `att` tinyint(3) unsigned NOT NULL default '0',
  `def` tinyint(3) unsigned NOT NULL default '0',
  `hp` smallint(5) unsigned NOT NULL default '0',
  `speed` smallint(5) unsigned NOT NULL default '0',
  `race` int(10) unsigned NOT NULL default '0',
  `race_lvl` smallint(5) unsigned NOT NULL default '0',
  `ability_0` int(10) unsigned NOT NULL default '0',
  `ability_1` int(10) unsigned NOT NULL default '0',
  `ability_2` int(10) unsigned NOT NULL default '0',
  `ability_3` int(10) unsigned NOT NULL default '0',
  `ability_0_lvl` smallint(5) unsigned NOT NULL default '0',
  `ability_1_lvl` smallint(5) unsigned NOT NULL default '0',
  `ability_2_lvl` smallint(5) unsigned NOT NULL default '0',
  `ability_3_lvl` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `template_id` (`template_id`)
) ENGINE=MyISAM AUTO_INCREMENT=40 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_mercenaries`
--

LOCK TABLES `template_mercenaries` WRITE;
/*!40000 ALTER TABLE `template_mercenaries` DISABLE KEYS */;
/*!40000 ALTER TABLE `template_mercenaries` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `template_restricted_sciences`
--

DROP TABLE IF EXISTS `template_restricted_sciences`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_restricted_sciences` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `template_id` int(10) unsigned NOT NULL default '0',
  `science_id` int(11) NOT NULL default '0',
  `level` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `template_id` (`template_id`,`science_id`)
) ENGINE=MyISAM AUTO_INCREMENT=32 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_restricted_sciences`
--

LOCK TABLES `template_restricted_sciences` WRITE;
/*!40000 ALTER TABLE `template_restricted_sciences` DISABLE KEYS */;
/*!40000 ALTER TABLE `template_restricted_sciences` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `template_triggers`
--

DROP TABLE IF EXISTS `template_triggers`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_triggers` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `template_sector_id` int(10) unsigned NOT NULL default '0',
  `type` smallint(5) unsigned NOT NULL default '0',
  `trigger_condition` smallint(5) unsigned NOT NULL default '0',
  `x` int(10) unsigned NOT NULL default '0',
  `y` int(10) unsigned NOT NULL default '0',
  `max_triggers` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `template_sector_id` (`template_sector_id`)
) ENGINE=MyISAM AUTO_INCREMENT=244 DEFAULT CHARSET=latin1 COMMENT='ACHTUNG template_sector_id muss keinen foreign enthalten';
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_triggers`
--

LOCK TABLES `template_triggers` WRITE;
/*!40000 ALTER TABLE `template_triggers` DISABLE KEYS */;
/*!40000 ALTER TABLE `template_triggers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `template_units`
--

DROP TABLE IF EXISTS `template_units`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `template_units` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `template_sector_id` int(10) unsigned NOT NULL default '0',
  `template_mercenary_id` int(10) unsigned NOT NULL default '0',
  `count` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `template_sector_id` (`template_sector_id`)
) ENGINE=MyISAM AUTO_INCREMENT=70 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `template_units`
--

LOCK TABLES `template_units` WRITE;
/*!40000 ALTER TABLE `template_units` DISABLE KEYS */;
/*!40000 ALTER TABLE `template_units` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tribes`
--

DROP TABLE IF EXISTS `tribes`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `tribes` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `uid` int(10) unsigned NOT NULL default '0',
  `name` varchar(40) NOT NULL default '',
  `race` smallint(5) unsigned NOT NULL default '0',
  `cost` float unsigned NOT NULL default '0',
  `pic_url` varchar(100) NOT NULL default '',
  `state` tinyint(3) unsigned NOT NULL default '0',
  `hp` smallint(5) unsigned NOT NULL default '0',
  `att` smallint(5) unsigned NOT NULL default '0',
  `def` smallint(5) unsigned NOT NULL default '0',
  `speed` smallint(5) unsigned NOT NULL default '0',
  `extra` mediumint(8) unsigned NOT NULL default '0',
  `count` float NOT NULL default '0',
  `add_value` float unsigned NOT NULL default '0',
  `add_stamp` int(10) unsigned NOT NULL default '0',
  `autodrop` tinyint(1) unsigned NOT NULL default '0',
  `dropped_once` tinyint(1) unsigned NOT NULL default '0',
  `mercenary_id` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `uid` (`uid`,`state`)
) ENGINE=MyISAM AUTO_INCREMENT=48494 DEFAULT CHARSET=utf8 PACK_KEYS=1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `tribes`
--

LOCK TABLES `tribes` WRITE;
/*!40000 ALTER TABLE `tribes` DISABLE KEYS */;
INSERT INTO `tribes` VALUES (48485,3760,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0),(48486,3762,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0),(48487,3764,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0),(48488,3766,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0),(48489,3768,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0),(48490,3770,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0),(48491,3772,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0),(48492,3774,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0),(48493,3776,'Scientists',2,1.5,'scientist.gif',0,1,1,1,1,0,4000,0,0,0,0,0);
/*!40000 ALTER TABLE `tribes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tribes_abilities`
--

DROP TABLE IF EXISTS `tribes_abilities`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `tribes_abilities` (
  `tribe_id` int(10) unsigned NOT NULL,
  `ability_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`tribe_id`,`ability_id`),
  KEY `tribe_id_k` (`tribe_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `tribes_abilities`
--

LOCK TABLES `tribes_abilities` WRITE;
/*!40000 ALTER TABLE `tribes_abilities` DISABLE KEYS */;
/*!40000 ALTER TABLE `tribes_abilities` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tribes_mercenaries`
--

DROP TABLE IF EXISTS `tribes_mercenaries`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `tribes_mercenaries` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `race_lvl` tinyint(4) unsigned NOT NULL default '0',
  `ability_0_lvl` tinyint(3) unsigned NOT NULL default '0',
  `ability_1_lvl` tinyint(3) unsigned NOT NULL default '0',
  `ability_2_lvl` tinyint(3) unsigned NOT NULL default '0',
  `ability_3_lvl` tinyint(3) unsigned NOT NULL default '0',
  `ability_4_lvl` tinyint(3) unsigned NOT NULL default '0',
  `template_mercenary_id` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `template_mercenary_id` (`template_mercenary_id`)
) ENGINE=MyISAM AUTO_INCREMENT=40 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `tribes_mercenaries`
--

LOCK TABLES `tribes_mercenaries` WRITE;
/*!40000 ALTER TABLE `tribes_mercenaries` DISABLE KEYS */;
/*!40000 ALTER TABLE `tribes_mercenaries` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tribes_on_map`
--

DROP TABLE IF EXISTS `tribes_on_map`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `tribes_on_map` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `tribe_id` int(10) unsigned NOT NULL,
  `name` varchar(40) character set utf8 default NULL,
  `path` varchar(200) NOT NULL,
  `world_id` smallint(5) unsigned NOT NULL default '0',
  `map_id` int(10) unsigned NOT NULL default '0',
  `sector_id` tinyint(3) unsigned NOT NULL,
  `amount` int(10) unsigned NOT NULL,
  `t_appearance` int(10) unsigned NOT NULL,
  `t_movement_started` int(10) unsigned NOT NULL,
  `moved_by` int(10) unsigned NOT NULL,
  `sex` tinyint(3) unsigned NOT NULL default '0',
  `outerwear` smallint(5) unsigned NOT NULL default '0',
  `trousers` smallint(5) unsigned NOT NULL default '0',
  `shoes` smallint(5) unsigned NOT NULL default '0',
  `jewelry` smallint(5) unsigned NOT NULL default '0',
  `hairhat` smallint(5) unsigned NOT NULL default '0',
  `weapon_style` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `sector_id` (`sector_id`),
  KEY `map_id` (`world_id`),
  KEY `tribe_id` (`tribe_id`),
  KEY `map_id_2` (`map_id`)
) ENGINE=MyISAM AUTO_INCREMENT=30754 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `tribes_on_map`
--

LOCK TABLES `tribes_on_map` WRITE;
/*!40000 ALTER TABLE `tribes_on_map` DISABLE KEYS */;
/*!40000 ALTER TABLE `tribes_on_map` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `users` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `session_identifier` int(10) unsigned NOT NULL default '0',
  `sid` varchar(255) NOT NULL,
  `homesector` int(10) unsigned NOT NULL default '0',
  `waiting_homesector` int(10) unsigned NOT NULL default '0',
  `waiting_state` int(10) unsigned NOT NULL default '0',
  `waiting_finished` int(10) unsigned NOT NULL default '0',
  `nick` varchar(25) NOT NULL default '',
  `nick_main` varchar(15) NOT NULL default '',
  `nick_suffix` int(10) unsigned NOT NULL default '0',
  `email` varchar(100) default NULL,
  `initial_email` varchar(50) NOT NULL default '',
  `pass` varchar(60) character set latin1 collate latin1_bin NOT NULL default '',
  `state` tinyint(3) unsigned NOT NULL default '0',
  `banned` tinyint(1) unsigned NOT NULL default '0',
  `signup` int(10) unsigned NOT NULL default '0',
  `last_login` int(10) unsigned NOT NULL default '0',
  `login_count` int(10) unsigned NOT NULL default '0',
  `last_access` int(10) unsigned NOT NULL default '2000000000',
  `tutorial` int(10) unsigned NOT NULL default '0',
  `last_score` float NOT NULL default '0',
  `add_keep_sciences` tinyint(4) NOT NULL default '0',
  `users_killed` smallint(5) unsigned NOT NULL default '0',
  `maps_won` smallint(5) unsigned NOT NULL default '0',
  `maps_lost` smallint(5) unsigned NOT NULL default '0',
  `sp_maps_won` smallint(5) unsigned NOT NULL default '0',
  `sp_maps_lost` smallint(5) unsigned NOT NULL default '0',
  `suicides` smallint(5) unsigned NOT NULL default '0',
  `play_time` int(10) unsigned NOT NULL default '0',
  `newsletter` tinyint(1) unsigned NOT NULL default '1',
  `ingame_language` char(2) NOT NULL default 'en',
  `fb_sync_friends` tinyint(1) unsigned NOT NULL default '1',
  PRIMARY KEY  (`id`),
  KEY `homesector` (`homesector`),
  KEY `nick` (`nick`),
  KEY `fb_sync_friends` (`fb_sync_friends`)
) ENGINE=MyISAM AUTO_INCREMENT=6770 DEFAULT CHARSET=latin1 PACK_KEYS=1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (3760,6366006,'',0,0,0,0,'Philipp','Philipp',0,'p.kemmeter@gausus.com','p.kemmeter@gausus.com','cfc43fddef8c657d0ac854369fd16a0b',3,0,1257320234,1298665174,1886,1298665179,7807,0,0,0,0,0,0,0,0,0,1,'en',1),(3762,0,'',0,0,0,0,'Mammun','Mammun',0,'phil.kemmeter@gmail.com','phil.kemmeter@gmail.com','430b565227268f290f1838e377eee3b8',3,0,1257320777,1298569950,65,1298569950,7807,0,0,0,0,0,0,0,0,0,1,'en',1),(3764,0,'',0,0,0,0,'Bernard','Bernard',0,'info@mammutheum.de','p.kemmeter@gausus.de','460b23f0558f0bbb1d0de83f101f9307',3,0,1257320830,1298569950,368,1298569950,7807,0,0,0,0,0,0,0,0,0,1,'en',1),(3766,0,'',0,0,0,0,'Robert','Robert',0,'r.kemmeter@gausus.com','r.kemmeter@gausus.com','77702a77aee82a35e6f7c15c122c0ff9',3,0,1257320834,1298569950,2100,1298569950,7807,0,0,0,0,0,0,0,0,0,1,'en',1),(3768,0,'',0,0,0,0,'admin','admin',0,'admin.mammun@web-kem.de','admin.mammun@web-kem.de','562b130a535f509cf1f6a234e712ecfd',3,0,1257321340,1298569950,51,1298569950,7807,0,0,0,0,0,0,0,0,0,1,'en',1),(3770,0,'',0,0,0,0,'Metatron','Metatron',0,'metatron@web-kem.de','metatron@web-kem.de','64bb5cad361bf10c99f53fe36cc5259b',3,0,1257321672,1298569950,389,1298569950,7807,0,0,0,0,0,0,0,0,0,1,'en',1),(3772,0,'',0,0,0,0,'Klomeister','Klomeister',0,'Arsch-Keks-Keks@web.de','Arsch-Keks-Keks@web.de','15bc60c1ba1134bde387cd307cff0239',3,0,1257321771,1298569950,157,1298569950,7807,0,0,0,0,0,0,0,0,0,1,'en',1),(3774,0,'',0,0,0,0,'Andreas','Andreas',0,'agoeb@gmx.net','agoeb@gmx.net','ae1b7ec7f699fe1875012787e25c7034',3,0,1257321795,1298569950,1573,1298569950,7807,0,0,0,0,0,0,0,0,0,1,'en',1),(3776,0,'',0,0,0,0,'Annika','Annika',0,'annika.voth@gmail.com','annika.voth@gmail.com','7087af90171f129d0315246b5a7266a4',3,0,1257321978,1298569950,1929,1298569950,7807,0,0,0,0,0,0,0,0,0,1,'en',1);
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users_brezeln`
--

DROP TABLE IF EXISTS `users_brezeln`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `users_brezeln` (
  `uid` bigint(20) unsigned NOT NULL,
  `amount` smallint(5) unsigned NOT NULL,
  PRIMARY KEY  (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `users_brezeln`
--

LOCK TABLES `users_brezeln` WRITE;
/*!40000 ALTER TABLE `users_brezeln` DISABLE KEYS */;
/*!40000 ALTER TABLE `users_brezeln` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users_map_keys`
--

DROP TABLE IF EXISTS `users_map_keys`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `users_map_keys` (
  `uid` bigint(20) unsigned NOT NULL,
  `auth_key` varchar(6) NOT NULL default '',
  `received_by` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`auth_key`,`received_by`),
  KEY `uid` (`uid`),
  KEY `auth_key` (`auth_key`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `users_map_keys`
--

LOCK TABLES `users_map_keys` WRITE;
/*!40000 ALTER TABLE `users_map_keys` DISABLE KEYS */;
/*!40000 ALTER TABLE `users_map_keys` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users_medals`
--

DROP TABLE IF EXISTS `users_medals`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `users_medals` (
  `uid` bigint(20) unsigned NOT NULL,
  `medal_id` smallint(5) unsigned NOT NULL,
  `t_stamp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uid`,`medal_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `users_medals`
--

LOCK TABLES `users_medals` WRITE;
/*!40000 ALTER TABLE `users_medals` DISABLE KEYS */;
/*!40000 ALTER TABLE `users_medals` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users_quests`
--

DROP TABLE IF EXISTS `users_quests`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `users_quests` (
  `uid` bigint(20) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `subtype` tinyint(3) unsigned NOT NULL,
  `resolved` tinyint(1) unsigned NOT NULL,
  `tstamp` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`uid`,`type`,`subtype`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `users_quests`
--

LOCK TABLES `users_quests` WRITE;
/*!40000 ALTER TABLE `users_quests` DISABLE KEYS */;
/*!40000 ALTER TABLE `users_quests` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users_waiting`
--

DROP TABLE IF EXISTS `users_waiting`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `users_waiting` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `fullname` varchar(40) NOT NULL default '',
  `email` varchar(50) NOT NULL default '',
  `newsletter` tinyint(1) unsigned NOT NULL default '0',
  `vip_key_mailed` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `users_waiting`
--

LOCK TABLES `users_waiting` WRITE;
/*!40000 ALTER TABLE `users_waiting` DISABLE KEYS */;
/*!40000 ALTER TABLE `users_waiting` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `worlds`
--

DROP TABLE IF EXISTS `worlds`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `worlds` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name_index` tinyint(3) unsigned NOT NULL,
  `free_maps` mediumint(3) unsigned NOT NULL default '100',
  `wait_maps` mediumint(3) unsigned NOT NULL default '0',
  `minimum_level` smallint(5) unsigned NOT NULL default '0',
  `maximum_level` smallint(5) unsigned NOT NULL default '0',
  `type` tinyint(1) unsigned NOT NULL default '0',
  `next_joint_world` int(10) unsigned NOT NULL default '0',
  `cur_joint_world` int(10) unsigned NOT NULL default '0',
  `first_joint_world` int(10) unsigned NOT NULL default '0',
  `map_id_offset` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=37 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `worlds`
--

LOCK TABLES `worlds` WRITE;
/*!40000 ALTER TABLE `worlds` DISABLE KEYS */;
INSERT INTO `worlds` VALUES (1,2,100,0,6,10,0,0,1,1,0),(2,3,100,0,11,15,0,0,2,2,0),(3,4,100,0,16,20,0,0,3,3,0),(4,5,100,0,21,25,0,0,4,4,0),(5,6,100,0,26,30,0,0,5,5,0),(6,0,100,0,0,1000,1,0,6,6,0),(7,0,100,0,0,5,0,0,7,7,0),(8,0,100,0,0,5,2,0,8,8,0),(9,7,100,0,31,35,0,0,9,9,0),(10,8,100,0,36,40,0,0,10,10,0),(11,9,100,0,41,45,0,0,11,11,0),(12,1,100,0,0,5,0,0,12,12,0),(13,10,100,0,46,50,0,0,13,0,0),(14,11,100,0,51,55,0,0,14,14,0),(15,12,100,0,56,60,0,0,15,15,0),(16,13,100,0,61,65,0,0,16,16,0),(17,14,100,0,66,70,0,0,17,17,0),(18,15,100,0,71,75,0,0,18,18,0),(19,16,100,0,76,80,0,0,19,19,0);
/*!40000 ALTER TABLE `worlds` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-02-25 20:20:26
