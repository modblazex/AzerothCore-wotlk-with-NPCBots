#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"

class spell_dk_frost_latch_taunt : public SpellScriptLoader
{
public:
    spell_dk_frost_latch_taunt() : SpellScriptLoader("spell_dk_frost_latch_taunt") { }

    class spell_dk_frost_latch_taunt_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_frost_latch_taunt_SpellScript);

        void HandleAfterCast()
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            Unit* target = GetExplTargetUnit();
            if (!target)
                return;

            if (caster->HasAura(1190013))  // Dark Guardian aura
            {
                caster->CastSpell(target, 1190015, true);  // Taunt spell to cast on target
            }
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_dk_frost_latch_taunt_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_frost_latch_taunt_SpellScript();
    }
};

void AddSC_spell_dk_frost_latch_taunt()
{
    new spell_dk_frost_latch_taunt();
}
