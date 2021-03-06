/*
 * Copyright (C) 2008-2013 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Updated by: Toba and Baeumchen (maddin)
 */

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "PassiveAI.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "SpellAuraEffects.h"
#include "SmartAI.h"
#include "icecrown_citadel.h"
#include "Pet.h"

// Weekly quest support
// * Deprogramming                (DONE)
// * Securing the Ramparts        (DONE)
// * Residue Rendezvous           (DONE)
// * Blood Quickening             (DONE)
// * Respite for a Tormented Soul

enum Texts
{
    // Highlord Tirion Fordring (at Light's Hammer)
    SAY_TIRION_INTRO_1              = 0,
    SAY_TIRION_INTRO_2              = 1,
    SAY_TIRION_INTRO_3              = 2,
    SAY_TIRION_INTRO_4              = 3,
    SAY_TIRION_INTRO_H_5            = 4,
    SAY_TIRION_INTRO_A_5            = 5,

    // The Lich King (at Light's Hammer)
    SAY_LK_INTRO_1                  = 0,
    SAY_LK_INTRO_2                  = 1,
    SAY_LK_INTRO_3                  = 2,
    SAY_LK_INTRO_4                  = 3,
    SAY_LK_INTRO_5                  = 4,

    // Highlord Bolvar Fordragon (at Light's Hammer)
    SAY_BOLVAR_INTRO_1              = 0,

    // High Overlord Saurfang (at Light's Hammer)
    SAY_SAURFANG_INTRO_1            = 15,
    SAY_SAURFANG_INTRO_2            = 16,
    SAY_SAURFANG_INTRO_3            = 17,
    SAY_SAURFANG_INTRO_4            = 18,

    // Muradin Bronzebeard (at Light's Hammer)
    SAY_MURADIN_INTRO_1             = 13,
    SAY_MURADIN_INTRO_2             = 14,
    SAY_MURADIN_INTRO_3             = 15,

    // Deathbound Ward
    SAY_TRAP_ACTIVATE               = 0,

    // Rotting Frost Giant
    EMOTE_DEATH_PLAGUE_WARNING      = 0,

    // Sister Svalna
    SAY_SVALNA_KILL_CAPTAIN         = 1, // happens when she kills a captain
    SAY_SVALNA_KILL                 = 4,
    SAY_SVALNA_CAPTAIN_DEATH        = 5, // happens when a captain resurrected by her dies
    SAY_SVALNA_DEATH                = 6,
    EMOTE_SVALNA_IMPALE             = 7,
    EMOTE_SVALNA_BROKEN_SHIELD      = 8,

    SAY_CROK_INTRO_1                = 0, // Ready your arms, my Argent Brothers. The Vrykul will protect the Frost Queen with their lives.
    SAY_ARNATH_INTRO_2              = 5, // Even dying here beats spending another day collecting reagents for that madman, Finklestein.
    SAY_CROK_INTRO_3                = 1, // Enough idle banter! Our champions have arrived - support them as we push our way through the hall!
    SAY_SVALNA_EVENT_START          = 0, // You may have once fought beside me, Crok, but now you are nothing more than a traitor. Come, your second death approaches!
    SAY_CROK_COMBAT_WP_0            = 2, // Draw them back to us, and we'll assist you.
    SAY_CROK_COMBAT_WP_1            = 3, // Quickly, push on!
    SAY_CROK_FINAL_WP               = 4, // Her reinforcements will arrive shortly, we must bring her down quickly!
    SAY_SVALNA_RESURRECT_CAPTAINS   = 2, // Foolish Crok. You brought my reinforcements with you. Arise, Argent Champions, and serve the Lich King in death!
    SAY_CROK_COMBAT_SVALNA          = 5, // I'll draw her attacks. Return our brothers to their graves, then help me bring her down!
    SAY_SVALNA_AGGRO                = 3, // Come, Scourgebane. I'll show the master which of us is truly worthy of the title of "Champion"!
    SAY_CAPTAIN_DEATH               = 0,
    SAY_CAPTAIN_RESURRECTED         = 1,
    SAY_CAPTAIN_KILL                = 2,
    SAY_CAPTAIN_SECOND_DEATH        = 3,
    SAY_CAPTAIN_SURVIVE_TALK        = 4,
    SAY_CROK_WEAKENING_GAUNTLET     = 6,
    SAY_CROK_WEAKENING_SVALNA       = 7,
    SAY_CROK_DEATH                  = 8,
};

enum Spells
{
    // Rotting Frost Giant
    SPELL_DEATH_PLAGUE              = 72879,
    SPELL_DEATH_PLAGUE_AURA         = 72865,
    SPELL_RECENTLY_INFECTED         = 72884,
    SPELL_DEATH_PLAGUE_KILL         = 72867,
    SPELL_STOMP                     = 64639,
    SPELL_STOMP_H                   = 64652,
    SPELL_ARCTIC_BREATH             = 72848,

    // Frost Freeze Trap
    SPELL_COLDFLAME_JETS            = 70460,

    // Alchemist Adrianna
    SPELL_HARVEST_BLIGHT_SPECIMEN   = 72155,
    SPELL_HARVEST_BLIGHT_SPECIMEN25 = 72162,

    // Crok Scourgebane
    SPELL_ICEBOUND_ARMOR            = 70714,
    SPELL_SCOURGE_STRIKE            = 71488,
    SPELL_DEATH_STRIKE              = 71489,

    // Sister Svalna
    SPELL_CARESS_OF_DEATH           = 70078,
    SPELL_IMPALING_SPEAR_KILL       = 70196,
    SPELL_REVIVE_CHAMPION           = 70053,
    SPELL_UNDEATH                   = 70089,
    SPELL_IMPALING_SPEAR            = 71443,
    SPELL_AETHER_SHIELD             = 71463,
    SPELL_HURL_SPEAR                = 71466,
    SPELL_DIVINE_SURGE              = 71465,

    // Captain Arnath
    SPELL_DOMINATE_MIND             = 14515,
    SPELL_FLASH_HEAL_NORMAL         = 71595,
    SPELL_POWER_WORD_SHIELD_NORMAL  = 71548,
    SPELL_SMITE_NORMAL              = 71546,
    SPELL_FLASH_HEAL_UNDEAD         = 71782,
    SPELL_POWER_WORD_SHIELD_UNDEAD  = 71780,
    SPELL_SMITE_UNDEAD              = 71778,

    // Captain Brandon
    SPELL_CRUSADER_STRIKE           = 71549,
    SPELL_DIVINE_SHIELD             = 71550,
    SPELL_JUDGEMENT_OF_COMMAND      = 71551,
    SPELL_HAMMER_OF_BETRAYAL        = 71784,

    // Captain Grondel
    SPELL_CHARGE                    = 71553,
    SPELL_MORTAL_STRIKE             = 71552,
    SPELL_SUNDER_ARMOR              = 71554,
    SPELL_CONFLAGRATION             = 71785,

    // Captain Rupert
    SPELL_FEL_IRON_BOMB_NORMAL      = 71592,
    SPELL_MACHINE_GUN_NORMAL        = 71594,
    SPELL_ROCKET_LAUNCH_NORMAL      = 71590,
    SPELL_FEL_IRON_BOMB_UNDEAD      = 71787,
    SPELL_MACHINE_GUN_UNDEAD        = 71788,
    SPELL_ROCKET_LAUNCH_UNDEAD      = 71786,

    // Invisible Stalker (Float, Uninteractible, LargeAOI)
    SPELL_SOUL_MISSILE              = 72585,
};

// Helper defines
// Captain Arnath
#define SPELL_FLASH_HEAL        (IsUndead ? SPELL_FLASH_HEAL_UNDEAD : SPELL_FLASH_HEAL_NORMAL)
#define SPELL_POWER_WORD_SHIELD (IsUndead ? SPELL_POWER_WORD_SHIELD_UNDEAD : SPELL_POWER_WORD_SHIELD_NORMAL)
#define SPELL_SMITE             (IsUndead ? SPELL_SMITE_UNDEAD : SPELL_SMITE_NORMAL)

// Captain Rupert
#define SPELL_FEL_IRON_BOMB     (IsUndead ? SPELL_FEL_IRON_BOMB_UNDEAD : SPELL_FEL_IRON_BOMB_NORMAL)
#define SPELL_MACHINE_GUN       (IsUndead ? SPELL_MACHINE_GUN_UNDEAD : SPELL_MACHINE_GUN_NORMAL)
#define SPELL_ROCKET_LAUNCH     (IsUndead ? SPELL_ROCKET_LAUNCH_UNDEAD : SPELL_ROCKET_LAUNCH_NORMAL)

enum EventTypes
{
    // Highlord Tirion Fordring (at Light's Hammer)
    // The Lich King (at Light's Hammer)
    // Highlord Bolvar Fordragon (at Light's Hammer)
    // High Overlord Saurfang (at Light's Hammer)
    // Muradin Bronzebeard (at Light's Hammer)
    EVENT_TIRION_INTRO_2                = 1,
    EVENT_TIRION_INTRO_3                = 2,
    EVENT_TIRION_INTRO_4                = 3,
    EVENT_TIRION_INTRO_5                = 4,
    EVENT_LK_INTRO_1                    = 5,
    EVENT_TIRION_INTRO_6                = 6,
    EVENT_LK_INTRO_2                    = 7,
    EVENT_LK_INTRO_3                    = 8,
    EVENT_LK_INTRO_4                    = 9,
    EVENT_BOLVAR_INTRO_1                = 10,
    EVENT_LK_INTRO_5                    = 11,
    EVENT_SAURFANG_INTRO_1              = 12,
    EVENT_TIRION_INTRO_H_7              = 13,
    EVENT_SAURFANG_INTRO_2              = 14,
    EVENT_SAURFANG_INTRO_3              = 15,
    EVENT_SAURFANG_INTRO_4              = 16,
    EVENT_SAURFANG_RUN                  = 17,
    EVENT_MURADIN_INTRO_1               = 18,
    EVENT_MURADIN_INTRO_2               = 19,
    EVENT_MURADIN_INTRO_3               = 20,
    EVENT_TIRION_INTRO_A_7              = 21,
    EVENT_MURADIN_INTRO_4               = 22,
    EVENT_MURADIN_INTRO_5               = 23,
    EVENT_MURADIN_RUN                   = 24,

    // Rotting Frost Giant
    EVENT_DEATH_PLAGUE                  = 25,
    EVENT_STOMP                         = 26,
    EVENT_ARCTIC_BREATH                 = 27,

    // Frost Freeze Trap
    EVENT_ACTIVATE_TRAP                 = 28,

    // Crok Scourgebane
    EVENT_SCOURGE_STRIKE                = 29,
    EVENT_DEATH_STRIKE                  = 30,
    EVENT_HEALTH_CHECK                  = 31,
    EVENT_CROK_INTRO_3                  = 32,
    EVENT_START_PATHING                 = 33,

    // Sister Svalna
    EVENT_ARNATH_INTRO_2                = 34,
    EVENT_SVALNA_START                  = 35,
    EVENT_SVALNA_RESURRECT              = 36,
    EVENT_SVALNA_COMBAT                 = 37,
    EVENT_IMPALING_SPEAR                = 38,
    EVENT_AETHER_SHIELD                 = 39,

    // Captain Arnath
    EVENT_ARNATH_FLASH_HEAL             = 40,
    EVENT_ARNATH_PW_SHIELD              = 41,
    EVENT_ARNATH_SMITE                  = 42,
    EVENT_ARNATH_DOMINATE_MIND          = 43,

    // Captain Brandon
    EVENT_BRANDON_CRUSADER_STRIKE       = 44,
    EVENT_BRANDON_DIVINE_SHIELD         = 45,
    EVENT_BRANDON_JUDGEMENT_OF_COMMAND  = 46,
    EVENT_BRANDON_HAMMER_OF_BETRAYAL    = 47,

    // Captain Grondel
    EVENT_GRONDEL_CHARGE_CHECK          = 48,
    EVENT_GRONDEL_MORTAL_STRIKE         = 49,
    EVENT_GRONDEL_SUNDER_ARMOR          = 50,
    EVENT_GRONDEL_CONFLAGRATION         = 51,

    // Captain Rupert
    EVENT_RUPERT_FEL_IRON_BOMB          = 52,
    EVENT_RUPERT_MACHINE_GUN            = 53,
    EVENT_RUPERT_ROCKET_LAUNCH          = 54,

    // Invisible Stalker (Float, Uninteractible, LargeAOI)
    EVENT_SOUL_MISSILE                  = 55,

    // Sindragosas Ward
    EVENT_SUB_WAVE_1                    = 56,
    EVENT_SUB_WAVE_2                    = 57,
    EVENT_UPDATE_CHECK                  = 58,

    // Deathbound Ward
    EVENT_DISRUPTING_SHOUT              = 59,
};

enum DataTypesICC
{
    DATA_DAMNED_KILLS       = 1,
};

enum Actions
{
    // Sister Svalna
    ACTION_KILL_CAPTAIN         = 1,
    ACTION_START_GAUNTLET       = 2,
    ACTION_RESURRECT_CAPTAINS   = 3,
    ACTION_CAPTAIN_DIES         = 4,
    ACTION_RESET_EVENT          = 5,
    ACTION_START_TRAP           = 6,
};

enum EventIds
{
    EVENT_AWAKEN_WARD_1 = 22900,
    EVENT_AWAKEN_WARD_2 = 22907,
    EVENT_AWAKEN_WARD_3 = 22908,
    EVENT_AWAKEN_WARD_4 = 22909,
};

enum MovementPoints
{
    POINT_LAND  = 1,
};

Position const SvalnaLandPos = { 4356.71f, 2484.33f, 358.5f, 1.571f };

#define MAX_NERUBAR_MAIN		4

Position const SindragosaGauntletSpawnWebreaver[MAX_NERUBAR_MAIN] =
{
    { 4137.93f, 2505.52f, 211.033f, 0.0f },
	{ 4200.92f, 2527.18f, 211.033f, 0.0f },
	{ 4222.01f, 2464.93f, 211.033f, 0.0f },
	{ 4161.86f, 2441.94f, 211.033f, 0 }
};

Position const SindragosaGauntletSpawnChampion[MAX_NERUBAR_MAIN] =
{
    { 4160.64f, 2528.13f, 211.033f, 0.0f },
	{ 4222.23f, 2503.56f, 211.033f, 0.0f },
	{ 4201.55f, 2441.03f, 211.033f, 0.0f },
	{ 4138.78f, 2463.95f, 211.033f, 0 }
};

#define MAX_FROSTWARDEN		3

Position const SindragosaGauntletSpawnFrostWardenRight[MAX_FROSTWARDEN] =
{
	{ 4174.81f, 2546.88f, 211.033f, 0.0f },
    { 4180.81f, 2546.88f, 211.033f, 0.0f },
	{ 4186.81f, 2546.88f, 211.033f, 0 }
};

Position const SindragosaGauntletSpawnFrostWardenLeft[MAX_FROSTWARDEN] =
{
	{ 4175.29f, 2420.38f, 211.033f, 0.0f },
    { 4181.29f, 2420.38f, 211.033f, 0.0f },
	{ 4187.29f, 2420.38f, 211.033f, 0 }
};

#define MAX_NERUBAR_OFF		12

Position const SindragosaGauntletSpawnBroodling[MAX_NERUBAR_OFF] =
{
    { 4130.71f, 2484.10f, 211.033f, 0.0f },
    { 4137.93f, 2505.52f, 211.033f, 0.0f },
    { 4160.64f, 2528.13f, 211.033f, 0.0f },
    { 4180.81f, 2533.88f, 211.033f, 0.0f },
    { 4200.92f, 2527.18f, 211.033f, 0.0f },
    { 4222.23f, 2503.56f, 211.033f, 0.0f },
    { 4229.40f, 2484.63f, 211.033f, 0.0f },
    { 4222.01f, 2464.93f, 211.033f, 0.0f },
    { 4201.55f, 2441.03f, 211.033f, 0.0f },
    { 4181.29f, 2433.38f, 211.033f, 0.0f },
    { 4161.86f, 2441.94f, 211.033f, 0.0f },
    { 4138.78f, 2463.95f, 211.033f, 0.0f },
};

class FrostwingVrykulSearcher
{
    public:
        FrostwingVrykulSearcher(Creature const* source, float range) : _source(source), _range(range) { }

        bool operator()(Unit* unit)
        {
            if (!unit->IsAlive())
                return false;

            switch (unit->GetEntry())
            {
                case NPC_YMIRJAR_BATTLE_MAIDEN:
                case NPC_YMIRJAR_DEATHBRINGER:
                case NPC_YMIRJAR_FROSTBINDER:
                case NPC_YMIRJAR_HUNTRESS:
                case NPC_YMIRJAR_WARLORD:
                    break;
                default:
                    return false;
            }

            if (!unit->IsWithinDist(_source, _range, false))
                return false;

            return true;
        }

