#include "Player.h"
#include "ScriptMgr.h"

class TitanGrip
{
public:
    static const uint32 SPELL_AURA_ID = 100265; // The Titan Grip Spell Aura ID
    static const uint32 REQUIRED_AURA_ID_1 = 49152; // The first Titan Grip Required Aura ID
    static const uint32 REQUIRED_AURA_ID_2 = 46917; // The second Titan Grip Required Aura ID
    static const uint8 EQUIPMENT_SLOT_MAINHAND = 15;
    static const uint8 EQUIPMENT_SLOT_OFFHAND = 16;

    static bool IsValidOneHandedWeapon(uint32 itemType, uint32 itemSubType)
    {
        return (itemType == 2) && (itemSubType == 0 || itemSubType == 4 || itemSubType == 7 || itemSubType == 13 || itemSubType == 15);
    }

    static bool PlayerUsingValidWeapons(Player* player)
    {
        Item* mainHand = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* offHand = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

        if (!mainHand || !offHand)
        {
            return false;
        }

        uint32 mainHandType = mainHand->GetTemplate()->Class;
        uint32 mainHandSubType = mainHand->GetTemplate()->SubClass;
        uint32 offHandType = offHand->GetTemplate()->Class;
        uint32 offHandSubType = offHand->GetTemplate()->SubClass;

        // Check if both main hand and offhand are valid one-handed weapons
        return IsValidOneHandedWeapon(mainHandType, mainHandSubType) && IsValidOneHandedWeapon(offHandType, offHandSubType);
    }
};

class TitanGrip_PlayerScript : public PlayerScript
{
public:
    TitanGrip_PlayerScript() : PlayerScript("TitanGrip_PlayerScript") {}

    void OnEquip(Player* player, Item* item, uint8 bag, uint8 slot, bool fromVendor) override
    {
        if (player->getClass() != CLASS_WARRIOR)
        {
            return;
        }

        if (player->HasAura(TitanGrip::REQUIRED_AURA_ID_1) || player->HasAura(TitanGrip::REQUIRED_AURA_ID_2))
        {
            if (TitanGrip::PlayerUsingValidWeapons(player))
            {
                    player->AddAura(TitanGrip::SPELL_AURA_ID, player);
            }
            else
            {
                if (player->HasAura(TitanGrip::SPELL_AURA_ID))
                {
                    player->RemoveAura(TitanGrip::SPELL_AURA_ID);
                }
            }
        }
    }
};


void AddSC_TitanGrip()
{
    new TitanGrip_PlayerScript();
}
