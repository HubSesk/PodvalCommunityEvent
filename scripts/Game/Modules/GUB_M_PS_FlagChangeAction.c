modded class PS_FlagChangeAction
{
    protected const float tick_sec = 1 / 30;
    protected float duration;
    protected SCR_FlagComponent fc;
    protected string start_faction_key;
    protected ResourceName start_material;
    protected SCR_Faction finish_faction;
    protected bool is_finish_flag;
	
	protected float start_time;

    override void OnActionStart(IEntity pUserEntity)
    {
		if (!Replication.IsServer())
			return;
		
        duration = GetActionDuration();
        is_finish_flag = false;
		start_time = GetGame().GetWorld().GetWorldTime();
        
        // Получаем SCR_FlagComponent
        IEntity entity = GetOwner();
        if (!entity)
        {
            Debug.Error("GUB_M_PS_FlagChangeAction: Can't find Owner");
            return;
        }
        Managed component = entity.FindComponent(SCR_FlagComponent);
        if (!component)
        {
            Debug.Error("GUB_M_PS_FlagChangeAction: Can't find SCR_FlagComponent");
            return;
        }
        fc = SCR_FlagComponent.Cast(component);
        
        // Получаем начальную фракцию
        start_material = fc.GetCurrentMaterial();
		start_faction_key = fc.m_sFactionKey;
        
        // Получаем фракцию захватчика
        FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(pUserEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
        return;
        finish_faction = SCR_Faction.Cast(factionAffiliationComponent.GetDefaultAffiliatedFaction());
        
        int tick_ms = 1000 * tick_sec;
        GetGame().GetCallqueue().CallLater(TickEvaluate, tick_ms, true);
    }

    override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!Replication.IsServer())
			return;
		
        GetGame().GetCallqueue().Remove(TickEvaluate);
        fc.ChangeFlagRaiseLevel(0);
		fc.m_sFactionKey = start_faction_key;
		fc.ChangeMaterial(start_material);
        is_finish_flag = false;
    }

    void TickEvaluate()
    {
		float thisTime = GetGame().GetWorld().GetWorldTime();
		float nowLevel = (thisTime - start_time) / duration / 1000;
		
        fc.ChangeFlagRaiseLevel((Math.AbsFloat(nowLevel - 0.5) * 2 - 1) * 0.85);
        if (!is_finish_flag && nowLevel > 0.5)
        {
            m_FlagComponent.ChangeMaterial(finish_faction.GetFactionFlagMaterial());
            is_finish_flag = true;
        }
        if (nowLevel >= 1)
        {
            GetGame().GetCallqueue().Remove(TickEvaluate);
            fc.ChangeFlagRaiseLevel(0);
        }
    }
}