    private:
        Creature const* _source;
        float _range;
};

class FrostwingGauntletRespawner
{
    public:
        void operator()(Creature* creature)
        {
            switch (creature->GetOriginalEntry())
            {
                case NPC_YMIRJAR_BATTLE_MAIDEN:
                case NPC_YMIRJAR_DEATHBRINGER:
                case NPC_YMIRJAR_FROSTBINDER:
                case NPC_YMIRJAR_HUNTRESS:
                case NPC_YMIRJAR_WARLORD:
                    break;
                case NPC_CROK_SCOURGEBANE:
                case NPC_CAPTAIN_ARNATH:
                case NPC_CAPTAIN_BRANDON:
                case NPC_CAPTAIN_GRONDEL:
                case NPC_CAPTAIN_RUPERT:
                    creature->AI()->DoAction(ACTION_RESET_EVENT);
                    break;
                case NPC_SISTER_SVALNA:
                    creature->AI()->DoAction(ACTION_RESET_EVENT);
                    // return, this creature is never dead if event is reset
                    return;
                default:
                    return;
            }

            uint32 corpseDelay = creature->GetCorpseDelay();
            uint32 respawnDelay = creature->GetRespawnDelay();
            creature->SetCorpseDelay(1);
            creature->SetRespawnDelay(2);

            if (CreatureData const* data = creature->GetCreatureData())
                creature->SetPosition(data->posX, data->posY, data->posZ, data->orientation);
            creature->DespawnOrUnsummon();

            creature->SetCorpseDelay(corpseDelay);
            creature->SetRespawnDelay(respawnDelay);
        }
};

class CaptainSurviveTalk : public BasicEvent
{
    public:
        explicit CaptainSurviveTalk(Creature const& owner) : _owner(owner) { }

        bool Execute(uint64 /*currTime*/, uint32 /*diff*/)
        {
            _owner.AI()->Talk(SAY_CAPTAIN_SURVIVE_TALK);
            return true;
        }

    private:
        Creature const& _owner;
};

// at Light's Hammer
class npc_highlord_tirion_fordring_lh : public CreatureScript
{
    public:
        npc_highlord_tirion_fordring_lh() : CreatureScript("npc_highlord_tirion_fordring_lh") { }

        struct npc_highlord_tirion_fordringAI : public ScriptedAI
        {
            npc_highlord_tirion_fordringAI(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript())
            {
                Initialize();
            }

            void Initialize()
            {
                _theLichKing = 0;
                _bolvarFordragon = 0;
                _factionNPC = 0;
                _damnedKills = 0;
            }

            void Reset() OVERRIDE
            {
                _events.Reset();
                Initialize();
            }

            // IMPORTANT NOTE: This is triggered from per-GUID scripts
            // of The Damned SAI
            void SetData(uint32 type, uint32 data) OVERRIDE
            {
                if (type == DATA_DAMNED_KILLS && data == 1)
                {
                    if (++_damnedKills == 2)
                    {
                        if (Creature* theLichKing = me->FindNearestCreature(NPC_THE_LICH_KING_LH, 150.0f))
                        {
                            if (Creature* bolvarFordragon = me->FindNearestCreature(NPC_HIGHLORD_BOLVAR_FORDRAGON_LH, 150.0f))
                            {
                                if (Creature* factionNPC = me->FindNearestCreature(_instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_SE_HIGH_OVERLORD_SAURFANG : NPC_SE_MURADIN_BRONZEBEARD, 50.0f))
                                {
                                    me->setActive(true);
                                    _theLichKing = theLichKing->GetGUID();
                                    theLichKing->setActive(true);
                                    _bolvarFordragon = bolvarFordragon->GetGUID();
                                    bolvarFordragon->setActive(true);
                                    _factionNPC = factionNPC->GetGUID();
                                    factionNPC->setActive(true);
                                }
                            }
                        }

                        if (!_bolvarFordragon || !_theLichKing || !_factionNPC)
                            return;

                        Talk(SAY_TIRION_INTRO_1);
                        _events.ScheduleEvent(EVENT_TIRION_INTRO_2, 4000);
                        _events.ScheduleEvent(EVENT_TIRION_INTRO_3, 14000);
                        _events.ScheduleEvent(EVENT_TIRION_INTRO_4, 18000);
                        _events.ScheduleEvent(EVENT_TIRION_INTRO_5, 31000);
                        _events.ScheduleEvent(EVENT_LK_INTRO_1, 35000);
                        _events.ScheduleEvent(EVENT_TIRION_INTRO_6, 51000);
                        _events.ScheduleEvent(EVENT_LK_INTRO_2, 58000);
                        _events.ScheduleEvent(EVENT_LK_INTRO_3, 74000);
                        _events.ScheduleEvent(EVENT_LK_INTRO_4, 86000);
                        _events.ScheduleEvent(EVENT_BOLVAR_INTRO_1, 100000);
                        _events.ScheduleEvent(EVENT_LK_INTRO_5, 108000);

                        if (_instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        {
                            _events.ScheduleEvent(EVENT_SAURFANG_INTRO_1, 120000);
                            _events.ScheduleEvent(EVENT_TIRION_INTRO_H_7, 129000);
                            _events.ScheduleEvent(EVENT_SAURFANG_INTRO_2, 139000);
                            _events.ScheduleEvent(EVENT_SAURFANG_INTRO_3, 150000);
                            _events.ScheduleEvent(EVENT_SAURFANG_INTRO_4, 162000);
                            _events.ScheduleEvent(EVENT_SAURFANG_RUN, 170000);
                        }
                        else
                        {
                            _events.ScheduleEvent(EVENT_MURADIN_INTRO_1, 120000);
                            _events.ScheduleEvent(EVENT_MURADIN_INTRO_2, 124000);
                            _events.ScheduleEvent(EVENT_MURADIN_INTRO_3, 127000);
                            _events.ScheduleEvent(EVENT_TIRION_INTRO_A_7, 136000);
                            _events.ScheduleEvent(EVENT_MURADIN_INTRO_4, 144000);
                            _events.ScheduleEvent(EVENT_MURADIN_INTRO_5, 151000);
                            _events.ScheduleEvent(EVENT_MURADIN_RUN, 157000);
                        }
                    }
                }
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (_damnedKills != 2)
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TIRION_INTRO_2:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                            break;
                        case EVENT_TIRION_INTRO_3:
                            Talk(SAY_TIRION_INTRO_2);
                            break;
                        case EVENT_TIRION_INTRO_4:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                            break;
                        case EVENT_TIRION_INTRO_5:
                            Talk(SAY_TIRION_INTRO_3);
                            break;
                        case EVENT_LK_INTRO_1:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_POINT_NO_SHEATHE);
                            if (Creature* theLichKing = ObjectAccessor::GetCreature(*me, _theLichKing))
                                theLichKing->AI()->Talk(SAY_LK_INTRO_1);
                            break;
                        case EVENT_TIRION_INTRO_6:
                            Talk(SAY_TIRION_INTRO_4);
                            break;
                        case EVENT_LK_INTRO_2:
                            if (Creature* theLichKing = ObjectAccessor::GetCreature(*me, _theLichKing))
                                theLichKing->AI()->Talk(SAY_LK_INTRO_2);
                            break;
                        case EVENT_LK_INTRO_3:
                            if (Creature* theLichKing = ObjectAccessor::GetCreature(*me, _theLichKing))
                                theLichKing->AI()->Talk(SAY_LK_INTRO_3);
                            break;
                        case EVENT_LK_INTRO_4:
                            if (Creature* theLichKing = ObjectAccessor::GetCreature(*me, _theLichKing))
                                theLichKing->AI()->Talk(SAY_LK_INTRO_4);
                            break;
                        case EVENT_BOLVAR_INTRO_1:
                            if (Creature* bolvarFordragon = ObjectAccessor::GetCreature(*me, _bolvarFordragon))
                            {
                                bolvarFordragon->AI()->Talk(SAY_BOLVAR_INTRO_1);
                                bolvarFordragon->setActive(false);
                            }
                            break;
                        case EVENT_LK_INTRO_5:
                            if (Creature* theLichKing = ObjectAccessor::GetCreature(*me, _theLichKing))
                            {
                                theLichKing->AI()->Talk(SAY_LK_INTRO_5);
                                theLichKing->setActive(false);
                            }
                            break;
                        case EVENT_SAURFANG_INTRO_1:
                            if (Creature* saurfang = ObjectAccessor::GetCreature(*me, _factionNPC))
                                saurfang->AI()->Talk(SAY_SAURFANG_INTRO_1);
                            break;
                        case EVENT_TIRION_INTRO_H_7:
                            Talk(SAY_TIRION_INTRO_H_5);
                            break;
                        case EVENT_SAURFANG_INTRO_2:
                            if (Creature* saurfang = ObjectAccessor::GetCreature(*me, _factionNPC))
                                saurfang->AI()->Talk(SAY_SAURFANG_INTRO_2);
                            break;
                        case EVENT_SAURFANG_INTRO_3:
                            if (Creature* saurfang = ObjectAccessor::GetCreature(*me, _factionNPC))
                                saurfang->AI()->Talk(SAY_SAURFANG_INTRO_3);
                            break;
                        case EVENT_SAURFANG_INTRO_4:
                            if (Creature* saurfang = ObjectAccessor::GetCreature(*me, _factionNPC))
                                saurfang->AI()->Talk(SAY_SAURFANG_INTRO_4);
                            break;
                        case EVENT_MURADIN_RUN:
                        case EVENT_SAURFANG_RUN:
                            if (Creature* factionNPC = ObjectAccessor::GetCreature(*me, _factionNPC))
                                factionNPC->GetMotionMaster()->MovePath(factionNPC->GetDBTableGUIDLow()*10, false);
                            me->setActive(false);
                            _damnedKills = 3;
                            break;
                        case EVENT_MURADIN_INTRO_1:
                            if (Creature* muradin = ObjectAccessor::GetCreature(*me, _factionNPC))
                                muradin->AI()->Talk(SAY_MURADIN_INTRO_1);
                            break;
                        case EVENT_MURADIN_INTRO_2:
                            if (Creature* muradin = ObjectAccessor::GetCreature(*me, _factionNPC))
                                muradin->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                            break;
                        case EVENT_MURADIN_INTRO_3:
                            if (Creature* muradin = ObjectAccessor::GetCreature(*me, _factionNPC))
                                muradin->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                            break;
                        case EVENT_TIRION_INTRO_A_7:
                            Talk(SAY_TIRION_INTRO_A_5);
                            break;
                        case EVENT_MURADIN_INTRO_4:
                            if (Creature* muradin = ObjectAccessor::GetCreature(*me, _factionNPC))
                                muradin->AI()->Talk(SAY_MURADIN_INTRO_2);
                            break;
                        case EVENT_MURADIN_INTRO_5:
                            if (Creature* muradin = ObjectAccessor::GetCreature(*me, _factionNPC))
                                muradin->AI()->Talk(SAY_MURADIN_INTRO_3);
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap _events;
            InstanceScript* const _instance;
            uint64 _theLichKing;
            uint64 _bolvarFordragon;
            uint64 _factionNPC;
            uint16 _damnedKills;
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<npc_highlord_tirion_fordringAI>(creature);
        }
};

class npc_rotting_frost_giant : public CreatureScript
{
    public:
        npc_rotting_frost_giant() : CreatureScript("npc_rotting_frost_giant") { }

        struct npc_rotting_frost_giantAI : public ScriptedAI
        {
            npc_rotting_frost_giantAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset() OVERRIDE
            {
                _events.Reset();
                _events.ScheduleEvent(EVENT_DEATH_PLAGUE, 15000);
                _events.ScheduleEvent(EVENT_STOMP, urand(5000, 8000));
                _events.ScheduleEvent(EVENT_ARCTIC_BREATH, urand(10000, 15000));
            }

            void JustDied(Unit* /*killer*/) OVERRIDE
            {
                _events.Reset();
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DEATH_PLAGUE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                            {
                                Talk(EMOTE_DEATH_PLAGUE_WARNING, target);
                                DoCast(target, SPELL_DEATH_PLAGUE);
                            }
                            _events.ScheduleEvent(EVENT_DEATH_PLAGUE, 15000);
                            break;
                        case EVENT_STOMP:
                            DoCast(me, RAID_MODE(SPELL_STOMP, SPELL_STOMP_H, SPELL_STOMP, SPELL_STOMP_H), false);
                            _events.ScheduleEvent(EVENT_STOMP, urand(15000, 18000));
                            break;
                        case EVENT_ARCTIC_BREATH:
                            DoCastVictim(SPELL_ARCTIC_BREATH);
                            _events.ScheduleEvent(EVENT_ARCTIC_BREATH, urand(26000, 33000));
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<npc_rotting_frost_giantAI>(creature);
        }
};

class npc_frost_freeze_trap : public CreatureScript
{
    public:
        npc_frost_freeze_trap() : CreatureScript("npc_frost_freeze_trap") { }

        struct npc_frost_freeze_trapAI: public ScriptedAI
        {
            npc_frost_freeze_trapAI(Creature* creature) : ScriptedAI(creature)
            {
                SetCombatMovement(false);
            }

            void DoAction(int32 action) OVERRIDE
            {
                switch (action)
                {
                    case 1000:
                    case 11000:
                        _events.ScheduleEvent(EVENT_ACTIVATE_TRAP, uint32(action));
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                _events.Update(diff);

                if (_events.ExecuteEvent() == EVENT_ACTIVATE_TRAP)
                {
                    DoCast(me, SPELL_COLDFLAME_JETS);
                    _events.ScheduleEvent(EVENT_ACTIVATE_TRAP, 22000);
                }
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<npc_frost_freeze_trapAI>(creature);
        }
};

class npc_alchemist_adrianna : public CreatureScript
{
    public:
        npc_alchemist_adrianna() : CreatureScript("npc_alchemist_adrianna") { }

        bool OnGossipHello(Player* player, Creature* creature) OVERRIDE
        {
            if (!creature->FindCurrentSpellBySpellId(SPELL_HARVEST_BLIGHT_SPECIMEN) && !creature->FindCurrentSpellBySpellId(SPELL_HARVEST_BLIGHT_SPECIMEN25))
                if (player->HasAura(SPELL_ORANGE_BLIGHT_RESIDUE) && player->HasAura(SPELL_GREEN_BLIGHT_RESIDUE))
                    creature->CastSpell(creature, SPELL_HARVEST_BLIGHT_SPECIMEN, false);
            return false;
        }
};

class boss_sister_svalna : public CreatureScript
{
    public:
        boss_sister_svalna() : CreatureScript("boss_sister_svalna") { }

        struct boss_sister_svalnaAI : public BossAI
        {
            boss_sister_svalnaAI(Creature* creature) : BossAI(creature, DATA_SISTER_SVALNA),
                _isEventInProgress(false)
            {
            }

            void InitializeAI() OVERRIDE
            {
                if (!me->isDead())
                    Reset();

                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() OVERRIDE
            {
                _Reset();
                me->SetReactState(REACT_DEFENSIVE);
                _isEventInProgress = false;
            }

            void JustDied(Unit* /*killer*/) OVERRIDE
            {
                _JustDied();
                Talk(SAY_SVALNA_DEATH);

                uint64 delay = 1;
                for (uint32 i = 0; i < 4; ++i)
                {
                    if (Creature* crusader = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_CAPTAIN_ARNATH + i)))
                    {
                        if (crusader->IsAlive() && crusader->GetEntry() == crusader->GetCreatureData()->id)
                        {
                            crusader->m_Events.AddEvent(new CaptainSurviveTalk(*crusader), crusader->m_Events.CalculateTime(delay));
                            delay += 6000;
                        }
                    }
                }
            }

            void EnterCombat(Unit* /*attacker*/) OVERRIDE
            {
                _EnterCombat();
                if (Creature* crok = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_CROK_SCOURGEBANE)))
                    crok->AI()->Talk(SAY_CROK_COMBAT_SVALNA);
                events.ScheduleEvent(EVENT_SVALNA_COMBAT, 1);
                events.ScheduleEvent(EVENT_IMPALING_SPEAR, urand(40000, 50000));
                events.ScheduleEvent(EVENT_AETHER_SHIELD, urand(100000, 110000));
                DoCast(SPELL_DIVINE_SURGE);
            }

