[BaseContainerProps()]
class GUB_ZoneControlLogic : GUB_ZoneControlLogicAbstract
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
	protected ref GUB_FlagController m_FlagController;

	// Условия
	[Attribute(category: "Conditions")]
	ref array<ref GUB_ZoneControlConditionAbstract> m_aConditions;

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
		if (!m_FlagController)
		{
			Debug.Error("GUB_ZoneControlLogic: Initialize GUB_FlagController!");
			return;
		}

		m_ZoneController.Init(m_mFactionCounts);
		m_FlagController.Init();

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
		if (m_sPreviewMessageToDescription)
			desc = desc + m_sPreviewMessageToDescription + "\n";
		desc = desc + "Время захвата: " + "<color hex=\"0xFFE2A74F\">" + FormatTime(m_Timer.GetTimeToComplete()) + "<color name>\n";
		
		foreach (int i, GUB_ZoneControlConditionAbstract cnd : m_aConditions)
			desc += cnd.FillDescription();

		return desc;
	}
	
	protected void EvaluateOnce()
	{
		m_ZoneController.EvaluateOnce();
		CheckAndMaybeComplete();
	}

	// Возвращает true, только если все условия рабочие и их больше 0, и все выполнены
	protected bool CheckConditions()
	{
		if (!m_aConditions || m_aConditions.Count() == 0)
			return false;

		for (int i = 0; i < m_aConditions.Count(); i++)
		{
			GUB_ZoneControlConditionAbstract cond = m_aConditions[i];
			if (!cond)
				return false;

			if (!cond.Evaluate(m_mFactionCounts))
				return false;
		}
		return true;
	}

	// Проверка условий и завершение
	protected void CheckAndMaybeComplete()
	{
		if (m_bCompleted)
			return;

		if (CheckConditions())
		{
			if (m_Timer.AddTime(m_fCheckPeriod))
				Complete();
		}
		m_FlagController.Update(m_Timer.GetCapturePercentage());
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
	void ~GUB_ZoneControlLogic()
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
		if (m_FlagController)
		{
			delete m_FlagController;
			m_FlagController = null;
		}
	}
}