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

// ============================================================================
// Условия выполнения
// ============================================================================
[BaseContainerProps()]
class GUB_ZoneControlConditionAbstract
{
	bool Evaluate(map<string, int> FactionCounts) {return false; }
	string FillDescription();

	protected int GetCountForFaction(map<string, int> FactionCounts, FactionKey key)
	{
		string skey = key;
		if (skey == "")
			return 0;

		int v = FactionCounts.Get(skey);
		return v;
	}
}

[BaseContainerProps()]
class GUB_ZoneControlCondition : GUB_ZoneControlConditionAbstract
{
	[Attribute(defvalue: "USSR", desc: "Faction Key")]
	FactionKey m_sFactionKey;

	[Attribute("0", UIWidgets.ComboBox, "Compare operator (=,<,>,<=,>=)", "", ParamEnumArray.FromEnum(DRG_ConditionOp))]
	int m_iOperator;

	[Attribute(defvalue: "0", desc: "Required count", uiwidget: UIWidgets.EditBox)]
	int m_iCount;

	override bool Evaluate(map<string, int> FactionCounts)
	{
		int FactionCount = GetCountForFaction(FactionCounts, m_sFactionKey);

		switch (m_iOperator)
		{
			case DRG_ConditionOp.EQUALS:            return FactionCount == m_iCount;
			case DRG_ConditionOp.LESS:              return FactionCount <  m_iCount;
			case DRG_ConditionOp.GREATER:           return FactionCount >  m_iCount;
			case DRG_ConditionOp.LESS_OR_EQUALS:    return FactionCount <= m_iCount;
			case DRG_ConditionOp.GREATER_OR_EQUALS: return FactionCount >= m_iCount;
		}
		return false;
	}

	override string FillDescription()
	{
		FactionManager factionManager = GetGame().GetFactionManager();				
		Faction faction = factionManager.GetFactionByKey(m_sFactionKey);
		
		string factionName = faction.GetFactionName();
						
		string operatorStr;			
		if (m_iOperator == DRG_ConditionOp.EQUALS)
		{
			operatorStr = "==";				
		} else if (m_iOperator == DRG_ConditionOp.LESS){
			operatorStr = "<";				
		} else if (m_iOperator == DRG_ConditionOp.GREATER){
			operatorStr = ">";
		} else if (m_iOperator == DRG_ConditionOp.LESS_OR_EQUALS){
			operatorStr = "<=";
		} else if (m_iOperator == DRG_ConditionOp.GREATER_OR_EQUALS){
			operatorStr = ">=";
		}
				
		string desc =" • " + factionName + " " + "<color hex=\"0xFFE2A74F\">" + operatorStr + "<color name>" + " " + m_iCount.ToString() + "\n";
		return desc;
	}
}

[BaseContainerProps()]
class GUB_ZoneControlBetweenFactionsCondition : GUB_ZoneControlConditionAbstract
{
	[Attribute(defvalue: "USSR", desc: "Faction Key")]
	FactionKey m_sFirstFactionKey;

	[Attribute(defvalue: "US", desc: "Other Faction Key to compare with")]
	FactionKey m_sSecondFactionKey;

	[Attribute("1.0", UIWidgets.Slider, "Comparison multiplier (e.g., 5.0 means 5 times more)", "0.1 10 0.1")]
	float m_fComparisonMultiplier;

	[Attribute("0", UIWidgets.ComboBox, "Compare operator (=,<,>,<=,>=)", "", ParamEnumArray.FromEnum(DRG_ConditionOp))]
	int m_iOperator;

