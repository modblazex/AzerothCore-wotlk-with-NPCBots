#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Unit.h"
#include "SpellInfo.h"

// 1190012 - Gorefiend's Grasp Player Aura
enum GorefiendsGraspSpells
{
    SPELL_DK_GOREFIEND_EXTRA_ATTACK = 1190011,
    SPELL_GOREFIENDS_GRASP_AURA = 1190010
};

class spell_dk_gorefiends_grasp_damage : public AuraScript
{
    PrepareAuraScript(spell_dk_gorefiends_grasp_damage);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_GOREFIEND_EXTRA_ATTACK });
    }

    bool Load() override
    {
        return true;
    }

    std::list<Unit*> SelectNearbyTargetsWithAura(Unit* refUnit, uint32 auraId, float range, uint32 maxTargets)
    {
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck checker(refUnit, refUnit, range);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(refUnit, targets, checker);
        Cell::VisitAllObjects(refUnit, searcher, range);

        targets.remove_if([auraId](Unit* unit) { return !unit->HasAura(auraId); });

        // Limit the number of targets to prevent server crash
        if (targets.size() > maxTargets)
            targets.resize(maxTargets);

        return targets;
    }

    // Modify CheckProc to include maxTargets
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* actor = eventInfo.GetActor();
        if (!actor || !IsSpellValid(eventInfo.GetSpellInfo(), actor))
        {
            return false;
        }

        // Adjust the range and maxTargets as needed
        _procTargets = SelectNearbyTargetsWithAura(actor, SPELL_GOREFIENDS_GRASP_AURA, 30.0f, 5); // Example: Max 5 targets
        return !_procTargets.empty();
    }

    bool IsSpellValid(SpellInfo const* spellInfo, Unit* actor)
    {
        // If the spellInfo is null, return false
        if (!spellInfo)
        {
            return false;
        }

        // Check if the spell is a single target spell or has no target (for auto attack, but not sure if this is working tbh)
        // Basically just to remove aoe and nondirect spells
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->Effects[i].TargetA.GetTarget() != 0 &&
                spellInfo->Effects[i].TargetA.GetTarget() != TARGET_UNIT_TARGET_ENEMY)
            {
                return false;
            }
        }

        return true; // The spell is valid
    }


    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* actor = eventInfo.GetActor();
        Unit* target = GetTarget();

        if (!actor || !target || _procTargets.empty())
        {
            return;
        }

        if (DamageInfo* damageInfo = eventInfo.GetDamageInfo())
        {
            int32 damage = damageInfo->GetUnmitigatedDamage();

            // Calculate 20% of the original damage
            int32 reducedDamage = CalculatePct(damage, 20);

            for (Unit* procTarget : _procTargets)
            {
                target->CastCustomSpell(procTarget, SPELL_DK_GOREFIEND_EXTRA_ATTACK, &reducedDamage, 0, 0, true, nullptr, aurEff);
            }
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_gorefiends_grasp_damage::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_dk_gorefiends_grasp_damage::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

private:
    std::list<Unit*> _procTargets;
};

void AddSC_spell_dk_gorefiends_grasp_damage()
{
    RegisterSpellScript(spell_dk_gorefiends_grasp_damage);
}
