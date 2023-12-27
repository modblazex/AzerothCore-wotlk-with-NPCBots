#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "Chat.h"
#include "ScriptedGossip.h"
#include "WorldSession.h"
#include <random>

//Spells
const static uint32 FRAGILE_WARRIOR_SPELL = 80089;
const static uint32 IRONMAN_SPELL = 80007;
const static uint32 IRONMAN_FAILED_SPELL = 80090;
const static uint32 PACIFIST_FAILED_SPELL = 80092;

//Items
const static uint32 HARDCORE_ITEM = 90000;
const static uint32 SLOW_AND_STEADY_ITEM = 800048;
const static uint32 PACIFIST_ITEM = 800051;
const static uint32 DIET_ITEM = 800084;
const static uint32 NOMAD_ITEM = 800085;
const static uint32 TAXATION_WITHOUT_REPRESENTATION_ITEM = 800086;
const static uint32 IRONMAN_ITEM = 800049;
const static uint32 RANDOMY_ATTACKED_ITEM = 800050;
// Other
const static uint32 IRONMAN_QUALITY_THRESHOLD = 2;
const static float TAX_MULTIPLIER = 0.25;

//Vectors
const static std::vector<uint32> HARDCORE_SPELLS = { 24939, 100117, 100118, 100105 };
const static std::vector<uint32> NOMAD_FORBIDDEN_SPELLS = { 8690, 54318, 54401, 3561, 3562, 3563, 3565, 3566, 3567, 3578, 556, 11417, 11416, 10059, 11418, 11419, 11420, 32267, 32266, 57535, 53141, 18960 };
const static std::vector<uint32> SLOW_AND_STEADY_FORBIDDEN_SPELLS = { 33388, 33389, 33391, 33392, 34090, 34091, 34092, 34093 };
const static std::vector<std::string> TAXATION_MESSAGES = {
    "The taxman cometh! 75% of your earnings have vanished into thin air.",
    "Like magic, but less fun, 75% of your earnings disappeared.",
    "Congratulations! 75% of your earnings have been donated to 'Taxation Without Representation Foundation'.",
    "Breaking news: Local adventurer loses 75% of their income. Details at 11.",
    "75% of your earnings decided they'd rather be somewhere else.",
    "A gnome ran off with 75% of your earnings. You should probably look into that.",
    "You've been visited by the tax fairy! Unfortunately, she took 75% of your earnings.",
    "Your earnings decided to take a 75% vacation.",
    "Remember that 75% of your earnings you used to have? Yeah, about that...",
    "Whoops! Looks like 75% of your earnings fell into an inter-dimensional portal.",
    "Your earnings have been taxed at the goblin rate. You're 75% lighter now.",
    "Some say wealth is overrated. Enjoy your 75% lighter purse.",
    "75% of your earnings were abducted by tax aliens. They said they come in peace.",
    "A mysterious force whisked away 75% of your earnings. It left a 'thank you' note.",
    "You feel a slight breeze as 75% of your earnings make a swift exit.",
    "A murloc ran by and ate 75% of your earnings. It seemed to enjoy it.",
    "75% of your earnings have been drafted into the army of the tax king.",
    "A gnome from the Taxation Bureau sends their regards. Also, they took 75% of your earnings.",
    "An invisible tax imp just did a dance and 75% of your earnings joined in.",
    "A taxing tornado just swept through your purse. It left 10% behind, how considerate!",
    "The taxman strikes again! 75% of your earnings are MIA.",
    "75% of your earnings got lost in the Twisting Nether. Happens to the best of us.",
    "A gnome engineer needed 75% of your earnings for... science. Don't ask.",
    "Looks like Nozdormu messed with your timeline. 75% of your earnings? Never existed.",
    "You feel lighter, and not in a good way. A rogue named Tax just pickpocketed 75% of your earnings!",
    "An ethereal trader just offered you a deal you couldn't refuse. 75% of your earnings for a shiny... pebble?",
    "A goblin shouts in the distance, 'Time is money, friend!' as 75% of your earnings inexplicably disappears.",
    "75% of your earnings were just donated to the 'Dwarven Ale Appreciation Society'. They thank you for your generosity.",
    "Pepe needed a new hat. Your 75% contribution will be remembered.",
    "The spirit healers have started charging a 'convenience fee'. There goes 75% of your earnings.",
    "You suddenly remember a hefty debt you owed to Gazlowe. 75% of your earnings should cover it... for now."
};

