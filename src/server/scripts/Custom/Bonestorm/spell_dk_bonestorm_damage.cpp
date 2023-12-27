#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include "Log.h"

#define SPELL_DK_BONESTORM 1200013
#define SPELL_DK_BONESTORM_DAMAGE 1200014
#define SPELL_DK_BONESTORM_AURA 1200015

// SpellScript for the damage spell
class spell_dk_bonestorm_damage : public SpellScript
{
    PrepareSpellScript(spell_dk_bonestorm_damage);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
      //  LOG_ERROR("scripts.custom", "Validating bonestorm damage spell");
        return ValidateSpellInfo({ SPELL_DK_BONESTORM_DAMAGE });
    }

    void HandleEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (Unit* target = GetHitUnit())
        {
           // LOG_ERROR("scripts.custom", "Handling bonestorm damage effect");
            int32 baseDamage = caster->GetTotalAttackPowerValue(BASE_ATTACK) * 0.12f;
           // LOG_ERROR("scripts.custom", "Bonestorm base damage before aura application: {}", baseDamage);

            if (Aura* bonestormAura = caster->GetAura(SPELL_DK_BONESTORM_AURA))
            {
                int32 storedRunicPower = bonestormAura->GetEffect(0)->GetAmount();
              //  LOG_ERROR("scripts.custom", "Stored runic power from Bonestorm aura: {}", storedRunicPower);

                // Correcting the calculation here: divide storedRunicPower by 10 to get the actual percentage
                int32 damageIncreasePercent = storedRunicPower / 10;
                int32 damageIncrease = CalculatePct(baseDamage, damageIncreasePercent);
               // LOG_ERROR("scripts.custom", "Damage increase from stored runic power: {}", damageIncrease);

                baseDamage += damageIncrease;
             //   LOG_ERROR("scripts.custom", "Bonestorm final damage after aura application: {}", baseDamage);
            }
            else
            {
             //   LOG_ERROR("scripts.custom", "Bonestorm aura not found on caster.");
            }

            SetHitDamage(baseDamage);
        }
    }

    void Register() override
    {
       // LOG_ERROR("scripts.custom", "Registering bonestorm damage spell");
        OnEffectHitTarget += SpellEffectFn(spell_dk_bonestorm_damage::HandleEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// AuraScript for the aura spell
class spell_dk_bonestorm_aura : public AuraScript
{
    PrepareAuraScript(spell_dk_bonestorm_aura);

    bool Load() override
    {
      //  LOG_ERROR("scripts.custom", "Loading bonestorm aura script");

        if (Player* playerCaster = GetCaster()->ToPlayer())
        {
            int32 runicPowerBeforeConsuming = playerCaster->GetPower(POWER_RUNIC_POWER);
           // LOG_ERROR("scripts.custom", "Bonestorm runic power before consuming: {}", runicPowerBeforeConsuming);

            playerCaster->SetPower(POWER_RUNIC_POWER, 0);
          //  LOG_ERROR("scripts.custom", "Bonestorm runic power set to 0.");

            int32 damageIncreasePercent = runicPowerBeforeConsuming; // 1% increase per Runic Power consumed
           // LOG_ERROR("scripts.custom", "Damage increase percent for Bonestorm aura: {}", damageIncreasePercent);

            playerCaster->CastCustomSpell(playerCaster, SPELL_DK_BONESTORM_AURA, &damageIncreasePercent, nullptr, nullptr, true);
          //  LOG_ERROR("scripts.custom", "Bonestorm aura applied with damage increase percent: {}", damageIncreasePercent);

            return true;
        }
        else
        {
         //   LOG_ERROR("scripts.custom", "Caster is NULL or not a player");
            return false;
        }
    }

    void Register() override
    {
      //  LOG_ERROR("scripts.custom", "Registering bonestorm aura script");
        // Here you can register any hooks you need for this aura script.
        // AfterEffectApply += AuraEffectApplyFn(spell_dk_bonestorm_aura::AfterApplyEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_spell_dk_scripts()
{
    RegisterSpellScript(spell_dk_bonestorm_damage);
    RegisterSpellScript(spell_dk_bonestorm_aura);
}
