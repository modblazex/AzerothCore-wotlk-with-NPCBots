#include "ScriptMgr.h"
#include "Player.h"
#include "SpellScript.h"

class Spell_DoubleCast : public SpellScript
{
    PrepareSpellScript(Spell_DoubleCast);

    void HandleHit()
    {
        // Get caster
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // Exclude NPC bots or pets using custom method
        if (caster->IsNPCBotOrPet())
            return;

        // Only proceed for players
        if (caster->GetTypeId() != TYPEID_PLAYER)
            return;

        // 14% chance to cast the spell again upon hit
        if (roll_chance_f(14.0f))
        {
            // Get spell info
            SpellInfo const* spellInfo = GetSpellInfo();
            if (!spellInfo)
                return;

            // Cast the spell again
            caster->CastSpell(GetHitUnit(), spellInfo->Id, true);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(Spell_DoubleCast::HandleHit);
    }
};

void AddSC_Spell_DoubleCast()
{
    RegisterSpellScript(Spell_DoubleCast);
}