// Maps
const static std::unordered_map<std::string_view, uint32> HARDCORE_ITEMS = {
    {"CUSTOM_STARTER_1", 60002},
    {"CUSTOM_STARTER_2", 10594},
    {"CUSTOM_STARTER_3", 65000},
    {"ITEM_90000", 90000},
    {"REST_REMOVER", 800048},
};

struct Rewards
{
    uint32 requiredModeItem;
    uint32 levelRequirement;
    std::vector<uint32> itemIds;
    std::vector<uint32> itemCounts;
    std::vector<uint32> spellIds;
    uint32 titleId;
};

const static std::vector<Rewards> REWARD_VECTOR = {
    {PACIFIST_ITEM, 9,  {37711}, {10}, {0}, 0},
    {PACIFIST_ITEM, 19, {37711}, {20}, {0}, 0},
    {PACIFIST_ITEM, 29, {37711}, {30}, {0}, 0},
    {PACIFIST_ITEM, 39, {37711}, {40}, {0}, 0},
    {PACIFIST_ITEM, 49, {37711}, {50}, {0}, 0},
    {PACIFIST_ITEM, 59, {37711}, {100}, {80114}, 185},

    {SLOW_AND_STEADY_ITEM, 9,  {37711}, {10}, {0}, 0},
    {SLOW_AND_STEADY_ITEM, 19, {37711}, {20}, {0}, 0},
    {SLOW_AND_STEADY_ITEM, 29, {37711}, {30}, {0}, 0},
    {SLOW_AND_STEADY_ITEM, 39, {37711}, {40}, {0}, 0},
    {SLOW_AND_STEADY_ITEM, 49, {37711}, {50}, {0}, 0},
    {SLOW_AND_STEADY_ITEM, 59, {37711}, {100}, {80110, 80108}, 187},

    {IRONMAN_ITEM, 9,  {37711}, {10}, {0}, 0},
    {IRONMAN_ITEM, 19, {37711}, {20}, {0}, 0},
    {IRONMAN_ITEM, 29, {37711}, {30}, {0}, 0},
    {IRONMAN_ITEM, 39, {37711}, {40}, {0}, 0},
    {IRONMAN_ITEM, 49, {37711}, {50}, {0}, 0},
    {IRONMAN_ITEM, 59, {37711}, {100}, {80109}, 186},

    {RANDOMY_ATTACKED_ITEM, 9,  {37711}, {10}, {0}, 0},
    {RANDOMY_ATTACKED_ITEM, 19, {37711}, {20}, {0}, 0},
    {RANDOMY_ATTACKED_ITEM, 29, {37711}, {30}, {0}, 0},
    {RANDOMY_ATTACKED_ITEM, 39, {37711}, {40}, {0}, 0},
    {RANDOMY_ATTACKED_ITEM, 49, {37711}, {50}, {0}, 0},
    {RANDOMY_ATTACKED_ITEM, 59, {37711}, {100}, {80111}, 180},

    {HARDCORE_ITEM, 9,  {37711}, {10}, {0}, 0},
    {HARDCORE_ITEM, 19, {37711}, {20}, {0}, 0},
    {HARDCORE_ITEM, 29, {37711}, {30}, {0}, 0},
    {HARDCORE_ITEM, 39, {37711}, {40}, {0}, 0},
    {HARDCORE_ITEM, 49, {37711}, {50}, {0}, 0},
    {HARDCORE_ITEM, 59, {36941}, {1}, {100125, 53082, 80112}, 0},

    {DIET_ITEM, 9,  {37711, 43490, 44616, 44838}, {10, 1, 1, 1}, {0}, 0},
    {DIET_ITEM, 19, {37711, 43490, 46887, 44839}, {20, 2, 2, 2}, {0}, 0},
    {DIET_ITEM, 29, {37711, 38466, 44791, 16169}, {30, 3, 3, 3}, {0}, 0},
    {DIET_ITEM, 39, {37711, 16168, 44616, 44838}, {40, 4, 4, 4}, {0}, 0},
    {DIET_ITEM, 49, {37711, 3927, 23160, 11415}, {50, 5, 5, 5}, {0}, 0},
    {DIET_ITEM, 59, {37711, 35565, 44616, 33208}, {50, 10, 10, 5}, {800120, 800006}, 190},

    {NOMAD_ITEM, 9,  {37711}, {10}, {0}, 0},
    {NOMAD_ITEM, 19, {37711}, {20}, {0}, 0},
    {NOMAD_ITEM, 29, {37711}, {30}, {0}, 0},
    {NOMAD_ITEM, 39, {37711}, {40}, {0}, 0},
    {NOMAD_ITEM, 49, {37711}, {50}, {0}, 0},
    {NOMAD_ITEM, 59, {37711}, {100}, {0}, 191},

    {TAXATION_WITHOUT_REPRESENTATION_ITEM, 9,  {37711}, {5}, {0}, 0},
    {TAXATION_WITHOUT_REPRESENTATION_ITEM, 19, {37711}, {10}, {0}, 0},
    {TAXATION_WITHOUT_REPRESENTATION_ITEM, 29, {37711}, {15}, {0}, 0},
    {TAXATION_WITHOUT_REPRESENTATION_ITEM, 39, {37711}, {20}, {0}, 0},
    {TAXATION_WITHOUT_REPRESENTATION_ITEM, 49, {37711}, {25}, {0}, 0},
    {TAXATION_WITHOUT_REPRESENTATION_ITEM, 59, {37711}, {50}, {0}, 192},
};                       

