/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

// Two Forms for Worgen, with your gear and a face of your own.
//
// The stock spell turns a Worgen into a creature display of a human: a fixed NPC with none of the player's armour.
// Here Two Forms puts the Worgen in the human player body instead (the body a human character has), which shows the
// gear. The Worgen's skin, face and hair numbers would pick random human options, so each character chooses a human
// look once (.temucraft twoforms look) and it is swapped in while in human form, and the Worgen look swapped back after.
//
// The look values are part of what the server saves for a character, so the Worgen look is also kept in
// mod_temucraft_two_forms and put back when the character logs in (a save can happen while in human form).

#include "TemucraftLocked.h"
#include "Chat.h"
#include "CommandScript.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GameObject.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellScriptLoader.h"
#include "TemucraftModuleText.h"
#include <unordered_map>

using namespace Acore::ChatCommands;

namespace
{
    char const* const MODULE = "mod-temucraft-two-forms";
    uint32 const MENU = 0x54574F46;   // our own gossip menu id for the look picker
    uint32 const SPELL_DARKFLIGHT = 68992;

    struct Settings
    {
        bool enable = true;
        uint8 race = 12;
        uint32 maleDisplay = 49, femaleDisplay = 50;
        bool dropInCombat = true;
    } cfg;

    struct Look { uint8 skin = 0, face = 0, hair = 0, color = 0, facial = 0; };
    struct State
    {
        Look human, worgen;
        bool chosen = false;   // a human look was picked
        bool isHuman = false;
        uint32 worgenNative = 0;
    };
    Temucraft::LockedMap<ObjectGuid::LowType, State> states;

    // how many choices the human body has, by gender (male, female)
    uint8 const LIMITS[2][5] = { { 10, 12, 17, 10, 9 }, { 10, 15, 24, 10, 7 } };

    char const* T(Player* player, uint32 id, char const* fallback) { return TemucraftModuleText(MODULE, player, id, fallback); }

    Look Current(Player* player)
    {
        return { player->GetByteValue(PLAYER_BYTES, 0), player->GetByteValue(PLAYER_BYTES, 1), player->GetByteValue(PLAYER_BYTES, 2),
                 player->GetByteValue(PLAYER_BYTES, 3), player->GetByteValue(PLAYER_BYTES_2, 0) };
    }

    void Wear(Player* player, Look const& look)
    {
        player->SetByteValue(PLAYER_BYTES, 0, look.skin);
        player->SetByteValue(PLAYER_BYTES, 1, look.face);
        player->SetByteValue(PLAYER_BYTES, 2, look.hair);
        player->SetByteValue(PLAYER_BYTES, 3, look.color);
        player->SetByteValue(PLAYER_BYTES_2, 0, look.facial);
    }

    void Store(Player* player, State const& s)
    {
        CharacterDatabase.Execute("REPLACE INTO mod_temucraft_two_forms (guid, chosen, is_human, skin, face, hair, color, facial, "
            "worgen_skin, worgen_face, worgen_hair, worgen_color, worgen_facial) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
            player->GetGUID().GetCounter(), s.chosen ? 1 : 0, s.isHuman ? 1 : 0, s.human.skin, s.human.face, s.human.hair, s.human.color,
            s.human.facial, s.worgen.skin, s.worgen.face, s.worgen.hair, s.worgen.color, s.worgen.facial);
    }

    bool IsWorgen(Player* player) { return cfg.enable && player->getRace() == cfg.race; }
    uint32 HumanDisplay(Player* player) { return player->getGender() == GENDER_FEMALE ? cfg.femaleDisplay : cfg.maleDisplay; }

    void ToHuman(Player* player, State& s)
    {
        if (s.isHuman)
            return;
        s.worgen = Current(player);
        s.worgenNative = player->GetNativeDisplayId();
        s.isHuman = true;
        Store(player, s);   // first, so a save in human form can be undone at the next login
        bool shown = player->GetDisplayId() == s.worgenNative;
        player->SetNativeDisplayId(HumanDisplay(player));
        Wear(player, s.human);
        if (shown)
            player->SetDisplayId(HumanDisplay(player));   // in a druid form or a disguise, the human body comes back after it
    }

    void ToWorgen(Player* player, State& s)
    {
        if (!s.isHuman)
            return;
        s.isHuman = false;
        Wear(player, s.worgen);
        uint32 native = s.worgenNative ? s.worgenNative : player->GetNativeDisplayId();
        bool shown = player->GetDisplayId() == HumanDisplay(player);
        player->SetNativeDisplayId(native);
        if (shown)
            player->SetDisplayId(native);
        Store(player, s);
    }