	override bool Evaluate(map<string, int> FactionCounts)
	{
		// Применяем коэффициент к количеству другой фракции
		int firstFactionCount = GetCountForFaction(FactionCounts, m_sFirstFactionKey);
		int secondFactionCount = GetCountForFaction(FactionCounts, m_sSecondFactionKey);
		
		float adjustedSecondFactionCount = secondFactionCount * m_fComparisonMultiplier;
		
		switch (m_iOperator)
		{
			case DRG_ConditionOp.EQUALS:            return firstFactionCount == adjustedSecondFactionCount;
			case DRG_ConditionOp.LESS:              return firstFactionCount <  adjustedSecondFactionCount;
			case DRG_ConditionOp.GREATER:           return firstFactionCount >  adjustedSecondFactionCount;
			case DRG_ConditionOp.LESS_OR_EQUALS:    return firstFactionCount <= adjustedSecondFactionCount;
			case DRG_ConditionOp.GREATER_OR_EQUALS: return firstFactionCount >= adjustedSecondFactionCount;
		}
		return false;
	}

	override string FillDescription()
	{
		FactionManager factionManager = GetGame().GetFactionManager();				
		Faction firstFaction = factionManager.GetFactionByKey(m_sFirstFactionKey);
		Faction secondFaction = factionManager.GetFactionByKey(m_sSecondFactionKey);
		
		string factionName = firstFaction.GetFactionName();
		string otherFactionName = secondFaction.GetFactionName();
						
		string operatorStr;			
		if (m_iOperator == DRG_ConditionOp.EQUALS)
		{
			operatorStr = "==";				
		} else if (m_iOperator == DRG_ConditionOp.LESS){
			operatorStr = "<";				
		} else if (m_iOperator == DRG_ConditionOp.GREATER){
			operatorStr = ">";
		} else if (m_iOperator == DRG_ConditionOp.LESS_OR_EQUALS){
			operatorStr = "<=";
		} else if (m_iOperator == DRG_ConditionOp.GREATER_OR_EQUALS){
			operatorStr = ">=";
		}
		
		string multiplierText = "";
		if (m_fComparisonMultiplier != 1.0)
		{
			multiplierText = " (x" + m_fComparisonMultiplier.ToString() + ")";
		}
				
		string desc = desc + " • " + factionName + " " + "<color hex=\"0xFFE2A74F\">" + operatorStr + "<color name>" + " " + otherFactionName + multiplierText + "\n";
		return desc;
	}
}

// ============================================================================
// Timer
// ============================================================================
class GUB_ZoneControlTimer
{
	protected ref array<ref GUB_ZoneControlConditionAbstract> conditions;
	protected ref map<string, int> factionCounts;
	protected float timeToComplete;
	protected bool continuously;
	protected float timeNow = 0;

    void SetParams(array<ref GUB_ZoneControlConditionAbstract> Conditions, 
		map<string, int> FactionCounts, float TimeToComplete, bool Continuously)
		{
			conditions = Conditions;
			factionCounts = FactionCounts;
			timeToComplete = TimeToComplete;
			continuously = Continuously;
		}

	void ResetTimeIfContinuously() 
	{
		if (continuously)
			timeNow = 0; 
	}
	bool AddTime(float time)
	{
		timeNow += time;
		if (timeNow >= timeToComplete)
			return true;
		return false;
	}
	bool Check(float TimePassed)
	{
		if (!conditions || conditions.Count() == 0)
			return false;

		for (int i = 0; i < conditions.Count(); i++)
		{
			GUB_ZoneControlConditionAbstract cond = conditions[i];
			if (!cond)
			{
				ResetTimeIfContinuously();
				return false;
			}

			bool ok = cond.Evaluate(factionCounts);
			if (!ok)
			{
				ResetTimeIfContinuously();
				return false;
			}
		}

		return AddTime(TimePassed);
	}

}

// ============================================================================
// Логика контроля одной зоны PS_PolyZone
// ============================================================================
[BaseContainerProps()]
class GUB_ZoneControlBetweenFactionsLogic
{
	// Имя сущности зоны (ShapeEntity / PolylineShapeEntity), на которой висит PS_PolyZone (или у её родителя)
	[Attribute(defvalue: "", desc: "Entity name of zone (ShapeEntity) that has PS_PolyZone component")]
	string m_sZoneEntityName;

