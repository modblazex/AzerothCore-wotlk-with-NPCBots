#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Spell.h"
#include "Unit.h"

class spell_molten_cleave : public SpellScript
{
    PrepareSpellScript(spell_molten_cleave);

    bool HasFlameShock(Unit* target)
    {
        const std::vector<uint32> flameShockIds = { 8050, 8052, 8053, 10447, 10448, 29228, 25457, 49232, 49233 };
        for (uint32 spellId : flameShockIds)
        {
            if (target->HasAura(spellId))
                return true;
        }
        return false;
    }

    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (HasFlameShock(target))
        {
            SetHitDamage(GetHitDamage() * 1.5);
        }

        if (caster->HasAura(67228))
        {
            const int32 triggered_spell_id = 71824;
            SpellInfo const* triggeredSpell = sSpellMgr->GetSpellInfo(triggered_spell_id);
            if (triggeredSpell)
            {
                int32 damage = GetHitDamage();
                int32 triggerAmount = 12;
                int32 basepoints0 = CalculatePct(damage, triggerAmount);

                caster->CastCustomSpell(target, triggered_spell_id, &basepoints0, nullptr, nullptr, true);
            }
        }

        if (caster->HasAura(863270))
        {
            caster->CastSpell(caster, 880040, true);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_molten_cleave::HandleOnHit);
    }
};


void AddSC_custom_spell_molten_cleave()
{
    RegisterSpellScript(spell_molten_cleave);
}