static void SendWorldMessage(std::string message)
{
    for (auto& pair : sWorld->GetAllSessions())
        if (Player* receiver = pair.second->GetPlayer())
            sWorld->SendServerMessage(SERVER_MSG_STRING, message, receiver);
}

static std::vector<Rewards> GetRewardsForLevelRequirement(uint32 modeItem, uint32 level) {
    std::vector<Rewards> result;

    for (const Rewards& reward : REWARD_VECTOR) {
        if (reward.requiredModeItem == modeItem && reward.levelRequirement == level) {
            result.push_back(reward);
        }
    }

    return result;
}

static void GiveRewards(Player* player, uint32 modeItem, uint32 level) {
    std::vector<Rewards> rewards = GetRewardsForLevelRequirement(modeItem, level);

    for (const Rewards& reward : rewards) {
        if (reward.itemIds.size() > 0 && reward.itemCounts.size() > 0)
        {
            for (uint32 i = 0; i < reward.itemIds.size(); ++i) {
                if (reward.itemCounts[i] == 0)
                    continue;
                else
                    player->AddItem(reward.itemIds[i], reward.itemCounts[i]);
            }
        }

        if (reward.spellIds.size() > 0)
        {
            for (uint32 spell : reward.spellIds) {
                if (spell == 0)
                    continue;
                else
                    player->learnSpell(spell);
            }
        }

        if (reward.titleId != 0) {
            CharTitlesEntry const* t = sCharTitlesStore.LookupEntry(reward.titleId);
            if (t) {
                player->SetTitle(t, false);
                ChatHandler(player->GetSession()).PSendSysMessage("|cff0000ffYou've earned a new title!|r");
            }
        }
        // This might need to be %s %u instead of {}.
        LOG_INFO("server.rewards", "Player {} has earned a reward for mode {} for reaching level {}", player->GetName(), modeItem, level);
        ChatHandler(player->GetSession()).PSendSysMessage("%s has recieved reward for reaching level %u!", player->GetName(), player->GetLevel());
    }
};

static void CanGetRewards(Player* player, uint32 level)
{
    if(player->HasItemCount(PACIFIST_ITEM, 1, true))
        GiveRewards(player, PACIFIST_ITEM, level);

    if(player->HasItemCount(HARDCORE_ITEM, 1, true))
        GiveRewards(player, HARDCORE_ITEM, level);

    if(player->HasItemCount(SLOW_AND_STEADY_ITEM, 1, true))
        GiveRewards(player, SLOW_AND_STEADY_ITEM, level);

    if(player->HasItemCount(IRONMAN_ITEM, 1, true))
        GiveRewards(player, IRONMAN_ITEM, level);

    if(player->HasItemCount(RANDOMY_ATTACKED_ITEM, 1, true))
        GiveRewards(player, RANDOMY_ATTACKED_ITEM, level);

    if(player->HasItemCount(DIET_ITEM, 1, true))
        GiveRewards(player, DIET_ITEM, level);

    if(player->HasItemCount(TAXATION_WITHOUT_REPRESENTATION_ITEM, 1, true))
        GiveRewards(player, TAXATION_WITHOUT_REPRESENTATION_ITEM, level);

    if(player->HasItemCount(NOMAD_ITEM, 1, true))
        GiveRewards(player, NOMAD_ITEM, level);
}

static std::string getRandomTaxationMessage() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, TAXATION_MESSAGES.size() - 1);

    return TAXATION_MESSAGES[dis(gen)];
}

