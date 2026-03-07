modded class SCR_BuildModeShowHideOnSupplyEnabledUIComponent : SCR_ScriptedWidgetComponent
{
	DE_TraderEntity trader;
	
	override void OnResourceTypeEnabledChanged(SCR_ResourceComponent resourceComponent, array<EResourceType> disabledResourceTypes)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingEditorComponent.GetProviderComponent();
		if (providerComponent)
			trader = DE_TraderEntity.Cast(providerComponent.GetOwner());
		
		if (!trader && m_ResourceComponent)
			trader = DE_TraderEntity.Cast(m_ResourceComponent.GetOwner());
		
		if (!trader)
			return super.OnResourceTypeEnabledChanged(resourceComponent, disabledResourceTypes);
		
		m_wRoot.SetVisible(true);
	}
}
