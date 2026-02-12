modded class SCR_PlayerController : PlayerController
{
	void NotifyPlayerDataChange(float resourceCost = 0)
	{
		Rpc(HandlePlayerDataChange, resourceCost);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void HandlePlayerDataChange(float itemCost)
	{
		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.GetInventoryMenu();
		if (menu)
		{
			menu.UpdatePlayerData();
			menu.RefreshLootUIListener();
			menu.RefreshPlayerWidget();
		}
		
		SCR_CampaignBuildingEditorComponent editorComp = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (editorComp)
			editorComp.OnResourcesChanged();
		
		if (itemCost > 0)
			SCR_NotificationsComponent.SendLocal(DE_EconomySystem.GetInstance().sellNotification, Math.AbsFloat(itemCost) * 100);
		else if (itemCost < 0)
			SCR_NotificationsComponent.SendLocal(DE_EconomySystem.GetInstance().buyNotification, Math.AbsFloat(itemCost) * 100);
	}
	
	void NotifyBankDataChange(RplId containerId, float amount)
	{
		Rpc(HandleBankDataChange, containerId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void HandleBankDataChange(RplId containerId, float amount)
	{
		// either player controller or character depending on bank or wallet
		IEntity containerOwner = IEntity.Cast(Replication.FindItem(containerId));
		if (!containerOwner) // wait for owner entity to be replicated (sometimes character takes a few frames)
			return DE_EconomySystem.GetInstance().callQueue.CallLater(HandleBankDataChange, 1000, param1: containerId, param2: amount);
		
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(containerOwner.FindComponent(SCR_ResourceComponent));
		if (!resource)
			return;
		
		SCR_ResourceContainer container = resource.GetContainer(EResourceType.CASH);
		container.SetResourceValue(amount);
		HandlePlayerDataChange(0);
	}
	
	void NotifyRepChange(RplId traderRplId, float amount)
	{
		Rpc(HandleRepChange, traderRplId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void HandleRepChange(RplId traderRplId, float amount)
	{
		PrintFormat("DE: Set trader rep %1 = %2", traderRplId, amount);
		DE_EconomySystem.GetInstance().localRepMap.Set(traderRplId, amount);
	}
	
	void RequestDeposit(RplId bankId, int playerId, float amount)
	{
		Rpc(DoDeposit, bankId, playerId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void DoDeposit(RplId bankId, int playerId, float amount)
	{
		DE_BankEntity bank = DE_BankEntity.Cast(Replication.FindItem(bankId));
		if (!bank)
			return;
		
		bank.DoDeposit(playerId, amount);
	}
	
	void RequestWithdraw(RplId bankId, int playerId, float amount)
	{
		Rpc(DoWithdraw, bankId, playerId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void DoWithdraw(RplId bankId, int playerId, float amount)
	{
		DE_BankEntity bank = DE_BankEntity.Cast(Replication.FindItem(bankId));
		if (!bank)
			return;
		
		bank.DoWithdraw(playerId, amount);
	}
}
