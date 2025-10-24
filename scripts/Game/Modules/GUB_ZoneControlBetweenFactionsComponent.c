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
	protected const int CHECK_INTERVAL_MS = 1000;

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

		GetGame().GetCallqueue().CallLater(TickEvaluate, CHECK_INTERVAL_MS, true);
	}

	protected void TickEvaluate()
	{
		if (!m_aControlLogics)
			return;

		foreach (GUB_ZoneControlBetweenFactionsLogic logic : m_aControlLogics)
		{
			if (!logic)
				continue;

			if (logic.TryComplete())
			{
				GetGame().GetCallqueue().Remove(TickEvaluate);
				break;
			}
		}
	}

	override void FillDescription(out string desc)
	{
		desc = desc + "<color hex=\"0xFFE2A74F\">" + "Контроль зоны между фракциями" + "<color name>\n";		
		desc = desc +  "" + "\n";
			
		foreach (GUB_ZoneControlBetweenFactionsLogic logic : m_aControlLogics) 
		{
			string sContinuously = "";
			if (!logic.m_bContinuously)
				sContinuously = "не ";

			desc = desc + "<color hex=\"0xFFE2A74F\">" + "Зона: " + logic.m_sZonePreviewName + "<color name>\n";
			desc = desc + "Удержать в течении: " + "<color hex=\"0xFFE2A74F\">" + logic.m_fTimeToComplete + " секунд" +"<color name>\n";
			desc = desc + "Если условине нарушается, таймер " + "<color hex=\"0xFFE2A74F\">" + sContinuously + "обнуляется!" +"<color name>\n";
			desc = desc + logic.m_sPreviewMessageToDescription + "\n";
			
			foreach (int i, GUB_ZoneControlConditionAbstract cnd : logic.m_aConditions)			
			{
				desc += cnd.FillDescription();
			}
		}
		desc = desc +  "\n\n";
	}
}
