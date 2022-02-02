-- DB update 2022_01_04_02 -> 2022_01_04_03
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2022_01_04_02';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2022_01_04_02 2022_01_04_03 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1641299782952581800'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--

INSERT INTO `version_db_world` (`sql_rev`) VALUES ('1641299782952581800');

DELETE FROM `gameobject_template` WHERE `entry` IN (300010,300015,300028,300052,300056,300064,300084,300107,300115,300126,300160,300162,300165,300180,300191,300195,300196,300205);
DELETE FROM `gameobject_template_addon` WHERE `entry` IN (300010,300015,300028,300052,300056,300064,300084,300107,300115,300126,300160,300162,300165,300180,300191,300195,300196,300205);

--
-- END UPDATING QUERIES
--
UPDATE version_db_world SET date = '2022_01_04_03' WHERE sql_rev = '1641299782952581800';
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;