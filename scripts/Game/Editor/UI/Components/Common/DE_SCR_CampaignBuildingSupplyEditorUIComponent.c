modded class SCR_CampaignBuildingSupplyEditorUIComponent : SCR_BaseEditorUIComponent
{
	ResourceName playerFundsPrefab = "{8A7A978B039EAED8}UI/layouts/Menus/PlayerFunds.layout";
	ref VerticalLayoutWidget fundsWidget;
	ref TextWidget walletWidget;
	ref TextWidget bankWidget;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		DE_VehicleTraderEntity trader = DE_VehicleTraderEntity.Cast(buildingEditorComponent.GetProviderEntity());
		if (!trader)
			return;
		
		HorizontalLayoutWidget mainContainer = HorizontalLayoutWidget.Cast(w.FindAnyWidget("Supply_InGame_CampaignBuilding"));
		if (!mainContainer)
			return;
				
		VerticalLayoutWidget fundsContainer = VerticalLayoutWidget.Cast(w.FindAnyWidget("Provider_Details"));
		if (!fundsContainer)
			return;
		
		fundsWidget = VerticalLayoutWidget.Cast(GetGame().GetWorkspace().CreateWidgets(playerFundsPrefab));
		//fundsWidget = VerticalLayoutWidget.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(playerFundsPrefab));
		if (!fundsWidget)
			return;
		
		fundsContainer.AddChild(fundsWidget);
		mainContainer.GetParent().AddChild(fundsContainer);
		mainContainer.SetVisible(false);
		
		walletWidget = TextWidget.Cast(fundsWidget.FindAnyWidget("WalletValue"));
		bankWidget = TextWidget.Cast(fundsWidget.FindAnyWidget("BankValue"));
		
		m_ProviderIcon.GetParent().GetParent().SetVisible(false);
		m_ProviderName.SetVisible(false);
		
		SizeLayoutWidget supplyValue = SizeLayoutWidget.Cast(w.FindAnyWidget("InGame_Supply_Size"));
		if (supplyValue)
			supplyValue.SetVisible(false);
		
		//buildingEditorComponent.SetForcedProvider();
		UpdateResources();
	}
	
	override protected void UpdateResources()
	{
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		DE_VehicleTraderEntity trader = DE_VehicleTraderEntity.Cast(buildingEditorComponent.GetProviderEntity());
		if (!trader)
			return super.UpdateResources();
		
		if (!m_ResourceComponent)
			return;
		
		SCR_ResourceConsumer bankConsumer;
		SCR_ResourceConsumer walletConsumer;
		DE_EconomySystem.GetInstance().GetPlayerCashConsumers(SCR_PlayerController.GetLocalPlayerId(), bankConsumer, walletConsumer);
		
		if (!bankConsumer || !walletConsumer)
			return;
		
		if (walletWidget)
			walletWidget.SetText("$" + FormatFloat(walletConsumer.GetAggregatedResourceValue()));

		if (bankWidget)
			bankWidget.SetText("$" + FormatFloat(bankConsumer.GetAggregatedResourceValue()));
	}
	
	override protected void SetProviderName(IEntity targetEntity)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(targetEntity.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		m_ProviderCallsign.SetText(providerComponent.GetProviderDisplayName());
	}
}
