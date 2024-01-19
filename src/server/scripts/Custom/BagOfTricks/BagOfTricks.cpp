#include "ScriptMgr.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

class spell_bags_of_tricks : public SpellScript
{
    PrepareSpellScript(spell_bags_of_tricks);

    void HandleAfterCast()
    {
        if (Player* player = GetCaster()->ToPlayer())
        {
            static const uint32 SPELL_IDS[] = { 54071, 100244, 100245, 100247 };
            uint32 spellId = SPELL_IDS[urand(0, 3)]; // Random spell from the list
            Unit* target = player->GetSelectedUnit();

            if (target)
            {
                player->CastSpell(target, spellId, true);
            }
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_bags_of_tricks::HandleAfterCast);
    }
};

void Addspell_bags_of_tricksScripts()
{
    RegisterSpellScript(spell_bags_of_tricks);
}
