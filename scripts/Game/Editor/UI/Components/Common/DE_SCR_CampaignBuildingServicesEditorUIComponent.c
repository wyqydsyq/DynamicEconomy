modded class SCR_CampaignBuildingServicesEditorUIComponent : SCR_BaseEditorUIComponent
{
	DE_VehicleTraderEntity trader;
	
	override void HandlerAttached(Widget w)
	{
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		trader = DE_VehicleTraderEntity.Cast(buildingEditorComponent.GetProviderEntity());
		if (!trader)
			return super.HandlerAttached(w);
		
		w.SetVisible(false);
	}
}
