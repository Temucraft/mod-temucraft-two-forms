# mod-temucraft-two-forms

Two Forms for Worgen that keeps your gear.

The stock spell turned a Worgen into a creature display of a human: a fixed NPC look with none of your armour. With
this module, Two Forms puts the Worgen in the **human player body** (the same body a human character has), which shows
your gear like any player.

- **Your human look:** the Worgen's skin, face and hair numbers would pick random human options, so each character picks
  a human look once. The first time you use Two Forms (or with `.twoforms look`) a menu opens: skin colour, face, hair
  style, hair colour and beard (earrings and markings for women), each with next and previous. You see every change at
  once.
- **Switching:** the Two Forms racial, or `.twoforms`. Entering combat or casting Darkflight turns you back into a Worgen
  (`Temucraft.TwoForms.WorgenInCombat`). Druid forms and disguises still work: you come back to the human body after them.
- **Safe saves:** the look values are part of the saved character, so the Worgen look is kept in the characters table
  `mod_temucraft_two_forms` and put back at the next login. You always log in as a Worgen.

## Install

Put the folder in `modules/`, re-run CMake, build, copy the conf file. The world SQL points the Two Forms spells
(68996 male, 68995 female, taught by mod-worgoblin-high-elf) at this module's script; the characters SQL adds the table.

## Not tested in game yet

What to check: the human body shows your armour and weapons; the picked look is right (skin, face, hair, colour,
beard); combat and Darkflight turn you back; a druid form ends in the right body; logging out in human form and back in
gives your Worgen look; nothing is wrong at the barber afterwards.

How many choices each option has (`LIMITS` in the code) follows the stock human options; with custom hair styles in the
client, raise them.
