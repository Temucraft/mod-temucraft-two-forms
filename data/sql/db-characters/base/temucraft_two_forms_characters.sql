-- mod-temucraft-two-forms: each Worgen's human look, and its Worgen look while it is in human form
CREATE TABLE IF NOT EXISTS `mod_temucraft_two_forms` (
  `guid` INT UNSIGNED NOT NULL,
  `chosen` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'a human look was picked',
  `is_human` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'in human form at the last change: the worgen_* look is put back at login',
  `skin` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `face` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `hair` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `color` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `facial` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `worgen_skin` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `worgen_face` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `worgen_hair` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `worgen_color` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `worgen_facial` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
