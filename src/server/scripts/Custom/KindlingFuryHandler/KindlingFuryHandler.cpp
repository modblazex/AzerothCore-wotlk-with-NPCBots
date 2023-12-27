#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"

class spell_kindling_fury_stacks : public SpellScript
{
    PrepareSpellScript(spell_kindling_fury_stacks);

    void HandleAfterCast()
    {
        if (Player* player = GetCaster()->ToPlayer())
        {
            if (Aura* aura = player->GetAura(80040)) 
            {
                if (aura->GetStackAmount() >= 15) 
                {
                    player->CastSpell(player, 920353, true); 
                    player->RemoveAura(aura);
                }
            }
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_kindling_fury_stacks::HandleAfterCast);
    }
};

void AddSC_custom_kindling_fury()
{
    RegisterSpellScript(spell_kindling_fury_stacks); 
}
