-- MySQL dump 10.11
--
-- Host: localhost    Database: user_personal
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
-- Table structure for table `user_personal`
--

DROP TABLE IF EXISTS `user_personal`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user_personal` (
  `id` bigint(20) unsigned NOT NULL,
  `id_wcf` int(10) unsigned NOT NULL,
  `id_fb` bigint(20) unsigned NOT NULL,
  `nick` varchar(30) NOT NULL,
  `email` varchar(100) default NULL,
  `gender` tinyint(3) unsigned NOT NULL default '0',
  `salutation` varchar(10) NOT NULL,
  `title` varchar(20) NOT NULL,
  `firstnames` varchar(30) NOT NULL,
  `surnames` varchar(30) NOT NULL,
  `birthday` date NOT NULL,
  `street` varchar(30) NOT NULL,
  `street_number` varchar(5) NOT NULL,
  `zipcode` mediumint(8) unsigned NOT NULL,
  `state` varchar(20) NOT NULL,
  `city` varchar(30) NOT NULL,
  `country` varchar(2) NOT NULL,
  `phone` varchar(20) NOT NULL,
  `mobile` varchar(20) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `surnames` (`surnames`),
  KEY `nick` (`nick`),
  KEY `id_fb` (`id_fb`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `user_personal`
--

LOCK TABLES `user_personal` WRITE;
/*!40000 ALTER TABLE `user_personal` DISABLE KEYS */;
INSERT INTO `user_personal` VALUES (3760,2,0,'','p.kemmeter@gausus.com',1,'','','','Ph Ke','2009-03-05','','0',0,'','Mua','DE','',''),(3776,18,100000889560086,'Annika','annika.voth@gmail.com',0,'','','','','1985-07-13','','',0,'','Buxtehude Jr. am Main','DE','',''),(3774,16,1121857122,'Andreas','agoeb@gmx.net',0,'','','','Andreas','1983-02-03','','',0,'','Darmstadt','DE','',''),(3772,17,0,'Klomeister','Arsch-Keks-Keks@web.de',0,'','','','','1987-00-13','','',0,'','','AX','',''),(3770,4,0,'Metatron','metatron@web-kem.de',0,'','','','Das Metatron','0000-00-00','','',0,'','','','',''),(3768,1,0,'admin','admin.mammun@web-kem.de',0,'','','','','0000-00-00','','',0,'','','','',''),(3766,12,100000462639481,'robert','r.kemmeter@gausus.com',0,'','','Robert','','1980-11-21','','',0,'','Mainz','DE','',''),(3764,13,0,'Bernard','p.kemmeter@gausus.de',0,'','','','','0000-00-00','','',0,'','','','',''),(3762,11,0,'Mammun','phil.kemmeter@gmail.com',0,'','','','','0000-00-00','','',0,'','','','','');
/*!40000 ALTER TABLE `user_personal` ENABLE KEYS */;
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
