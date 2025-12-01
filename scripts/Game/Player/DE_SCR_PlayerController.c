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
			SCR_NotificationsComponent.SendLocal(DE_EconomySystem.GetInstance().sellNotification, Math.AbsFloat(itemCost));
		else if (itemCost < 0)
			SCR_NotificationsComponent.SendLocal(DE_EconomySystem.GetInstance().buyNotification, Math.AbsFloat(itemCost));
	}
	
	void NotifyBankDataChange(RplId containerId, float amount)
	{
		Rpc(HandleBankDataChange, containerId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void HandleBankDataChange(RplId containerId, float amount)
	{
		PrintFormat("DE: HandleBankDataChange(%1, %2)", containerId, amount);
		
		// either player controller or character depending on bank or wallet
		IEntity containerOwner = IEntity.Cast(Replication.FindItem(containerId));
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(containerOwner.FindComponent(SCR_ResourceComponent));
		if (!resource)
			return;
		
		SCR_ResourceContainer container = resource.GetContainer(EResourceType.CASH);
		container.SetResourceValue(amount);
		HandlePlayerDataChange(0);
		PrintFormat("DE: HandleBankDataChange - Update resource values: %1 value = %2", container, amount);
	}
	
	void RequestDeposit(RplId bankId, int playerId, float amount)
	{
		PrintFormat("DE: PC.RequestDeposit");
		Rpc(DoDeposit, bankId, playerId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void DoDeposit(RplId bankId, int playerId, float amount)
	{
		DE_BankEntity bank = DE_BankEntity.Cast(Replication.FindItem(bankId));
		if (!bank)
			return;
		
		PrintFormat("DE: PC.DoDeposit");
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
