#include "ScriptMgr.h"
#include "Player.h"
#include "SpellAuraEffects.h"
#include <chrono>

class spell_dk_bone_shield_duskhaven : public AuraScript
{
    PrepareAuraScript(spell_dk_bone_shield_duskhaven);

    std::chrono::milliseconds lastProcTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    void HandleProc(ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

        if ((currentTime - lastProcTime).count() < 1000)
        {
            return;
        }

        lastProcTime = currentTime;  // Update last proc time

        if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->IsTargetingArea()) // Ensure effect is not proccing from an aoe spell
        {
            return;
        }

        Aura* aura = GetAura();
        if (!aura)
        {
            return;
        }

        // Remove aura stacks accordingly
        if (aura->GetStackAmount() > 1)
        {
            aura->ModStackAmount(-1);
        }
        else
        {
            aura->Remove();
        }
    }

    void Register() override
    {
        OnProc += AuraProcFn(spell_dk_bone_shield_duskhaven::HandleProc);
    }
};

void AddSC_spell_dk_bone_shield_duskhaven()
{
    RegisterSpellScript(spell_dk_bone_shield_duskhaven);
}
