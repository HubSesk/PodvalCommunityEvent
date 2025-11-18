[BaseContainerProps()]
class GUB_FlagControllerCapture
{
	[Attribute()]
	protected ref array<string> m_aFlagNames;
    
	protected ref array<SCR_FlagComponent> flagComponents;
    protected FactionKey nowFactionKey;

	void Init()
	{
        if (!m_aFlagNames || m_aFlagNames.Count() == 0)
        {
            Debug.Error("GUB_FlagController: No FlagNames!");
            return;
        }

        flagComponents = new array<SCR_FlagComponent>();
        for (int i = 0; i < m_aFlagNames.Count(); i++)
        {
            IEntity entity = GetGame().GetWorld().FindEntityByName(m_aFlagNames[i]);
            if (!entity)
            {
                Debug.Error("GUB_FlagController: Can't find Entity by FlagName: {" + m_aFlagNames[i] + "}");
				delete flagComponents;
				flagComponents = null;
                continue;
            }
            Managed component = entity.FindComponent(SCR_FlagComponent);
            if (!component)
            {
                Debug.Error("GUB_FlagController: Can't find component by FlagName: {" + m_aFlagNames[i] + "}");
                delete flagComponents;
				flagComponents = null;
				continue;
            }
            flagComponents.Insert(SCR_FlagComponent.Cast(component));
        }
	}

	void Update(float Percentage, FactionKey factionKey)
	{
		if (!flagComponents)
			Init();

		for (int i = 0; i < flagComponents.Count(); i++)
		{
			float raiseLevel = (Percentage - 1) * 0.85;
            flagComponents[i].ChangeFlagRaiseLevel(raiseLevel);

            if (nowFactionKey != factionKey)
                ChangeFlags(factionKey);
		}
	}

    void ChangeFlags(FactionKey factionKey)
	{
        SCR_Faction ScrFaction = GetFaction(factionKey);
		for (int i = 0; i < m_aFlagNames.Count(); i++)
		{
			flagComponents[i].m_sFactionKey = ScrFaction.GetFactionKey();
			flagComponents[i].ChangeMaterial(ScrFaction.GetFactionFlagMaterial());
		}
        nowFactionKey = factionKey;
	}

    protected SCR_Faction GetFaction(FactionKey factionKey)
    {
        SCR_Faction faction;
		SCR_SortedArray<SCR_Faction> outFactions = new SCR_SortedArray<SCR_Faction>();
		SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetSortedFactionsList(outFactions);
		for(int i = 0; i < outFactions.Count(); i++)
		{
			if(outFactions[i].GetFactionKey() == factionKey)
				faction = outFactions[i];
		}
        return faction;
    }
}