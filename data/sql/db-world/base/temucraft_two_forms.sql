-- mod-temucraft-two-forms: Two Forms (male 68996, female 68995) runs the module's script; player texts
SET @MALE := 68996; SET @FEMALE := 68995;
DELETE FROM `spell_script_names` WHERE `spell_id` IN (@MALE, @FEMALE) AND `ScriptName` = 'spell_temucraft_two_forms';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (@MALE, 'spell_temucraft_two_forms'), (@FEMALE, 'spell_temucraft_two_forms');

DELETE FROM `module_string` WHERE `module` = 'mod-temucraft-two-forms';
INSERT INTO `module_string` (`module`, `id`, `string`) VALUES
('mod-temucraft-two-forms', 1, 'Skin colour'),
('mod-temucraft-two-forms', 2, 'Face'),
('mod-temucraft-two-forms', 3, 'Hair style'),
('mod-temucraft-two-forms', 4, 'Hair colour'),
('mod-temucraft-two-forms', 5, 'Beard'),
('mod-temucraft-two-forms', 6, 'next'),
('mod-temucraft-two-forms', 7, 'previous'),
('mod-temucraft-two-forms', 8, 'Done'),
('mod-temucraft-two-forms', 9, 'Only Worgen have Two Forms.'),
('mod-temucraft-two-forms', 10, 'Not in combat.');
