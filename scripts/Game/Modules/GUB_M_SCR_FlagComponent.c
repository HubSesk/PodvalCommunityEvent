modded class SCR_FlagComponent
{
    [RplProp(onRplName: "OnFlagRaiseLevelChange")]
	protected float m_fFlagRaiseLevel = 1.0;

    [RplProp(onRplName: "OnFlagRaiseLevelChange")]
    protected vector startLocalPose;

    [Attribute(category: "FlagMovement")]
    bool m_bEnabled = false;

    protected SlotManagerComponent slotManager;

    override void EOnInit(IEntity owner)
    {
        super.EOnInit(owner);

        if (!m_bEnabled){
            return;
        }

        if (!Replication.IsServer()){
            return;
        }
    }

    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);

        if (!m_bEnabled){
            return;
        }

        slotManager = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
        if (!slotManager)
            return;
        
        vector matLS[4];
        slotManager.GetSlotByName("Flag").GetLocalTransform(matLS);
        startLocalPose = matLS[3];
    }

    float GetFlagRaiseLevel()
    {
        if (!m_bEnabled){
            return 1.0;
        }

        return m_fFlagRaiseLevel;
    }

    void ChangeFlagRaiseLevel(float NewRaiseLevel)
    {
        if (!m_bEnabled){
            return;
        }
        
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
        if (!m_bEnabled){
            return;
        }
        
        ChangeFlagRaiseLevel(m_fFlagRaiseLevel);
    }
}