[BaseContainerProps()]
modded class SCR_InventoryUIInfo : UIInfo
{
	override string GetInventoryItemDescription(InventoryItemComponent item)
	{
		DE_CashComponent cash = DE_CashComponent.Cast(item.GetOwner().FindComponent(DE_CashComponent));
		if (cash)
			return "$" + FormatFloat(cash.value);
		
		return GetDescription();
	}
}