            void KilledUnit(Unit* victim) OVERRIDE
            {
                switch (victim->GetTypeId())
                {
                    case TYPEID_PLAYER:
                        Talk(SAY_SVALNA_KILL);
                        break;
                    case TYPEID_UNIT:
                        switch (victim->GetEntry())
                        {
                            case NPC_CAPTAIN_ARNATH:
                            case NPC_CAPTAIN_BRANDON:
                            case NPC_CAPTAIN_GRONDEL:
                            case NPC_CAPTAIN_RUPERT:
                                Talk(SAY_SVALNA_KILL_CAPTAIN);
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }

            void JustReachedHome() OVERRIDE
            {
                _JustReachedHome();
                me->SetReactState(REACT_PASSIVE);
                me->SetDisableGravity(false);
                me->SetHover(false);
            }

            void DoAction(int32 action) OVERRIDE
            {
                switch (action)
                {
                    case ACTION_KILL_CAPTAIN:
                        me->CastCustomSpell(SPELL_CARESS_OF_DEATH, SPELLVALUE_MAX_TARGETS, 1, me, true);
                        break;
                    case ACTION_START_GAUNTLET:
                        me->setActive(true);
                        _isEventInProgress = true;
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE);
                        events.ScheduleEvent(EVENT_SVALNA_START, 25000);
                        break;
                    case ACTION_RESURRECT_CAPTAINS:
                        events.ScheduleEvent(EVENT_SVALNA_RESURRECT, 7000);
                        break;
                    case ACTION_CAPTAIN_DIES:
                        Talk(SAY_SVALNA_CAPTAIN_DEATH);
                        break;
                    case ACTION_RESET_EVENT:
                        me->setActive(false);
                        Reset();
                        break;
                    default:
                        break;
                }
            }

            void SpellHit(Unit* caster, SpellInfo const* spell) OVERRIDE
            {
                if (spell->Id == SPELL_HURL_SPEAR && me->HasAura(SPELL_AETHER_SHIELD))
                {
                    me->RemoveAurasDueToSpell(SPELL_AETHER_SHIELD);
                    Talk(EMOTE_SVALNA_BROKEN_SHIELD, caster);
                }
            }

            void MovementInform(uint32 type, uint32 id) OVERRIDE
            {
                if (type != EFFECT_MOTION_TYPE || id != POINT_LAND)
                    return;

                _isEventInProgress = false;
                me->setActive(false);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE);
                me->SetDisableGravity(false);
                me->SetHover(false);
                DoZoneInCombat(me, 150.0f);
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell) OVERRIDE
            {
                switch (spell->Id)
                {
                    case SPELL_IMPALING_SPEAR_KILL:
                        me->Kill(target);
                        break;
                    case SPELL_IMPALING_SPEAR:
                        if (TempSummon* summon = target->SummonCreature(NPC_IMPALING_SPEAR, *target))
                        {
                            Talk(EMOTE_SVALNA_IMPALE, target);
                            summon->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, target, false);
                            summon->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_UNK1 | UNIT_FLAG2_ALLOW_ENEMY_INTERACT);
                        }
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (!UpdateVictim() && !_isEventInProgress)
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SVALNA_START:
                            Talk(SAY_SVALNA_EVENT_START);
                            break;
                        case EVENT_SVALNA_RESURRECT:
                            Talk(SAY_SVALNA_RESURRECT_CAPTAINS);
                            me->CastSpell(me, SPELL_REVIVE_CHAMPION, false);
                            break;
                        case EVENT_SVALNA_COMBAT:
                            me->SetReactState(REACT_DEFENSIVE);
                            Talk(SAY_SVALNA_AGGRO);
                            break;
                        case EVENT_IMPALING_SPEAR:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true, -SPELL_IMPALING_SPEAR))
                            {
                                DoCast(me, SPELL_AETHER_SHIELD);
                                DoCast(target, SPELL_IMPALING_SPEAR);
                            }
                            events.ScheduleEvent(EVENT_IMPALING_SPEAR, urand(20000, 25000));
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            bool _isEventInProgress;
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<boss_sister_svalnaAI>(creature);
        }
};

class npc_crok_scourgebane : public CreatureScript
{
    public:
        npc_crok_scourgebane() : CreatureScript("npc_crok_scourgebane") { }

        struct npc_crok_scourgebaneAI : public npc_escortAI
        {
            npc_crok_scourgebaneAI(Creature* creature) : npc_escortAI(creature),
                _instance(creature->GetInstanceScript()), _respawnTime(creature->GetRespawnDelay()),
                _corpseDelay(creature->GetCorpseDelay())
            {
                Initialize();
                SetDespawnAtEnd(false);
                SetDespawnAtFar(false);
                _isEventActive = false;
                _isEventDone = _instance->GetBossState(DATA_SISTER_SVALNA) == DONE;
                _currentWPid = 0;
            }

            void Initialize()
            {
                _isEventActive = false;
                _didUnderTenPercentText = false;
                _wipeCheckTimer = 1000;
                _aliveTrash.clear();
                _currentWPid = 0;
            }

            void Reset() OVERRIDE
            {
                _events.Reset();
                _events.ScheduleEvent(EVENT_SCOURGE_STRIKE, urand(7500, 12500));
                _events.ScheduleEvent(EVENT_DEATH_STRIKE, urand(25000, 30000));
                me->SetReactState(REACT_DEFENSIVE);

                if (_instance)
                    _isEventDone = _instance->GetBossState(DATA_SISTER_SVALNA) == DONE;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                Initialize();

            }

            void DoAction(int32 action) OVERRIDE
            {
                if (action == ACTION_START_GAUNTLET)
                {
                    if (_isEventDone || !me->IsAlive())
                        return;

                    _isEventActive = true;
                    _isEventDone = true;
                    // Load Grid with Sister Svalna
                    me->GetMap()->LoadGrid(4356.71f, 2484.33f);
                    if (Creature* svalna = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_SISTER_SVALNA)))
                        svalna->AI()->DoAction(ACTION_START_GAUNTLET);
                    Talk(SAY_CROK_INTRO_1);
                    _events.ScheduleEvent(EVENT_ARNATH_INTRO_2, 7000);
                    _events.ScheduleEvent(EVENT_CROK_INTRO_3, 14000);
                    _events.ScheduleEvent(EVENT_START_PATHING, 37000);
                    me->setActive(true);
                    for (uint32 i = 0; i < 4; ++i)
                        if (Creature* crusader = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_CAPTAIN_ARNATH + i)))
                            crusader->AI()->DoAction(ACTION_START_GAUNTLET);
                }
                else if (action == ACTION_RESET_EVENT)
                {
                    _isEventActive = false;
                    _isEventDone = _instance->GetBossState(DATA_SISTER_SVALNA) == DONE;
                    me->setActive(false);
                    _aliveTrash.clear();
                    _currentWPid = 0;
                }
            }

            void SetGUID(uint64 guid, int32 type/* = 0*/) OVERRIDE
            {
                if (type == ACTION_VRYKUL_DEATH)
                {
                    _aliveTrash.erase(guid);
                    if (_aliveTrash.empty())
                    {
                        SetEscortPaused(false);
                        if (_currentWPid == 4 && _isEventActive)
                        {
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            _isEventActive = false;
                            me->setActive(false);
                            Talk(SAY_CROK_FINAL_WP);
                            if (Creature* svalna = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_SISTER_SVALNA)))
                                svalna->AI()->DoAction(ACTION_RESURRECT_CAPTAINS);
                        }
                    }
                }
            }

            void WaypointReached(uint32 waypointId) OVERRIDE
            {
                switch (waypointId)
                {
                    // pause pathing until trash pack is cleared
                    case 0:
                        Talk(SAY_CROK_COMBAT_WP_0);
                        if (!_aliveTrash.empty())
                            SetEscortPaused(true);
                        break;
                    case 1:
                        Talk(SAY_CROK_COMBAT_WP_1);
                        if (!_aliveTrash.empty())
                            SetEscortPaused(true);
                        break;
                    case 4:
                        if (_aliveTrash.empty() && _isEventActive)
                        {
                            _isEventActive = false;
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->setActive(false);
                            Talk(SAY_CROK_FINAL_WP);
                            if (Creature* svalna = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_SISTER_SVALNA)))
                                svalna->AI()->DoAction(ACTION_RESURRECT_CAPTAINS);
                        }
                        break;
                    default:
                        break;
                }
            }

            void WaypointStart(uint32 waypointId) OVERRIDE
            {
                _currentWPid = waypointId;
                switch (waypointId)
                {
                    case 0:
                    case 1:
                    case 4:
                    {
                        // get spawns by home position
                        float minY = 2600.0f;
                        float maxY = 2650.0f;
                        if (waypointId == 1)
                        {
                            minY -= 50.0f;
                            maxY -= 50.0f;
                            // at waypoints 1 and 2 she kills one captain
                            if (Creature* svalna = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_SISTER_SVALNA)))
                                svalna->AI()->DoAction(ACTION_KILL_CAPTAIN);
                        }
                        else if (waypointId == 4)
                        {
                            minY -= 100.0f;
                            maxY -= 100.0f;
                        }

                        // get all nearby vrykul
                        std::list<Creature*> temp;
                        FrostwingVrykulSearcher check(me, 80.0f);
                        Trinity::CreatureListSearcher<FrostwingVrykulSearcher> searcher(me, temp, check);
                        me->VisitNearbyGridObject(80.0f, searcher);

                        _aliveTrash.clear();
                        for (std::list<Creature*>::iterator itr = temp.begin(); itr != temp.end(); ++itr)
                            if ((*itr)->GetHomePosition().GetPositionY() < maxY && (*itr)->GetHomePosition().GetPositionY() > minY)
                                _aliveTrash.insert((*itr)->GetGUID());
                        break;
                    }
                    // at waypoints 1 and 2 she kills one captain
                    case 2:
                        if (Creature* svalna = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_SISTER_SVALNA)))
                            svalna->AI()->DoAction(ACTION_KILL_CAPTAIN);
                        break;
                    default:
                        break;
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) OVERRIDE
            {
                // check wipe
                if (!_wipeCheckTimer)
                {
                    _wipeCheckTimer = 1000;
                    Player* player = NULL;
                    Trinity::AnyPlayerInObjectRangeCheck check(me, 60.0f);
                    Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, player, check);
                    me->VisitNearbyWorldObject(60.0f, searcher);
                    // wipe
                    if (!player)
                    {
                        damage *= 100;
                        if (damage >= me->GetHealth())
                        {
                            FrostwingGauntletRespawner respawner;
                            Trinity::CreatureWorker<FrostwingGauntletRespawner> worker(me, respawner);
                            me->VisitNearbyGridObject(333.0f, worker);
                            Talk(SAY_CROK_DEATH);
                        }
                        return;
                    }
                }

                if (HealthBelowPct(10) || damage >= me->GetHealth())
                {
                    if (!_didUnderTenPercentText)
                    {
                        _didUnderTenPercentText = true;
                        if (_isEventActive)
                            Talk(SAY_CROK_WEAKENING_GAUNTLET);
                        else
                            Talk(SAY_CROK_WEAKENING_SVALNA);
                    }

                    damage = 0;
                    DoCast(me, SPELL_ICEBOUND_ARMOR);
                    _events.ScheduleEvent(EVENT_HEALTH_CHECK, 1000);
                }
            }

            void UpdateEscortAI(uint32 const diff) OVERRIDE
            {
                if (_wipeCheckTimer <= diff)
                    _wipeCheckTimer = 0;
                else
                    _wipeCheckTimer -= diff;

                if (!UpdateVictim() && !_isEventActive)
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ARNATH_INTRO_2:
                            if (Creature* arnath = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_CAPTAIN_ARNATH)))
                                arnath->AI()->Talk(SAY_ARNATH_INTRO_2);
                            break;
                        case EVENT_CROK_INTRO_3:
                            Talk(SAY_CROK_INTRO_3);
                            break;
                        case EVENT_START_PATHING:
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            Start(true, true);
                            break;
                        case EVENT_SCOURGE_STRIKE:
                            DoCastVictim(SPELL_SCOURGE_STRIKE);
                            _events.ScheduleEvent(EVENT_SCOURGE_STRIKE, urand(10000, 14000));
                            break;
                        case EVENT_DEATH_STRIKE:
                            if (HealthBelowPct(20))
                                DoCastVictim(SPELL_DEATH_STRIKE);
                            _events.ScheduleEvent(EVENT_DEATH_STRIKE, urand(5000, 10000));
                            break;
                        case EVENT_HEALTH_CHECK:
                            if (HealthAbovePct(15))
                            {
                                me->RemoveAurasDueToSpell(SPELL_ICEBOUND_ARMOR);
                                _didUnderTenPercentText = false;
                            }
                            else
                            {
                                me->DealHeal(me, me->CountPctFromMaxHealth(5));
                                _events.ScheduleEvent(EVENT_HEALTH_CHECK, 1000);
                            }
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            bool CanAIAttack(Unit const* target) const OVERRIDE
            {
                // do not see targets inside Frostwing Halls when we are not there
                return (me->GetPositionY() > 2660.0f) == (target->GetPositionY() > 2660.0f);
            }

        private:
            EventMap _events;
            std::set<uint64> _aliveTrash;
            InstanceScript* _instance;
            uint32 _currentWPid;
            uint32 _wipeCheckTimer;
            uint32 const _respawnTime;
            uint32 const _corpseDelay;
            bool _isEventActive;
            bool _isEventDone;
            bool _didUnderTenPercentText;
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<npc_crok_scourgebaneAI>(creature);
        }
};

struct npc_argent_captainAI : public ScriptedAI
{
    public:
        npc_argent_captainAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()), _firstDeath(true)
        {
            FollowAngle = PET_FOLLOW_ANGLE;
            FollowDist = PET_FOLLOW_DIST;
            IsUndead = false;
        }

        void JustDied(Unit* /*killer*/) OVERRIDE
        {
            if (_firstDeath)
            {
                _firstDeath = false;
                Talk(SAY_CAPTAIN_DEATH);
            }
            else
                Talk(SAY_CAPTAIN_SECOND_DEATH);
        }

        void KilledUnit(Unit* victim) OVERRIDE
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_CAPTAIN_KILL);
        }

        void DoAction(int32 action) OVERRIDE
        {
            if (action == ACTION_START_GAUNTLET)
            {
                if (Creature* crok = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_CROK_SCOURGEBANE)))
                {
                    me->SetReactState(REACT_DEFENSIVE);
                    FollowAngle = me->GetAngle(crok) + me->GetOrientation();
                    FollowDist = me->GetDistance2d(crok);
                    me->GetMotionMaster()->MoveFollow(crok, FollowDist, FollowAngle, MOTION_SLOT_IDLE);
                }

                me->setActive(true);
            }
            else if (action == ACTION_RESET_EVENT)
            {
                _firstDeath = true;
            }
        }

        void EnterCombat(Unit* /*target*/) OVERRIDE
        {
            me->SetHomePosition(*me);
        }

        bool CanAIAttack(Unit const* target) const OVERRIDE
        {
            // do not see targets inside Frostwing Halls when we are not there
            return (me->GetPositionY() > 2660.0f) == (target->GetPositionY() > 2660.0f);
        }

        void EnterEvadeMode() OVERRIDE
        {
            if (IsUndead)
                me->DespawnOrUnsummon();

            // not yet following
            if (me->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_IDLE) != CHASE_MOTION_TYPE)
            {
                ScriptedAI::EnterEvadeMode();
                return;
            }

            if (!_EnterEvadeMode())
                return;

            if (!me->GetVehicle())
            {
                me->GetMotionMaster()->Clear(false);
                if (Creature* crok = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_CROK_SCOURGEBANE)))
                    me->GetMotionMaster()->MoveFollow(crok, FollowDist, FollowAngle, MOTION_SLOT_IDLE);
            }

            Reset();
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell) OVERRIDE
        {
            if (spell->Id == SPELL_REVIVE_CHAMPION && !IsUndead)
            {
                IsUndead = true;
                me->setDeathState(JUST_RESPAWNED);
                uint32 newEntry = 0;
                switch (me->GetEntry())
                {
                    case NPC_CAPTAIN_ARNATH:
                        newEntry = NPC_CAPTAIN_ARNATH_UNDEAD;
                        break;
                    case NPC_CAPTAIN_BRANDON:
                        newEntry = NPC_CAPTAIN_BRANDON_UNDEAD;
                        break;
                    case NPC_CAPTAIN_GRONDEL:
                        newEntry = NPC_CAPTAIN_GRONDEL_UNDEAD;
                        break;
                    case NPC_CAPTAIN_RUPERT:
                        newEntry = NPC_CAPTAIN_RUPERT_UNDEAD;
                        break;
                    default:
                        return;
                }

                Talk(SAY_CAPTAIN_RESURRECTED);
                me->UpdateEntry(newEntry, instance->GetData(DATA_TEAM_IN_INSTANCE), me->GetCreatureData());                
                DoCast(me, SPELL_UNDEATH, true);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 150.0f);
            }
        }

    protected:
        EventMap Events;
        InstanceScript* instance;
        float FollowAngle;
        float FollowDist;
        bool IsUndead;

    private:
        bool _firstDeath;
};