	[Attribute(defvalue: "", desc: "Preview name of zone")]
	string m_sZonePreviewName;

	// Период опроса (сек)
	[Attribute("1", UIWidgets.Slider, "Check period (seconds)", "0.2 10 0.2")]
	float m_fCheckPeriod;

	// Фильтры и поведение
	[Attribute("1", UIWidgets.CheckBox, "Count alive only (DEAD/DESTROYED excluded)", "")]
	bool m_bAliveOnly;

	[Attribute("1", UIWidgets.CheckBox, "Wait until freeze time ends (GameMode state == GAME)", "")]
	bool m_bWaitFreezeEnd;

	// --- Testing mode ---
	[Attribute(defvalue: "8", desc: "Number of players participating in tests.", uiwidget: UIWidgets.EditBox, category: "Testing")]
	int m_iTestingPlayerCounts;

	[Attribute(defvalue: "true", desc: "Use TestingMode", uiwidget: UIWidgets.EditBox, category: "Testing")]
	bool m_bUseTestingMode;

	[Attribute("0", UIWidgets.CheckBox, "Debug logging (print counts each tick)", "")]
	bool m_bDebug;

	// Тайминги/стейдж/уведомления
	[Attribute("5", UIWidgets.EditBox, "Advance game state after N seconds", "")]
	int m_iAdvanceAfterSec;

	[Attribute("0", UIWidgets.CheckBox, "Advance game stage to AAR (Debriefing)", "")]
	bool m_bAdvanceGameStage;

	[Attribute(defvalue: "", desc: "Objectives marked completed on success")]
	ref array<string> m_sSuccessObjectiveNames;

	[Attribute(defvalue: "", desc: "Objectives marked failed on completion")]
	ref array<string> m_sFailedObjectiveNames;

	[Attribute(defvalue: "", desc: "Notification message on completion (localized key or plain text)")]
	string m_sMessage;
	
	[Attribute(defvalue: "", desc: "")]
	string m_sPreviewMessageToDescription;

	[Attribute(defvalue: "60.0", desc: "How much time is needed for the condition to be fully met (sec)")]
	float m_fTimeToComplete;

	[Attribute("0", UIWidgets.CheckBox, "If the condition is interrupted, should the time be counted again?")]
	bool m_bContinuously;

	// Условия
	[Attribute(category: "Conditions")]
	ref array<ref GUB_ZoneControlConditionAbstract> m_aConditions;

	// Runtime
	protected PS_GameModeCoop m_GameModeCoop;
	protected bool m_bCompleted;

	protected IEntity m_ZoneEntity;                 // сущность зоны (ShapeEntity / PolylineShapeEntity)
	protected PS_PolyZone m_ZoneComp;               // компонент полигона
	protected BaseGameTriggerEntity m_Trigger;      // вспомогательный сферический триггер
	protected bool m_bQueryInFlight;

	protected ref GUB_ZoneControlTimer m_Timer;
	protected ref map<string, int> m_mFactionCounts; // счётчики по фракциям

	// Префаб сферического триггера (как в Seizing)
	protected const ResourceName RN_SPHERE_TRIGGER = "{59A6F1EBC6C64F79}Prefabs/Logic/SeizingTrigger.et";

	void Init(PS_GameModeCoop mode)
	{
		m_GameModeCoop = mode;
		m_bCompleted = false;
		m_bQueryInFlight = false;
		m_mFactionCounts = new map<string, int>();

		if (m_fTimeToComplete < 0)
			m_fTimeToComplete = 0;

		m_Timer = new GUB_ZoneControlTimer();
		m_Timer.SetParams(m_aConditions, m_mFactionCounts, m_fTimeToComplete, m_bContinuously);
		
		if (m_fCheckPeriod <= 0)
		m_fCheckPeriod = 1.0;
		

		ResolveZoneAndPrepare();
	}

