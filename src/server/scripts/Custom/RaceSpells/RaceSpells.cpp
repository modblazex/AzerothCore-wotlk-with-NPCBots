#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuraEffects.h"

class spell_paladin_holy_wrath : public SpellScriptLoader
{
public:
    spell_paladin_holy_wrath() : SpellScriptLoader("spell_paladin_holy_wrath") { }

    class spell_paladin_holy_wrath_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_paladin_holy_wrath_SpellScript);

        // Filter function to check creature type or player race
        bool FilterTargets(WorldObject* target)
        {
            if (Creature* creature = target->ToCreature())
            {
                return creature->GetCreatureType() == CREATURE_TYPE_DEMON || creature->GetCreatureType() == CREATURE_TYPE_UNDEAD;
            }
            else if (Player* player = target->ToPlayer())
            {
                uint32 raceMask = player->getRaceMask();
                return raceMask == 65536 || raceMask == 16;
            }

            return false;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([this](WorldObject* obj) { return !FilterTargets(obj); });
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_paladin_holy_wrath_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_paladin_holy_wrath_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_paladin_holy_wrath_SpellScript();
    }
};

class spell_demon_undead_beast_no_hit : public SpellScriptLoader
{
public:
    spell_demon_undead_beast_no_hit() : SpellScriptLoader("spell_demon_undead_beast_no_hit") { }

    class spell_demon_undead_beast_no_hit_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_demon_undead_beast_no_hit_SpellScript);

        SpellCastResult CheckTargetRace()
        {
            if (Unit* target = GetExplTargetUnit())
            {
                // Check for race masks
                if (target->getRaceMask() == 65536 || target->getRaceMask() == 16 || target->getRaceMask() == 32768)
                {
                    return SPELL_FAILED_BAD_TARGETS;
                }
            }
            return SPELL_CAST_OK;
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_demon_undead_beast_no_hit_SpellScript::CheckTargetRace);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_demon_undead_beast_no_hit_SpellScript();
    }
};

class spell_demon_undead_no_hit : public SpellScriptLoader
{
public:
    spell_demon_undead_no_hit() : SpellScriptLoader("spell_demon_undead_no_hit") { }

    class spell_demon_undead_no_hit_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_demon_undead_no_hit_SpellScript);

        SpellCastResult CheckTargetRace()
        {
            if (Unit* target = GetExplTargetUnit())
            {
                // Check for race masks
                if (target->getRaceMask() == 65536 || target->getRaceMask() == 16)
                {
                    return SPELL_FAILED_BAD_TARGETS;
                }
            }
            return SPELL_CAST_OK;
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_demon_undead_no_hit_SpellScript::CheckTargetRace);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_demon_undead_no_hit_SpellScript();
    }
};

void AddSC_spell_paladin_holy_wrath()
{
    new spell_paladin_holy_wrath();
    new spell_demon_undead_beast_no_hit();
    new spell_demon_undead_no_hit();
}
