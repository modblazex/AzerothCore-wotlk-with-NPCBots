#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Spell.h"
#include "Unit.h"


class spell_heigan_eruption : public SpellScriptLoader
{
public:
    spell_heigan_eruption() : SpellScriptLoader("spell_heigan_eruption") { }

    class spell_heigan_eruption_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_heigan_eruption_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* obj)
                {
                    if (Unit* unit = obj->ToUnit())
                    {
                        return unit->IsNPCBotOrPet();
                    }
                    return true;
                });
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_heigan_eruption_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_heigan_eruption_SpellScript();
    }
};

void AddSC_your_script_name()
{
    new spell_heigan_eruption();
}
