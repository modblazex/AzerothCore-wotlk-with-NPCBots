#include "AttackerSpawnModule.h"
#include "ScriptMgr.h"
#include "Creature.h"
#include "Map.h"

// Definitions
const uint32 AttackerSpawnModule::ITEM_ID = 800050;
const uint32 AttackerSpawnModule::MAX_NPC_SPAWN = 3;
const uint32 AttackerSpawnModule::ATTACK_CHANCE = 13;
const uint32 AttackerSpawnModule::DAZE_SPELL_ID = 100201;

const std::map<uint32, std::vector<uint32>> AttackerSpawnModule::creatureEntriesByLevelBracket = {
    {1, {38}},
    {11, {16303}},
    {21, {485}},
    {31, {4147}},
    {41, {5255}},
    {51, {6513}},
    {61, {16951}},
    {71, {26948}}
};

const std::map<uint32, std::vector<uint32>> AttackerSpawnModule::extraCreatureEntriesByLevelBracket = {
    {1, {99}},
    {11, {11519}},
    {21, {4829}},
    {31, {7345}},
    {41, {5256}},
    {51, {10317}},
    {61, {19997}},
    {71, {28793}}
};

bool AttackerSpawnModule::HasRequiredItem(Player* player) {
    return player->HasItemCount(ITEM_ID);
}

std::pair<std::vector<uint32>, std::vector<uint32>> AttackerSpawnModule::GetCreatureEntriesForLevel(uint32 level) {
    for (auto it = creatureEntriesByLevelBracket.rbegin(); it != creatureEntriesByLevelBracket.rend(); ++it) {
        if (level >= it->first) {
            return { it->second, extraCreatureEntriesByLevelBracket.at(it->first) };
        }
    }
    return { {}, {} };
}

std::pair<float, float> AttackerSpawnModule::GetRandomPositionWithinLOS(Player* player) {
    float x, y, z, o;
    player->GetPosition(x, y, z, o);
    uint32 tries = 0;
    const uint32 max_tries = 20;
    float new_x, new_y;
    while (tries < max_tries) {
        new_x = x + frand(-18.0f, 18.0f);
        new_y = y + frand(-18.0f, 18.0f);
        if (player->IsWithinLOS(new_x, new_y, z)) {
            return { new_x, new_y };
        }
        ++tries;
    }
    return { x, y };
}

void AttackerSpawnModule::SetCreatureHealthRelativeToPlayer(Player* player, Creature* creature) {
    uint32 playerHealth = player->GetMaxHealth();
    uint32 newHealth = playerHealth * 0.57f;
    creature->SetMaxHealth(newHealth);
    creature->SetHealth(newHealth);
}

void AttackerSpawnModule::SpawnAttacker(Player* player) {
    // Check if the player is in a Battleground, lacks the required item, or is flying
    if (player->GetMap()->IsBattleground() || !HasRequiredItem(player) || player->IsFlying()) {
        return;
    }
    const uint32 COOLDOWN_AURA_SPELL_ID = 842053;

    // Check if the cooldown is active
    if (player->HasAura(COOLDOWN_AURA_SPELL_ID)) {
        // Cooldown is active, do not spawn mobs
        return;
    }

    uint32 chance = urand(0, 100);
    if (chance > ATTACK_CHANCE) {
        return;
    }

    uint32 level = player->GetLevel();
    auto [creatureEntries, extraCreatureEntries] = GetCreatureEntriesForLevel(level);

    if (creatureEntries.empty()) {
        player->GetSession()->SendNotification("No creatures to spawn for your level.");
        return;
    }

    uint32 mapId = player->GetMapId();
    float x = player->GetPositionX();
    float y = player->GetPositionY();
    float z = player->GetPositionZ();
    float o = player->GetOrientation();
    uint32 npcCount = urand(1, MAX_NPC_SPAWN);

    if (player->IsMounted() && !player->IsFlying()) {
        uint32 dazeChance = urand(0, 100);
        if (dazeChance <= 50) {
            player->Dismount();
            player->CastSpell(player, DAZE_SPELL_ID, true);
            player->GetSession()->SendNotification("You have been knocked off your mount!");
        }
    }

    if (player->GetVehicle() == nullptr) {
        if (creatureEntries.empty()) {
            player->GetSession()->SendNotification("No creatures to spawn for your level.");
            return;
        }
        for (uint32 i = 0; i < npcCount; ++i) {
            uint32 selectedCreatureIndex = urand(0, creatureEntries.size() - 1);
            uint32 selectedCreature = creatureEntries[selectedCreatureIndex];
            auto [randomX, randomY] = GetRandomPositionWithinLOS(player);
            Creature* spawnedCreature = player->SummonCreature(selectedCreature, randomX, randomY, z, o, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 130000);
            SetCreatureHealthRelativeToPlayer(player, spawnedCreature);
            spawnedCreature->SetLevel(level);
            spawnedCreature->AI()->AttackStart(player);
        }

        bool isExtra = (urand(0, 100) <= 10) && !extraCreatureEntries.empty();
        if (isExtra) {
            if (extraCreatureEntries.empty()) {
                player->GetSession()->SendNotification("No extra creatures to spawn for your level.");
                return;
            }
            uint32 selectedExtraCreatureIndex = urand(0, extraCreatureEntries.size() - 1);
            uint32 selectedExtraCreature = extraCreatureEntries[selectedExtraCreatureIndex];
            auto [randomX, randomY] = GetRandomPositionWithinLOS(player);
            Creature* spawnedExtraCreature = player->SummonCreature(selectedExtraCreature, randomX, randomY, z, o, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 130000);
            spawnedExtraCreature->SetLevel(level);
            spawnedExtraCreature->AI()->AttackStart(player);
        }
        player->CastSpell(player, COOLDOWN_AURA_SPELL_ID, true);
    }
}

// Hook to the UpdateZone event
class AttackerSpawnModule_PlayerScript : public PlayerScript {
public:
    AttackerSpawnModule_PlayerScript() : PlayerScript("AttackerSpawnModule_PlayerScript") { }

    void OnUpdateZone(Player* player, uint32 /*newZone*/, uint32 /*newArea*/) override {
        AttackerSpawnModule::SpawnAttacker(player);
    }
};

void AddSC_AttackerSpawnModule() {
    new AttackerSpawnModule_PlayerScript();
}
