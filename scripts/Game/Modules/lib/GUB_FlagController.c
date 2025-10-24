class GUB_FlagController
{
	protected ref array<string> flagNames;
	protected ref array<vector> flagStartLocalPoses;

	protected ref array<SlotManagerComponent> slotManagers;
	protected ref array<SCR_FlagComponent> flagComponents;

	protected FactionKey startFactionKey;
	protected FactionKey endFactionKey;

	protected ref SCR_Faction startFaction;
	protected ref SCR_Faction endFaction;

	protected bool isSecondFlag = false;

	void SetParams(array<string> FlagNames, FactionKey StartFactionKey, FactionKey EndFactionKey)
	{
		flagNames = FlagNames;
		startFactionKey = StartFactionKey;
		endFactionKey = EndFactionKey;
	}

	void Start()
	{
		slotManagers = new array<SlotManagerComponent>();
		flagComponents = new array<SCR_FlagComponent>();
		for (int i = 0; i < flagNames.Count(); i++)
		{
			slotManagers.Insert(SlotManagerComponent.Cast(GetGame().GetWorld().FindEntityByName(flagNames[i]).FindComponent(SlotManagerComponent)));
			flagComponents.Insert(SCR_FlagComponent.Cast(GetGame().GetWorld().FindEntityByName(flagNames[i]).FindComponent(SCR_FlagComponent)));
		}

		SCR_SortedArray<SCR_Faction> outFactions = new SCR_SortedArray<SCR_Faction>();
		SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetSortedFactionsList(outFactions);
		for(int i = 0; i < outFactions.Count(); i++)
		{
			if(outFactions[i].GetFactionKey() == startFactionKey)
				startFaction = outFactions[i];
			if (outFactions[i].GetFactionKey() == endFactionKey)
				endFaction = outFactions[i];
		}
		
		flagStartLocalPoses = new array<vector>();
		for (int i = 0; i < flagNames.Count(); i++)
		{
			vector matLS[4];
			slotManagers[i].GetSlotByName("Flag").GetLocalTransform(matLS);
			flagStartLocalPoses.Insert(matLS[3]);
		}

		ChangeFlag(false);
	}

	void ChangeFlag(bool IsSecondFlag)
	{
		isSecondFlag = IsSecondFlag;
		for (int i = 0; i < flagNames.Count(); i++)
		{
			if (!IsSecondFlag)
			{
				flagComponents[i].m_sFactionKey = startFactionKey;
				flagComponents[i].ChangeMaterial(startFaction.GetFactionFlagMaterial());
			}
			else
			{
				flagComponents[i].m_sFactionKey = endFactionKey;
				flagComponents[i].ChangeMaterial(endFaction.GetFactionFlagMaterial());
			}
		}
	}

	void Update(float percentage)
	{
		if (flagStartLocalPoses == null)
			Start();

		if (percentage > 0.5 && !isSecondFlag)
			ChangeFlag(true);
		else if (percentage <= 0.5 && isSecondFlag)
			ChangeFlag(false);

		for (int i = 0; i < flagNames.Count(); i++)
		{
			vector matLS[4];
			slotManagers[i].GetSlotByName("Flag").GetLocalTransform(matLS);
			matLS[3][1] = flagStartLocalPoses[i][1] * (Math.AbsFloat(percentage - 0.5) * 2 - 1);
			slotManagers[i].GetSlotByName("Flag").SetAdditiveTransformLS(matLS);
		}
		return;
	}
}