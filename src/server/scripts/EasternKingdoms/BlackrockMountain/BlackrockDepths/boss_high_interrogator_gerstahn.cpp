/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureScript.h"
#include "ScriptedCreature.h"
#include "blackrock_depths.h"

enum Spells
{
    SPELL_SHADOWWORDPAIN                                   = 10894,
    SPELL_MANABURN                                         = 14033,
    SPELL_PSYCHICSCREAM                                    = 8122,
    SPELL_SHADOWSHIELD                                     = 22417
};

class boss_high_interrogator_gerstahn : public CreatureScript
{
public:
    boss_high_interrogator_gerstahn() : CreatureScript("boss_high_interrogator_gerstahn") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetBlackrockDepthsAI<boss_high_interrogator_gerstahnAI>(creature);
    }

    struct boss_high_interrogator_gerstahnAI : public ScriptedAI
    {
        boss_high_interrogator_gerstahnAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 ShadowWordPain_Timer;
        uint32 ManaBurn_Timer;
        uint32 PsychicScream_Timer;
        uint32 ShadowShield_Timer;

        void Reset() override
        {
            ShadowWordPain_Timer = 4000;
            ManaBurn_Timer = 14000;
            PsychicScream_Timer = 32000;
            ShadowShield_Timer = 8000;
        }

        void JustEngagedWith(Unit* /*who*/) override { }

        void UpdateAI(uint32 diff) override
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            //ShadowWordPain_Timer
            if (ShadowWordPain_Timer <= diff)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
                    DoCast(target, SPELL_SHADOWWORDPAIN);
                ShadowWordPain_Timer = 7000;
            }
            else ShadowWordPain_Timer -= diff;

            //ManaBurn_Timer
            if (ManaBurn_Timer <= diff)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100, true))
                    DoCast(target, SPELL_MANABURN);
                ManaBurn_Timer = 10000;
            }
            else ManaBurn_Timer -= diff;

            //PsychicScream_Timer
            if (PsychicScream_Timer <= diff)
            {
                DoCastVictim(SPELL_PSYCHICSCREAM);
                PsychicScream_Timer = 30000;
            }
            else PsychicScream_Timer -= diff;

            //ShadowShield_Timer
            if (ShadowShield_Timer <= diff)
            {
                DoCast(me, SPELL_SHADOWSHIELD);
                ShadowShield_Timer = 25000;
            }
            else ShadowShield_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_high_interrogator_gerstahn()
{
    new boss_high_interrogator_gerstahn();
}