static bool ActivateSlowAndSteady(uint32 guid) {
    std::string guidString = std::to_string(guid);
    WorldDatabase.Query("UPDATE `custom_xp` SET `Rate` = '0.5' WHERE `CharID` = '" + guidString + "'");
    CharacterDatabase.Query("UPDATE `custom_gather_rates` SET `GatherRate` = '1' WHERE `CharID` = '" + guidString + "'");
    CharacterDatabase.Query("UPDATE `custom_rep_rates` SET `RepRate` = '1' WHERE `CharID` = '" + guidString + "'");
    CharacterDatabase.Query("UPDATE `custom_craft_rates` SET `CraftRate` = '1' WHERE `CharID` = '" + guidString + "'");

    return 1;
}

static void RemoveSpells(Player* player) {
    for (auto spell : HARDCORE_SPELLS)
        player->RemoveAura(spell);
}

static bool RemoveItems(Player* player) {
    bool removed = false;
    for (auto& [name, item] : HARDCORE_ITEMS) {
        if (name != "ITEM_90000" && name != "REST_REMOVER") {
            auto itemCount = player->GetItemCount(item);
            if (itemCount > 0) {
                for (int i = 0; i < itemCount; i++) {
                    player->DestroyItem(item, 1, true);
                }
                removed = true;
            }
        }
    }
    return removed;
}

class HARDCORE_NPC : public CreatureScript
{
public:
    HARDCORE_NPC() : CreatureScript("HARDCORE_NPC") { }

