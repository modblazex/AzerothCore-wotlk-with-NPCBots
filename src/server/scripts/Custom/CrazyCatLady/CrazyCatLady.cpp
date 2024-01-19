#include "ScriptMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"

#define NPC_CAT 6368
#define NPC_CRAZY_CAT_LADY 6367
#define SPELL_DEATH 5
#define EMOTE_ID 15

std::vector<std::string> YellTexts = {
    "You killed Mr. Whiskers? Prepare to meet Mr. Death!",
    "Nine lives for him, zero for you!",
    "Congratulations, you've earned a one-way ticket to the litter box!",
    "You've just cat-astrophically messed up!",
    "Now you've done it, it's claw-mageddon time!",
    "A cat has nine lives, but you've only got one. Say goodbye!"
};

class npc_crazy_cat_lady : public CreatureScript
{
public:
    npc_crazy_cat_lady() : CreatureScript("npc_crazy_cat_lady") {}

    struct npc_crazy_cat_ladyAI : public ScriptedAI
    {
        npc_crazy_cat_ladyAI(Creature* creature) : ScriptedAI(creature) {}

        TaskScheduler scheduler;

        void UpdateAI(uint32 diff) override
        {
            scheduler.Update(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_crazy_cat_ladyAI(creature);
    }
};

class npc_cat : public CreatureScript
{
public:
    npc_cat() : CreatureScript("npc_cat") {}

    struct npc_catAI : public ScriptedAI
    {
        npc_catAI(Creature* creature) : ScriptedAI(creature) {}

        void JustDied(Unit* killer) override
        {
            if (!killer || killer->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Creature* crazyCatLady = me->FindNearestCreature(NPC_CRAZY_CAT_LADY, 50.0f))
            {
                uint32 yellIndex = urand(0, YellTexts.size() - 1);
                crazyCatLady->Yell(YellTexts[yellIndex], LANG_UNIVERSAL);

                if (npc_crazy_cat_lady::npc_crazy_cat_ladyAI* crazyCatLadyAI = dynamic_cast<npc_crazy_cat_lady::npc_crazy_cat_ladyAI*>(crazyCatLady->AI()))
                {
                    crazyCatLadyAI->scheduler.Schedule(Seconds(3), [killer, crazyCatLady](TaskContext /*context*/)
                        {
                            if (Player* player = killer->ToPlayer())
                            {
                                crazyCatLady->CastSpell(player, SPELL_DEATH, true);
                                crazyCatLady->HandleEmoteCommand(EMOTE_ID);
                            }
                        });
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_catAI(creature);
    }
};

void AddSC_crazy_cat_lady_and_cat()
{
    new npc_crazy_cat_lady();
    new npc_cat();
}
