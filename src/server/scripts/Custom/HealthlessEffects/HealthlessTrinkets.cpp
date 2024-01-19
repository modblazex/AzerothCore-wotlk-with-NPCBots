#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Player.h"

class spell_custom_health_less_35_trigger : public SpellScriptLoader
{
public:
    spell_custom_health_less_35_trigger() : SpellScriptLoader("spell_custom_health_less_35_trigger") {}

    class spell_custom_health_less_35_trigger_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_custom_health_less_35_trigger_AuraScript);

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            Unit* target = eventInfo.GetActionTarget();
            if (!target)
                return false;

            return target->GetHealthPct() < 35.0f;
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_custom_health_less_35_trigger_AuraScript::CheckProc);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_custom_health_less_35_trigger_AuraScript();
    }
};

void AddSC_custom_healthless()
{
    new spell_custom_health_less_35_trigger();
}
