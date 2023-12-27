#include "ScriptMgr.h"
#include "Player.h"

class PlayerScript_remove_aura_on_resurrect : public PlayerScript
{
public:
    PlayerScript_remove_aura_on_resurrect() : PlayerScript("PlayerScript_remove_aura_on_resurrect") { }

    void OnPlayerResurrect(Player* player, float /*healthPercent*/, bool /*isRessurectedBySoulstone*/) override
    {
        const uint32 spellId = 855164;

        if (player->HasAura(spellId))
            player->RemoveAura(spellId);
    }
};

void AddSC_player_script_remove_aura_on_resurrect()
{
    new PlayerScript_remove_aura_on_resurrect();
}
