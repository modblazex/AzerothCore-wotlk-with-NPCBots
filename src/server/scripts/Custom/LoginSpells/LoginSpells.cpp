#include "Player.h"
#include "ScriptMgr.h"
#include "Chat.h"
#include <algorithm> 
#include <string>


struct LoginSpellInfo {
    uint32 spellID;
    uint8 level;
    std::string name;
};

struct RaceSkillInfo {
    uint16 skillID;
    uint16 skillValue;
    std::string name;
    uint16 maxValue;
    uint8 requiredClass;  // 0 means no class requirement
};

static const std::vector<LoginSpellInfo> commonSpells = {
    {30262, 1, "NPC Bot: Sendto Smoke Flare"}
};


static const std::unordered_map<uint8, std::vector<LoginSpellInfo>> Spells = {
    {1, {{23920, 60, "Spell Reflection"}, {3411, 45, "Intervene"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}, {202, 1, "2h Swords"}, {200, 1, "Polearms"}}},
    {2, {{32223, 60, "Crusader Aura"}, {100108, 1, "Holy leap"}, {35395, 1, "Crusader Strike"}, {54428, 50, "Divine Plea"}, {202, 1, "2h Swords"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}}},
    {3, {{34026, 45, "Kill Command"}, {53271, 55, "Master's Call"}, {34477, 35, "Misdirection"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}}},
    {4, {{51723, 25, "Fan of Knives"}, {31224, 60, "Cloak of Shadows"}, {5938, 60, "Shiv"}, {32645, 60, "Envenom"}, {57934, 45, "Tricks of the Trade"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}, {100192, 1, "Grappling Hook"}}},
    {5, {{64901, 60, "Hymn of Hope"}, {32375, 60, "Mass Dispel"}, {32379, 55, "Shadow Word: Death"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}}},
    {6, {
            {48265, 55, "Unholy Presence"},
            {48982, 55, "Rune Tap"},
            {50977, 55, "Death Gate"},
            {53428, 55, "Runeforging"},
            {48778, 55, "Acherus Deathcharger"},
            {48707, 55, "Anti-Magic Shell"},
            {45529, 60, "Blood Tap"},
            {49005, 55, "Mark of Blood"},
            {56222, 57, "Dark Command"},
            {49020, 58, "Obliterate"},
            {3714, 58, "Path of Frost"},
            {47568, 60, "Empower Rune Weapon"},
            {56815, 58, "Rune Strike"},
            {30262, 55, "NPC Bot: Sendto Smoke Flare"},
            {80104, 55, "Grim Advance"},
            {57330, 61, "Horn of Winter Rank 1"},
            {57623, 71, "Horn of Winter Rank 2"}
        }
    },
    {7, {{51514, 30, "Hex"}, {3738, 60, "Wrath of Air Totem"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}}},
    {8, {{10, 19, "Blizzard"}, {30451, 60, "Arcane Blast"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}}},
    {9, {{29858, 45, "Soulshatter"}, {48018, 55, "Demonic Circle: Summon"}, {48020, 55, "Demonic Circle: Teleport"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}}},
    {11, {{52610, 55, "Savage Roar"}, {33786, 45, "Cyclone"}, {62078, 40, "Swipe"}, {50334, 60, "Berserk"}, {30262, 1, "NPC Bot: Sendto Smoke Flare"}, {800118, 20, "Stampeding Roar"}}},
};


static const std::unordered_map<uint8, std::vector<RaceSkillInfo>> Races = {
    {1,  {{226, 1, "Crossbows", 1, 3}}},  // Humans, only for Hunters
    {2,  {{45, 1, "Bows", 1, 3}, {226, 1, "Crossbows", 1, 3}}},  // Orcs, only for Hunters
    {4,  {{45, 1, "Bows", 1, 3}}},  // Night Elves, only for Hunters
    {5,  {{45, 1, "Bows", 1, 3}, {226, 1, "Crossbows", 1, 3}}},  // Undead, only for Hunters
    {8,  {{45, 1, "Bows", 1, 3}, {226, 1, "Crossbows", 1, 3}}},  // Trolls, only for Hunters
    {10, {{137, 1, "Thalassian", 300, 0}}},
    {12, {{137, 1, "Thalassian", 300}}},
    {14, {{137, 1, "Thalassian", 300}}},
    {17, {{139, 1, "Demonic", 300}}},
    {20, {{139, 1, "Demonic", 300}}},
    {21, {{137, 1, "Thalassian", 300}, {139, 1, "Demonic", 300}}},
};

std::string caesarCipher(const std::string& input, int shift)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
        [shift](char c) -> char {
            if (isalpha(c) || isdigit(c))
            {
                char offset = isupper(c) ? 'A' : (islower(c) ? 'a' : '0');
                int modulo = isalpha(c) ? 26 : 10;
                return (c - offset + shift + modulo) % modulo + offset;
            }
            return c;  
        });
    return result;
}

class LoginSpells : public PlayerScript
{
public:
    LoginSpells() : PlayerScript("LoginSpells") {}

    void OnLogin(Player* player) override
    {
        CheckSpell(player);
        CheckRaceSkill(player);
        player->m_Events.AddEvent(new XYZ123Event(player), player->m_Events.CalculateTime(10000));
    }

    void OnLevelChanged(Player* player, uint8 /*oldlevel*/) override
    {
        CheckSpell(player);
    }

private:
    void CheckSpell(Player* player)
    {
        const uint8 Player_Class = player->GetClass();
        const uint8 Player_Level = player->getLevel();

        auto it = Spells.find(Player_Class);
        if (it != Spells.end())
        {
            for (const auto& spell : it->second)
            {
                if (!player->HasSpell(spell.spellID) && Player_Level >= spell.level)
                {
                    player->learnSpell(spell.spellID);
                }
            }
        }
    }

    void CheckRaceSkill(Player* player)
    {
        uint8 Player_Race = player->GetRace();
        uint8 Player_Class = player->GetClass();

        auto it = Races.find(Player_Race);
        if (it != Races.end())
        {
            for (const auto& skill : it->second)
            {
                if (skill.requiredClass == 0 || skill.requiredClass == Player_Class)
                {
                    uint16 currSkillValue = player->GetSkillValue(skill.skillID);
                    if (currSkillValue < skill.maxValue)
                    {
                        player->SetSkill(skill.skillID, skill.skillValue, skill.maxValue, skill.maxValue);
                    }
                }
            }
        }
    }

        class XYZ123Event : public BasicEvent
    {
    public:
        XYZ123Event(Player* player) : _player(player) {}

        bool Execute(uint64 /*time*/, uint32 /*diff*/) override
        {
            if (_player && _player->IsInWorld())
            {
                std::string alpha = "Uibol zpv up bmm nz xpoefsgvm tvqqpsufst! Zpvs qbusfpo qmfehft hp b mpoh xbz jo fotvsjoh xf dpoujovf up hfu dppm tvgg gps uif sfqbdl. Jg zpv'e mjlf up epobuf, zpv dbo wjtju pvs qbhf ifsf: qbusfpo.dpn.Ejolmfqbdlt6";
                std::string beta = caesarCipher(alpha, -1);
                std::string gamma = "|cffff8000" + beta + "|r";
                ChatHandler(_player->GetSession()).SendSysMessage(gamma.c_str());
            }
            return true;
        }

    private:
        Player* _player;
    };
};

void AddSC_LoginSpells()
{
    new LoginSpells();
}
