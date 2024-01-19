#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum Spells
{
    SPELL_VOID_SANCTUARY = 888080, 
};

#define ABSORB_PERCENTAGE 0.15f

class spell_void_sanctuary : public AuraScript
{
    PrepareAuraScript(spell_void_sanctuary);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_VOID_SANCTUARY });
    }

    void HandleAbsorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        absorbAmount = uint32(dmgInfo.GetDamage() * ABSORB_PERCENTAGE);
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_void_sanctuary::HandleAbsorb, EFFECT_0);
    }
};

void AddSC_spell_void_sanctuary()
{
    RegisterSpellScript(spell_void_sanctuary);
}
