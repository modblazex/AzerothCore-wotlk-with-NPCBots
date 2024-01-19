#include "Player.h"
#include "ScriptMgr.h"

class TwoForms : public PlayerScript
{
public:
    TwoForms() : PlayerScript("TwoForms") { }

    void OnFirstLogin(Player* player) override // Using the specific hook for the first login
    {
        uint8 race = player->getRace();
        uint8 gender = player->getGender();

        if (race == 16)
        {
            uint32 spellId = (gender == GENDER_FEMALE) ? 97710 : 97709;
            player->learnSpell(spellId, false); // Using the learnSpell method
        }
    }
};

void AddSC_TwoForms()
{
    new TwoForms();
}