class npc_captain_arnath : public CreatureScript
{
    public:
        npc_captain_arnath() : CreatureScript("npc_captain_arnath") { }

        struct npc_captain_arnathAI : public npc_argent_captainAI
        {
            npc_captain_arnathAI(Creature* creature) : npc_argent_captainAI(creature)
            {
            }

            void Reset() OVERRIDE
            {
                Events.Reset();
                Events.ScheduleEvent(EVENT_ARNATH_FLASH_HEAL, urand(4000, 7000));
                Events.ScheduleEvent(EVENT_ARNATH_PW_SHIELD, urand(8000, 14000));
                Events.ScheduleEvent(EVENT_ARNATH_SMITE, urand(3000, 6000));
                if (Is25ManRaid() && IsUndead)
                    Events.ScheduleEvent(EVENT_ARNATH_DOMINATE_MIND, urand(22000, 27000));
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (!UpdateVictim())
                    return;

                Events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = Events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ARNATH_FLASH_HEAL:
                            if (Creature* target = FindFriendlyCreature())
                                DoCast(target, SPELL_FLASH_HEAL);
                            Events.ScheduleEvent(EVENT_ARNATH_FLASH_HEAL, urand(6000, 9000));
                            break;
                        case EVENT_ARNATH_PW_SHIELD:
                        {
                            std::list<Creature*> targets = DoFindFriendlyMissingBuff(40.0f, SPELL_POWER_WORD_SHIELD);
                            DoCast(Trinity::Containers::SelectRandomContainerElement(targets), SPELL_POWER_WORD_SHIELD);
                            Events.ScheduleEvent(EVENT_ARNATH_PW_SHIELD, urand(15000, 20000));
                            break;
                        }
                        case EVENT_ARNATH_SMITE:
                            DoCastVictim(SPELL_SMITE);
                            Events.ScheduleEvent(EVENT_ARNATH_SMITE, urand(4000, 7000));
                            break;
                        case EVENT_ARNATH_DOMINATE_MIND:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(target, SPELL_DOMINATE_MIND);
                            Events.ScheduleEvent(EVENT_ARNATH_DOMINATE_MIND, urand(28000, 37000));
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            Creature* FindFriendlyCreature() const
            {
                Creature* target = NULL;
                Trinity::MostHPMissingInRange u_check(me, 60.0f, 0);
                Trinity::CreatureLastSearcher<Trinity::MostHPMissingInRange> searcher(me, target, u_check);
                me->VisitNearbyGridObject(60.0f, searcher);
                return target;
            }
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<npc_captain_arnathAI>(creature);
        }
};

class npc_captain_brandon : public CreatureScript
{
    public:
        npc_captain_brandon() : CreatureScript("npc_captain_brandon") { }

        struct npc_captain_brandonAI : public npc_argent_captainAI
        {
            npc_captain_brandonAI(Creature* creature) : npc_argent_captainAI(creature)
            {
            }

            void Reset() OVERRIDE
            {
                Events.Reset();
                Events.ScheduleEvent(EVENT_BRANDON_CRUSADER_STRIKE, urand(6000, 10000));
                Events.ScheduleEvent(EVENT_BRANDON_DIVINE_SHIELD, 500);
                Events.ScheduleEvent(EVENT_BRANDON_JUDGEMENT_OF_COMMAND, urand(8000, 13000));
                if (IsUndead)
                    Events.ScheduleEvent(EVENT_BRANDON_HAMMER_OF_BETRAYAL, urand(25000, 30000));
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (!UpdateVictim())
                    return;

                Events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = Events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BRANDON_CRUSADER_STRIKE:
                            DoCastVictim(SPELL_CRUSADER_STRIKE);
                            Events.ScheduleEvent(EVENT_BRANDON_CRUSADER_STRIKE, urand(6000, 12000));
                            break;
                        case EVENT_BRANDON_DIVINE_SHIELD:
                            if (HealthBelowPct(20))
                            {
                                DoCast(me, SPELL_DIVINE_SHIELD);
                                Events.RescheduleEvent(EVENT_BRANDON_DIVINE_SHIELD, 5*MINUTE*IN_MILLISECONDS); 
                            }
                            else
                                Events.ScheduleEvent(EVENT_BRANDON_DIVINE_SHIELD, 500);
                            break;
                        case EVENT_BRANDON_JUDGEMENT_OF_COMMAND:
                            DoCastVictim(SPELL_JUDGEMENT_OF_COMMAND);
                            Events.ScheduleEvent(EVENT_BRANDON_JUDGEMENT_OF_COMMAND, urand(8000, 13000));
                            break;
                        case EVENT_BRANDON_HAMMER_OF_BETRAYAL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(target, SPELL_HAMMER_OF_BETRAYAL);
                            Events.ScheduleEvent(EVENT_BRANDON_HAMMER_OF_BETRAYAL, urand(45000, 60000));
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<npc_captain_brandonAI>(creature);
        }
};

class npc_captain_grondel : public CreatureScript
{
    public:
        npc_captain_grondel() : CreatureScript("npc_captain_grondel") { }

        struct npc_captain_grondelAI : public npc_argent_captainAI
        {
            npc_captain_grondelAI(Creature* creature) : npc_argent_captainAI(creature)
            {
            }

            void Reset() OVERRIDE
            {
                Events.Reset();
                Events.ScheduleEvent(EVENT_GRONDEL_CHARGE_CHECK, 500);
                Events.ScheduleEvent(EVENT_GRONDEL_MORTAL_STRIKE, urand(8000, 14000));
                Events.ScheduleEvent(EVENT_GRONDEL_SUNDER_ARMOR, urand(3000, 12000));
                if (IsUndead)
                    Events.ScheduleEvent(EVENT_GRONDEL_CONFLAGRATION, urand(12000, 17000));
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (!UpdateVictim())
                    return;

                Events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = Events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_GRONDEL_CHARGE_CHECK:
                            DoCastVictim(SPELL_CHARGE);
                            Events.ScheduleEvent(EVENT_GRONDEL_CHARGE_CHECK, 500);
                            break;
                        case EVENT_GRONDEL_MORTAL_STRIKE:
                            DoCastVictim(SPELL_MORTAL_STRIKE);
                            Events.ScheduleEvent(EVENT_GRONDEL_MORTAL_STRIKE, urand(10000, 15000));
                            break;
                        case EVENT_GRONDEL_SUNDER_ARMOR:
                            DoCastVictim(SPELL_SUNDER_ARMOR);
                            Events.ScheduleEvent(EVENT_GRONDEL_SUNDER_ARMOR, urand(5000, 17000));
                            break;
                        case EVENT_GRONDEL_CONFLAGRATION:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(target, SPELL_CONFLAGRATION);
                            Events.ScheduleEvent(EVENT_GRONDEL_CONFLAGRATION, urand(10000, 15000));
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<npc_captain_grondelAI>(creature);
        }
};

class npc_captain_rupert : public CreatureScript
{
    public:
        npc_captain_rupert() : CreatureScript("npc_captain_rupert") { }

        struct npc_captain_rupertAI : public npc_argent_captainAI
        {
            npc_captain_rupertAI(Creature* creature) : npc_argent_captainAI(creature)
            {
            }

            void Reset() OVERRIDE
            {
                Events.Reset();
                Events.ScheduleEvent(EVENT_RUPERT_FEL_IRON_BOMB, urand(15000, 20000));
                Events.ScheduleEvent(EVENT_RUPERT_MACHINE_GUN, urand(25000, 30000));
                Events.ScheduleEvent(EVENT_RUPERT_ROCKET_LAUNCH, urand(10000, 15000));
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (!UpdateVictim())
                    return;

                Events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = Events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RUPERT_FEL_IRON_BOMB:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_FEL_IRON_BOMB);
                            Events.ScheduleEvent(EVENT_RUPERT_FEL_IRON_BOMB, urand(15000, 20000));
                            break;
                        case EVENT_RUPERT_MACHINE_GUN:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                                DoCast(target, SPELL_MACHINE_GUN);
                            Events.ScheduleEvent(EVENT_RUPERT_MACHINE_GUN, urand(25000, 30000));
                            break;
                        case EVENT_RUPERT_ROCKET_LAUNCH:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                                DoCast(target, SPELL_ROCKET_LAUNCH);
                            Events.ScheduleEvent(EVENT_RUPERT_ROCKET_LAUNCH, urand(10000, 15000));
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return GetIcecrownCitadelAI<npc_captain_rupertAI>(creature);
        }
};

class npc_frostwing_vrykul : public CreatureScript
{
    public:
        npc_frostwing_vrykul() : CreatureScript("npc_frostwing_vrykul") { }

        struct npc_frostwing_vrykulAI : public SmartAI
        {
            npc_frostwing_vrykulAI(Creature* creature) : SmartAI(creature)
            {
            }

            bool CanAIAttack(Unit const* target) const OVERRIDE
            {
                // do not see targets inside Frostwing Halls when we are not there
                return (me->GetPositionY() > 2660.0f) == (target->GetPositionY() > 2660.0f) && SmartAI::CanAIAttack(target);
            }
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return new npc_frostwing_vrykulAI(creature);
        }
};

class npc_impaling_spear : public CreatureScript
{
    public:
        npc_impaling_spear() : CreatureScript("npc_impaling_spear") { }

        struct npc_impaling_spearAI : public CreatureAI
        {
            npc_impaling_spearAI(Creature* creature) : CreatureAI(creature)
            {
                Initialize();
            }

            void Initialize()
            {
                _vehicleCheckTimer = 500;
            }

            void Reset() OVERRIDE
            {
                me->SetReactState(REACT_PASSIVE);
                Initialize();
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (_vehicleCheckTimer <= diff)
                {
                    _vehicleCheckTimer = 500;
                    if (!me->GetVehicle())
                        me->DespawnOrUnsummon(100);
                }
                else
                    _vehicleCheckTimer -= diff;
            }

            uint32 _vehicleCheckTimer;
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            return new npc_impaling_spearAI(creature);
        }
};

class npc_arthas_teleport_visual : public CreatureScript
{
    public:
        npc_arthas_teleport_visual() : CreatureScript("npc_arthas_teleport_visual") { }

        struct npc_arthas_teleport_visualAI : public NullCreatureAI
        {
            npc_arthas_teleport_visualAI(Creature* creature) : NullCreatureAI(creature), _instance(creature->GetInstanceScript())
            {
            }

            void Reset() OVERRIDE
            {
                _events.Reset();
                if (_instance->GetBossState(DATA_PROFESSOR_PUTRICIDE) == DONE &&
                    _instance->GetBossState(DATA_BLOOD_QUEEN_LANA_THEL) == DONE &&
                    _instance->GetBossState(DATA_SINDRAGOSA) == DONE)
                    _events.ScheduleEvent(EVENT_SOUL_MISSILE, urand(1000, 6000));
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (_events.Empty())
                    return;

                _events.Update(diff);

                if (_events.ExecuteEvent() == EVENT_SOUL_MISSILE)
                {
                    DoCastAOE(SPELL_SOUL_MISSILE);
                    _events.ScheduleEvent(EVENT_SOUL_MISSILE, urand(5000, 7000));
                }
            }

        private:
            InstanceScript* _instance;
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const OVERRIDE
        {
            // Distance from the center of the spire
            if (creature->GetExactDist2d(4357.052f, 2769.421f) < 100.0f && creature->GetHomePosition().GetPositionZ() < 315.0f)
                return GetIcecrownCitadelAI<npc_arthas_teleport_visualAI>(creature);

            // Default to no script
            return NULL;
        }
};

class spell_icc_stoneform : public SpellScriptLoader
{
    public:
        spell_icc_stoneform() : SpellScriptLoader("spell_icc_stoneform") { }

        class spell_icc_stoneform_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_icc_stoneform_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Creature* target = GetTarget()->ToCreature())
                {
                    target->SetReactState(REACT_PASSIVE);
                    target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                    target->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_CUSTOM_SPELL_02);
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Creature* target = GetTarget()->ToCreature())
                {
                    target->SetReactState(REACT_AGGRESSIVE);
                    target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                    target->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                }
            }

            void Register() OVERRIDE
            {
                OnEffectApply += AuraEffectApplyFn(spell_icc_stoneform_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_icc_stoneform_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const OVERRIDE
        {
            return new spell_icc_stoneform_AuraScript();
        }
};

class spell_icc_sprit_alarm : public SpellScriptLoader
{
    public:
        spell_icc_sprit_alarm() : SpellScriptLoader("spell_icc_sprit_alarm") { }

        class spell_icc_sprit_alarm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_icc_sprit_alarm_SpellScript);

            void HandleEvent(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                uint32 trapId = 0;
                switch (GetSpellInfo()->Effects[effIndex].MiscValue)
                {
                    case EVENT_AWAKEN_WARD_1:
                        trapId = GO_SPIRIT_ALARM_1;
                        break;
                    case EVENT_AWAKEN_WARD_2:
                        trapId = GO_SPIRIT_ALARM_2;
                        break;
                    case EVENT_AWAKEN_WARD_3:
                        trapId = GO_SPIRIT_ALARM_3;
                        break;
                    case EVENT_AWAKEN_WARD_4:
                        trapId = GO_SPIRIT_ALARM_4;
                        break;
                    default:
                        return;
                }

                if (GameObject* trap = GetCaster()->FindNearestGameObject(trapId, 5.0f))
                    trap->SetRespawnTime(trap->GetGOInfo()->GetAutoCloseTime());

                std::list<Creature*> wards;
                GetCaster()->GetCreatureListWithEntryInGrid(wards, NPC_DEATHBOUND_WARD, 150.0f);
                wards.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));
                for (std::list<Creature*>::iterator itr = wards.begin(); itr != wards.end(); ++itr)
                {
                    if ((*itr)->IsAlive() && (*itr)->HasAura(SPELL_STONEFORM))
                    {
                        (*itr)->AI()->DoAction(ACTION_AWAKEN);
                        (*itr)->RemoveAurasDueToSpell(SPELL_STONEFORM);
                        if (Unit* target = (*itr)->SelectNearestTarget(80.0f))
                            (*itr)->AI()->AttackStart(target);
                        break;
                    }
                }
            }

            void Register() OVERRIDE
            {
                OnEffectHit += SpellEffectFn(spell_icc_sprit_alarm_SpellScript::HandleEvent, EFFECT_2, SPELL_EFFECT_SEND_EVENT);
            }
        };

        SpellScript* GetSpellScript() const OVERRIDE
        {
            return new spell_icc_sprit_alarm_SpellScript();
        }
};

class DeathPlagueTargetSelector
{
    public:
        explicit DeathPlagueTargetSelector(Unit* caster) : _caster(caster) { }

        bool operator()(WorldObject* object) const
        {
            if (object == _caster)
                return true;

            if (object->GetTypeId() != TYPEID_PLAYER)
                return true;

            if (object->ToUnit()->HasAura(SPELL_RECENTLY_INFECTED) || object->ToUnit()->HasAura(SPELL_DEATH_PLAGUE_AURA))
                return true;

            return false;
        }

    private:
        Unit* _caster;
};

class spell_frost_giant_death_plague : public SpellScriptLoader
{
    public:
        spell_frost_giant_death_plague() : SpellScriptLoader("spell_frost_giant_death_plague") { }

        class spell_frost_giant_death_plague_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_frost_giant_death_plague_SpellScript);

        public:
            spell_frost_giant_death_plague_SpellScript()
            {
                _failed = false;
            }

        private:
            // First effect
            void CountTargets(std::list<WorldObject*>& targets)
            {
                targets.remove(GetCaster());
                _failed = targets.empty();
            }

            // Second effect
            void FilterTargets(std::list<WorldObject*>& targets)
            {
                // Select valid targets for jump
                targets.remove_if(DeathPlagueTargetSelector(GetCaster()));
                if (!targets.empty())
                {
                    WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                    targets.clear();
                    targets.push_back(target);
                }

                targets.push_back(GetCaster());
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (GetHitUnit() != GetCaster())
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_DEATH_PLAGUE_AURA, true);
                else if (_failed)
                    GetCaster()->CastSpell(GetCaster(), SPELL_DEATH_PLAGUE_KILL, true);
            }

            void Register() OVERRIDE
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_frost_giant_death_plague_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_frost_giant_death_plague_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ALLY);
                OnEffectHitTarget += SpellEffectFn(spell_frost_giant_death_plague_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }

            bool _failed;
        };

        SpellScript* GetSpellScript() const OVERRIDE
        {
            return new spell_frost_giant_death_plague_SpellScript();
        }
};

