[BaseContainerProps()]
class GUB_ZoneControlLogicCapture : GUB_ZoneControlLogicAbstract
{
	[Attribute(category: "Description", desc: "Preview name of zone")]
	string m_sZonePreviewName;

	[Attribute(category: "Description")]
	string m_sPreviewMessageToDescription;

	// Период опроса (сек)
	[Attribute(category: "Mode", defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Check period (seconds)", params: "0.2 10 0.2")]
	float m_fCheckPeriod;

	// Тайминги/стейдж/уведомления
	[Attribute(category: "Mode", defvalue: "0", desc: "Advance game stage to AAR (Debriefing)", "")]
	bool m_bAdvanceGameStage;
	
	[Attribute(category: "Mode", defvalue: "5", desc: "Advance game state after N seconds")]
	int m_iAdvanceAfterSec;

	[Attribute(category: "Mode", desc: "Objectives marked completed on success")]
	ref array<string> m_sSuccessObjectiveNames;

	[Attribute(category: "Mode", desc: "Objectives marked failed on completion")]
	ref array<string> m_sFailedObjectiveNames;

	[Attribute(category: "Mode", desc: "Notification message on completion (localized key or plain text)")]
	string m_sMessage;

	[Attribute()]
	protected ref GUB_ZoneControlTimer m_Timer;
	[Attribute()]
	protected ref GUB_ZoneController m_ZoneController;
	[Attribute()]
	protected ref GUB_FlagControllerCapture m_FlagControllerCapture;
	
	[Attribute(desc: "Start Faction, this faction can't win")]
	protected FactionKey neutralFaction;

	protected FactionKey nowFaction;

	// Runtime
	protected PS_GameModeCoop m_GameModeCoop;
	protected bool m_bCompleted;

	protected ref map<string, int> m_mFactionCounts; // счётчики по фракциям

	override void Init(PS_GameModeCoop mode)
	{
		m_GameModeCoop = mode;
		m_bCompleted = false;
		m_mFactionCounts = new map<string, int>();

		if (m_fCheckPeriod <= 0)
		m_fCheckPeriod = 1.0;

		if (!m_Timer)
		{
			Debug.Error("GUB_ZoneControlLogic: Initialize GUB_ZoneControlTimer!");
			return;
		}
		if (!m_ZoneController)
		{
			Debug.Error("GUB_ZoneControlLogic: Initialize GUB_ZoneController!");
			return;
		}
		if (!m_FlagControllerCapture)
		{
			Debug.Error("GUB_ZoneControlLogic: Initialize GUB_FlagControllerCapture!");
			return;
		}

		m_ZoneController.Init(m_mFactionCounts);

		if (neutralFaction != "")
		{
			m_Timer.AddTime(m_Timer.GetTimeToComplete());
			nowFaction = neutralFaction;
		}

		SwitchObjectives(m_sSuccessObjectiveNames, false);
		SwitchObjectives(m_sFailedObjectiveNames, true);

		// периодический опрос (параллельно с TryComplete — не конфликтует)
		GetGame().GetCallqueue().CallLater(EvaluateOnce, m_fCheckPeriod * 1000, true);
	}

	override string FillDescription()
	{
		string desc = "";
		if (m_sZonePreviewName)
			desc = desc + "Зона: " + "<color hex=\"0xFFE2A74F\">" + m_sZonePreviewName + "<color name>\n";
		if (neutralFaction != "")
			desc = desc + "Территория изначально принадлежит: " + "<color hex=\"0xFFE2A74F\">" + neutralFaction + "<color name>\n";
		if (m_sPreviewMessageToDescription)
			desc = desc + m_sPreviewMessageToDescription + "\n";
		desc = desc + "Время захвата: " + "<color hex=\"0xFFE2A74F\">" + FormatTime(m_Timer.GetTimeToComplete()) + "<color name>\n";

		return desc;
	}
	
	protected void EvaluateOnce()
	{
		m_ZoneController.EvaluateOnce();
		CheckAndMaybeComplete();
	}

	protected FactionKey GetDominatingFaction()
	{
		int maxValue = 0;
		string dominatingFaction = "";

		int totalKeys = m_mFactionCounts.Count();
		for (int k = 0; k < totalKeys; k++)
		{
			string keyStr = m_mFactionCounts.GetKey(k);
			int val = m_mFactionCounts.GetElement(k);
			
			if (val == maxValue)
				dominatingFaction = "";
			if (val > maxValue)
			{
				maxValue = val;
				dominatingFaction = keyStr;
			}
		}
		return dominatingFaction;
	}

	// Проверка условий и завершение
	protected void CheckAndMaybeComplete()
	{
		if (m_bCompleted)
			return;

		FactionKey dominatingFaction = GetDominatingFaction();
		if (GetDominatingFaction())
		{
			if (nowFaction == dominatingFaction)
			{
				if (neutralFaction != dominatingFaction)
				{
					if (m_Timer.AddTime(m_fCheckPeriod))
						Complete();
				}
			}
			else if (dominatingFaction != "")
			{
				if (m_Timer.MinusTime(m_fCheckPeriod))
					nowFaction = dominatingFaction;
			}
		}
		
		m_FlagControllerCapture.Update(m_Timer.GetCapturePercentage(), nowFaction);
	}

	// --- Завершение/уведомления/стейт ---
	protected void Complete()
	{
		m_bCompleted = true;

		SwitchObjectives(m_sSuccessObjectiveNames, true);
		SwitchObjectives(m_sFailedObjectiveNames, false);

		if (m_sMessage != "")
			NotifyPlayers(m_sMessage);

		if (m_bAdvanceGameStage)
		{
			int delayMs = 0;
			if (m_iAdvanceAfterSec > 0)
				delayMs = m_iAdvanceAfterSec * 1000;

			GetGame().GetCallqueue().CallLater(AdvanceState, delayMs, false);
		}

		// Останавливаем периодику и чистим триггер
		GetGame().GetCallqueue().Remove(EvaluateOnce);
	}

	protected void AdvanceState()
	{
		if (!m_GameModeCoop)
			m_GameModeCoop = PS_GameModeCoop.Cast(GetGame().GetGameMode());

		if (m_GameModeCoop)
			m_GameModeCoop.AdvanceGameState(SCR_EGameModeState.GAME);
	}

	protected void SwitchObjectives(array<string> names, bool completed)
	{
		if (!names)
			return;

		foreach (string n : names)
		{
			if (n == "")
				continue;

			IEntity e = GetGame().GetWorld().FindEntityByName(n);
			if (!e)
				continue;

			PS_Objective o = PS_Objective.Cast(e);
			if (o)
				o.SetCompleted(completed);
		}
	}

	protected void NotifyPlayers(string msg)
	{
		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		if (!chatPanelManager)
			return;

		ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("smsg");
		if (invoker)
			invoker.Invoke(null, msg);
	}

	// Чистка при удалении
	void ~GUB_ZoneControlLogicCapture()
	{
		GetGame().GetCallqueue().Remove(EvaluateOnce);
		if (m_Timer)
		{
			delete m_Timer;
			m_Timer = null;
		}
		if (m_ZoneController)
		{
			delete m_ZoneController;
			m_ZoneController = null;
		}
		if (m_FlagControllerCapture)
		{
			delete m_FlagControllerCapture;
			m_FlagControllerCapture = null;
		}
	}
}