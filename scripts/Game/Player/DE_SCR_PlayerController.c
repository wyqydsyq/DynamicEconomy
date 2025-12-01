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
			SCR_NotificationsComponent.SendLocal(DE_EconomySystem.GetInstance().sellNotification, itemCost);
		else if (itemCost < 0)
			SCR_NotificationsComponent.SendLocal(DE_EconomySystem.GetInstance().buyNotification, itemCost);
	}
}
