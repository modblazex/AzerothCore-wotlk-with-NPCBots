#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

class spell_dk_blood_drinker : public AuraScript
{
    PrepareAuraScript(spell_dk_blood_drinker);

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
        {
            // Adjusted the percentage to half because for some reason the damage was double.
            // Damage per tick should add up to 40% of player's hp.
            amount += CalculatePct(caster->GetMaxHealth(), 6.67f);
        }
    }

    void HandleEffectPeriodic(AuraEffect const* aurEff)
    {
        if (Unit* caster = GetCaster())
        {
            // Separate healing calculation because healing is correct amount
            int32 healAmount = CalculatePct(caster->GetMaxHealth(), 13.33f);
            SpellInfo const* spellInfo = GetSpellInfo();

            HealInfo healInfo(caster, caster, healAmount, spellInfo, spellInfo->GetSchoolMask());

            caster->HealBySpell(healInfo);
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_blood_drinker::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_LEECH);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_blood_drinker::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_LEECH);
    }
};

void AddSC_spell_dk_blood_drinker()
{
    RegisterSpellScript(spell_dk_blood_drinker);
}