    void ShowPicker(Player* player)
    {
        State& s = states[player->GetGUID().GetCounter()];
        if (!s.chosen)
        {
            s.human = Current(player);   // a start: the Worgen's own numbers, kept inside the human limits below
            uint8 const* lim = LIMITS[player->getGender() == GENDER_FEMALE ? 1 : 0];
            s.human = { uint8(s.human.skin % lim[0]), uint8(s.human.face % lim[1]), uint8(s.human.hair % lim[2]),
                        uint8(s.human.color % lim[3]), uint8(s.human.facial % lim[4]) };
            s.chosen = true;
            Store(player, s);
        }
        if (!s.isHuman)
            ToHuman(player, s);   // so every change shows at once
        ClearGossipMenuFor(player);
        char const* names[5] = { T(player, 1, "Skin colour"), T(player, 2, "Face"), T(player, 3, "Hair style"),
                                 T(player, 4, "Hair colour"), player->getGender() == GENDER_FEMALE ? T(player, 11, "Earrings and markings") : T(player, 5, "Beard") };
        uint8 const* values[5] = { &s.human.skin, &s.human.face, &s.human.hair, &s.human.color, &s.human.facial };
        uint8 const* lim = LIMITS[player->getGender() == GENDER_FEMALE ? 1 : 0];
        for (uint8 i = 0; i < 5; ++i)
        {
            std::string label = Acore::StringFormat("{} {}/{}", names[i], *values[i] + 1, lim[i]);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormat("{}: {}", label, T(player, 6, "next")), 1, i * 2);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormat("{}: {}", label, T(player, 7, "previous")), 1, i * 2 + 1);
        }
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, T(player, 8, "Done"), 1, 100);
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(MENU);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, player->GetGUID());
    }
}

class TemucraftTwoForms_Player : public PlayerScript
{
public:
    TemucraftTwoForms_Player() : PlayerScript("TemucraftTwoForms_Player") { }

    void OnPlayerLogin(Player* player) override
    {
        if (!IsWorgen(player))
            return;
        State s;
        if (QueryResult result = CharacterDatabase.Query("SELECT chosen, is_human, skin, face, hair, color, facial, worgen_skin, worgen_face, "
                "worgen_hair, worgen_color, worgen_facial FROM mod_temucraft_two_forms WHERE guid = {}", player->GetGUID().GetCounter()))
        {
            Field* f = result->Fetch();
            s.chosen = f[0].Get<bool>();
            bool wasHuman = f[1].Get<bool>();
            s.human = { f[2].Get<uint8>(), f[3].Get<uint8>(), f[4].Get<uint8>(), f[5].Get<uint8>(), f[6].Get<uint8>() };
            s.worgen = { f[7].Get<uint8>(), f[8].Get<uint8>(), f[9].Get<uint8>(), f[10].Get<uint8>(), f[11].Get<uint8>() };
            if (wasHuman)
            {
                // the last save may hold the human look: give the character its Worgen look back, and write it to the
                // character row in the same transaction that clears is_human, so a crash can not lose the Worgen look
                Wear(player, s.worgen);
                CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
                trans->Append("UPDATE characters SET skin = {}, face = {}, hairStyle = {}, hairColor = {}, facialStyle = {} WHERE guid = {}",
                    s.worgen.skin, s.worgen.face, s.worgen.hair, s.worgen.color, s.worgen.facial, player->GetGUID().GetCounter());
                trans->Append("UPDATE mod_temucraft_two_forms SET is_human = 0 WHERE guid = {}", player->GetGUID().GetCounter());
                CharacterDatabase.CommitTransaction(trans);
            }
        }
        states[player->GetGUID().GetCounter()] = s;
    }

    // before the logout save, so the character is saved with its Worgen look
    void OnPlayerBeforeLogout(Player* player) override
    {
        auto it = states.find(player->GetGUID().GetCounter());
        if (it != states.end() && it->second.isHuman)
            ToWorgen(player, it->second);
    }

    void OnPlayerLogout(Player* player) override { states.erase(player->GetGUID().GetCounter()); }

    void OnPlayerEnterCombat(Player* player, Unit* /*enemy*/) override
    {
        if (!cfg.dropInCombat || !IsWorgen(player))
            return;
        auto it = states.find(player->GetGUID().GetCounter());
        if (it != states.end() && it->second.isHuman)
            ToWorgen(player, it->second);
    }

