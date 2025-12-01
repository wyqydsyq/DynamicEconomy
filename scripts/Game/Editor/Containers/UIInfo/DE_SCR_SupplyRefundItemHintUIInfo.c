[BaseContainerProps(configRoot: true)]
modded class SCR_SupplyRefundItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (m_fItemSupplyRefund == 0 || m_bIsSupplyStorageAvailable)
			return WidgetManager.Translate(GetName(), FormatFloat(m_fItemSupplyRefund));
		else 
			return WidgetManager.Translate(GetDescription(), FormatFloat(m_fItemSupplyRefund));
	}
}
