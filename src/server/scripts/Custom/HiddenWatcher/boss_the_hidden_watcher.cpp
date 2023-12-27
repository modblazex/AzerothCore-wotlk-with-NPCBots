#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "InstanceScript.h"

enum WatcherSpells
{
    SPELL_AOE_RAID = 15237,
    SPELL_RANDOM_PLAYER = 67890
};

enum WatcherSay
{
    SAY_INTRO_0 = 0,
    SAY_INTRO_1 = 1,
    SAY_INTRO_2 = 2
};

enum WatcherEvents
{
    EVENT_INTRO = 1,
    EVENT_INTRO_1,
    EVENT_INTRO_2,
    EVENT_START_FIGHT,
    EVENT_SUMMON_TENTACLES,
    EVENT_SUMMON_HERALDS,
    EVENT_DESPAWN_PORTALS,
    EVENT_CAST_AOE,
    EVENT_CAST_RANDOM_PLAYER
};

std::vector<Position> const tentaclePositions = {
    {-8971.02f, 1253.79f, -112.29f, 1.76f}, // Tentacle 1 Front right
    {-8946.07f, 1258.44f, -112.29, 1.07f}, // Tentacle 2 Front left
    {-8946.07f, 1258.44f, -112.29, 1.07f}, // Tentacle 3
    {-8946.07f, 1258.44f, -112.29, 1.07f}, // Tentacle 4
    {-8946.07f, 1258.44f, -112.29, 1.07f}, // Tentacle 5
    {-8946.07f, 1258.44f, -112.29, 1.07f}  // Tentacle 6
};

std::vector<Position> const heraldPositions = {
    {-8928.85f, 1186.05f, -112.29f, 1.68f}, // Herald 1
    {-8953.15f, 1181.83f, -112.29f, 1.68f}  // Herald 2
};

std::vector<Position> const portalPositions = {
    {-8928.85f, 1186.05f, -112.29f, 1.68f}, // Portal 1
    {-8953.15f, 1181.83f, -112.29f, 1.68f}  // Portal 2
};

struct boss_the_hidden_watcher : public ScriptedAI
{
    boss_the_hidden_watcher(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void Reset() override
    {
        events.ScheduleEvent(EVENT_INTRO, 5s);
        events.ScheduleEvent(EVENT_CAST_RANDOM_PLAYER, 15s); // Schedule the event to cast random spells
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        ResetEncounter();
        ScriptedAI::EnterEvadeMode();
    }

    void ResetEncounter()
    {
        summons.DespawnAll();
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO:
                Talk(SAY_INTRO_0);
                events.ScheduleEvent(EVENT_INTRO_1, 5s);
                break;
            case EVENT_INTRO_1:
                Talk(SAY_INTRO_1);
                events.ScheduleEvent(EVENT_INTRO_2, 5s);
                break;
            case EVENT_INTRO_2:
                Talk(SAY_INTRO_2);
                events.ScheduleEvent(EVENT_START_FIGHT, 5s);
                break;
            case EVENT_START_FIGHT:
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                for (const auto& pos : tentaclePositions)
                    me->SummonCreature(815725, pos);
                events.ScheduleEvent(EVENT_SUMMON_TENTACLES, 30s);
                events.ScheduleEvent(EVENT_SUMMON_HERALDS, 10s);
                break;
            case EVENT_SUMMON_TENTACLES:
                for (const auto& pos : tentaclePositions)
                    me->SummonCreature(815725, pos);
                events.Repeat(30s);
                break;
            case EVENT_SUMMON_HERALDS:
                for (const auto& pos : heraldPositions)
                    me->SummonCreature(815728, pos);
                for (const auto& pos : portalPositions)
                    me->SummonCreature(24925, pos);
                events.ScheduleEvent(EVENT_DESPAWN_PORTALS, 3s);
                events.Repeat(45s);
                break;
            case EVENT_DESPAWN_PORTALS:
                summons.DespawnEntry(24925);
                break;
            case EVENT_CAST_RANDOM_PLAYER:
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                {
                    DoCast(target, SPELL_RANDOM_PLAYER);
                }
                events.Repeat(15s);
                break;
            }
        }
    }

private:
    EventMap events;
    SummonList summons;
};

void AddSC_boss_the_hidden_watcher()
{
    RegisterCreatureAI(boss_the_hidden_watcher);
}
