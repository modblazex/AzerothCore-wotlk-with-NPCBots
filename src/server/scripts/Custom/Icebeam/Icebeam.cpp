#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Chat.h"

enum Constants {
    NPC_ID = 817260,
    SPELL_ID = 820089,
    WARNING_SPELL_ID = 80005,
    SOUND_ID = 8332,  // Added this line for the sound ID
    MIN_SPELL_TIMER = 3000, // 3 seconds
    MAX_SPELL_TIMER = 7000, // 5 seconds
    PLAYER_RANGE = 90 // Max distance to select a player
};

class npc_custom_spawner : public CreatureScript
{
public:
    npc_custom_spawner() : CreatureScript("npc_custom_spawner") { }

    struct npc_custom_spawnerAI : public ScriptedAI
    {
        npc_custom_spawnerAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            // Initialize the spell timer
            spellTimer = urand(MIN_SPELL_TIMER, MAX_SPELL_TIMER);
        }

        void DoCastSpell()
        {
            if (!me->IsNonMeleeSpellCast(false))
            {
                // Select a random player within 90 yards and in LoS
                if (Player* player = me->SelectNearestPlayer(PLAYER_RANGE))
                {
                    if (me->IsWithinLOSInMap(player))
                    {
                        // Orient towards the player
                        me->SetFacingToObject(player);

                        // Cast the warning spell on the player
                        DoCast(player, WARNING_SPELL_ID);

                        // Play the sound to the player
                        player->PlayDirectSound(SOUND_ID);  

                        // Send a warning to the player as a raid warning
                        ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000[Raid Warning]|r: You are the target of Glacial Ray!");

                        // Cast the main spell
                        DoCast(me, SPELL_ID);
                    }
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (spellTimer <= diff)
            {
                DoCastSpell();
                spellTimer = urand(MIN_SPELL_TIMER, MAX_SPELL_TIMER);
            }
            else
            {
                spellTimer -= diff;
            }
        }

    private:
        uint32 spellTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_custom_spawnerAI(creature);
    }
};

void AddSC_npc_custom_spawner()
{
    new npc_custom_spawner();
}