class spell_icc_harvest_blight_specimen : public SpellScriptLoader
{
    public:
        spell_icc_harvest_blight_specimen() : SpellScriptLoader("spell_icc_harvest_blight_specimen") { }

        class spell_icc_harvest_blight_specimen_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_icc_harvest_blight_specimen_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->RemoveAurasDueToSpell(uint32(GetEffectValue()));
            }

            void HandleQuestComplete(SpellEffIndex /*effIndex*/)
            {
                GetHitUnit()->RemoveAurasDueToSpell(uint32(GetEffectValue()));
            }

            void Register() OVERRIDE
            {
                OnEffectHitTarget += SpellEffectFn(spell_icc_harvest_blight_specimen_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnEffectHitTarget += SpellEffectFn(spell_icc_harvest_blight_specimen_SpellScript::HandleQuestComplete, EFFECT_1, SPELL_EFFECT_QUEST_COMPLETE);
            }
        };

        SpellScript* GetSpellScript() const OVERRIDE
        {
            return new spell_icc_harvest_blight_specimen_SpellScript();
        }
};

class AliveCheck
{
    public:
        bool operator()(WorldObject* object) const
        {
            if (Unit* unit = object->ToUnit())
                return unit->IsAlive();
            return true;
        }
};

class spell_svalna_revive_champion : public SpellScriptLoader
{
    public:
        spell_svalna_revive_champion() : SpellScriptLoader("spell_svalna_revive_champion") { }

        class spell_svalna_revive_champion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_svalna_revive_champion_SpellScript);

            void RemoveAliveTarget(std::list<WorldObject*>& targets)
            {
                targets.remove_if(AliveCheck());
                Trinity::Containers::RandomResizeList(targets, 2);
            }

            void Land(SpellEffIndex /*effIndex*/)
            {
                Creature* caster = GetCaster()->ToCreature();
                if (!caster)
                    return;

                Position pos;
                caster->GetNearPosition(pos, 5.0f, 0.0f);
                //pos.m_positionZ = caster->GetBaseMap()->GetHeight(caster->GetPhaseMask(), pos.GetPositionX(), pos.GetPositionY(), caster->GetPositionZ(), true, 50.0f);
                //pos.m_positionZ += 0.05f;
                caster->SetHomePosition(pos);
                caster->GetMotionMaster()->MoveLand(POINT_LAND, pos);
            }

            void Register() OVERRIDE
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_svalna_revive_champion_SpellScript::RemoveAliveTarget, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
                OnEffectHit += SpellEffectFn(spell_svalna_revive_champion_SpellScript::Land, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const OVERRIDE
        {
            return new spell_svalna_revive_champion_SpellScript();
        }
};

class spell_svalna_remove_spear : public SpellScriptLoader
{
    public:
        spell_svalna_remove_spear() : SpellScriptLoader("spell_svalna_remove_spear") { }

        class spell_svalna_remove_spear_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_svalna_remove_spear_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Creature* target = GetHitCreature())
                {
                    if (Unit* vehicle = target->GetVehicleBase())
                        vehicle->RemoveAurasDueToSpell(SPELL_IMPALING_SPEAR);
                    target->DespawnOrUnsummon(1);
                }
            }

            void Register() OVERRIDE
            {
                OnEffectHitTarget += SpellEffectFn(spell_svalna_remove_spear_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const OVERRIDE
        {
            return new spell_svalna_remove_spear_SpellScript();
        }
};

// 72585 - Soul Missile
class spell_icc_soul_missile : public SpellScriptLoader
{
    public:
        spell_icc_soul_missile() : SpellScriptLoader("spell_icc_soul_missile") { }

        class spell_icc_soul_missile_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_icc_soul_missile_SpellScript);

            void RelocateDest()
            {
                static Position const offset = { 0.0f, 0.0f, 200.0f, 0.0f };
                const_cast<WorldLocation*>(GetExplTargetDest())->RelocateOffset(offset);
            }

            void Register() OVERRIDE
            {
                OnCast += SpellCastFn(spell_icc_soul_missile_SpellScript::RelocateDest);
            }
        };

        SpellScript* GetSpellScript() const OVERRIDE
        {
            return new spell_icc_soul_missile_SpellScript();
        }
};

class at_icc_saurfang_portal : public AreaTriggerScript
{
    public:
        at_icc_saurfang_portal() : AreaTriggerScript("at_icc_saurfang_portal") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/) OVERRIDE
        {
            InstanceScript* instance = player->GetInstanceScript();
            if (!instance || instance->GetBossState(DATA_DEATHBRINGER_SAURFANG) != DONE)
                return true;

            player->TeleportTo(631, 4120.90f, 2769.29f, 350.963f, 0.0f);

            if (instance->GetData(DATA_COLDFLAME_JETS) == NOT_STARTED)
            {
                // Process relocation now, to preload the grid and initialize traps
                player->GetMap()->PlayerRelocation(player, 4120.90f, 2769.29f, 350.963f, 0.0f);

                instance->SetData(DATA_COLDFLAME_JETS, IN_PROGRESS);
                std::list<Creature*> traps;
                GetCreatureListWithEntryInGrid(traps, player, NPC_FROST_FREEZE_TRAP, 120.0f);
                traps.sort(Trinity::ObjectDistanceOrderPred(player));
                bool instant = false;
                for (std::list<Creature*>::iterator itr = traps.begin(); itr != traps.end(); ++itr)
                {
                    (*itr)->AI()->DoAction(instant ? 1000 : 11000);
                    instant = !instant;
                }
            }

            return true;
        }
};

class at_icc_shutdown_traps : public AreaTriggerScript
{
    public:
        at_icc_shutdown_traps() : AreaTriggerScript("at_icc_shutdown_traps") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/) OVERRIDE
        {
            if (InstanceScript* instance = player->GetInstanceScript())
                instance->SetData(DATA_UPPERSPIRE_TELE_ACT, DONE);

            return true;
        }
};

class at_icc_start_blood_quickening : public AreaTriggerScript
{
    public:
        at_icc_start_blood_quickening() : AreaTriggerScript("at_icc_start_blood_quickening") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/) OVERRIDE
        {
            if (InstanceScript* instance = player->GetInstanceScript())
                if (instance->GetData(DATA_BLOOD_QUICKENING_STATE) == NOT_STARTED)
                    instance->SetData(DATA_BLOOD_QUICKENING_STATE, IN_PROGRESS);
            return true;
        }
};

class at_icc_start_frostwing_gauntlet : public AreaTriggerScript
{
    public:
        at_icc_start_frostwing_gauntlet() : AreaTriggerScript("at_icc_start_frostwing_gauntlet") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/) OVERRIDE
        {
            if (InstanceScript* instance = player->GetInstanceScript())
                if (Creature* crok = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_CROK_SCOURGEBANE)))
                    crok->AI()->DoAction(ACTION_START_GAUNTLET);
            return true;
        }
};

class at_icc_start_sindragosa_gauntlet : public AreaTriggerScript
{
    public:
        at_icc_start_sindragosa_gauntlet() : AreaTriggerScript("at_icc_start_sindragosa_gauntlet") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
                if (Creature* ward = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_SINDRAGOSA_GAUNTLET)))
                    ward->AI()->DoAction(ACTION_START_GAUNTLET);
            return true;
        }
};

#define SPELL_WEB_BEAM			69887
class npc_sindragosas_ward : public CreatureScript
{
    public:
        npc_sindragosas_ward() : CreatureScript("npc_sindragosas_ward") { }

        struct npc_sindragosas_wardAI : public BossAI
        {
            npc_sindragosas_wardAI(Creature* creature) : BossAI(creature, DATA_SINDRAGOSA_GAUNTLET) { }

            void Reset() OVERRIDE
            {
                _Reset();
                _isEventInProgressOrDone = false;
                _spawnCountToBeSummonedInWave = 0;
                _waveNumber = 0;
                // Open this door DATA_SINDRAGOSA_DOOR
                instance->HandleGameObject(instance->GetData64(DATA_SINDRAGOSA_DOOR), true);
            }

            void DoAction(int32 action) OVERRIDE
            {
                if (action == ACTION_START_GAUNTLET)
				{
                    if (!_isEventInProgressOrDone && !IsAnyPlayerOutOfRange())
						DoZoneInCombat();
				}
            }

            void EnterCombat(Unit* /*attacker*/) OVERRIDE
			{
				_EnterCombat();
				_isEventInProgressOrDone = true;
				_spawnCountToBeSummonedInWave = 32;
				_waveNumber = 1;
				DoSummonWave(_waveNumber);
				events.ScheduleEvent(EVENT_SUB_WAVE_1, 10000);
				events.ScheduleEvent(EVENT_SUB_WAVE_2, 25000);
				events.ScheduleEvent(EVENT_UPDATE_CHECK, 5000);
                // Close this door DATA_SINDRAGOSA_DOOR
                instance->HandleGameObject(instance->GetData64(DATA_SINDRAGOSA_DOOR), false);
			}