    // Darkflight is a Worgen-form ability
    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (spell->GetSpellInfo()->Id != SPELL_DARKFLIGHT || !IsWorgen(player))
            return;
        auto it = states.find(player->GetGUID().GetCounter());
        if (it != states.end() && it->second.isHuman)
            ToWorgen(player, it->second);
    }

    void OnPlayerGossipSelect(Player* player, uint32 menu, uint32 /*sender*/, uint32 action) override
    {
        if (menu != MENU || !IsWorgen(player))
            return;
        State& s = states[player->GetGUID().GetCounter()];
        if (action == 100 || (cfg.dropInCombat && player->IsInCombat()))
        {
            CloseGossipMenuFor(player);   // the picker shows the human form, which is not for combat
            return;
        }
        uint8* values[5] = { &s.human.skin, &s.human.face, &s.human.hair, &s.human.color, &s.human.facial };
        uint8 const* lim = LIMITS[player->getGender() == GENDER_FEMALE ? 1 : 0];
        uint8 i = std::min<uint32>(action / 2, 4);
        *values[i] = action % 2 == 0 ? uint8((*values[i] + 1) % lim[i]) : uint8((*values[i] + lim[i] - 1) % lim[i]);
        if (s.isHuman)
            Wear(player, s.human);
        Store(player, s);
        ShowPicker(player);
    }
};

// Two Forms (the racial): switches between the Worgen and the human body instead of the stock creature illusion
class spell_temucraft_two_forms : public SpellScript
{
    PrepareSpellScript(spell_temucraft_two_forms);

    void HandleHit()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !IsWorgen(player))
            return;
        PreventHitAura();   // no creature illusion
        State& s = states[player->GetGUID().GetCounter()];
        if (s.isHuman)
            ToWorgen(player, s);
        else if (cfg.dropInCombat && player->IsInCombat())
            return;   // the human form is not for combat
        else if (!s.chosen)
            ShowPicker(player);   // the first time: pick a human look
        else
            ToHuman(player, s);
    }

    void Register() override { OnHit += SpellHitFn(spell_temucraft_two_forms::HandleHit); }
};

class TemucraftTwoForms_Command : public CommandScript
{
public:
    TemucraftTwoForms_Command() : CommandScript("TemucraftTwoForms_Command") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable sub =
        {
            { "look", HandleLook, 103450, Console::No },
            { "",     HandleToggle, 103451, Console::No },
        };
        static ChatCommandTable root = { { "temucraft twoforms", sub } };
        return root;
    }

    static bool HandleLook(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!IsWorgen(player))
        {
            handler->SendSysMessage(T(player, 9, "Only Worgen have Two Forms."));
            return true;
        }
        if (cfg.dropInCombat && player->IsInCombat())
        {
            handler->SendSysMessage(T(player, 10, "Not in combat."));
            return true;
        }
        ShowPicker(player);
        return true;
    }

    static bool HandleToggle(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!IsWorgen(player))
        {
            handler->SendSysMessage(T(player, 9, "Only Worgen have Two Forms."));
            return true;
        }
        if (player->IsInCombat())
        {
            handler->SendSysMessage(T(player, 10, "Not in combat."));
            return true;
        }
        State& s = states[player->GetGUID().GetCounter()];
        if (!s.chosen)
            ShowPicker(player);
        else if (s.isHuman)
            ToWorgen(player, s);
        else
            ToHuman(player, s);
        return true;
    }
};

class TemucraftTwoForms_World : public WorldScript
{
public:
    TemucraftTwoForms_World() : WorldScript("TemucraftTwoForms_World") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        cfg.enable = sConfigMgr->GetOption<bool>("Temucraft.TwoForms.Enable", true);
        cfg.race = sConfigMgr->GetOption<uint8>("Temucraft.TwoForms.Race", 12);
        cfg.maleDisplay = sConfigMgr->GetOption<uint32>("Temucraft.TwoForms.HumanMaleDisplay", 49);
        cfg.femaleDisplay = sConfigMgr->GetOption<uint32>("Temucraft.TwoForms.HumanFemaleDisplay", 50);
        cfg.dropInCombat = sConfigMgr->GetOption<bool>("Temucraft.TwoForms.WorgenInCombat", true);
    }
};

// the barber shop changes the Worgen look: a Worgen in human form turns back before sitting down, so the change is kept
class TemucraftTwoForms_Barber : public AllGameObjectScript
{
public:
    TemucraftTwoForms_Barber() : AllGameObjectScript("TemucraftTwoForms_Barber") { }

    bool CanGameObjectGossipHello(Player* player, GameObject* go) override
    {
        if (go->GetGoType() != GAMEOBJECT_TYPE_BARBER_CHAIR || !IsWorgen(player))
            return false;
        auto it = states.find(player->GetGUID().GetCounter());
        if (it != states.end() && it->second.isHuman)
        {
            ToWorgen(player, it->second);
            ChatHandler(player->GetSession()).SendSysMessage(TemucraftModuleText(MODULE, player, 12, "The barber works on your Worgen form: you change back first."));
        }
        return false;   // the chair works as usual
    }
};

void AddTemucraftTwoFormsScripts()
{
    new TemucraftTwoForms_Barber();
    new TemucraftTwoForms_Player();
    RegisterSpellScript(spell_temucraft_two_forms);
    new TemucraftTwoForms_Command();
    new TemucraftTwoForms_World();
}
