#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Player.h"

class dk_spell_flesh_goliath : public SpellScript
{
    PrepareSpellScript(dk_spell_flesh_goliath);

    int32 totalDamage = 0;
    int32 targetCount = 0;

    void CountTargets(std::list<WorldObject*>& targetList)
    {
        targetCount = targetList.size();
    }

    void CalculateTotalDamage(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* playerCaster = caster->ToPlayer())
            {
                int32 maxHealth = playerCaster->GetMaxHealth();
                float multiplier = 1.0f;  // Base multiplier

                int32 increasedMaxHealth = static_cast<int32>(maxHealth * multiplier);

                // Calculate 25% of the increased max health
                totalDamage = CalculatePct(increasedMaxHealth, 25);
            }
        }
    }

    void HandleEffectHitTarget(SpellEffIndex /*effIndex*/)
    {
        // Ensure there is at least one target to avoid division by zero
        if (targetCount > 0)
        {
            // Divide the total damage evenly among all targets
            int32 splitDamage = totalDamage / targetCount;
            SetHitDamage(splitDamage);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(dk_spell_flesh_goliath::CountTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectLaunch += SpellEffectFn(dk_spell_flesh_goliath::CalculateTotalDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectHitTarget += SpellEffectFn(dk_spell_flesh_goliath::HandleEffectHitTarget, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

void AddSC_dk_spell_flesh_goliath()
{
    RegisterSpellScript(dk_spell_flesh_goliath);
}
