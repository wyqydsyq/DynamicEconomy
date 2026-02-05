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
	
	void NotifyRepChange(UUID playerUuid, RplId traderRplId, float amount)
	{
		Rpc(HandleRepChange, playerUuid, traderRplId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void HandleRepChange(UUID playerUuid, RplId traderRplId, float amount)
	{
		DE_TraderEntity trader = DE_TraderEntity.Cast(Replication.FindItem(traderRplId));
		if (!trader)
			return;
		
		if (!trader.repMap.Contains(playerUuid))
			trader.repMap.Insert(playerUuid, 0);
		
		trader.repMap.Set(playerUuid, amount);
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
