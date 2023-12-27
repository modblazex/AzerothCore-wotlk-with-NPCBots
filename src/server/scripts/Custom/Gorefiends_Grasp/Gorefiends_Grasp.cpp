#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Unit.h"
#include "GridNotifiers.h"
#include "Creature.h"

// 1190009 - Gorefiend's Grasp (grip effect)
class spell_dk_gorefiends_grasp : public SpellScript
{
    PrepareSpellScript(spell_dk_gorefiends_grasp);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
        {
            Unit* caster = GetCaster();
            std::list<Unit*> targetList;

            // Check if the target is a boss
            bool isTargetBoss = target->ToCreature() && target->ToCreature()->GetCreatureTemplate()->rank == 3;

            if (isTargetBoss)
            {
                // Target is a boss: pull units friendly to the boss, hostile to the player
                Acore::AnyFriendlyUnitInObjectRangeCheck checker(target, target, 15.0f);
                Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck> searcher(target, targetList, checker);
                Cell::VisitAllObjects(target, searcher, 15.0f);
            }
            else if (target->IsFriendlyTo(caster))
            {
                // Target is friendly to the caster: pull unfriendly units to the friendly target
                Acore::AnyUnfriendlyUnitInObjectRangeCheck checker(target, caster, 15.0f);
                Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(target, targetList, checker);
                Cell::VisitAllObjects(target, searcher, 15.0f);
            }
            else
            {
                // Target is unfriendly to the caster: pull units friendly to the unfriendly target
                Acore::AnyFriendlyUnitInObjectRangeCheck checker(target, target, 15.0f);
                Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck> searcher(target, targetList, checker);
                Cell::VisitAllObjects(target, searcher, 15.0f);
            }

            for (Unit* unit : targetList)
            {
                // Check if the unit has the DISABLE_MOVE and PACIFIED flags
                if (unit->HasUnitFlag(UNIT_FLAG_DISABLE_MOVE) || unit->HasUnitFlag(UNIT_FLAG_PACIFIED))
                    continue;

                // Exclude the boss from being gripped
                if (isTargetBoss && unit == target)
                    continue;

                // Casting the spell 1190010 on the main target from each unit. This makes targets movejump to main target
                unit->CastSpell(target, 1190010, true);

                // If the target is unfriendly, make the unit start attacking the caster
                if (!target->IsFriendlyTo(caster))
                {
                    unit->Attack(caster, true);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_gorefiends_grasp::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_spell_dk_gorefiends_grasp()
{
    RegisterSpellScript(spell_dk_gorefiends_grasp);
}

