#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "MotionMaster.h"

constexpr float TOTAL_DISTANCE = 83.0f;
constexpr float INCREMENT = 2.5f;

class npc_flame_tsunami : public CreatureScript
{
public:
    npc_flame_tsunami() : CreatureScript("npc_flame_tsunami") { }

    struct npc_flame_tsunamiAI : public ScriptedAI
    {
        npc_flame_tsunamiAI(Creature* creature) : ScriptedAI(creature), motionMaster(creature->GetMotionMaster())
        {
            distanceMoved = 0.0f;
        }

        float distanceMoved;
        MotionMaster* motionMaster;

        void Reset() override
        {
            distanceMoved = 0.0f;
            MoveIncrement();
        }

        void MoveIncrement()
        {
            if (distanceMoved >= TOTAL_DISTANCE)
                return;

            float x, y, z, o;
            me->GetPosition(x, y, z, o);

            float targetX = x + INCREMENT * cos(o);
            float targetY = y + INCREMENT * sin(o);

            me->GetMotionMaster()->MovePoint(0, targetX, targetY, z);  // Move to the new point
            distanceMoved += INCREMENT;
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (motionMaster->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
            {
                if (distanceMoved < TOTAL_DISTANCE)
                {
                    MoveIncrement();
                }
                else
                {
                    me->DespawnOrUnsummon(500);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_flame_tsunamiAI(creature);
    }
};

void AddSC_flame_tsunami()
{
    new npc_flame_tsunami();
}
