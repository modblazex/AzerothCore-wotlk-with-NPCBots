#include "ScriptMgr.h"
#include "Chat.h"
#include "Player.h"
#include "GridNotifiers.h"

class spread_command : public CommandScript
{
public:
    spread_command() : CommandScript("spread_command") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> spreadCommandTable =
        {
            { "spread", SEC_PLAYER, false, &HandleSpreadCommand, "" }
        };

        return spreadCommandTable;
    }

    static bool HandleSpreadCommand(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        // Get the player's target, can be friendly or hostile, but not the player itself
        Unit* target = player->GetSelectedUnit();

        // If the target is not valid, use the player's position as the spread point
        if (!target)
        {
            target = player;
        }

        float initialDistance = 35.0f; // Default spread distance
        const float maxSpreadDistance = 80.0f; // Maximum spread distance
        const float searchDistance = 100.0f; // Search distance for nearby NPC bots
        const float minDistance = 1.0f; // Minimum spread distance

        // Parse the distance argument if provided
        if (args && *args)
        {
            initialDistance = atof(args);
            if (initialDistance <= 0.0f || initialDistance > maxSpreadDistance)
            {
                handler->SendSysMessage("Invalid distance. Please provide a positive number up to 80.");
                return false;
            }
        }

        std::list<Unit*> targetList;
        Acore::AnyUnitInObjectRangeCheck checker(player, searchDistance);
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(player, targetList, checker);
        Cell::VisitAllObjects(player, searcher, searchDistance);

        // Filter out units that are not NPC Bots, and the target itself
        targetList.remove_if([target](Unit* unit) {
            return !unit->IsNPCBot() || unit == target;
            });

        if (targetList.empty())
        {
            handler->SendSysMessage("No valid NPC Bots found.");
            return true;
        }

        float angleStep = 2 * M_PI / targetList.size();
        float angle = 0.0f;

        for (Unit* unit : targetList)
        {
            float distance = initialDistance;
            float x, y, z;
            bool inLoS;

            do {
                x = target->GetPositionX() + distance * cos(angle);
                y = target->GetPositionY() + distance * sin(angle);
                z = target->GetMap()->GetHeight(unit->GetPhaseMask(), x, y, target->GetPositionZ() + 2.0f, true);
                inLoS = target->IsWithinLOS(x, y, z);

                if (!inLoS && distance > minDistance) {
                    distance -= 3.0f; // Decrement distance if not in LoS and above min distance
                    if (distance > maxSpreadDistance) { // Ensure distance does not exceed max spread distance
                        distance = maxSpreadDistance;
                    }
                }
            } while (!inLoS && distance > minDistance);

            // If a point within LoS is not found within minDistance, don't move the unit
            if (!inLoS) {
                continue;
            }

            float speedXY = 40.0f; // Horizontal speed
            float speedZ = 15.0f;  // Vertical speed

            unit->GetMotionMaster()->MoveJump(x, y, z, speedXY, speedZ);
            angle += angleStep;
        }

        return true;
    }
};

void AddSC_spread_command()
{
    new spread_command();
}

class fan_command : public CommandScript
{
public:
    fan_command() : CommandScript("fan_command") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> fanCommandTable =
        {
            { "fan", SEC_PLAYER, false, &HandleFanCommand, "" }
        };

        return fanCommandTable;
    }

    static bool HandleFanCommand(ChatHandler* handler, const char* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Unit* target = player->GetSelectedUnit(); // Get the player's target

        // If the target is not valid, use the player's position as the fan point
        if (!target)
        {
            target = player;
        }

        bool fanFront = true; // Default to fan in front
        float distanceBetweenBots = 30.0f; // Default distance between bots in the fan

        // Parse the arguments if provided
        if (args && *args)
        {
            char direction[10] = "";
            sscanf(args, "%9s %f", direction, &distanceBetweenBots);
            std::string dirStr = direction;

            if (dirStr == "back")
            {
                fanFront = false;
            }
            else if (dirStr != "front")
            {
                handler->SendSysMessage("Invalid argument. Use '.fan front [distance]' or '.fan back [distance]'.");
                return false;
            }
        }

        const float halfCircle = M_PI; // Half circle for fan formation
        float facingAngle = target->GetOrientation(); // Get the orientation of the target
        const float maxDistance = 50.0f; // Maximum distance for bots

        std::list<Unit*> targetList;
        Acore::AnyFriendlyUnitInObjectRangeCheck checker(player, target, maxDistance);
        Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck> searcher(player, targetList, checker);
        Cell::VisitAllObjects(player, searcher, maxDistance);

        // Filter out units that are not NPC Bots
        targetList.remove_if([target](Unit* unit) {
            return !unit->IsNPCBot() || unit == target;
            });

        if (targetList.empty())
        {
            handler->SendSysMessage("No valid NPC Bots found.");
            return true;
        }

        float angleStep = halfCircle / (targetList.size() - 1);
        float startAngle = fanFront ? (facingAngle - halfCircle / 2) : (facingAngle + M_PI / 2);

        for (Unit* unit : targetList)
        {
            float angle = startAngle + angleStep * (&unit - &*targetList.begin());
            float distance = distanceBetweenBots;
            bool inLoS = false;
            float x, y, z; // Declare x, y, z outside the while loop

            while (distance <= maxDistance && !inLoS)
            {
                x = target->GetPositionX() + distance * cos(angle);
                y = target->GetPositionY() + distance * sin(angle);
                z = target->GetMap()->GetHeight(unit->GetPhaseMask(), x, y, target->GetPositionZ() + 2.0f, true);

                inLoS = target->IsWithinLOS(x, y, z);
                if (!inLoS)
                {
                    distance += 3.0f; // Increment distance if not in LoS
                }
            }

            if (inLoS)
            {
                float speedXY = 40.0f; // Horizontal speed
                float speedZ = 15.0f;  // Vertical speed
                unit->GetMotionMaster()->MoveJump(x, y, z, speedXY, speedZ);
            }
        }

        return true;
    }
};

void AddSC_fan_command()
{
    new fan_command();
}