			void SummonNerubar(uint32 entry, uint8 position, const Position spawn[])
			{
				if (Creature* creature = me->SummonCreature(entry, spawn[position].m_positionX, spawn[position].m_positionY, spawn[position].m_positionZ + (urand(30, 70)), 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
				{
					creature->CastSpell(me, SPELL_WEB_BEAM, true);
					creature->GetMotionMaster()->MoveFall();
				}
			}

            void DoSummonWave(uint8 number)
            {
                switch (number)
                {
                    case 1:
                    case 3:
						for (uint8 i = 0; i < MAX_NERUBAR_MAIN; i++)
						{
							SummonNerubar(NPC_NERUBAR_WEBWEAVER, i, SindragosaGauntletSpawnWebreaver);
							SummonNerubar(NPC_NERUBAR_CHAMPION, i, SindragosaGauntletSpawnChampion);
						}
						break;
					case 2:
						me->SummonCreature(NPC_FROSTWARDEN_WARRIOR, SindragosaGauntletSpawnFrostWardenLeft[0], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
						me->SummonCreature(NPC_FROSTWARDEN_SORCERESS, SindragosaGauntletSpawnFrostWardenLeft[1], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
						me->SummonCreature(NPC_FROSTWARDEN_WARRIOR, SindragosaGauntletSpawnFrostWardenLeft[2], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
						me->SummonCreature(NPC_FROSTWARDEN_WARRIOR, SindragosaGauntletSpawnFrostWardenRight[0], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
						me->SummonCreature(NPC_FROSTWARDEN_SORCERESS, SindragosaGauntletSpawnFrostWardenRight[1], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
						me->SummonCreature(NPC_FROSTWARDEN_WARRIOR, SindragosaGauntletSpawnFrostWardenRight[2], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
						break;
					case EVENT_SUB_WAVE_1:
                    case EVENT_SUB_WAVE_2:
						for (uint8 i = 0; i < MAX_NERUBAR_OFF; i++)
							SummonNerubar(NPC_NERUBAR_BROODLING, i, SindragosaGauntletSpawnBroodling);
                        break;
                    default:
                        break;
                }
            }

            bool IsAnyPlayerOutOfRange()
            {
                if (!me->GetMap())
                    return true;

                Map::PlayerList const& playerList = me->GetMap()->GetPlayers();

                if (playerList.isEmpty())
                    return true;

                for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                {
                    if (Player* player = itr->GetSource())
                    {
                        if (player->IsGameMaster())
                            continue;

                        if (player->IsAlive() && me->GetDistance(player) > 125.0f)
                            return true;
                    }
                }

                return false;
            }

            void JustSummoned(Creature* summon) OVERRIDE
            {
                summons.Summon(summon);
                DoZoneInCombat(summon, 150.0f);
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*who*/) OVERRIDE
            {
                _spawnCountToBeSummonedInWave--;
                summon->DespawnOrUnsummon(30000);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                // This one should never happen, if summoned creature despawns alive, reset!
                if (summon->IsAlive())
                {
                    EnterEvadeMode();
                    return;
                }

                summons.Despawn(summon);
            }

            void UpdateAI(uint32 diff) OVERRIDE
            {
                if (!UpdateVictim() || !_isEventInProgressOrDone)
                    return;

                events.Update(diff);

               while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SUB_WAVE_1:
                            DoSummonWave(EVENT_SUB_WAVE_1);
                            break;
                        case EVENT_SUB_WAVE_2:
                            DoSummonWave(EVENT_SUB_WAVE_2);
                            break;
                       case EVENT_UPDATE_CHECK:
                        {
                            if (_spawnCountToBeSummonedInWave <= 5)
                            {
                               if (summons.size() < _spawnCountToBeSummonedInWave)
                                    _spawnCountToBeSummonedInWave = summons.size();

                                if (!_spawnCountToBeSummonedInWave)
                                {
                                    switch (_waveNumber)
                                    {
                                        case 1:
                                            _spawnCountToBeSummonedInWave += 30;
                                            break;
                                        case 2:
                                            _spawnCountToBeSummonedInWave += 32;
                                            break;
                                        case 3:
                                            instance->HandleGameObject(instance->GetData64(DATA_SINDRAGOSA_DOOR), true);
                                            me->Kill(me);
                                            break;
                                        default:
                                            break;
                                    }
                                    _waveNumber++;
                                    DoSummonWave(_waveNumber);
                                    events.ScheduleEvent(EVENT_SUB_WAVE_1, 10000);
                                    events.ScheduleEvent(EVENT_SUB_WAVE_2, 25000);
                                }
                            }

                            if (IsAnyPlayerOutOfRange())
                            {
                                EnterEvadeMode();
                                return;
                            }

                            events.ScheduleEvent(EVENT_UPDATE_CHECK, 5000);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

        private:
            bool _isEventInProgressOrDone;
            uint32 _spawnCountToBeSummonedInWave;
            uint8 _waveNumber;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetIcecrownCitadelAI<npc_sindragosas_wardAI>(creature);
        }
};

// Deathbound ward  - 37007
class npc_deathbound_ward : public CreatureScript
{
public:
    npc_deathbound_ward() : CreatureScript("npc_deathbound_ward") { }

    struct npc_deathbound_wardAI : public ScriptedAI
    {
        npc_deathbound_wardAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void EnterCombat(Unit* /*who*/) OVERRIDE
        {
            events.Reset();
            events.ScheduleEvent(EVENT_DISRUPTING_SHOUT, urand(17000, 23000));
        }

        void DoAction(int32 action) OVERRIDE
        {
            if (action != ACTION_AWAKEN)
                return;

            Talk(SAY_TRAP_ACTIVATE);
            me->RemoveAurasDueToSpell(SPELL_STONEFORM);
            if (Unit* target = me->SelectNearestTarget(30.0f))
                AttackStart(target);
        }

        void UpdateAI(uint32 diff) OVERRIDE
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_DISRUPTING_SHOUT:
                    DoCast(me, SPELL_DISRUPTING_SHOUT);
                    events.ScheduleEvent(EVENT_DISRUPTING_SHOUT, urand(17000, 23000));
                    break;
                default:
                    break;
                }
            }

            if (!me->GetCurrentSpell(CURRENT_MELEE_SPELL))
                DoCastVictim(SPELL_SABERLASH);

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return GetIcecrownCitadelAI<npc_deathbound_wardAI>(creature);
    }
};

enum PutricideTrapData
{
    EVENT_SUMMON_INSECTS     = 1,
    EVENT_FLESH_EATING_BITE  = 2,
    EVENT_CHECK_INSTANCE     = 3
};

class npc_putricide_insect_trap : public CreatureScript
{
public:
    npc_putricide_insect_trap() : CreatureScript("npc_putricide_insect_trap") { }

    struct npc_putricide_insect_trapAI : public ScriptedAI
    {
        npc_putricide_insect_trapAI(Creature* creature) : ScriptedAI(creature), _summons(me)
        {
            _instance = creature->GetInstanceScript();
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            creature->SetDisplayId(11686);
        }

        void DoAction(int32 actionId) OVERRIDE
        {
            switch (actionId)
            {
            case ACTION_START_TRAP_EVENT:
                me->AI()->DoCastAOE(SPELL_GIANT_INSECT_SWARM);
                _events.ScheduleEvent(EVENT_SUMMON_INSECTS, 2000);
                _events.ScheduleEvent(EVENT_CHECK_INSTANCE, 5000);
                break;
            case ACTION_END_TRAP_EVENT:
                CleanUpEvent(false);
                _events.Reset();
                break;
            case ACTION_FAIL_TRAP_EVENT:
                CleanUpEvent(true);
                _events.Reset();
                _instance->SetData(DATA_INSECT_SWARM_EVENT, NOT_STARTED);
                break;
            default:
                break;
            }
        }

        void CleanUpEvent(bool DespawnAdds)
        {
            me->InterruptNonMeleeSpells(true);
            EnterEvadeMode();
            me->SetVisible(false);
            me->ClearAllReactives();

            if (DespawnAdds)
                _summons.DespawnAll();
        }

        void EnterCombat(Unit* /* who */) OVERRIDE
        {
            if (_instance->GetData(DATA_INSECT_SWARM_EVENT) == IN_PROGRESS)
                SummonInsects(me);
        }

        bool CanAIAttack(Unit const* /*who*/) const OVERRIDE
        {
            if (_instance->GetData(DATA_INSECT_SWARM_EVENT) == DONE)
                return false;

            return true;
        }

        void UpdateAI(uint32 diff) OVERRIDE
        {
            if (_instance->GetData(DATA_INSECT_SWARM_EVENT) == DONE)
                me->DespawnOrUnsummon();

            if (_instance->GetData(DATA_INSECT_SWARM_EVENT) != IN_PROGRESS)
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_SUMMON_INSECTS)
                {
                    if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 25.0f, true))
                        SummonInsects(victim);

                    _events.ScheduleEvent(EVENT_SUMMON_INSECTS, 3000);
                }
                else if (eventId == EVENT_CHECK_INSTANCE)
                {
                    if (_instance->GetData(DATA_INSECT_SWARM_EVENT) == DONE)
                        return;

                    bool HasInsectSwarm = false;
                    Map::PlayerList const &PlList = me->GetMap()->GetPlayers();

                    if (PlList.isEmpty())
                    {
                        DoAction(ACTION_FAIL_TRAP_EVENT);
                        return;
                    }

                    for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
                    {
                        if (Player* player = i->GetSource())
                        {
                            if (player->HasAura(SPELL_GIANT_INSECT_SWARM))
                                HasInsectSwarm = true;
                        }
                    }

                    if (!HasInsectSwarm)
                        DoAction(ACTION_FAIL_TRAP_EVENT);

                    _events.ScheduleEvent(EVENT_CHECK_INSTANCE, 5000);
                }
            }
        }

        void JustSummoned(Creature* summon) OVERRIDE
        {
            _summons.Summon(summon);
            summon->SetReactState(REACT_AGGRESSIVE);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) OVERRIDE
        {
            _summons.Despawn(summon);
        }

        void SummonInsects(Unit* victim)
        {
            for (uint32 i = 0; i < 5; ++i)
                if (Creature* insect = me->SummonCreature(NPC_FLESH_EATING_INSECT, victim->GetPositionX() + float(irand(-7, 7)), victim->GetPositionY() + float(irand(-7, 7)), victim->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 120000))
                    if (victim != me)
                        insect->AI()->AttackStart(victim);

        }

    private:
        EventMap _events;
        InstanceScript* _instance;
        SummonList _summons;
    };

    CreatureAI *GetAI(Creature* creature) const OVERRIDE
    {
        return new npc_putricide_insect_trapAI(creature);
    }
};

class npc_flesh_eating_insect : public CreatureScript
{
public:
    npc_flesh_eating_insect() : CreatureScript("npc_flesh_eating_insect") { }

    struct npc_flesh_eating_insectAI : public ScriptedAI
    {
        npc_flesh_eating_insectAI(Creature* creature) : ScriptedAI(creature)
        {
            _instance = creature->GetInstanceScript();
        }

        void EnterCombat(Unit* /*who*/) OVERRIDE
        {
            _events.ScheduleEvent(EVENT_FLESH_EATING_BITE, 2000);
        }

        void DamageTaken(Unit* /*done_by*/, uint32&) OVERRIDE { }

        void UpdateAI(uint32 diff) OVERRIDE
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                if (eventId == EVENT_FLESH_EATING_BITE)
                {
                    if (Unit* target = me->GetVictim())
                        DoCast(target, SPELL_FLESH_EATING_BITE, true);
                    _events.ScheduleEvent(EVENT_FLESH_EATING_BITE, 3000);
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap _events;
        InstanceScript* _instance;
    };

    CreatureAI *GetAI(Creature* creature) const OVERRIDE
    {
        return new npc_flesh_eating_insectAI(creature);
    }
};

/* ============================
==========The Rampart of Skulls ======
=============================*/

enum EventsRampart
{
	// Rampart of Skulls NPCs Events
	EVENT_WRATH = 1,
	EVENT_HEAL,

	EVENT_SUNDER_ARMOR,
	EVENT_SPELL_REFLECT,
	EVENT_THUNDERCLAP,
	EVENT_DEVASTATE,

	EVENT_FROST_BREATH,
	EVENT_BLIZZARD,
	EVENT_CLEAVE,

	// Skybreaker Vindicator
	EVENT_AVENGERS_SHILED,
	EVENT_CONCENTRATION,
	EVENT_HOLY_WRATH,

	// First Squad Assisted
	EVENT_FIRST_SQUAD_ASSISTED_1,
	EVENT_FIRST_SQUAD_ASSISTED_2,

	// Second Squad Assisted
	EVENT_SECOND_SQUAD_ASSISTED_1,
	EVENT_SECOND_SQUAD_ASSISTED_2,
	EVENT_SECOND_SQUAD_ASSISTED_3
};

enum SpellsRampart
{
    // Rampart of Skulls NPCs Spells
    // Kor'kron Primalist
    SPELL_WRATH                       = 69968,
    SPELL_HEALING_TOUCH               = 69899,
    SPELL_REGROWTH                    = 69882,
    SPELL_REJUVENATION                = 69898,
    SPELL_SUMMON_BATTLE_STANDART_A    = 69810,
    SPELL_SUMMON_BATTLE_STANDART_H    = 69811,

    // Kor'kron Defender
    SPELL_DEVASTATE                   = 69902,
    SPELL_THUNDERCLAP                 = 69965,
    SPELL_SUNDER_ARMORS               = 57807,
    SPELL_SPELL_REFLECT               = 69901,

    // Skybreaker Vindicator
    SPELL_AVENGERS_SHIELD             = 69927,
    SPELL_CONCENTRATION               = 69930,
    SPELL_HOLY_WRATH                  = 69934,

    // Frostwyrm
    SPELL_FROST_BREATH                = 70116,
    SPELL_BLIZZARD                    = 70362,
    SPELL_FROST_CLEAVE                = 70361
};

enum Texts_Trash
{
    // Kor'kron Primalist
    SAY_FIRST_SQUAD_RESCUED_HORDE_0  = 0,

    // Kor'kron Invoker
    SAY_FIRST_SQUAD_RESCUED_HORDE_1  = 0,

    // Kor'kron Defender
    SAY_SECOND_SQUAD_RESCUED_HORDE_0 = 0,
    SAY_SECOND_SQUAD_RESCUED_HORDE_1 = 1,
    SAY_FROSTWYRM_SUMMON_0           = 2,
    SAY_FROSTWYRM_SUMMON_1           = 3,

    // Skybreaker Vindicator
    SAY_FIRST_SQUAD_RESCUED_ALLIANCE_0  = 0,

    // Skybreaker Sorcerer
    SAY_FIRST_SQUAD_RESCUED_ALLIANCE_1  = 0,

    // Skybreaker Protector
    SAY_SECOND_SQUAD_RESCUED_ALLIANCE_0 = 0,
    SAY_SECOND_SQUAD_RESCUED_ALLIANCE_1 = 1,
    SAY_SECOND_SQUAD_RESCUED_ALLIANCE_2 = 2,

    // Kor'kron Invoker & Skybreaker Sorcerer
    SAY_SUMMON_BATTLE_STANDARD          = 1,

    // Froswyrm
    SAY_FROSTWYRM_LAND_H_0           = 0,
    SAY_FROSTWYRM_LAND_A_1           = 1,

};

enum Point
{
    POINT_TAKEOFF                    = 0,
};

Position const FrostWyrmPosH   = {-435.429f, 2077.556f, 219.1148f, 4.767166f};
Position const FrostWyrmPosA   = {-437.409f, 2349.026f, 219.1148f, 1.483120f};

/* ----------------------------------- Rampart of Skulls NPCs ----------------------------------- */

/* Kor'kron Primalist  37030*/
class npc_korkron_primalist: public CreatureScript
{
    public:
        npc_korkron_primalist() : CreatureScript("npc_korkron_primalist") { }

        struct npc_korkron_primalistAI : public ScriptedAI
        {
            npc_korkron_primalistAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset()
            {
                _started = instance->GetBossState(DATA_ICECROWN_GUNSHIP_BATTLE) == DONE;
                events.Reset();
                events.ScheduleEvent(EVENT_WRATH, 10000); // TODO: Fix the timers
                events.ScheduleEvent(EVENT_HEAL, 20000); // TODO: Fix the timers
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!instance)
                    return;

                if (instance->GetData(DATA_TEAM_IN_INSTANCE) != HORDE)
                    return;

                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (instance->GetData(DATA_FIRST_SQUAD_STATE) == DONE)
                    return;

                if (!_started && me->IsWithinDistInMap(who, 25.0f))
                {
                    _started = true;
                    events.ScheduleEvent(EVENT_FIRST_SQUAD_ASSISTED_1, 1000);
                    events.ScheduleEvent(EVENT_FIRST_SQUAD_ASSISTED_2, 15000); // TODO : fix the timer
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!instance)
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIRST_SQUAD_ASSISTED_1:
                            Talk(SAY_FIRST_SQUAD_RESCUED_HORDE_0);
                            break;
                        case EVENT_FIRST_SQUAD_ASSISTED_2:
                            if (Creature* tempUnit = me->FindNearestCreature(NPC_KORKRON_INVOKER, 120.0f, true))
                            {
                                tempUnit->AI()->Talk(SAY_FIRST_SQUAD_RESCUED_HORDE_1);
                                tempUnit->AI()->Talk(SAY_SUMMON_BATTLE_STANDARD);
                                tempUnit->CastSpell(tempUnit, SPELL_SUMMON_BATTLE_STANDART_H, true);
                            }
                            instance->SetData(DATA_FIRST_SQUAD_STATE, DONE);
                            break;
                        case EVENT_WRATH:
                            if (UpdateVictim())
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f))
                                    DoCast(target, SPELL_WRATH);
                            events.ScheduleEvent(EVENT_WRATH, 10000);
                            break;
                        case EVENT_HEAL:
                            if (UpdateVictim())
                            {
                                std::list<Unit*> TargetList;
                                Unit* finalTarget = me;
                                Trinity::AnyFriendlyUnitInObjectRangeCheck checker(me, me, 30.0f);
                                Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(me, TargetList, checker);
                                me->VisitNearbyObject(30.0f, searcher);
                                for (std::list<Unit*>::iterator itr = TargetList.begin(); itr != TargetList.end(); ++itr)
                                    if ((*itr)->GetHealthPct() < finalTarget->GetHealthPct())
                                        finalTarget = *itr;

                                uint32 spellId = SPELL_HEALING_TOUCH;
                                uint32 healthPct = uint32(finalTarget->GetHealthPct());
                                if (healthPct > 15 && healthPct < 20)
                                    spellId = (urand (0, 1) ? SPELL_REGROWTH : SPELL_HEALING_TOUCH);
                                else if (healthPct >= 20 && healthPct < 40)
                                    spellId = SPELL_REGROWTH;
                                else if (healthPct > 40)
                                    spellId = (urand(0, 1) ? SPELL_REJUVENATION : SPELL_REGROWTH);

                                me->CastSpell(finalTarget, spellId, false);
                                events.ScheduleEvent(EVENT_HEAL, 20000);
                            }
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                bool _started;
                EventMap events;
                InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_korkron_primalistAI(pCreature);
        }
};

/* Kor'kron Defender  37032*/
class npc_korkron_defender: public CreatureScript
{
    public:
        npc_korkron_defender() : CreatureScript("npc_korkron_defender") { }

        struct npc_korkron_defenderAI : public ScriptedAI
        {
            npc_korkron_defenderAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_DEVASTATE, 10000);
                events.ScheduleEvent(EVENT_THUNDERCLAP, 20000);
                events.ScheduleEvent(EVENT_SPELL_REFLECT, 25000);
                events.ScheduleEvent(EVENT_SUNDER_ARMOR, 15000);
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!instance)
                    return;

                if (instance->GetData(DATA_TEAM_IN_INSTANCE) != HORDE)
                    return;

                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (me->IsWithinDistInMap(who, 25.0f))
                {
                    if (instance->GetData(DATA_FIRST_SQUAD_STATE) == DONE && instance->GetData(DATA_SECOND_SQUAD_STATE) != DONE)
                    {
                        instance->SetData(DATA_SECOND_SQUAD_STATE, DONE);
                        events.ScheduleEvent(EVENT_SECOND_SQUAD_ASSISTED_1, 1000);
                        events.ScheduleEvent(EVENT_SECOND_SQUAD_ASSISTED_2, 15000); // TODO : fix the timer
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!instance)
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SECOND_SQUAD_ASSISTED_1:
                            Talk(SAY_SECOND_SQUAD_RESCUED_HORDE_0);
                            break;
                        case EVENT_SECOND_SQUAD_ASSISTED_2:
                            Talk(SAY_SECOND_SQUAD_RESCUED_HORDE_1);
                            Talk(SAY_FROSTWYRM_SUMMON_0);
                            break;
                        case EVENT_DEVASTATE:
                            if (UpdateVictim())
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                    DoCast(target, SPELL_DEVASTATE);
                            }
                            events.ScheduleEvent(EVENT_DEVASTATE, 10000);
                            break;
                        case EVENT_SPELL_REFLECT:
                            if (UpdateVictim())
                            {
                                DoCast(me, SPELL_SPELL_REFLECT);
                            }
                            events.ScheduleEvent(EVENT_SPELL_REFLECT, 25000);
                            break;
                        case EVENT_THUNDERCLAP:
                            if (UpdateVictim())
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                                    DoCast(target, SPELL_THUNDERCLAP);
                            }
                            events.ScheduleEvent(EVENT_THUNDERCLAP, 20000);
                            break;
                        case EVENT_SUNDER_ARMOR:
                            if (UpdateVictim())
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                                    DoCast(target, SPELL_SUNDER_ARMORS);
                            }
                            events.ScheduleEvent(EVENT_SUNDER_ARMOR, 15000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                EventMap events;
                InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_korkron_defenderAI(pCreature);
        }
};

/* Skybreaker Vindicator  37003*/
class npc_skybreaker_vindicator: public CreatureScript
{
    public:
        npc_skybreaker_vindicator() : CreatureScript("npc_skybreaker_vindicator") { }

        struct npc_skybreaker_vindicatorAI : public ScriptedAI
        {
            npc_skybreaker_vindicatorAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset()
            {
                _started = instance->GetBossState(DATA_ICECROWN_GUNSHIP_BATTLE) == DONE;
                events.Reset();
                events.ScheduleEvent(EVENT_AVENGERS_SHILED, 10000); // TODO: Fix the timers
                events.ScheduleEvent(EVENT_CONCENTRATION, 20000); // TODO: Fix the timers
                events.ScheduleEvent(EVENT_HOLY_WRATH, 20000); // TODO: Fix the timers
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!instance)
                    return;

                if (instance->GetData(DATA_TEAM_IN_INSTANCE) != ALLIANCE)
                    return;

                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (instance->GetData(DATA_FIRST_SQUAD_STATE) == DONE)
                    return;

                if (!_started && me->IsWithinDistInMap(who, 25.0f))
                {
                    _started = true;
                    events.ScheduleEvent(EVENT_FIRST_SQUAD_ASSISTED_1, 1000);
                    events.ScheduleEvent(EVENT_FIRST_SQUAD_ASSISTED_2, 15000); // TODO : fix the timer
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!instance)
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIRST_SQUAD_ASSISTED_1:
                            Talk(SAY_FIRST_SQUAD_RESCUED_ALLIANCE_0);
                            break;
                        case EVENT_FIRST_SQUAD_ASSISTED_2:
                            if (Creature* tempUnit = me->FindNearestCreature(NPC_SKYBREAKER_SORCERERS, 120.0f, true))
                            {
                                tempUnit->AI()->Talk(SAY_FIRST_SQUAD_RESCUED_ALLIANCE_1);
                                tempUnit->AI()->Talk(SAY_SUMMON_BATTLE_STANDARD);
                                tempUnit->CastSpell(tempUnit, SPELL_SUMMON_BATTLE_STANDART_A, true);
                            }
                            instance->SetData(DATA_FIRST_SQUAD_STATE, DONE);
                            break;
                        case EVENT_AVENGERS_SHILED:
                            if (UpdateVictim())
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f))
                                    DoCast(target, SPELL_WRATH);
                            events.ScheduleEvent(EVENT_AVENGERS_SHILED, 10000);
                            break;
                        case EVENT_CONCENTRATION:
                            if (UpdateVictim())
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f))
                                    DoCast(target, SPELL_CONCENTRATION);
                            events.ScheduleEvent(EVENT_CONCENTRATION, 20000);
                            break;
                        case EVENT_HOLY_WRATH:
                            if (UpdateVictim())
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f))
                                    DoCast(target, SPELL_HOLY_WRATH);
                            events.ScheduleEvent(EVENT_HOLY_WRATH, 15000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                bool _started;
                EventMap events;
                InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_skybreaker_vindicatorAI(pCreature);
        }
};

