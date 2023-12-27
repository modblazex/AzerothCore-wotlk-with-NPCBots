#include "ScriptMgr.h"
#include "Player.h"

const static std::vector<uint32> AREA_ID_LIST = { 976, 541, 3425, 1446, 69, 42, 35, 2268, 152, 108, 1099, 392, 99, 117, 608, 2255 };
const static std::vector<uint32> ALLIANCE_ZONE_ID_LIST = { 1519 }; // Add Alliance-specific zone IDs here
const static std::vector<uint32> HORDE_ZONE_ID_LIST = { 1637 }; // Add Horde-specific zone IDs here
const static uint32 AURA = 80094;

class SafeAreas : PlayerScript
{
public:
    SafeAreas() : PlayerScript("SafeAreas") { }

    void OnUpdateArea(Player* player, uint32 /*oldArea*/, uint32 newArea) override
    {
        uint32 currentZone = player->GetZoneId(); // Retrieve the current Zone ID of the player

        if (std::find(AREA_ID_LIST.begin(), AREA_ID_LIST.end(), newArea) != AREA_ID_LIST.end())
        {
            player->AddAura(AURA, player);
        }
        else if (player->GetTeamId() == TEAM_ALLIANCE &&
            std::find(ALLIANCE_ZONE_ID_LIST.begin(), ALLIANCE_ZONE_ID_LIST.end(), currentZone) != ALLIANCE_ZONE_ID_LIST.end())
        {
            player->AddAura(AURA, player);
        }
        else if (player->GetTeamId() == TEAM_HORDE &&
            std::find(HORDE_ZONE_ID_LIST.begin(), HORDE_ZONE_ID_LIST.end(), currentZone) != HORDE_ZONE_ID_LIST.end())
        {
            player->AddAura(AURA, player);
        }
        else
        {
            player->RemoveAura(AURA);
        }
    }
};

void AddSC_SafeAreas()
{
    new SafeAreas();
}