    // Function to check if the player has any challenge mode items or auras
    bool PlayerHasChallenges(Player* player) {
        std::vector<uint32> challengeItems = { HARDCORE_ITEM, SLOW_AND_STEADY_ITEM, PACIFIST_ITEM, DIET_ITEM, NOMAD_ITEM, TAXATION_WITHOUT_REPRESENTATION_ITEM, IRONMAN_ITEM, RANDOMY_ATTACKED_ITEM };
        for (uint32 item : challengeItems) {
            if (player->HasItemCount(item, 1, true))
                return true;
        }

        std::vector<uint32> challengeSpells = { FRAGILE_WARRIOR_SPELL, IRONMAN_SPELL, PACIFIST_FAILED_SPELL };
        for (uint32 spell : challengeSpells) {
            if (player->HasAura(spell))
                return true;
        }

        return false;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        Group* group = player->GetGroup();
        if (group) {
            ChatHandler(player->GetSession()).PSendSysMessage("You must leave your group before selecting challenge modes.");
            return false;
        }

        if (!group)
        {
            ClearGossipMenuFor(player);

            if (player->GetLevel() == 1)
            {
                player->PlayDirectSound(183254);
                AddGossipItemFor(player, 0, "Click here to try Hardcore Mode!", 0, 1);
                AddGossipItemFor(player, 0, "Click here to try Slow and Steady Mode!", 0, 9);
                AddGossipItemFor(player, 0, "Click here to try Fragile Warrior Mode!", 0, 11);
                AddGossipItemFor(player, 0, "Click here to try Ironman Mode!", 0, 14);
                AddGossipItemFor(player, 0, "Click here to try Randomly Attacked Mode!", 0, 19);
                AddGossipItemFor(player, 0, "Click here to try Pacifist Mode!", 0, 22);
                AddGossipItemFor(player, 0, "Click here to try Diet Mode!", 0, 25);
                AddGossipItemFor(player, 0, "Click here to try Nomad Mode!", 0, 27);
                AddGossipItemFor(player, 0, "Click here to try Taxation Without Representation Mode!", 0, 30);
                if (!player->HasItemCount(SLOW_AND_STEADY_ITEM, 1, true) && !player->HasItemCount(HARDCORE_ITEM, 1, true) && !player->HasItemCount(800085, 1, true) && !player->HasItemCount(65000, 1, true))
                    AddGossipItemFor(player, 0, "Click here to try Dinklestone Mode!", 0, 18);
            }

            if (player->HasItemCount(HARDCORE_ITEM, 1, true))
                AddGossipItemFor(player, 0, "Remove hardcore mode from your character.", 0, 8);

            if (player->HasItemCount(SLOW_AND_STEADY_ITEM, 1, true))
                AddGossipItemFor(player, 0, "Remove Slow and Steady mode from your character.", 0, 17);

            auto level = player->GetLevel();
            if (player->HasAura(FRAGILE_WARRIOR_SPELL) && (level == 1 || level == 10 || level == 20 || level == 30 || level == 40 || level == 50 || level == 60 || level == 70 || level == 80))
                AddGossipItemFor(player, 0, "Remove Fragile Warrior mode from your character.", 0, 13);

            if (player->HasItemCount(800049, 1, true))
                AddGossipItemFor(player, 0, "Remove Ironman mode from your character.", 0, 16);

            if (player->HasItemCount(800050, 1, true))
                AddGossipItemFor(player, 0, "Remove Randomly Attacked mode from your character.", 0, 21);

            if (player->HasItemCount(PACIFIST_ITEM, 1, true))
                AddGossipItemFor(player, 0, "Remove Pacifist Mode from your character.", 0, 23);

            if (player->HasItemCount(DIET_ITEM, 1, true))
                AddGossipItemFor(player, 0, "Remove Diet Mode from your character.", 0, 26);

            if (player->HasItemCount(NOMAD_ITEM, 1, true))
                AddGossipItemFor(player, 0, "Remove Nomad Mode from your character.", 0, 28);

            if (player->HasItemCount(TAXATION_WITHOUT_REPRESENTATION_ITEM, 1, true))
                AddGossipItemFor(player, 0, "Remove Taxation Without Representation Mode from your character.", 0, 31);

            if (!PlayerHasChallenges(player) && !player->HasItemCount(65000, 1, true)) {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Give me my Dinklestone back!", 0, 35);
            }

            SendGossipMenuFor(player, 1, creature->GetGUID());
        }
        return true;
    }


    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        ClearGossipMenuFor(player);
        switch (action)
        {
        case 1:
            AddGossipItemFor(player, 0, "This challenge will heavily penalize you for any form of death. Additionally it will remove all your current gold, remove bonus starter items and Murky will no longer be with you! This challenge mode offers various rewards at certain milestones.", 0, 2);
            AddGossipItemFor(player, 0, "NO TAKE ME BACK!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;

        case 2:
            player->AddItem(HARDCORE_ITEM, 1);
            player->SetMoney(0);
            player->DestroyItemCount(60002, player->GetItemCount(60002, true), true);
            player->DestroyItemCount(10594, player->GetItemCount(10594, true), true);
            player->DestroyItemCount(65000, player->GetItemCount(65000, true), true);
            player->removeSpell(24939, SPEC_MASK_ALL, false);
            player->removeSpell(100117, SPEC_MASK_ALL, false);
            player->removeSpell(100118, SPEC_MASK_ALL, false);
            player->removeSpell(100105, SPEC_MASK_ALL, false);
            break;

        case 3:
            OnGossipHello(player, creature);
            break;

        case 8:
            player->DestroyItemCount(HARDCORE_ITEM, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Hardcore mode has been removed from your character.");
            CloseGossipMenuFor(player);
            break;

        case 9:
            AddGossipItemFor(player, 0, "Slow and Steady is a choice for adventurers seeking to savor their journey by halving the pace of their experience gains and removing the ability to train mounts. Along this measured path, enticing rewards await those who embrace the challenge. Dinklestone and other beneficial items will be removed during this challenge.", 0, 10);
            AddGossipItemFor(player, 0, "NO TAKE ME BACK!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;

        case 10:
            ChatHandler(player->GetSession()).PSendSysMessage("You have embraced the Slow and Steady Mode. Brace yourself for a sluggish adventure!");
            player->AddItem(SLOW_AND_STEADY_ITEM, 1);
            ActivateSlowAndSteady(player->GetGUID().GetCounter());
            player->SetMoney(0);
            player->DestroyItemCount(60002, player->GetItemCount(60002, true), true);
            player->DestroyItemCount(10594, player->GetItemCount(10594, true), true);
            player->DestroyItemCount(65000, player->GetItemCount(65000, true), true);
            player->removeSpell(24939, SPEC_MASK_ALL, false);
            player->removeSpell(100117, SPEC_MASK_ALL, false);
            player->removeSpell(100118, SPEC_MASK_ALL, false);
            player->removeSpell(100105, SPEC_MASK_ALL, false);
            player->DestroyItemCount(65000, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Your experience gains have been set to half!");
            CloseGossipMenuFor(player);
            break;

        case 11:
            AddGossipItemFor(player, 0, "Fragile Warrior Mode is for those seeking to test their skills under precarious conditions. In this mode, your damage done is decreased by 15% and damage taken is tripled. Brave the challenges and seek the path of the fragile warrior. Do you dare to take on this trial? You will only be able to remove this mode at 10 level intervals and currently only level 60 rewards are available for this mode.", 0, 12);
            AddGossipItemFor(player, 0, "NO TAKE ME BACK!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;

        case 12:
            player->CastSpell(player, FRAGILE_WARRIOR_SPELL, true);
            player->HandleEmoteCommand(1);
            creature->Say("You have embraced the Fragile Warrior Mode. May your battles be fierce and your victories hard-earned.", LANG_UNIVERSAL, player);
            CloseGossipMenuFor(player);
            break;

        case 13:
            player->RemoveAura(FRAGILE_WARRIOR_SPELL);
            ChatHandler(player->GetSession()).PSendSysMessage("Fragile Warrior Mode has been removed from your character.");
            CloseGossipMenuFor(player);
            break;

        case 14:
            AddGossipItemFor(player, 0, "Ironman Mode is for those seeking a true challenge. In this mode, you can only equip common or poor quality items. Brace yourself for a test of skill and perseverance. Are you up to the task?", 0, 15);
            AddGossipItemFor(player, 0, "NO TAKE ME BACK!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;
        case 15:
            player->AddItem(800049, 1);
            creature->Say("You have embraced the Ironman Mode. May your journey be treacherous and your rewards well-deserved.", LANG_UNIVERSAL, player);
            CloseGossipMenuFor(player);
            break;
        case 16:
            player->DestroyItemCount(800049, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Ironman mode has been removed from your character.");
            CloseGossipMenuFor(player);
            break;
        case 17:
            player->DestroyItemCount(SLOW_AND_STEADY_ITEM, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Slow and Steady mode has been removed from your character.");
            CloseGossipMenuFor(player);
            break;
        case 18:
            player->AddItem(65000, 1);
            CloseGossipMenuFor(player);
            break;
        case 19:
            AddGossipItemFor(player, 0, "In Randomly Attacked Mode, you will be randomly attacked by enemies along your journey. Be vigilant and stay on your guard! Are you ready for the challenge?", 0, 20);
            AddGossipItemFor(player, 0, "NO TAKE ME BACK!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;
        case 20:
            player->AddItem(800050, 1);
            ChatHandler(player->GetSession()).PSendSysMessage("You have embraced the Randomly Attacked Mode. Be vigilant and stay on your guard as enemies may attack you at any moment!");
            CloseGossipMenuFor(player);
            break;
        case 21:
            player->DestroyItemCount(800050, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Randomly Attacked mode has been removed from your character.");
            CloseGossipMenuFor(player);
            break;
        case 22:
            AddGossipItemFor(player, 0, "Pacifist Mode calls to those who wish to traverse the vast expanses of Azeroth, not as conquerors, but as harmonious explorers. Embrace this path, and no creature shall fall by your hand. Are you prepared to embody tranquility and tread a path of peace?", 0, 24);
            AddGossipItemFor(player, 0, "NO TAKE ME BACK!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;
        case 23:
            player->DestroyItemCount(PACIFIST_ITEM, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Pacifist Mode has been removed from your character.");
            CloseGossipMenuFor(player);
            break;
        case 24:
            player->AddItem(PACIFIST_ITEM, 1);
            ChatHandler(player->GetSession()).PSendSysMessage("You have embraced the Pacifist Mode. In this mode, you will not be able to kill any creature. Failing the challenge will leave you with a 7 day debuff. Joining a group in this mode will fail your challenge.");
            CloseGossipMenuFor(player);
            break;
        case 25:
            AddGossipItemFor(player, 0, "Embrace the Diet Mode, perfect for those looking to shed some pixels. Eating and drinking? Overrated! Prepare for an adventure that's as lean as it gets. Ready to experience the 'hangry' side of Azeroth?", 0, 29);
            AddGossipItemFor(player, 0, "NO TAKE ME BACK!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;
        case 26:
            player->DestroyItemCount(DIET_ITEM, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Diet Mode has been removed from your character.");
            CloseGossipMenuFor(player);
            break;
        case 27:
            AddGossipItemFor(player, 0, "Nomad Mode - because who needs a cozy inn or teleportation spells when the whole world is your home? Say goodbye to your Dinkletone and Hearthstone and say hello to a life of wandering. Ready to give up your creature comforts?", 0, 33);
            AddGossipItemFor(player, 0, "NO TAKE ME BACK!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;
        case 28:
            player->DestroyItemCount(NOMAD_ITEM, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Nomad Mode has been removed from your character.");
            CloseGossipMenuFor(player);
            break;
        case 29:
            player->AddItem(DIET_ITEM, 1);
            ChatHandler(player->GetSession()).PSendSysMessage("You have embraced the Diet Mode. Brace yourself for a lean and challenging adventure!");
            CloseGossipMenuFor(player);
            break;
        case 30:
            AddGossipItemFor(player, 0, "Think you're brave? Try Taxation Without Representation Mode! With a 75% gold tax, your wealth will disappear faster than a murloc's dignity at a fish market! Note: Being in a guild with guild funds module active will make earnings 90% of whatever you earned for guild funds. Highy reccomended to not do this challenge while in a guild, unless you want to be extra hardcore.", 0, 34);
            AddGossipItemFor(player, 0, "Wait, I changed my mind... I like my gold!", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;
        case 31:
            player->DestroyItemCount(TAXATION_WITHOUT_REPRESENTATION_ITEM, 1, true);
            ChatHandler(player->GetSession()).PSendSysMessage("Whew, you're free from the Taxation Without Representation Mode. Your coin purse breathes a sigh of relief.");
            CloseGossipMenuFor(player);
            break;
        case 33:
            player->AddItem(NOMAD_ITEM, 1);
            player->DestroyItemCount(65000, player->GetItemCount(65000, true), true);
            ChatHandler(player->GetSession()).PSendSysMessage("You have embraced the Nomad Mode. Get ready for a life of wandering!");
            CloseGossipMenuFor(player);
            break;
        case 34:
            player->AddItem(TAXATION_WITHOUT_REPRESENTATION_ITEM, 1);
            player->SetMoney(0);
            ChatHandler(player->GetSession()).PSendSysMessage("You've chosen the Taxation Without Representation Mode. Hope you're not too attached to your gold!");
            CloseGossipMenuFor(player);
            break;
        case 35: // Handle the new option for Dinklestone
            player->AddItem(65000, 1);
            ChatHandler(player->GetSession()).PSendSysMessage("Here's your Dinklestone, take good care of it!");
            CloseGossipMenuFor(player);
            break;
        }
        return true;
    }
};

class HARDCORE_PLAYER : public PlayerScript
{
public:
    HARDCORE_PLAYER() : PlayerScript("HARDCORE_PLAYER") { }

    void OnCreatureKill(Player* killer, Creature* killed) override
    {
        if (killer->HasItemCount(PACIFIST_ITEM, 1, true))
        {
            killer->CastSpell(killer, PACIFIST_FAILED_SPELL);
            killer->DestroyItemCount(PACIFIST_ITEM, 1, true);
            killer->PlayDirectSound(183253);
            ChatHandler(killer->GetSession()).PSendSysMessage("You have failed the Pacifist Mode Challenge!");
        }
    }

    void OnPlayerKilledByCreature(Creature* killer, Player* killed) override
    {
        if (!killed || !killer)
            return;

        if (killed->HasItemCount(HARDCORE_ITEM, 1, true))
        {
            //TODO: Send message to all players
            LOG_INFO("server.hardcore", fmt::format("{} was killed by {} and has failed the Hardcore Mode Challenge!", killed->GetName(), killer->GetName()));
            killed->PlayDirectSound(183253);
            killed->DestroyItemCount(HARDCORE_ITEM, 1, true);
            killed->CastSpell(killed, 80115, true);
        }
    }

    void OnPVPKill(Player* killer, Player* killed) override
    {
        if (!killed || !killer)
            return;

        //Pacifist Mode
        if (killer->HasItemCount(PACIFIST_ITEM, 1, true))
        {
            killer->CastSpell(killer, PACIFIST_FAILED_SPELL);
            killer->DestroyItemCount(PACIFIST_ITEM, 1, true);
            killer->PlayDirectSound(183253);
            ChatHandler(killer->GetSession()).PSendSysMessage("You have failed the Pacifist Mode Challenge!");
        }

        //Hardcore Mode
        if (killed->HasItemCount(HARDCORE_ITEM, 1, true))
        {
            SendWorldMessage(fmt::format("{} was killed by {} and has failed the Hardcore Mode Challenge!", killed->GetName(), killer->GetName()));
            LOG_INFO("server.hardcore", fmt::format("{} was killed by {} and has failed the Hardcore Mode Challenge!", killed->GetName(), killer->GetName()));
            killed->PlayDirectSound(183253);
            killed->DestroyItemCount(HARDCORE_ITEM, 1, true);
            killed->CastSpell(killed, 80115, true);
        }
    }

    void OnLevelChanged(Player* player, uint8 oldlevel) override
    {
        auto level = player->GetLevel();
        if (player->HasAura(FRAGILE_WARRIOR_SPELL) && (level == 1 || level == 10 || level == 20 || level == 30 || level == 40 || level == 50 || level == 60 || level == 70 || level == 80))
            ChatHandler(player->GetSession()).PSendSysMessage("You've reached a milestone in Fragile Warrior Mode. You have the option to remove the challenge by visiting the NPC.");

        CanGetRewards(player, oldlevel);
    }

    void OnLogin(Player* player) override
    {
        if (player->HasItemCount(SLOW_AND_STEADY_ITEM, 1, true))
            player->SetRestBonus(0);

        if (player->HasItemCount(HARDCORE_ITEM, 1, true))
        {
            if (RemoveItems(player)) {
                ChatHandler(player->GetSession()).PSendSysMessage("Welcome to Hardcore Mode. Please watch your step!"); \
            }
            RemoveSpells(player);
        }
    }

    void OnEquip(Player* player, Item* it, uint8 /*bag*/, uint8 /*slot*/, bool /*update*/) {
        if (player->HasItemCount(IRONMAN_ITEM))
        {
            if (it->GetTemplate()->Quality >= IRONMAN_QUALITY_THRESHOLD)
            {
                player->DestroyItemCount(IRONMAN_ITEM, 1, true);
                SendWorldMessage(fmt::format("{} has failed the Ironman Mode Challenge!", player->GetName()));
                LOG_INFO("server.ironman", fmt::format("{} has failed the Ironman Mode Challenge!", player->GetName()));
                player->PlayDirectSound(183253);
                player->CastSpell(player, IRONMAN_FAILED_SPELL, true);
            }
        }
    }

    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (player->HasItemCount(NOMAD_ITEM, 1, true))
        {
            auto spellId = spell->GetSpellInfo()->Id;
            for (auto& forbiddenSpell : NOMAD_FORBIDDEN_SPELLS)
            {
                if (forbiddenSpell == spellId)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Trying to sneak a teleport in Nomad Challenge Mode, huh? Nice try, but the only 'teleport' here is leg power!");
                    spell->cancel();
                }
            }
        }
    }

    void OnMoneyChanged(Player* player, int32& amount) override
    {
        // Only apply the taxation logic if the amount is positive
        if (amount > 0 && player->HasItemCount(TAXATION_WITHOUT_REPRESENTATION_ITEM, 1, true))
        {
            auto newAmount = amount * TAX_MULTIPLIER;
            if (newAmount < 1) // Check if the new amount is less than 1 copper
            {
                newAmount = 1; // Set the new amount to 1 copper
            }
            ChatHandler(player->GetSession()).SendSysMessage(getRandomTaxationMessage().c_str());
            amount = newAmount;
        }
    }

    void OnLearnSpell(Player* player, uint32 spellID) override
    {
        if (player->HasItemCount(SLOW_AND_STEADY_ITEM, 1, true))
        {
            for (auto& forbiddenSpell : SLOW_AND_STEADY_FORBIDDEN_SPELLS)
            {
                if (forbiddenSpell == spellID)
                {
                    player->removeSpell(spellID, SPEC_MASK_ALL, false);
                    player->CastSpell(player, 12158, true);
                    ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000Riding unlearned. You can't learn that spell while Slow and Steady mode is active.|r");
                }
            }
        }
    }
};

class HARDCORE_GROUP : public GroupScript
{
public:
    HARDCORE_GROUP() : GroupScript("HARDCORE_GROUP") {}

    void OnAddMember(Group* group, ObjectGuid guid) override
    {
        auto player = ObjectAccessor::FindPlayer(guid);
        if (player && player->HasItemCount(PACIFIST_ITEM, 1, true))
        {
            player->DestroyItemCount(PACIFIST_ITEM, player->GetItemCount(PACIFIST_ITEM, true), true);
            player->CastSpell(player, 12158, true);
            player->CastSpell(player, PACIFIST_FAILED_SPELL, true);
            player->PlayDirectSound(183253);
            SendWorldMessage(fmt::format("{} has failed the Pacifist Mode Challenge!", player->GetName()));
            LOG_INFO("server.pacifist", fmt::format("{} has failed the Pacifist Mode Challenge!", player->GetName()));
        }
    }

    void OnCreate(Group* group, Player* leader) override
    {
        if (leader->HasItemCount(PACIFIST_ITEM, 1, true))
        {
            leader->DestroyItemCount(PACIFIST_ITEM, leader->GetItemCount(PACIFIST_ITEM, true), true);
            leader->CastSpell(leader, 12158, true);
            leader->CastSpell(leader, PACIFIST_FAILED_SPELL, true);
            leader->PlayDirectSound(183253);
            SendWorldMessage(fmt::format("{} has failed the Pacifist Mode Challenge!", leader->GetName()));
            LOG_INFO("server.pacifist", fmt::format("{} has failed the Pacifist Mode Challenge!", leader->GetName()));
        }
    }
};

void AddSC_Hardcore()
{
    new HARDCORE_NPC();
    new HARDCORE_PLAYER();
    new HARDCORE_GROUP();
}
