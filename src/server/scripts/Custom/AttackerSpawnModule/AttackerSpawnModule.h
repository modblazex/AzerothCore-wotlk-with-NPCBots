#ifndef ATTACKER_SPAWN_MODULE_H
#define ATTACKER_SPAWN_MODULE_H

#include "Player.h"
#include <map>
#include <vector>

class AttackerSpawnModule
{
public:
    static void SpawnAttacker(Player* player);

private:
    static const uint32 ITEM_ID;
    static const uint32 MAX_NPC_SPAWN;
    static const uint32 ATTACK_CHANCE;
    static const uint32 DAZE_SPELL_ID;

    static const std::map<uint32, std::vector<uint32>> creatureEntriesByLevelBracket;
    static const std::map<uint32, std::vector<uint32>> extraCreatureEntriesByLevelBracket;

    static bool HasRequiredItem(Player* player);
    static std::pair<std::vector<uint32>, std::vector<uint32>> GetCreatureEntriesForLevel(uint32 level);
    static std::pair<float, float> GetRandomPositionWithinLOS(Player* player);
    static void SetCreatureHealthRelativeToPlayer(Player* player, Creature* creature);
};

#endif // ATTACKER_SPAWN_MODULE_H
