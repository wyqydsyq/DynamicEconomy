[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_ItemResourceName", true)]
modded class SCR_ArsenalItemStandalone : SCR_ArsenalItem
{
	void SetItemResourceName(ResourceName resourceName)
	{
		m_ItemResourceName = resourceName;
		m_ItemResource = Resource.Load(m_ItemResourceName);
	}
}
