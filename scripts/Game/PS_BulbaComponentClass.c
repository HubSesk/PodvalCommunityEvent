//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "GameScripted/Character", description: "Set character playable", color: "0 0 255 255", icon: HYBRID_COMPONENT_ICON)]
class PS_BulbaComponentClass: ScriptComponentClass
{
};

[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class PS_BulbaComponent : ScriptComponent
{
	IEntity m_cOwner;
	InventoryItemComponent m_Inventory;
	
	IEntity m_cInventoryOwner;
	SCR_ChimeraCharacter m_cInventoryCharacter;
	InventoryStorageSlot m_Slot;
	SCR_InventoryStorageManagerComponent m_inventoryManager;
	BaseWeaponManagerComponent m_baseWeaponManagerComponent;
	
	override void OnPostInit(IEntity owner)
	{
		m_cOwner = owner;
		m_Inventory = InventoryItemComponent.Cast(m_cOwner.FindComponent(InventoryItemComponent));
		m_Inventory.m_OnParentSlotChangedInvoker.Insert(SlotChange);
	}
	
	void SlotChange(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		GetGame().GetCallqueue().Remove(DropBulba);
		
		m_Slot = newSlot;
		
		if (!m_Slot)
			return;
		
		m_cInventoryOwner = newSlot.GetOwner();
		
		if (!m_cInventoryOwner)
			return;
		
		m_cInventoryCharacter = SCR_ChimeraCharacter.Cast(m_cInventoryOwner);
		
		if (!m_cInventoryCharacter)
			return;
		
		m_inventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_cInventoryCharacter.FindComponent(SCR_InventoryStorageManagerComponent));
		m_baseWeaponManagerComponent = BaseWeaponManagerComponent.Cast(m_cInventoryCharacter.FindComponent(BaseWeaponManagerComponent));
		
		GetGame().GetCallqueue().CallLater(DropBulba, 2000, true);
	}
	
	void DropBulba()
	{
		IEntity currentWeaponEntity = null;
		BaseWeaponComponent weaponComponent = m_baseWeaponManagerComponent.GetCurrentWeapon();
		if (weaponComponent)
			currentWeaponEntity = weaponComponent.GetOwner();
		if (currentWeaponEntity != m_cOwner)
			m_inventoryManager.TryRemoveItemFromInventory(m_cOwner);
	}
};