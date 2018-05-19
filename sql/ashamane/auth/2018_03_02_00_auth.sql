UPDATE `account` SET `expansion`=7 WHERE `expansion`=6;

ALTER TABLE `account` CHANGE `expansion` `expansion` tinyint(3) UNSIGNED NOT NULL DEFAULT 7;
