#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "Chat.h"
#include "ScriptedGossip.h"
#include "WorldSession.h"

const static uint32 KARAZHAN_MAP_ID = 532;
const static uint32 MAX_LOW_LEVEL = 60;
const static uint32 MIN_HIGH_LEVEL = 61;
const static uint32 GOSSIP_ICON = 0;
const static float KARAZHAN_X = -11089.735;
const static float KARAZHAN_Y = -1988.7;
const static float KARAZHAN_Z = 49.755;
const static float KARAZHAN_ORIENTATION = 6;
const static uint32 REFRESH_INTERVAL = 10;
const static uint32 MINIMUM_LEVEL = 61;
const static uint32 MAXIMUM_LEVEL = 80;
const static uint32 CREATURE_SPELL_ID = 108000;
const static uint32 PLAYER_SPELL_ID = 108001;
const static uint32 SPELL_TO_CAST = 19484;

static std::unordered_map<uint32, uint32> ORIGINAL_CREATURE_LEVELS;

/* Ignore These CopyPasteFromEluna BEGIN */
class KaraWorldObjectInRangeCheck
{
public:
    KaraWorldObjectInRangeCheck(bool nearest, WorldObject const* obj, float range,
        uint16 typeMask = 0, uint32 entry = 0, uint32 hostile = 0, uint32 dead = 0);
    WorldObject const& GetFocusObject() const;
    bool operator()(WorldObject* u);

    WorldObject const* const i_obj;
    Unit const* i_obj_unit;
    FactionTemplateEntry const* i_obj_fact;
    uint32 const i_hostile; // 0 both, 1 hostile, 2 friendly
    uint32 const i_entry;
    float i_range;
    uint16 const i_typeMask;
    uint32 const i_dead; // 0 both, 1 alive, 2 dead
    bool const i_nearest;
};

KaraWorldObjectInRangeCheck::KaraWorldObjectInRangeCheck(bool nearest, WorldObject const* obj, float range,
    uint16 typeMask, uint32 entry, uint32 hostile, uint32 dead) :
    i_obj(obj), i_obj_unit(nullptr), i_obj_fact(nullptr), i_hostile(hostile), i_entry(entry), i_range(range), i_typeMask(typeMask), i_dead(dead), i_nearest(nearest)
{
    i_obj_unit = i_obj->ToUnit();
    if (!i_obj_unit)
        if (GameObject const* go = i_obj->ToGameObject())
            i_obj_unit = go->GetOwner();
    if (!i_obj_unit)
        i_obj_fact = sFactionTemplateStore.LookupEntry(14);
};

bool KaraWorldObjectInRangeCheck::KaraWorldObjectInRangeCheck::operator()(WorldObject* u)
{
    if (i_typeMask && !u->isType(TypeMask(i_typeMask)))
        return false;
    if (i_entry && u->GetEntry() != i_entry)
        return false;
    if (i_obj->GetGUID() == u->GetGUID())
        return false;
    if (!i_obj->IsWithinDistInMap(u, i_range))
        return false;
    Unit const* target = u->ToUnit();
    if (!target)
        if (GameObject const* go = u->ToGameObject())
            target = go->GetOwner();
    if (target)
    {
        if (i_dead && (i_dead == 1) != target->IsAlive())
            return false;

        if (i_hostile)
        {
            if (!i_obj_unit)
            {
                if (i_obj_fact)
                {
                    if ((i_obj_fact->IsHostileTo(*target->GetFactionTemplateEntry())) != (i_hostile == 1))
                        return false;
                }
                else if (i_hostile == 1)
                    return false;
            }
            else if ((i_hostile == 1) != i_obj_unit->IsHostileTo(target))
                return false;
        }
    }
    if (i_nearest)
        i_range = i_obj->GetDistance(u);
    return true;
};
/* Ignore These CopyPasteFromEluna END */

static std::list<Creature*> GetCreaturesInRangeOfObject(WorldObject* obj, float range)
{
    std::list<Creature*> list;

    KaraWorldObjectInRangeCheck checker(false, obj, range, TYPEMASK_UNIT, 0, 0, 1);
    Acore::CreatureListSearcher<KaraWorldObjectInRangeCheck> searcher(obj, list, checker);
    Cell::VisitAllObjects(obj, searcher, range);
    return list;
};

