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
        startLocalPose[0] = 0;
        startLocalPose[2] = 0;
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
        matLS[3][0] = startLocalPose[2];
        matLS[3][1] = startLocalPose[1] * m_fFlagRaiseLevel;
        matLS[3][2] = startLocalPose[2];
        slotManager.GetSlotByName("Flag").SetAdditiveTransformLS(matLS);

        if (Replication.IsServer())
            Replication.BumpMe();
    }

    void OnFlagRaiseLevelChange()
    {
        ChangeFlagRaiseLevel(m_fFlagRaiseLevel);
    }

    ResourceName GetCurrentMaterial()
    {
        return m_sCurrentMaterial;
    }
}