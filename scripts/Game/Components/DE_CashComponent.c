class DE_CashComponentClass : ScriptComponentClass
{}

class DE_CashComponent : ScriptComponent
{
	[RplProp(onRplName: "OnCashValueChanged"), Attribute("0", UIWidgets.Auto, desc: "Cash value contained in stack", category: "Dynamic Economy")]
	float value;
	
	void OnCashValueChanged()
	{
		// refresh loot listener to update item description in vicinity
		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.GetInventoryMenu();
		if (menu)
		{
			menu.UpdatePlayerData();
			menu.RefreshLootUIListener();
			menu.RefreshPlayerWidget();
		}
	}
	
	void ForceRplBump()
	{
		Replication.BumpMe();
	}
}