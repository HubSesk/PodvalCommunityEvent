// ============================================================================
// GUB_ZoneControlComponent.c
// Считает SCR_ChimeraCharacter (игроки + ИИ) внутри PS_PolyZone и проверяет условия.
// Триггер-обводка создаётся автоматически и покрывает полигон (радиус считается из точек).
// Во время фризтайма (пока GameMode != GAME) подсчёты и завершение сценария не выполняются.
// Поддержан "тестовый режим": при недостаточном числе игроков завершение логики отключается.
// ============================================================================

class GUB_ZoneControlComponentClass : DRG_MissionModuleComponentClass {}

class GUB_ZoneControlComponent : DRG_MissionModuleComponent
{
	[Attribute(category: "Zone Control")]
	ref array<ref GUB_ZoneControlLogicAbstract> m_aControlLogics;

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
			foreach (GUB_ZoneControlLogicAbstract lg : m_aControlLogics)
				if (lg) lg.Init(m_GameModeCoop);
		}
	}

	override void FillDescription(out string desc)
	{
		desc = desc + "<color hex=\"0xFFE2A74F\">" + "Контроль зоны между фракциями" + "<color name>\n";		
			
		foreach (GUB_ZoneControlLogicAbstract logic : m_aControlLogics) 
		{
			desc = desc + logic.FillDescription();
		}
		desc = desc + "\n";
	}
}
