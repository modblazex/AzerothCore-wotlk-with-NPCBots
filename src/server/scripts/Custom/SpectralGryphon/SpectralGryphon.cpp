#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"

class spell_custom_spell_usable_while_dead : public SpellScriptLoader
{
public:
    spell_custom_spell_usable_while_dead() : SpellScriptLoader("spell_custom_spell_usable_while_dead") { }

    class spell_custom_spell_usable_while_dead_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_custom_spell_usable_while_dead_SpellScript);

        SpellCastResult CheckIfDead()
        {
            if (Player* caster = GetCaster()->ToPlayer())
            {
                if (!caster->isDead())
                    return SPELL_FAILED_TARGET_NOT_DEAD;

                if (caster->GetMap()->IsDungeon() || caster->GetMap()->IsRaid() || caster->GetMap()->IsBattleground())
                    return SPELL_FAILED_NOT_HERE;
            }
            return SPELL_CAST_OK;
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_custom_spell_usable_while_dead_SpellScript::CheckIfDead);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_custom_spell_usable_while_dead_SpellScript();
    }
};

void AddSC_spell_custom_spell_usable_while_dead()
{
    new spell_custom_spell_usable_while_dead();
}