static std::list<Player*> GetPlayersInRangeOfObject(WorldObject* obj, float range)
{
    std::list<Player*> list;

    KaraWorldObjectInRangeCheck checker(false, obj, range, TYPEMASK_PLAYER, 0, 0, 1);
    Acore::PlayerListSearcher<KaraWorldObjectInRangeCheck> searcher(obj, list, checker);
    Cell::VisitAllObjects(obj, searcher, range);
    return list;
};

static void RemovePlayersFromGroup(Player* player)
{
    auto group = player->GetGroup();

    if (!group)
        return;

    bool lowLevelGroup = true;
    bool highLevelGroup = true;
    Player* member;

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        member = itr->GetSource();

        if (!member || !member->GetSession())
            continue;

        if (member->GetLevel() > MAX_LOW_LEVEL)
        {
            lowLevelGroup = false;
        }

        if (member->GetLevel() < MIN_HIGH_LEVEL)
        {
            highLevelGroup = false;
        }
    }

    if (!lowLevelGroup && !highLevelGroup)
    {
        group->RemoveMember(player->GetGUID());
        player->GetSession()->SendNotification("You were removed because your group must be either all level 60 and below, or all 61 and above, to enter Karazhan.");
    }
};

static void ApplyTimewalkingBuff(Player* player, bool apply)
{
    // Assuming ORIGINAL_CREATURE_LEVELS is a global or accessible cache, with key type changed to uint64
    auto& cachedLevels = ORIGINAL_CREATURE_LEVELS;
    std::list<Creature*> creatures = GetCreaturesInRangeOfObject(player, 100.0f);

    for (Creature* creature : creatures)
    {
        if (creature->GetInstanceId() != player->GetInstanceId() || creature->IsNPCBotOrPet())
            continue;

        // Use the correct method to get a uint64 representation of ObjectGuid
        uint64 creatureGUID = creature->GetGUID().GetRawValue(); // Replace GetRawValue with the actual method

        if (apply)
        {
            auto it = cachedLevels.find(creatureGUID);
            if (it == cachedLevels.end()) {
                cachedLevels[creatureGUID] = creature->GetLevel();
            }

            creature->AddAura(CREATURE_SPELL_ID, creature);
            creature->SetLevel(60);
        }
        else
        {
            creature->RemoveAura(CREATURE_SPELL_ID);
            auto it = cachedLevels.find(creatureGUID);
            if (it != cachedLevels.end())
                creature->SetLevel(it->second);
        }
    }

    if (apply)
    {
        if (!player->HasAura(PLAYER_SPELL_ID))
            player->AddAura(PLAYER_SPELL_ID, player);
    }
    else
    {
        if (player->HasAura(PLAYER_SPELL_ID))
            player->RemoveAura(PLAYER_SPELL_ID);
    }
}

static void RefreshTimeWalkingBuff(Player* player)
{
    if (player->GetMapId() != KARAZHAN_MAP_ID)
    {
        player->m_scheduler.CancelGroup(PLAYER_SPELL_ID);
        ApplyTimewalkingBuff(player, false);
        ChatHandler(player->GetSession()).PSendSysMessage("You have abandoned your Level 60 Karazhan event.");
        return;
    }

    std::list<Creature*> list = GetCreaturesInRangeOfObject(player, 100.0f);
    if (list.empty())
        return;

    for (auto& creature : list)
    {
        // Skip NPCBots and pets using the IsNPCBotOrPet function
        if (creature->GetInstanceId() == player->GetInstanceId() && !creature->IsNPCBotOrPet())
        {
            creature->AddAura(CREATURE_SPELL_ID, creature);
            creature->SetLevel(60);
        }
    }

    ApplyTimewalkingBuff(player, true);
}

class KARA_TIMEWALKING_PLAYER : public PlayerScript
{
public:
    KARA_TIMEWALKING_PLAYER() : PlayerScript("KARA_TIMEWALKING_PLAYER") { }