/* Skybreaker Protector  36998*/
class npc_skybreaker_protector: public CreatureScript
{
    public:
        npc_skybreaker_protector() : CreatureScript("npc_skybreaker_protector") { }

        struct npc_skybreaker_protectorAI : public ScriptedAI
        {
            npc_skybreaker_protectorAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_DEVASTATE, 10000);
                events.ScheduleEvent(EVENT_THUNDERCLAP, 20000);
                events.ScheduleEvent(EVENT_SPELL_REFLECT, 25000);
                events.ScheduleEvent(EVENT_SUNDER_ARMOR, 15000);
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!instance)
                    return;

                if (instance->GetData(DATA_TEAM_IN_INSTANCE) != ALLIANCE)
                    return;

                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (me->IsWithinDistInMap(who, 25.0f))
                {
                    if (instance->GetData(DATA_FIRST_SQUAD_STATE) == DONE && instance->GetData(DATA_SECOND_SQUAD_STATE) != DONE)
                    {
                        instance->SetData(DATA_SECOND_SQUAD_STATE, DONE);
                        events.ScheduleEvent(EVENT_SECOND_SQUAD_ASSISTED_1, 1000);
                        events.ScheduleEvent(EVENT_SECOND_SQUAD_ASSISTED_2, 11000); // TODO : fix the timer
                        events.ScheduleEvent(EVENT_SECOND_SQUAD_ASSISTED_3, 17000); // TODO : fix the timer
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!instance)
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SECOND_SQUAD_ASSISTED_1:
                            Talk(SAY_SECOND_SQUAD_RESCUED_ALLIANCE_0);
                            break;
                        case EVENT_SECOND_SQUAD_ASSISTED_2:
                            Talk(SAY_SECOND_SQUAD_RESCUED_ALLIANCE_1);
                            break;
                        case EVENT_SECOND_SQUAD_ASSISTED_3:
                            Talk(SAY_SECOND_SQUAD_RESCUED_ALLIANCE_2);
                            Talk(SAY_FROSTWYRM_SUMMON_1);
                            break;
                        case EVENT_DEVASTATE:
                            if (UpdateVictim())
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                    DoCast(target, SPELL_DEVASTATE);
                            }
                            events.ScheduleEvent(EVENT_DEVASTATE, 10000);
                            break;
                        case EVENT_SPELL_REFLECT:
                            if (UpdateVictim())
                            {
                                DoCast(me, SPELL_SPELL_REFLECT);
                            }
                            events.ScheduleEvent(EVENT_SPELL_REFLECT, 25000);
                            break;
                        case EVENT_THUNDERCLAP:
                            if (UpdateVictim())
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                                    DoCast(target, SPELL_THUNDERCLAP);
                            }
                            events.ScheduleEvent(EVENT_THUNDERCLAP, 20000);
                            break;
                        case EVENT_SUNDER_ARMOR:
                            if (UpdateVictim())
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                                    DoCast(target, SPELL_SUNDER_ARMORS);
                            }
                            events.ScheduleEvent(EVENT_SUNDER_ARMOR, 15000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                EventMap events;
                InstanceScript* instance;
        };
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_skybreaker_protectorAI(pCreature);
        }
};

/* Spire Frostwyrm  37230*/
class npc_icc_spire_frostwyrm: public CreatureScript
{
    public:
        npc_icc_spire_frostwyrm() : CreatureScript("npc_icc_spire_frostwyrm") { }

        struct npc_icc_spire_frostwyrmAI : public ScriptedAI
        {
            npc_icc_spire_frostwyrmAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            void Reset()
            {
                landed = false;
                events.Reset();
                me->setActive(true);
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                me->SetReactState(REACT_AGGRESSIVE);
                me->StopMoving();
                events.ScheduleEvent(EVENT_FROST_BREATH, 20000);
                events.ScheduleEvent(EVENT_BLIZZARD, 25000);
                events.ScheduleEvent(EVENT_CLEAVE, 10000);
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!instance)
                    return;

                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (!who->isTargetableForAttack())
                    return;

                if (!landed && me->IsWithinDistInMap(who, 35.0f))
                {
                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                        Talk(SAY_FROSTWYRM_LAND_H_0);
                    else
                        Talk(SAY_FROSTWYRM_LAND_A_1);
                    landed = true;
                    me->SetInCombatWith(who);
                    me->AddThreat(who, 1.0f);
                    me->GetMotionMaster()->MoveChase(who);
                }

                if (me->GetPositionZ() < 200.0f)
                {
                    me->setActive(false);
                    me->SetCanFly(false);
                    me->SetDisableGravity(false);
                    me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FROST_BREATH:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f))
                                DoCast(target, SPELL_FROST_BREATH);
                            events.ScheduleEvent(EVENT_FROST_BREATH, 20000);
                            break;
                        case EVENT_BLIZZARD:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f))
                                DoCast(target, SPELL_BLIZZARD);
                            events.ScheduleEvent(EVENT_BLIZZARD, 25000);
                            break;
                        case EVENT_CLEAVE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f))
                                DoCast(target, SPELL_FROST_CLEAVE);
                            events.ScheduleEvent(EVENT_CLEAVE, 10000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                bool landed;
                EventMap events;
                InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_icc_spire_frostwyrmAI(pCreature);
        }
};

/* ---------------------------------- AreaTrigger Scripts ------------------------------------- */
class at_icc_land_frostwyrm : public AreaTriggerScript
{
    public:
        at_icc_land_frostwyrm() : AreaTriggerScript("at_icc_land_frostwyrm") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
            {
                if (instance->GetData(DATA_SPIRE_FROSTWYRM_STATE) == NOT_STARTED && instance->GetData(DATA_SECOND_SQUAD_STATE) == DONE)
                {
                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                        player->GetMap()->SummonCreature(NPC_SPIRE_FROSTWYRM, FrostWyrmPosA);
                    else
                        player->GetMap()->SummonCreature(NPC_SPIRE_FROSTWYRM, FrostWyrmPosH);

                    instance->SetData(DATA_SPIRE_FROSTWYRM_STATE, IN_PROGRESS);
                }
            }
            return true;
        }
};

enum SpellsValkyrHerald
{
    SPELL_SEVERED_ESSENCE_10        = 71906,
    SPELL_SEVERED_ESSENCE_25        = 71942,
    //Severed essence spells
    SPELL_CLONE_PLAYER              = 57507,
    //Druid spells
    SPELL_CAT_FORM                  = 57655,
    SPELL_MANGLE                    = 71925,
    SPELL_RIP                       = 71926,
    //Warlock
    SPELL_CORRUPTION                = 71937,
    SPELL_SHADOW_BOLT               = 71936,
    SPELL_RAIN_OF_CHAOS             = 71965,
    //Shaman
    SPELL_REPLENISHING_RAINS        = 71956,
    SPELL_LIGHTNING_BOLT            = 71934,
    //Rouge
    SPELL_DISENGAGE                 = 57635,
    SPELL_FOCUSED_ATTACKS           = 71955,
    SPELL_SINISTER_STRIKE           = 57640,
    SPELL_EVISCERATE                = 71933,
    //Mage
    SPELL_FIREBALL                  = 71928,
    //Warior
    SPELL_BLOODTHIRST               = 71938,
    SPELL_HEROIC_LEAP               = 71961,
    //Dk
    SPELL_DEATH_GRIP                = 57602,
    SPELL_NECROTIC_STRIKE           = 71951,
    SPELL_PLAGUE_STRIKE             = 71924,
    //Priest
    SPELL_GREATER_HEAL              = 71931,
    SPELL_RENEW                     = 71932,
    //Paladin
    SPELL_CLEANSE                   = 57767,
    SPELL_FLASH_OF_LIGHT            = 71930,
    SPELL_RADIANCE_AURA             = 71953,
    //Hunter
    SPELL_SHOOT_10                  = 71927,
    SPELL_SHOOT_25                  = 72258,
};

enum EventsValkyrHerald
{
	// Val'kyr Herald
    EVENT_SEVERED_ESSENCE   = 1,

    // Druid spells
    EVENT_CAT_FORM,
    EVENT_MANGLE,
    EVENT_RIP,
    // Warlock
    EVENT_CORRUPTION,
    EVENT_SHADOW_BOLT,
    EVENT_RAIN_OF_CHAOS,
    // Shaman
    EVENT_REPLENISHING_RAINS,
    EVENT_LIGHTNING_BOLT,
    EVENT_CAN_CAST_REPLENISHING_RAINS,
    // Rogue
    EVENT_FOCUSED_ATTACKS,
    EVENT_SINISTER_STRIKE,
    EVENT_EVISCERATE,
    // Mage
    EVENT_FIREBALL,
    // Warrior
    EVENT_BLOODTHIRST,
    EVENT_HEROIC_LEAP,
    // Dead Knight
    EVENT_DEATH_GRIP,
    EVENT_NECROTIC_STRIKE,
    EVENT_PLAGUE_STRIKE,
    // Priest
    EVENT_GREATER_HEAL,
    EVENT_RENEW,
    // Paladin
    EVENT_CLEANSE,
    EVENT_FLASH_OF_LIGHT,
    EVENT_RADIANCE_AURA,
    EVENT_CAN_CAST_RADIANCE_AURA,
    // Hunter
    EVENT_SHOOT,
    EVENT_DISENGAGE,
    EVENT_CAN_CAST_DISENGAGE
};

#define DATA_PLAYER_CLASS		2

