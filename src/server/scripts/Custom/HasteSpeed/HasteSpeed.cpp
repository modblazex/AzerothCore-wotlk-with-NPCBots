#include "ScriptMgr.h"
#include "Player.h"
#include "SpellAuras.h"
#include "SpellScript.h"

class spell_custom_haste_to_speed : public SpellScriptLoader
{
public:
    spell_custom_haste_to_speed() : SpellScriptLoader("spell_custom_haste_to_speed") { }

    class spell_custom_haste_to_speed_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_custom_haste_to_speed_AuraScript);

        float CalculateHastePercentage(uint32 rating, uint8 level)
        {
            float hastePct = 0.0f;
            if (level <= 10)
            {
                hastePct = 26.0f / 10.0f * rating;
            }
            else if (level >= 11 && level <= 60)
            {
                hastePct = (17.33f - 1.0f) / (11.0f - 60.0f) * (level - 11) + 17.33f;
                hastePct *= rating / 10.0f;
            }
            else if (level >= 61 && level <= 70)
            {
                hastePct = (0.96f - 0.63f) / (61.0f - 70.0f) * (level - 61) + 0.96f;
                hastePct *= rating / 10.0f;
            }
            else if (level >= 71 && level <= 80)
            {
                hastePct = (0.59f - 0.3f) / (71.0f - 80.0f) * (level - 71) + 0.59f;
                hastePct *= rating / 10.0f;
            }

            return hastePct;
        }


        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Player* player = caster->ToPlayer())
                {
                    // Get Effect0's base points (percentage to convert)
                    int32 conversionPercentage = GetSpellInfo()->Effects[EFFECT_0].BasePoints;

                    // Get the player's haste rating
                    uint32 hasteRating = player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HASTE_MELEE);

                    // Get the player's level
                    uint8 level = player->getLevel();

                    // Calculate the haste percentage
                    float hastePct = CalculateHastePercentage(hasteRating, level);

                    // Calculate the percentage of haste percentage to be converted to movement speed
                    float speedPct = hastePct * conversionPercentage / 100.0f;

                    // Assign the result to the amount
                    amount = int32(speedPct);
                }
            }
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_custom_haste_to_speed_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_SPEED_NOT_STACK);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_custom_haste_to_speed_AuraScript();
    }
};

void AddSC_spell_custom_haste_to_speed()
{
    new spell_custom_haste_to_speed();
}
