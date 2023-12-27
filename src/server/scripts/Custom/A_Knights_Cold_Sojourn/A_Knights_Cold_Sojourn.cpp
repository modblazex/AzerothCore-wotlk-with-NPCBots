#include "ScriptMgr.h"
#include "Player.h"

// Spell ID: 1190003
// Script Name: A Knight's Cold Sojourn

class dk_spell_a_knights_cold_sojourn : public AuraScript
{
    PrepareAuraScript(dk_spell_a_knights_cold_sojourn);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        if (!GetTarget() || GetTarget()->IsInCombat())
        {
            PreventDefaultAction();
            return;
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(dk_spell_a_knights_cold_sojourn::HandlePeriodic, EFFECT_0, SPELL_AURA_OBS_MOD_HEALTH);
    }
};

void AddSC_dk_a_knights_cold_sojourn()
{
    RegisterSpellScript(dk_spell_a_knights_cold_sojourn);
}
