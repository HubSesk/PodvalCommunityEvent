modded class SCR_FlagComponent
{
    [RplProp(onRplName: "OnFlagRaiseLevelChange")]
	protected float m_fFlagRaiseLevel;

    [RplProp(onRplName: "OnFlagRaiseLevelChange")]
    protected vector startLocalPose;

    protected SlotManagerComponent slotManager;

    override void EOnInit(IEntity owner)
    {
        super.EOnInit(owner);

        if (!Replication.IsServer()){
			return;
		}

        m_fFlagRaiseLevel = 1.0;
    }

    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);

        slotManager = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
        if (!slotManager)
            return;
        
        vector matLS[4];
        slotManager.GetSlotByName("Flag").GetLocalTransform(matLS);
        startLocalPose = matLS[3];
    }

    float GetFlagRaiseLevel()
    {
        return m_fFlagRaiseLevel;
    }

    void ChangeFlagRaiseLevel(float NewRaiseLevel)
    {
        m_fFlagRaiseLevel = NewRaiseLevel;

        if (!slotManager)
        {
            Debug.Error("GUB_M_SCR_FlagComponent: not SlotManagerComponent in owner entity");
            return;
        }

        vector matLS[4];
        slotManager.GetSlotByName("Flag").GetLocalTransform(matLS);
        matLS[3][1] = startLocalPose[1] * m_fFlagRaiseLevel;
        slotManager.GetSlotByName("Flag").SetAdditiveTransformLS(matLS);

        if (Replication.IsServer())
            Replication.BumpMe();
    }

    void OnFlagRaiseLevelChange()
    {
        ChangeFlagRaiseLevel(m_fFlagRaiseLevel);
    }
}