class npc_val_kyr_herald : public CreatureScript
{
public:
    npc_val_kyr_herald() : CreatureScript("npc_val_kyr_herald") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_val_kyr_heraldAI (pCreature);
    }

    struct npc_val_kyr_heraldAI : public ScriptedAI
    {
        npc_val_kyr_heraldAI(Creature *c) : ScriptedAI(c), summons(c) { }

        void Reset()
        {
			summons.DespawnAll();
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
			DoZoneInCombat();
            events.ScheduleEvent(EVENT_SEVERED_ESSENCE, 20000);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

		void JustDied(Unit* /*killer*/)
		{
            summons.DespawnAll();
		}

        void SpellHitTarget(Unit* target, SpellInfo const* /*spell*/)
        {
            if (!target || target->GetTypeId() != TYPEID_PLAYER || !target->IsAlive())
                return;

            // Summon clone
            if (Creature* essence = me->SummonCreature(NPC_SEVERED_ESSENCE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(),TEMPSUMMON_CORPSE_DESPAWN, 0))
				essence->AI()->SetGUID(target->GetGUID());
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SEVERED_ESSENCE:
                    {
						DoCastAOE(RAID_MODE<uint32>(SPELL_SEVERED_ESSENCE_10, SPELL_SEVERED_ESSENCE_25, SPELL_SEVERED_ESSENCE_10, SPELL_SEVERED_ESSENCE_25));
                        events.ScheduleEvent(EVENT_SEVERED_ESSENCE, 60000);
                        break;
                    }
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
        private:
            EventMap events;
            SummonList summons;
    };

};

class npc_severed_essence : public CreatureScript
{
public:
    npc_severed_essence() : CreatureScript("npc_severed_essence") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_severed_essenceAI(pCreature);
    }

    struct npc_severed_essenceAI : public ScriptedAI
    {
        npc_severed_essenceAI(Creature *c) : ScriptedAI(c)
        {
            instance = me->GetInstanceScript();
            playerClass = 0;
			playerGUID = 0;
        }

        void Reset()
        {
            events.Reset();
        }

        void KilledUnit(Unit* /*victim*/)
        {
            if (Unit *newVictim = SelectTarget(SELECT_TARGET_RANDOM, 0, -5.0f))
                AttackStart(newVictim);
        }

		void SetGUID(uint64 guid, int32 /*data*/)
		{
			playerGUID = guid;
			DoZoneInCombat();
		}

        void EnterCombat(Unit* /*who*/)
        {
			if (!playerGUID)
				return;

			if (Unit* player = sObjectAccessor->GetPlayer(*me, playerGUID))
            {
                player->CastSpell(me, SPELL_CLONE_PLAYER, true);
                SetData(DATA_PLAYER_CLASS, player->getClass());
            }
            else
            {
                me->DespawnOrUnsummon();
                return;
            }

            ASSERT(playerClass != 0);
			EventsInitializing();
        }

		void EventsInitializing()
		{
			events.Reset();

			switch (playerClass)
			{
			case CLASS_DRUID:
				{
					events.ScheduleEvent(EVENT_CAT_FORM, 100);
					events.ScheduleEvent(EVENT_MANGLE, 3000);
					events.ScheduleEvent(EVENT_RIP, 8000);
					break;
				}
			case CLASS_WARLOCK:
				{
					events.ScheduleEvent(EVENT_CORRUPTION, 100);
					events.ScheduleEvent(EVENT_SHADOW_BOLT, 3000);
					events.ScheduleEvent(EVENT_RAIN_OF_CHAOS, 8000);
					break;
				}
			case CLASS_SHAMAN:
				{
					events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 3000);
					bCanCastReplenishingRains = true;
					break;
				}
			case CLASS_ROGUE:
				{
					events.ScheduleEvent(EVENT_FOCUSED_ATTACKS, 10000);
					events.ScheduleEvent(EVENT_SINISTER_STRIKE, 2000);
					events.ScheduleEvent(EVENT_EVISCERATE, 8000);
					break;
				}
			case CLASS_MAGE:
				{
					events.ScheduleEvent(EVENT_FIREBALL, 100);
					break;
				}
			case CLASS_WARRIOR:
				{
					events.ScheduleEvent(EVENT_BLOODTHIRST, 5000);
					events.ScheduleEvent(EVENT_HEROIC_LEAP, 8000);
					break;
				}
			case CLASS_DEATH_KNIGHT:
				{
					events.ScheduleEvent(EVENT_DEATH_GRIP, 100);
					events.ScheduleEvent(EVENT_NECROTIC_STRIKE, 4000);
					events.ScheduleEvent(EVENT_PLAGUE_STRIKE, 7000);
					break;
				}
			case CLASS_HUNTER:
				{
					events.ScheduleEvent(EVENT_SHOOT, 100);
					bCanCastDisengage = true;
					break;
				}
			case CLASS_PALADIN:
				{
					events.ScheduleEvent(EVENT_FLASH_OF_LIGHT, 100);
					events.ScheduleEvent(EVENT_CLEANSE, 3000);
					bCanCastRadianceAura = true;
					break;
				}
			case CLASS_PRIEST:
				{
					events.ScheduleEvent(EVENT_GREATER_HEAL, 100);
					events.ScheduleEvent(EVENT_RENEW, 1000);
					break;
				}
			default:
				break;
			}
		}

        void DamageTaken(Unit* /*done_by*/, uint32& /*damage*/)
        {
            switch (playerClass)
            {
                case CLASS_SHAMAN:
                {
                    if (HealthBelowPct(30) && bCanCastReplenishingRains)
                    {
                        events.ScheduleEvent(EVENT_REPLENISHING_RAINS, 100);
                        events.ScheduleEvent(EVENT_CAN_CAST_REPLENISHING_RAINS, 15000);
                        bCanCastReplenishingRains = false;
                    }
                }
                case CLASS_HUNTER:
                {
                    if (!bCanCastDisengage)
                        break;
					
                    ThreatContainer::StorageType const &players = me->getThreatManager().getThreatList();
                    if (players.empty())
                        break;

                    for (ThreatContainer::StorageType::const_iterator it = players.begin(); it != players.end(); ++it)
                    {
                        if (Unit *curTarget = (*it)->getTarget())
                        {
                            if (me->GetDistance2d(curTarget) < 5.0f)
                            {
                                me->SetFacingToObject(curTarget);
                                DoCast(curTarget, SPELL_DISENGAGE);
                                bCanCastDisengage = false;
                                events.ScheduleEvent(EVENT_CAN_CAST_DISENGAGE, 15000);
                                break;
                            }
                        }
                    }
                    break;
                }
                case CLASS_PALADIN:
                    if (HealthBelowPct(30) && bCanCastRadianceAura)
                    {
                        events.ScheduleEvent(EVENT_RADIANCE_AURA, 100);
                        events.ScheduleEvent(EVENT_CAN_CAST_RADIANCE_AURA, 15000);
                        bCanCastRadianceAura = false;
                    }
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            if (type == DATA_PLAYER_CLASS)
                playerClass = data;
        }

		void DoCastByEssence(uint32 spell)
		{
			Unit* valkyr = me->FindNearestCreature(NPC_VALKYR_HERALD, 40.0f);
			if (valkyr && urand(0, 1))
				DoCast(valkyr, spell);
			else
			{
				Unit* target = NULL;
                Trinity::MostHPMissingInRange u_check(me, 40.0f, NPC_SEVERED_ESSENCE);
                Trinity::UnitLastSearcher<Trinity::MostHPMissingInRange> searcher(me, target, u_check);
				me->VisitNearbyObject(40.0f, searcher);
				if (target)
					DoCast(target, spell);
				else if (valkyr)
					DoCast(valkyr, spell);
			}
		}

        void HandleDruidEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_CAT_FORM:
                    DoCast(me, SPELL_CAT_FORM);
                    break;
                case EVENT_MANGLE:
                    DoCast(me->GetVictim(), SPELL_MANGLE);
                    events.ScheduleEvent(EVENT_MANGLE, 2000);
                    break;
                case EVENT_RIP:
                    DoCast(me->GetVictim(), SPELL_RIP);
                    events.ScheduleEvent(EVENT_RIP, 8000);
                    break;
            }
        }

        void HandleWarlockEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_CORRUPTION:
                    if (Unit *pUnit = SelectTarget(SELECT_TARGET_RANDOM, 1, -5.0f, true, -SPELL_CORRUPTION))
                        DoCast(pUnit, SPELL_CORRUPTION);
                    events.ScheduleEvent(EVENT_CORRUPTION, 20000);
                    break;
                case EVENT_SHADOW_BOLT:
                    DoCast(me->GetVictim(), SPELL_SHADOW_BOLT);
                    events.ScheduleEvent(EVENT_SHADOW_BOLT, 5000);
                    break;
                case EVENT_RAIN_OF_CHAOS:
                    DoCast(me->GetVictim(), SPELL_RAIN_OF_CHAOS);
                    events.ScheduleEvent(EVENT_RAIN_OF_CHAOS, 18000);
                    break;
            }
        }

        void HandleShamanEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_CAN_CAST_REPLENISHING_RAINS:
                    bCanCastReplenishingRains = true;
                    break;
                case EVENT_REPLENISHING_RAINS:
                    DoCast(me, SPELL_REPLENISHING_RAINS);
                    break;
                case EVENT_LIGHTNING_BOLT:
                    DoCast(me->GetVictim(), SPELL_LIGHTNING_BOLT);
                    events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 4000);
                    break;

            }
        }

        void HandleRogueEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_FOCUSED_ATTACKS:
                    if (Unit *pUnit = SelectTarget(SELECT_TARGET_RANDOM, 1, 5.0f, true, -SPELL_FOCUSED_ATTACKS))
                        DoCast(pUnit, SPELL_FOCUSED_ATTACKS);
                    events.ScheduleEvent(EVENT_FOCUSED_ATTACKS, 15000);
                    break;
                case EVENT_SINISTER_STRIKE:
                    DoCast(me->GetVictim(), SPELL_SINISTER_STRIKE);
                    events.ScheduleEvent(EVENT_SINISTER_STRIKE, 1000);
                    break;
                case EVENT_EVISCERATE:
                    DoCast(me->GetVictim(), SPELL_EVISCERATE);
                    events.ScheduleEvent(EVENT_EVISCERATE, 6000);
                    break;
            }
        }

        void HandleMageEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_FIREBALL:
                    DoCast(me->GetVictim(), SPELL_FIREBALL);
                    events.ScheduleEvent(EVENT_FIREBALL, 3500);
                    break;
            }
        }

        void HandleWarriorEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_BLOODTHIRST:
                {
                    if (Unit *pUnit = SelectTarget(SELECT_TARGET_RANDOM, 0, 5.0f, true))
                        DoCast(pUnit, SPELL_BLOODTHIRST);
                    events.ScheduleEvent(EVENT_BLOODTHIRST, 12000);
                    break;
                }
                case EVENT_HEROIC_LEAP:
                {
                    if (Unit *pUnit = SelectTarget(SELECT_TARGET_RANDOM, 1, 8.0f, true))
                        DoCast(pUnit, SPELL_HEROIC_LEAP);
                    events.ScheduleEvent(EVENT_HEROIC_LEAP, 45000);
                    break;
                }
            }
        }

        void HandleDeathKnightEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_DEATH_GRIP:
                {
                    if (Unit *pUnit = SelectTarget(SELECT_TARGET_RANDOM, 0, -5.0f, true))
                    {
                        DoCast(pUnit, EVENT_DEATH_GRIP);
                        me->getThreatManager().clearReferences();
                        AttackStart(pUnit);
                    }
                    events.ScheduleEvent(EVENT_DEATH_GRIP, 35000);
                    break;
                }
                case EVENT_NECROTIC_STRIKE:
                {
                    DoCast(me->GetVictim(), SPELL_NECROTIC_STRIKE);
                    events.ScheduleEvent(EVENT_NECROTIC_STRIKE, 6000);
                    break;
                }
                case EVENT_PLAGUE_STRIKE:
                {
                    if (Unit *pUnit = SelectTarget(SELECT_TARGET_RANDOM, 0, 5.0f, true))
                        DoCast(pUnit, SPELL_PLAGUE_STRIKE);
                    events.ScheduleEvent(EVENT_PLAGUE_STRIKE, 7000);
                    break;
                }

            }
        }

        void HandleHunterEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_CAN_CAST_DISENGAGE:
                {
                    bCanCastDisengage = true;
                    break;
                }
                case EVENT_SHOOT:
                {
                    DoCast(me->GetVictim(), RAID_MODE<uint32>(SPELL_SHOOT_10, SPELL_SHOOT_25, SPELL_SHOOT_10, SPELL_SHOOT_25));
                    events.ScheduleEvent(EVENT_SHOOT, 2000);
                    break;
                }
            }
        }

        void HandlePaladinEvents()
        {
            switch (events.ExecuteEvent())
            {
                case EVENT_CAN_CAST_RADIANCE_AURA:
                {
                    bCanCastRadianceAura = true;
                    break;
                }
                case EVENT_FLASH_OF_LIGHT:
                {
					DoCastByEssence(SPELL_FLASH_OF_LIGHT);
                    events.ScheduleEvent(EVENT_FLASH_OF_LIGHT, 5000);
                    break;
                }
                case EVENT_CLEANSE:
                {
                    if (Creature* valkyr = me->FindNearestCreature(NPC_VALKYR_HERALD, 30.0f))
                        DoCast(valkyr, SPELL_CLEANSE);
                    events.ScheduleEvent(EVENT_CLEANSE, 5000);
                    break;
                }
                case EVENT_RADIANCE_AURA:
                {
                    DoCast(me, SPELL_RADIANCE_AURA);
                    break;
                }
            }
        }

        void HandlePriestEvents()
        {
           
            switch (events.ExecuteEvent())
            {
                case EVENT_RENEW:
                {
					Creature* valkyr = me->FindNearestCreature(NPC_VALKYR_HERALD, 40.0f);
                    if (valkyr && urand(0, 1))
                        DoCast(valkyr, SPELL_RENEW);
                    else
                    {
                        std::list<Creature*> others;
                        GetCreatureListWithEntryInGrid(others, me, NPC_SEVERED_ESSENCE, 40.0f);
                        Unit* target = NULL;
                        for (std::list<Creature*>::const_iterator itr = others.begin(); itr != others.end(); ++itr)
						{
                            if ((*itr) && !(*itr)->HasAura(SPELL_RENEW))
                                target = (*itr);
						}
                        if (!target)
                            target = valkyr;
                        if (target)
                            DoCast(target, SPELL_RENEW);
                    }
                    events.ScheduleEvent(EVENT_RENEW, 5000);
                    break;
                }
                case EVENT_GREATER_HEAL:
                {
					DoCastByEssence(SPELL_GREATER_HEAL);
                    events.ScheduleEvent(EVENT_GREATER_HEAL, 5000);
                    break;
                }
            }
        }


        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            switch (playerClass)
            {
                case CLASS_DRUID:
                    HandleDruidEvents();
                    break;
                case CLASS_WARLOCK:
                    HandleWarlockEvents();
					break;
                case CLASS_SHAMAN:
                    HandleShamanEvents();
					break;
                case CLASS_ROGUE:
                    HandleRogueEvents();
					break;
                case CLASS_MAGE:
                    HandleMageEvents();
					break;
                case CLASS_WARRIOR:
                    HandleWarriorEvents();
					break;
                case CLASS_DEATH_KNIGHT:
                    HandleDeathKnightEvents();
					break;
                case CLASS_HUNTER:
                    HandleHunterEvents();
					break;
                case CLASS_PALADIN:
                    HandlePaladinEvents();
					break;
                case CLASS_PRIEST:
                    HandlePriestEvents();
					break;
                default:
                    break;
            }
            DoMeleeAttackIfReady();
        }
        private:
            uint8 playerClass;
            InstanceScript *instance;
            EventMap events;
			uint64 playerGUID;
            bool bCanCastReplenishingRains;
            bool bCanCastDisengage;
            bool bCanCastRadianceAura;
    };
};

enum NerubAr_BroodKeeper
{
    SPELL_DARK_MENDING_N = 72322,
    SPELL_DARK_MENDING_HC = 72324,
    SPELL_CRYPT_SCARABS = 70965,
    SPELL_WEB_WRAP = 70980,
    SPELL_WEB_WRAP_DEBUFF = 71010,
};

class npc_nerubar_broodkeeper : public CreatureScript
{
public:
    npc_nerubar_broodkeeper() : CreatureScript("npc_nerubar_broodkeeper") {}

    struct npc_nerubar_broodkeeperAI : public ScriptedAI
    {
        npc_nerubar_broodkeeperAI(Creature* creature) : ScriptedAI(creature)
        {
            active = false;
        }

        uint32 cryptscarabtimer;
        uint32 webwraptimer;
        uint32 darkmendingtimer;
        uint32 checkTimer;
        bool active;

        void UpdateAI(uint32 diff) OVERRIDE
        {
            if (!active)
            {
                if (checkTimer <= diff)
                {
                    CheckActive();
                    checkTimer = 1000;
                }
                else checkTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (cryptscarabtimer <= diff)
            {
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                {
                    if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        me->CastSpell(victim, SPELL_CRYPT_SCARABS);
                    cryptscarabtimer = urand(3000, 5000); // should spam cast (source: youtube)
                }
                else
                    cryptscarabtimer = 1; // cast it next
            }
            else cryptscarabtimer -= diff;

            if (webwraptimer <= diff)
            {
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                {
                    if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true, -71010))
                        if (!victim->HasAura(SPELL_WEB_WRAP))
                            me->CastSpell(victim, SPELL_WEB_WRAP);
                    webwraptimer = urand(5000, 9000);
                }
                else
                    webwraptimer = 1; // cast it next
            }
            else webwraptimer -= diff;

            if (darkmendingtimer <= diff)
            {
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                {
                    HealFriendly();
                    darkmendingtimer = urand(8000, 13000);
                }
                else
                    darkmendingtimer = 1; // cast it next
            }
            else darkmendingtimer -= diff;
        }

        void CheckActive()
        {
            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* player = i->GetSource())
                {
                    if (me->GetDistance2d(player) < 80.0f)
                    {
                        active = true;
                        me->CastSpell(me, SPELL_WEB_BEAM, false);
                        me->GetMotionMaster()->MoveCharge(me->GetPositionX(), me->GetPositionY(), me->GetMap()->GetHeight(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), true, 50.0f), 4.5f, 0);

                        Position HomePos;
                        me->GetPosition(&HomePos);
                        HomePos.m_positionZ = me->GetMap()->GetHeight(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), true, 50.0f);
                        me->SetHomePosition(HomePos);
                    }
                }
            }
        }

        void HealFriendly()
        {
            if (Unit* target = DoSelectLowestHpFriendly(30.0f, 100000))
            {
                if (me->GetInstanceScript() && (me->GetMap()->GetSpawnMode() & 0 || me->GetMap()->GetSpawnMode() & 1))
                    me->CastSpell(target, SPELL_DARK_MENDING_N);
                else
                    me->CastSpell(target, SPELL_DARK_MENDING_HC);
            }
        }

        void Reset() OVERRIDE
        {
            if (!active)
            {
                me->SetCanFly(true);
                me->SetHover(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_PC);
            }
            else
            {
                me->SetCanFly(false);
                me->SetHover(false);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_PC);
            }

            cryptscarabtimer = urand(1000, 3000);
            webwraptimer = urand(6000, 8000);
            darkmendingtimer = urand(10000, 15000);
            checkTimer = 1000;
        }

        void MovementInform(uint32 type, uint32 id) OVERRIDE
        {
            switch (id)
            {
            case 0: // on the ground
                me->SetCanFly(false);
                me->SetHover(false);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_PC);
                me->SetOrientation(0.0f);
                break;
            default:
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const OVERRIDE
    {
        return new npc_nerubar_broodkeeperAI(creature);
    }
};

class spell_icc_buff_pet : public SpellScriptLoader
{
public:
    spell_icc_buff_pet() : SpellScriptLoader("spell_icc_buff_pet") { }

    class spell_icc_buff_pet_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_icc_buff_pet_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Player* player = GetCaster()->ToPlayer())
            {
                if (Pet* pet = player->GetPet())
                {
                    if (uint32 buff = (player->ToPlayer()->GetTeam() == HORDE ? 73822 : 73828))
                        pet->AddAura(buff, pet);
                }
            }
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Player* player = GetCaster()->ToPlayer())
            {
                if (Pet* pet = player->GetPet())
                {
                    if (uint32 buff = (player->ToPlayer()->GetTeam() == HORDE ? 73822 : 73828))
                        pet->RemoveAura(buff);
                }
            }
        }

        void Register() OVERRIDE
        {
            OnEffectApply += AuraEffectApplyFn(spell_icc_buff_pet_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(spell_icc_buff_pet_AuraScript::OnApply, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            OnEffectApply += AuraEffectApplyFn(spell_icc_buff_pet_AuraScript::OnApply, EFFECT_2, SPELL_AURA_MOD_HEALING_DONE_PERCENT, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_icc_buff_pet_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_icc_buff_pet_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_icc_buff_pet_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_MOD_HEALING_DONE_PERCENT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const OVERRIDE
    {
        return new spell_icc_buff_pet_AuraScript();
    }
};

void AddSC_icecrown_citadel()
{
    new npc_highlord_tirion_fordring_lh();
    new npc_rotting_frost_giant();
    new npc_frost_freeze_trap();
    new npc_alchemist_adrianna();
    new boss_sister_svalna();
    new npc_crok_scourgebane();
    new npc_captain_arnath();
    new npc_captain_brandon();
    new npc_captain_grondel();
    new npc_captain_rupert();
    new npc_frostwing_vrykul();
    new npc_impaling_spear();
    new npc_arthas_teleport_visual();
    new spell_icc_stoneform();
    new spell_icc_sprit_alarm();
    new spell_frost_giant_death_plague();
    new spell_icc_harvest_blight_specimen();
    new spell_trigger_spell_from_caster("spell_svalna_caress_of_death", SPELL_IMPALING_SPEAR_KILL);
    new spell_svalna_revive_champion();
    new spell_svalna_remove_spear();
    new spell_icc_soul_missile();
    new at_icc_saurfang_portal();
    new at_icc_shutdown_traps();
    new at_icc_start_blood_quickening();
    new at_icc_start_frostwing_gauntlet();
    new at_icc_start_sindragosa_gauntlet();
    new npc_sindragosas_ward();
    new npc_deathbound_ward();
    new npc_flesh_eating_insect();
    new npc_putricide_insect_trap();
    new npc_val_kyr_herald();
    new npc_severed_essence();
    new npc_korkron_primalist();
    new npc_korkron_defender();
    new npc_skybreaker_vindicator();
    new npc_skybreaker_protector();
    new npc_icc_spire_frostwyrm();
    new at_icc_land_frostwyrm();
    new npc_nerubar_broodkeeper();
    new spell_icc_buff_pet();
}
