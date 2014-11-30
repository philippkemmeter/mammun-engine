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
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-11-25 17:04:26
