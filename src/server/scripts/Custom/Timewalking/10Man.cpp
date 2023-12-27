#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "Chat.h"
#include "ScriptedGossip.h"
#include "WorldSession.h"

const static uint32 GOSSIP_ICON = 0;
const static uint32 MINIMUM_LEVEL = 61;
const static uint32 MAXIMUM_LEVEL = 80;
const static uint32 PLAYER_SPELL_ID = 108001;
const static uint32 ALL_PLAYER_SPELL_ID = 108002;
const static uint32 TYRANNICAL_SPELL_ID = 800142;
const static uint32 DECAY_SPELL_ID = 800139;
const static uint32 NECROTIC_SPELL_ID = 107099;

static void ApplyAllPlayerBuff(Player* player, bool apply)
{
    std::list<Player*> nearbyPlayers;
    Acore::AnyPlayerInObjectRangeCheck checker(player, 70.0f); 
    Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(player, nearbyPlayers, checker);
    Cell::VisitWorldObjects(player, searcher, 70.0f); 

    for (Player* nearbyPlayer : nearbyPlayers)
    {
        if (apply)
        {
            if (!nearbyPlayer->HasAura(ALL_PLAYER_SPELL_ID))
                nearbyPlayer->AddAura(ALL_PLAYER_SPELL_ID, nearbyPlayer);
        }
        else
        {
            if (nearbyPlayer->HasAura(ALL_PLAYER_SPELL_ID))
                nearbyPlayer->RemoveAura(ALL_PLAYER_SPELL_ID);
        }
    }
};


static void ApplyTimewalkingBuff(Player* player, bool apply)
{
    if (apply)
    {
        if (!player->HasAura(ALL_PLAYER_SPELL_ID)) {
            player->AddAura(ALL_PLAYER_SPELL_ID, player);
        }
    }
    else
    {
        player->RemoveAura(ALL_PLAYER_SPELL_ID);
        player->RemoveAura(PLAYER_SPELL_ID);
    }
};

class TENMAN_NPC : public CreatureScript
{
public:
    TENMAN_NPC() : CreatureScript("TENMAN_NPC") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON, "10-man Mode: Adjust the difficulty for smaller groups.", 0, 1);
        AddGossipItemFor(player, GOSSIP_ICON, "Tyrannical: Creatures have 35% increased health and 15% increased damage.", 0, 2);
        AddGossipItemFor(player, GOSSIP_ICON, "Decay: You and your raid members take periodic damage.", 0, 3);
        AddGossipItemFor(player, GOSSIP_ICON, "Necrotic: All direct damaging abilities apply a stacking blight.", 0, 4);
        AddGossipItemFor(player, GOSSIP_ICON, "Nevermind...", 0, 5);
        SendGossipMenuFor(player, 1, creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        ClearGossipMenuFor(player);
        switch (action)
        {
        case 1: // 10-man Mode
            ApplyTimewalkingBuff(player, !player->HasAura(PLAYER_SPELL_ID));
            ApplyAllPlayerBuff(player, true);
            player->CastSpell(player, 800143, true); 
            CloseGossipMenuFor(player);
            break;
        case 2: // Tyrannical mode
            player->CastSpell(player, TYRANNICAL_SPELL_ID, true);
            CloseGossipMenuFor(player);
            break;
        case 3: // Decay mode
            player->CastSpell(player, DECAY_SPELL_ID, true);
            CloseGossipMenuFor(player);
            break;
        case 4: // Necrotic mode
            player->CastSpell(player, NECROTIC_SPELL_ID, true);
            CloseGossipMenuFor(player);
            break;
        case 5: // "Nevermind" or exit option
            CloseGossipMenuFor(player);
            break;
        }
        return true;
    }
};

void AddSC_TENMAN()
{
    new TENMAN_NPC();
}
