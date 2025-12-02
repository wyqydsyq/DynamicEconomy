modded class SCR_InventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
{
	override protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		DE_CashComponent cashComp = DE_CashComponent.Cast(item.FindComponent(DE_CashComponent));
		if (!Replication.IsServer() || !cashComp || cashComp.value <= 0 || !item)
			return super.OnItemAdded(storageOwner, item);
		
		DE_EconomySystem.GetInstance().callQueue.Call(MergeCashIntoWallet, cashComp, storageOwner, item);
	}
	
	void MergeCashIntoWallet(DE_CashComponent cashComp, BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		IEntity storageEntity = storageOwner.GetOwner();
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(storageEntity.GetParent());
		if (!char || char.GetCharacterController().GetLifeState() != ECharacterLifeState.ALIVE || char.GetCharacterController().IsUnconscious())
			return;
		
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		if (!charResource)
			return;
		
		SCR_ResourceContainer wallet = charResource.GetContainer(EResourceType.CASH);
		if (!wallet)
			return;
		
		wallet.IncreaseResourceValue(cashComp.value);
		SCR_EntityHelper.DeleteEntityAndChildren(item);
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(char);
		if (!playerId)
			return;
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!pc)
			return;
		
		pc.NotifyBankDataChange(Replication.FindId(char), wallet.GetResourceValue());
	}
}