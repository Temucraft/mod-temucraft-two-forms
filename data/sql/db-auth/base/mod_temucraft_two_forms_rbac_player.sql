-- mod-temucraft-two-forms: its commands (all under .temucraft) as RBAC permissions 103450-103499. Every command is admin only by
-- default: linked to the administrator roles 192 and 1014. To give one to players, link it to 195 and 1010 here (or
-- grant it per account with .rbac account grant <account> <id>). "Temucraft: use ..." permissions are not
-- commands: they stay linked to the player roles so the items keep working.
DELETE FROM `rbac_linked_permissions` WHERE `linkedId` BETWEEN 103450 AND 103499;
DELETE FROM `rbac_permissions` WHERE `id` BETWEEN 103450 AND 103499;
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES
(103450, 'Command: temucraft twoforms look'),
(103451, 'Command: temucraft twoforms');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES
(192, 103450),
(1014, 103450),
(192, 103451),
(1014, 103451);
