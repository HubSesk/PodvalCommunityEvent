class GUB_FlagControllerClass : ScriptComponentClass {}

class GUB_FlagController : ScriptComponent
{
	[Attribute()]
	protected ref array<string> m_aFlagNames;
	
	[Attribute()]
	protected FactionKey m_fStartFactionKey;
	[Attribute()]
	protected FactionKey m_fEndFactionKey;
	
	protected ref array<SCR_FlagComponent> flagComponents;

	protected ref SCR_Faction startFaction;
	protected ref SCR_Faction endFaction;

	[RplProp()]
	protected bool m_bIsSecondFlag = false;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!Replication.IsServer())
			return;
	}

	void Init()
	{
		flagComponents = new array<SCR_FlagComponent>();
		for (int i = 0; i < m_aFlagNames.Count(); i++)
		{
			IEntity entity = GetGame().GetWorld().FindEntityByName(m_aFlagNames[i]);
			if (!entity)
			{
				Debug.Error("GUB_FlagController: Can't find Entity by FlagName: {" + m_aFlagNames[i] + "}");
			}
			Managed component = entity.FindComponent(SCR_FlagComponent);
			if (!component)
			{
				Debug.Error("GUB_FlagController: Can't find component by FlagName: {" + m_aFlagNames[i] + "}");
				continue;
			}
			flagComponents.Insert(SCR_FlagComponent.Cast(component));
		}

		SCR_SortedArray<SCR_Faction> outFactions = new SCR_SortedArray<SCR_Faction>();
		SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetSortedFactionsList(outFactions);
		for(int i = 0; i < outFactions.Count(); i++)
		{
			if(outFactions[i].GetFactionKey() == m_fStartFactionKey)
				startFaction = outFactions[i];
			if (outFactions[i].GetFactionKey() == m_fEndFactionKey)
				endFaction = outFactions[i];
		}

		ChangeFlag(false);
	}

	void ChangeFlag(bool IsSecondFlag)
	{
		m_bIsSecondFlag = IsSecondFlag;
		for (int i = 0; i < m_aFlagNames.Count(); i++)
		{
			if (!IsSecondFlag)
			{
				flagComponents[i].m_sFactionKey = m_fStartFactionKey;
				flagComponents[i].ChangeMaterial(startFaction.GetFactionFlagMaterial());
			}
			else
			{
				flagComponents[i].m_sFactionKey = m_fEndFactionKey;
				flagComponents[i].ChangeMaterial(endFaction.GetFactionFlagMaterial());
			}
		}
	}

	void Update(float percentage)
	{
		if (!flagComponents)
			Init();

		if (!Replication.IsServer())
			return;

		bool shouldChangeFlag = (percentage > 0.5 && !m_bIsSecondFlag) || (percentage <= 0.5 && m_bIsSecondFlag);
		if (shouldChangeFlag)
            ChangeFlag(percentage > 0.5);

		for (int i = 0; i < m_aFlagNames.Count(); i++)
		{
			if (!flagComponents[i])
                continue;

			float raiseLevel = (Math.AbsFloat(percentage - 0.5) * 2 - 1) * 0.85;
            flagComponents[i].ChangeFlagRaiseLevel(raiseLevel);
		}
		return;
	}
}