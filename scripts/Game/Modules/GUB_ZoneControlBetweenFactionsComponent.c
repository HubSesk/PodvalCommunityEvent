// ============================================================================
// GUB_ZoneControlBetweenFactionsComponent.c
// Считает SCR_ChimeraCharacter (игроки + ИИ) внутри PS_PolyZone и проверяет условия.
// Триггер-обводка создаётся автоматически и покрывает полигон (радиус считается из точек).
// Во время фризтайма (пока GameMode != GAME) подсчёты и завершение сценария не выполняются.
// Поддержан "тестовый режим": при недостаточном числе игроков завершение логики отключается.
// ============================================================================

class GUB_ZoneControlBetweenFactionsComponentClass : DRG_MissionModuleComponentClass {}

class GUB_ZoneControlBetweenFactionsComponent : DRG_MissionModuleComponent
{
	[Attribute(category: "Zone Control")]
	ref array<ref GUB_ZoneControlBetweenFactionsLogic> m_aControlLogics;

	protected PS_GameModeCoop m_GameModeCoop;

	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!Replication.IsServer())
			return;

		m_GameModeCoop = PS_GameModeCoop.Cast(GetGame().GetGameMode());
		DRG_MissionManagerComponent.BumpDescription();

		if (!GetGame().InPlayMode())
			return;

		if (m_aControlLogics)
		{
			foreach (GUB_ZoneControlBetweenFactionsLogic lg : m_aControlLogics)
				if (lg) lg.Init(m_GameModeCoop);
		}
	}

	override void FillDescription(out string desc)
	{
		desc = desc + "<color hex=\"0xFFE2A74F\">" + "Контроль зоны между фракциями" + "<color name>\n";		
			
		foreach (GUB_ZoneControlBetweenFactionsLogic logic : m_aControlLogics) 
		{
			if (logic.m_sZonePreviewName)
				desc = desc + "Зона: " + "<color hex=\"0xFFE2A74F\">" + logic.m_sZonePreviewName + "<color name>\n";
			if (logic.m_sPreviewMessageToDescription)
				desc = desc + logic.m_sPreviewMessageToDescription + "\n";
			desc = desc + "Время захвата: " + "<color hex=\"0xFFE2A74F\">" + FormatTime(logic.GetTimeToComplete()) + "<color name>\n";
			
			foreach (int i, GUB_ZoneControlConditionAbstract cnd : logic.m_aConditions)
				desc += cnd.FillDescription();
		}
		desc = desc + "\n";
	}

	string FormatTime(int seconds) {
    // Проверка на отрицательное время
    if (seconds < 0) {
        return "00:00:00";
    }
    
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    string result = string.Format("%1:%2:%3", hours, minutes, secs);
    return result;
	}
}
