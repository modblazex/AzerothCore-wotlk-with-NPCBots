#include "ScriptMgr.h"
#include "SpellScript.h"
#include "botmgr.h"
#include "Unit.h"

class spell_filter_npcbots : public SpellScriptLoader
{
public:
    spell_filter_npcbots() : SpellScriptLoader("spell_filter_npcbots") { }

    class spell_filter_npcbots_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_filter_npcbots_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            // Only filter for SPELL_EFFECT_SCHOOL_DAMAGE
            if (GetSpellInfo()->Effects[EFFECT_0].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
            {
                targets.remove_if([](WorldObject* obj) -> bool
                    {
                        if (Unit* unit = obj->ToUnit())
                        {
                            // Check for NPC Bots or Pets
                            if (unit->IsNPCBotOrPet())
                            {
                                return true;
                            }

                            // Additional check for specific creatures
                            if (Creature* creature = unit->ToCreature())
                            {
                                uint32 entry = creature->GetEntry();
                                if (entry >= 70000 && entry <= 89000)
                                {
                                    return true;
                                }
                            }
                        }
                        return false;
                    });
            }
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_filter_npcbots_SpellScript::FilterTargets, EFFECT_0, TARGET_DEST_DYNOBJ_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_filter_npcbots_SpellScript();
    }
};

void AddSC_spell_filter_npcbots()
{
    new spell_filter_npcbots();
}
