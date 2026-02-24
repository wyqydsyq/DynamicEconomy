modded class SCR_CampaignBuildingSupplyEditorUIComponent : SCR_BaseEditorUIComponent
{
	ResourceName playerFundsPrefab = "{8A7A978B039EAED8}UI/layouts/Menus/PlayerFunds.layout";
	ResourceName playerRepPrefab = "{AD41270B13749FC2}UI/layouts/Menus/PlayerRep.layout";
	ref VerticalLayoutWidget fundsContainerWidget;
	ref VerticalLayoutWidget repContainerWidget;
	ref TextWidget repWidget;
	ref TextWidget walletWidget;
	ref TextWidget bankWidget;
	DE_VehicleTraderEntity trader;
	
	override void HandlerAttached(Widget w)
	{
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		trader = DE_VehicleTraderEntity.Cast(buildingEditorComponent.GetProviderEntity());
		super.HandlerAttached(w);
		if (!trader)
			return;
		
		if (m_FactionComponent)
			m_FactionComponent.SetAffiliatedFaction(SCR_FactionManager.SGetPlayerFaction(SCR_PlayerController.GetLocalPlayerId()));
		
		HorizontalLayoutWidget mainContainer = HorizontalLayoutWidget.Cast(w.FindAnyWidget("Supply_InGame_CampaignBuilding"));
		if (!mainContainer)
			return;
				
		VerticalLayoutWidget fundsContainer = VerticalLayoutWidget.Cast(w.FindAnyWidget("Provider_Details"));
		if (!fundsContainer)
			return;
		
		fundsContainerWidget = VerticalLayoutWidget.Cast(GetGame().GetWorkspace().CreateWidgets(playerFundsPrefab));
		if (!fundsContainerWidget)
			return;
		
		repContainerWidget = VerticalLayoutWidget.Cast(GetGame().GetWorkspace().CreateWidgets(playerRepPrefab));
		if (!repContainerWidget)
			return;
		
		fundsContainer.AddChild(repContainerWidget);
		fundsContainer.AddChild(fundsContainerWidget);
		mainContainer.GetParent().AddChild(fundsContainer);
		mainContainer.SetVisible(false);
		
		repWidget = TextWidget.Cast(repContainerWidget.FindAnyWidget("RepValue"));
		walletWidget = TextWidget.Cast(fundsContainerWidget.FindAnyWidget("WalletValue"));
		bankWidget = TextWidget.Cast(fundsContainerWidget.FindAnyWidget("BankValue"));
		
		m_ProviderIcon.GetParent().GetParent().SetVisible(false);
		m_ProviderName.SetVisible(false);
		
		SizeLayoutWidget supplyValue = SizeLayoutWidget.Cast(w.FindAnyWidget("InGame_Supply_Size"));
		if (supplyValue)
			supplyValue.SetVisible(false);
		
		buildingEditorComponent.Event_OnResourcesChanged.Insert(UpdateResources);
		UpdateResources();
	}
	
	override void SetSourceIcon(IEntity targetEntity)
	{
		if (!trader)
			return super.SetSourceIcon(targetEntity);
		
		// unused in DE Vehicle Traders
		return;
	}
	
	void UpdateValues()
	{
		UpdateResources();
	}
	
	override protected void UpdateResources()
	{
		if (!trader)
			return super.UpdateResources();
		
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		trader = DE_VehicleTraderEntity.Cast(buildingEditorComponent.GetProviderEntity());
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
		
		if (repWidget)
			repWidget.SetText(trader.GetLocalPlayerRep().ToString());
	}
	
	override protected void SetProviderName(IEntity targetEntity)
	{
		if (!trader)
			return super.SetProviderName(targetEntity);

		m_ProviderCallsign.SetText(trader.traderName);
	}
}
