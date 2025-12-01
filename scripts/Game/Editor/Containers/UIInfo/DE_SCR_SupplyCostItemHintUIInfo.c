[BaseContainerProps(configRoot: true)]
modded class SCR_SupplyCostItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	override string GetItemHintName(InventoryItemComponent item)
	{
		return WidgetManager.Translate(GetName(), FormatFloat(m_fItemSupplyCost));
	}
}
