#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

class spell_up_up_and_away : public SpellScript
{
    PrepareSpellScript(spell_up_up_and_away);

    void HandleOnCast()
    {
        if (Unit* caster = GetCaster())
        {
            if (Aura* aura = caster->GetAura(100225))
            {
                if (aura->GetStackAmount() > 2)
                {
                    caster->CastSpell(caster, 69669, true);
                }
            }
        }
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_up_up_and_away::HandleOnCast);
    }
};

void AddSC_spell_up_up_and_away()
{
    RegisterSpellScript(spell_up_up_and_away);
}
