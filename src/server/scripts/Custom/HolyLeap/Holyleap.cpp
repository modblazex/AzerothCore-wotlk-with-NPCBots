#include "ScriptMgr.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "EventMap.h"

const uint32 SPELL_HOLY_LEAP_EFFECT = 53385;

class npc_straight_line_walker : public CreatureScript
{
public:
    npc_straight_line_walker() : CreatureScript("npc_straight_line_walker") {}

    struct npc_straight_line_walkerAI : public ScriptedAI
    {
        explicit npc_straight_line_walkerAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        {
            // Set the creature to passive
            me->SetReactState(REACT_PASSIVE);

            // Cast the spell immediately on spawn
            me->CastSpell(me, SPELL_HOLY_LEAP_EFFECT, true);

            // Set up a repeating event to cast the spell every 500 ms
            events.ScheduleEvent(1, 500);

            // Make the creature wander within a 5.0f radius
            me->GetMotionMaster()->MoveRandom(5.0f);

            // Despawn the creature after 3 seconds
            me->DespawnOrUnsummon(3000);
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case 1:
                    me->CastSpell(me, SPELL_HOLY_LEAP_EFFECT, true);
                    events.ScheduleEvent(1, 500);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_straight_line_walkerAI(creature);
    }
};

void AddSC_npc_straight_line_walker()
{
    new npc_straight_line_walker();
}
