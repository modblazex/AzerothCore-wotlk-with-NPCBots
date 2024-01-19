enum Spells
{
    SPELL_FROGGER_EXPLODE = 825699,
};

constexpr float CHECK_RANGE = 0.5f;
constexpr float TOTAL_DISTANCE = 83.0f;
constexpr float INCREMENT = 1.5f;

class npc_hallowed_frogger : public CreatureScript
{
public:
    npc_hallowed_frogger() : CreatureScript("npc_hallowed_frogger") { }

    struct npc_hallowed_froggerAI : public ScriptedAI
    {
        npc_hallowed_froggerAI(Creature* creature) : ScriptedAI(creature), motionMaster(creature->GetMotionMaster()) { }

        float distanceMoved = 0.0f;
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

        void CheckAndExplode()
        {
            Unit* target = nullptr;
            Acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, CHECK_RANGE);
            Acore::UnitLastSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, target, u_check);
            Cell::VisitAllObjects(me, searcher, CHECK_RANGE);

            if (target && target->GetTypeId() == TYPEID_PLAYER && !target->ToPlayer()->IsNPCBotOrPet())
            {
                float distance = me->GetExactDist(target); // Using GetExactDist to get the exact distance between centers
                float playerCombatReach = target->GetCombatReach();

                if (distance <= (playerCombatReach - 0.615f))  // How far to encroach on player's hitbox/combat reach
                {
                    me->CastSpell(me, SPELL_FROGGER_EXPLODE, true);
                }
            }
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

            CheckAndExplode();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hallowed_froggerAI(creature);
    }
};

void AddSC_hallowed_frogger()
{
    new npc_hallowed_frogger();
}
