modded class SCR_OpenStorageAction : SCR_InventoryAction
{
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		DE_TraderEntity trader = DE_TraderEntity.Cast(pOwnerEntity);
		if (!trader)
		{
			super.PerformActionInternal(manager, pOwnerEntity, pUserEntity);
			return;
		}
		
		CharacterVicinityComponent vicinity = CharacterVicinityComponent.Cast(pUserEntity.FindComponent(CharacterVicinityComponent));
		if (!vicinity)
			return;
		
		//vicinity.SetItemOfInterest(pOwnerEntity);
		manager.SetStorageToOpen(pOwnerEntity);
		manager.SetLootStorage(pOwnerEntity);
 	 	manager.OpenInventory();
	}
};