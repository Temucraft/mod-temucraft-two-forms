-- mod-temucraft-two-forms: its player commands (all under .temucraft) as RBAC permissions 103450-103499. Linked to the stock player
-- role 195 and the Temucraft player role 1010, so every account has them by default. To take one away, remove its
-- link here (or deny it per account with .rbac account deny <account> <id>); to give it to staff only, link it to
-- 1011-1014 instead.
DELETE FROM `rbac_linked_permissions` WHERE `linkedId` BETWEEN 103450 AND 103499;
DELETE FROM `rbac_permissions` WHERE `id` BETWEEN 103450 AND 103499;
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES
(103450, 'Command: temucraft twoforms look'),
(103451, 'Command: temucraft twoforms');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES
(195, 103450),
(1010, 103450),
(195, 103451),
(1010, 103451);