	// метод, который зовёт внешний компонент раз в секунду
	bool TryComplete()
	{
		if (m_bCompleted)
			return true;

		// Если надо ждать окончания фриза — не продолжаем
		if (m_bWaitFreezeEnd && !IsFreezeTimeOver())
			return false;

		// Тестовый режим: не продолжаем, если игроков меньше порога
		if (m_bUseTestingMode && !HasEnoughPlayersForTesting())
			return false;

		// пнуть опрос, если всё готово
		EvaluateOnce();

		return m_bCompleted;
	}

	// Проверка «фризтайм окончен?»
	protected bool IsFreezeTimeOver()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return true; // нет режима — не блокируем

		if (gameMode.GetState() == SCR_EGameModeState.GAME)
			return true;

		return false;
	}

	// Проверка тестового режима (достаточно ли игроков)
	protected bool HasEnoughPlayersForTesting()
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return false;

		array<int> ids = {};
		pm.GetPlayers(ids);

		int curr = ids.Count();
		if (curr >= m_iTestingPlayerCounts)
			return true;

		return false;
	}

	// Поиск зоны и подготовка триггера
	protected void ResolveZoneAndPrepare()
	{
		if (m_ZoneComp && m_Trigger)
			return;

		if (m_sZoneEntityName == "")
			return;

		IEntity ent = GetGame().GetWorld().FindEntityByName(m_sZoneEntityName);
		if (!ent)
		{
			if (m_bDebug)
				Print(string.Format("GUB_ZoneControlBetweenFactions: zone entity '%1' not found (retry)", m_sZoneEntityName));
			return;
		}

		m_ZoneEntity = ent;
		m_ZoneComp = PS_PolyZone.Cast(ent.FindComponent(PS_PolyZone));
		if (!m_ZoneComp && ent.GetParent())
			m_ZoneComp = PS_PolyZone.Cast(ent.GetParent().FindComponent(PS_PolyZone));

		if (!m_ZoneComp)
		{
			if (m_bDebug)
				Print(string.Format("GUB_ZoneControlBetweenFactions: PS_PolyZone not found on '%1' or parent (retry)", m_sZoneEntityName));
			return;
		}

		Resource res = Resource.Load(RN_SPHERE_TRIGGER);
		if (!res)
		{
			Print("GUB_ZoneControlBetweenFactions: failed to load SeizingTrigger prefab", LogLevel.ERROR);
			return;
		}

		m_Trigger = BaseGameTriggerEntity.Cast(GetGame().SpawnEntityPrefabLocal(res, GetGame().GetWorld()));
		if (!m_Trigger)
		{
			Print("GUB_ZoneControlBetweenFactions: failed to spawn trigger", LogLevel.ERROR);
			return;
		}

		// радиус вычисляем автоматически по точкам шейпа
		float autoR = ComputeAutoRadius();
		m_Trigger.SetSphereRadius(autoR);

		// центрируем триггер на зоне
		m_ZoneEntity.AddChild(m_Trigger, -1);
		
				
		SwitchObjectives(m_sSuccessObjectiveNames, false);
		SwitchObjectives(m_sFailedObjectiveNames, true);

		// периодический опрос (параллельно с TryComplete — не конфликтует)
		GetGame().GetCallqueue().CallLater(EvaluateOnce, m_fCheckPeriod * 1000, true);
	}

	// Вычисляем радиус сферы, чтобы она накрывала полигон (по XZ)
	protected float ComputeAutoRadius()
	{
		ShapeEntity shape = ShapeEntity.Cast(m_ZoneEntity);
		if (!shape)
			return 150.0;

		array<vector> pts = {};
		shape.GetPointsPositions(pts);
		if (pts.IsEmpty())
			return 150.0;

		vector origin = m_ZoneEntity.GetOrigin();
		float r = 0.0;

		for (int i = 0; i < pts.Count(); i++)
		{
			vector pw = pts[i] + origin;
			float dx = pw[0] - origin[0];
			float dz = pw[2] - origin[2];
			float d  = Math.Sqrt(dx * dx + dz * dz);

			if (d > r)
				r = d;
		}

		r = r * 1.10;          // запас 10%
		if (r < 5.0) r = 5.0;  // минимальный радиус

		return r;
	}

	// Один шаг опроса
	protected void EvaluateOnce()
	{
		if (m_bCompleted)
			return;

		// Не трогаем ничего, пока фризтайм (если требуется ждать)
		if (m_bWaitFreezeEnd && !IsFreezeTimeOver())
			return;

		// Тестовый режим: не считаем, если игроков меньше порога
		if (m_bUseTestingMode && !HasEnoughPlayersForTesting())
			return;

		if (!m_ZoneComp || !m_Trigger)
		{
			ResolveZoneAndPrepare();
			return;
		}

		if (m_bQueryInFlight)
			return;

		m_Trigger.GetOnQueryFinished().Insert(OnTriggerQueryFinished);
		m_bQueryInFlight = true;
		m_Trigger.QueryEntitiesInside();
	}

	// Обработка результата триггера
	protected void OnTriggerQueryFinished(BaseGameTriggerEntity trigger)
	{
		m_bQueryInFlight = false;
		m_Trigger.GetOnQueryFinished().Remove(OnTriggerQueryFinished);

		m_mFactionCounts.Clear();

		array<IEntity> inside = {};
		int cnt = m_Trigger.GetEntitiesInside(inside);

		for (int i = 0; i < cnt; i++)
		{
			IEntity e = inside[i];

			// считаем только SCR_ChimeraCharacter (игроки + ИИ пехота)
			SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(e);
			if (!ch)
				continue;

			// жив?
			if (m_bAliveOnly)
			{
				CharacterControllerComponent ctrl = ch.GetCharacterController();
				if (!ctrl)
					continue;

				if (ctrl.GetLifeState() != ECharacterLifeState.ALIVE)
					continue;
			}

			// действительно в ПОЛИГОНЕ
			vector pos = ch.GetOrigin();
			if (!m_ZoneComp.IsInsidePolygon(pos))
				continue;

			// фракция
			string fkey = ch.GetFactionKey();
			if (fkey == "")
				continue;

			int cur = m_mFactionCounts.Get(fkey);
			if (cur == 0)
				m_mFactionCounts.Insert(fkey, 1);
			else
				m_mFactionCounts.Set(fkey, cur + 1);
		}

		// Отладка — перебор map по индексам
		if (m_bDebug)
		{
			int totalKeys = m_mFactionCounts.Count();
			for (int k = 0; k < totalKeys; k++)
			{
				string keyStr = m_mFactionCounts.GetKey(k);
				int    val    = m_mFactionCounts.GetElement(k);
				Print(string.Format("[GUB_ZoneControlBetweenFactions] Zone=%1 Faction=%2 Count=%3", m_sZoneEntityName, keyStr, val));
			}
		}
		
		CheckAndMaybeComplete();
	}

	// Проверка условий и завершение
	protected void CheckAndMaybeComplete()
	{
		if (m_bCompleted)
			return;

		// Пока фризтайм — не завершаем
		if (m_bWaitFreezeEnd && !IsFreezeTimeOver())
			return;

		// Тестовый режим: блокируем завершение при недостатке игроков
		if (m_bUseTestingMode && !HasEnoughPlayersForTesting())
			return;

		if (m_Timer.Check(m_fCheckPeriod))
			Complete();
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
		if (m_Trigger)
		{
			delete m_Trigger;
			m_Trigger = null;
		}
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
	void ~GUB_ZoneControlBetweenFactionsLogic()
	{
		GetGame().GetCallqueue().Remove(EvaluateOnce);
		if (m_Trigger)
		{
			delete m_Trigger;
			m_Trigger = null;
		}
	}
}