    void OnMapChanged(Player* player) override
    {
        if (player->GetMapId() != KARAZHAN_MAP_ID)
        {
            RemovePlayersFromGroup(player);
            if (player->HasAura(PLAYER_SPELL_ID))
            {
                player->m_scheduler.CancelGroup(PLAYER_SPELL_ID);
                ApplyTimewalkingBuff(player, false);
                ChatHandler(player->GetSession()).PSendSysMessage("You have left the dungeon. Timewalking has been reset.");
            }
        }
    }

    void OnPlayerResurrect(Player* player, float /*restore_percent*/, bool /*applySickness*/) override
    {
        if (player->HasAura(PLAYER_SPELL_ID) && player->GetMapId() == KARAZHAN_MAP_ID)
            ApplyTimewalkingBuff(player, true);
    }
};



class KARA_TIMEWALKING_NPC_ACESSOR : public CreatureScript
{
public:
    KARA_TIMEWALKING_NPC_ACESSOR() : CreatureScript("KARA_TIMEWALKING_NPC_ACESSOR") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetLevel() >= MINIMUM_LEVEL && player->GetLevel() <= MAXIMUM_LEVEL)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("You must be level 60 to access this feature.");
            return true;
        }

        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON, "Teleport me to Karazhan.", 0, 1);
        AddGossipItemFor(player, GOSSIP_ICON, "Nevermind..", 0, 2);
        SendGossipMenuFor(player, 1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        ClearGossipMenuFor(player);
        switch (action)
        {
            case 1:
                player->TeleportTo(KARAZHAN_MAP_ID, KARAZHAN_X, KARAZHAN_Y, KARAZHAN_Z, KARAZHAN_ORIENTATION);
                CloseGossipMenuFor(player);
                break;

            case 2: 
                CloseGossipMenuFor(player);
                break;
        }
        return true;
    }
};


class KARA_TIMEWALKING_NPC_ACTIVATE : public CreatureScript
{
public:
    KARA_TIMEWALKING_NPC_ACTIVATE() : CreatureScript("KARA_TIMEWALKING_NPC_ACTIVATE") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetLevel() >= MINIMUM_LEVEL && player->GetLevel() <= MAXIMUM_LEVEL)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Your level is too high to interact with this event.");
            return true;
        }

        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON, "Level 60 Karazhan. (Under maintenance. May not work as intended on multiplayer servers.)", 0, 1);
        AddGossipItemFor(player, GOSSIP_ICON, "Nevermind..", 0, 2);
        SendGossipMenuFor(player, 1, creature->GetGUID());

        auto location = player->GetPosition();
        auto map = player->GetMapId();
        auto group = player->GetGroup();
        if (group)
        {
            for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();

                if (!member || !member->GetSession())
                    continue;

                member->TeleportTo(map, location.GetPositionX(), location.GetPositionY(), location.GetPositionZ(), location.GetOrientation());
                member->CastSpell(member, SPELL_TO_CAST, true);
            }
        }
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        ClearGossipMenuFor(player);
        switch (action)
        {
        case 1:
            AddGossipItemFor(player, GOSSIP_ICON, "Please ensure all raid members are present prior to starting the event. As raid leader, should you leave the raid at any point, the event will cease to continue and scaling will no longer persist.\n\nProceed to speak with me again if you are prepared and wish to activate this event.", 1, 2);
            AddGossipItemFor(player, GOSSIP_ICON, "Nevermind..", 0, 3);
            SendGossipMenuFor(player, 1, creature->GetGUID());
            break;

        case 2:
            if (player->HasAura(PLAYER_SPELL_ID)) {
                player->m_scheduler.CancelGroup(PLAYER_SPELL_ID);
                ApplyTimewalkingBuff(player, false);
            }
            else
            {
            player->m_scheduler.Schedule(std::chrono::milliseconds(100), PLAYER_SPELL_ID, [player](TaskContext context)
                {
                    context.Repeat(std::chrono::milliseconds(100));
                    RefreshTimeWalkingBuff(player);
                });
            ApplyTimewalkingBuff(player, true);
            }
            CloseGossipMenuFor(player);
            break;
        case 3:
            CloseGossipMenuFor(player);
            break;
        }
        return true;
    }
};

void AddSC_KarazhanTW()
{
    new KARA_TIMEWALKING_PLAYER();
    new KARA_TIMEWALKING_NPC_ACESSOR();
    new KARA_TIMEWALKING_NPC_ACTIVATE();
}
