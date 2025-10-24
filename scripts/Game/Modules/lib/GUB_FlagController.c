class GUB_FlagControllerClass : ScriptComponentClass {}

class GUB_FlagController : ScriptComponent
{
	[Attribute("", desc: "Flags, which indicates capturing status")]
	ref array<string> m_aFlagNames;

	[Attribute("")]
	FactionKey m_sStartFactionKey;

	[Attribute("")]
	FactionKey m_sEndFactionKey;
	
	protected ref array<vector> flagStartLocalPoses;

	protected ref array<SlotManagerComponent> slotManagers;
	protected ref array<SCR_FlagComponent> flagComponents;

	protected ref SCR_Faction startFaction;
	protected ref SCR_Faction endFaction;

	protected bool isSecondFlag = false;

	void Start()
	{
		slotManagers = new array<SlotManagerComponent>();
		flagComponents = new array<SCR_FlagComponent>();
		for (int i = 0; i < m_aFlagNames.Count(); i++)
		{
			slotManagers.Insert(SlotManagerComponent.Cast(GetGame().GetWorld().FindEntityByName(m_aFlagNames[i]).FindComponent(SlotManagerComponent)));
			flagComponents.Insert(SCR_FlagComponent.Cast(GetGame().GetWorld().FindEntityByName(m_aFlagNames[i]).FindComponent(SCR_FlagComponent)));
		}

		SCR_SortedArray<SCR_Faction> outFactions = new SCR_SortedArray<SCR_Faction>();
		SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetSortedFactionsList(outFactions);
		for(int i = 0; i < outFactions.Count(); i++)
		{
			if(outFactions[i].GetFactionKey() == m_sStartFactionKey)
				startFaction = outFactions[i];
			if (outFactions[i].GetFactionKey() == m_sEndFactionKey)
				endFaction = outFactions[i];
		}
		
		flagStartLocalPoses = new array<vector>();
		for (int i = 0; i < m_aFlagNames.Count(); i++)
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
		for (int i = 0; i < m_aFlagNames.Count(); i++)
		{
			if (!IsSecondFlag)
			{
				flagComponents[i].m_sFactionKey = m_sStartFactionKey;
				// flagComponents[i].ChangeMaterial(startFaction.GetFactionFlagMaterial());
				UpdateFlagMaterialClients(m_aFlagNames[i], startFaction.GetFactionFlagMaterial());
                Rpc(UpdateFlagMaterialServer, m_aFlagNames[i], startFaction.GetFactionFlagMaterial());
                Rpc(UpdateFlagMaterialOwner, m_aFlagNames[i], startFaction.GetFactionFlagMaterial());
                Rpc(UpdateFlagMaterialClients, m_aFlagNames[i], startFaction.GetFactionFlagMaterial());
			}
			else
			{
				flagComponents[i].m_sFactionKey = m_sEndFactionKey;
				// flagComponents[i].ChangeMaterial(endFaction.GetFactionFlagMaterial());
				UpdateFlagMaterialClients(m_aFlagNames[i], endFaction.GetFactionFlagMaterial());
                Rpc(UpdateFlagMaterialServer, m_aFlagNames[i], endFaction.GetFactionFlagMaterial());
                Rpc(UpdateFlagMaterialOwner, m_aFlagNames[i], endFaction.GetFactionFlagMaterial());
                Rpc(UpdateFlagMaterialClients, m_aFlagNames[i], endFaction.GetFactionFlagMaterial());
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

		for (int i = 0; i < m_aFlagNames.Count(); i++)
		{
			vector matLS[4];
			slotManagers[i].GetSlotByName("Flag").GetLocalTransform(matLS);
			matLS[3][1] = flagStartLocalPoses[i][1] * (Math.AbsFloat(percentage - 0.5) * 2 - 1);
			// slotManagers[i].GetSlotByName("Flag").SetAdditiveTransformLS(matLS);
            UpdateFlagPositionClients(m_aFlagNames[i], matLS);
            Rpc(UpdateFlagPositionServer, m_aFlagNames[i], matLS);
			Rpc(UpdateFlagPositionOwner, m_aFlagNames[i], matLS);
			Rpc(UpdateFlagPositionClients, m_aFlagNames[i], matLS);
		}
		return;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void UpdateFlagPositionServer(string FlagName, vector MatLS[4])
	{
		SlotManagerComponent.Cast(GetGame().GetWorld().FindEntityByName(FlagName).FindComponent(SlotManagerComponent)).GetSlotByName("Flag").SetAdditiveTransformLS(MatLS);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void UpdateFlagMaterialServer(string FlagName, ResourceName FlagMaterial)
	{
		SCR_FlagComponent.Cast(GetGame().GetWorld().FindEntityByName(FlagName).FindComponent(SCR_FlagComponent)).ChangeMaterial(FlagMaterial);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void UpdateFlagPositionOwner(string FlagName, vector MatLS[4])
	{
		SlotManagerComponent.Cast(GetGame().GetWorld().FindEntityByName(FlagName).FindComponent(SlotManagerComponent)).GetSlotByName("Flag").SetAdditiveTransformLS(MatLS);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void UpdateFlagMaterialOwner(string FlagName, ResourceName FlagMaterial)
	{
		SCR_FlagComponent.Cast(GetGame().GetWorld().FindEntityByName(FlagName).FindComponent(SCR_FlagComponent)).ChangeMaterial(FlagMaterial);
	}

    [RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
    void UpdateFlagPositionClients(string FlagName, vector MatLS[4])
    {
        SlotManagerComponent.Cast(GetGame().GetWorld().FindEntityByName(FlagName).FindComponent(SlotManagerComponent)).GetSlotByName("Flag").SetAdditiveTransformLS(MatLS);
    }
    [RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
    void UpdateFlagMaterialClients(string FlagName, ResourceName FlagMaterial)
    {
        SCR_FlagComponent.Cast(GetGame().GetWorld().FindEntityByName(FlagName).FindComponent(SCR_FlagComponent)).ChangeMaterial(FlagMaterial);
    }
}