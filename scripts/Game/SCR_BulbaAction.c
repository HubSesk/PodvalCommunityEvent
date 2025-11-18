class SCR_BulbaAction : SCR_MineEquipWeaponAction
{	
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		
		BaseWeaponManagerComponent baseWeaponManagerComponent = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		
		if (baseWeaponManagerComponent.GetCurrentWeapon())
			SetCannotPerformReason("Руки заняты");
		
		return controller.CanPlayItemGesture() && !baseWeaponManagerComponent.GetCurrentWeapon();
 	}
}