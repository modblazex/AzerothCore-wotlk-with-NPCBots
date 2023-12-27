class Ghoul_Dust_Trinket : public PlayerScript
{
public:
    Ghoul_Dust_Trinket() : PlayerScript("Ghoul_Dust_Trinket") {}

    void OnPlayerKilledByCreature(Creature* /*killer*/, Player* killed) override
    {
        OnPlayerDeath(killed);
    }

    void OnPVPKill(Player* /*killer*/, Player* killed) override
    {
        OnPlayerDeath(killed);
    }

    void OnUpdate(Player* player, uint32 diff) override
    {
        auto it = delayedCasts.find(player->GetGUID());
        if (it != delayedCasts.end())
        {
            it->second -= diff;
            if (it->second <= 0)
            {
                player->CastSpell(player, SPELL_ID, true);
                delayedCasts.erase(it);
            }
        }
    }

private:
    static const uint32 ITEM_ID = 60090;
    static const uint32 SPELL_ID = 46619;
    static const uint32 CAST_CHANCE = 30;
    static const uint32 DELAY = 1000; // in milliseconds

    std::map<ObjectGuid, int32> delayedCasts;

    bool HasEquippedItem(Player* player, uint32 itemID)
    {
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (item && item->GetEntry() == itemID)
                return true;
        }
        return false;
    }

    void OnPlayerDeath(Player* player)
    {
        // Check if the killed player has the item equipped
        if (!HasEquippedItem(player, ITEM_ID)) // Early return if not equipped
        {
            return;
        }

        // Roll for the chance to cast the spell
        if (urand(1, 100) <= CAST_CHANCE)
        {
            delayedCasts[player->GetGUID()] = DELAY;
        }
    }
};

void AddSC_Ghoul_Dust_Trinket()
{
    new Ghoul_Dust_Trinket();
}
