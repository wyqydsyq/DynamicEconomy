modded class SCR_CampaignBuildingPlacingEditorComponent : SCR_PlacingEditorComponent
{
	DE_EconomySystem economySystem;
	DE_TraderEntity trader;
	
	bool handlerBound = false;
	
	override void InitVariables()
	{
		super.InitVariables();
		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;
		
		economySystem = DE_EconomySystem.GetInstance();
		if (!economySystem)
			return;
		
		trader = DE_TraderEntity.Cast(providerComponent.GetOwner());
		if (!trader)
			return;
		
		if (!handlerBound)
		{
			GetOnPlaceEntityServer().Insert(HandleEntityPlaced);
			handlerBound = true;
		}
	}
	
	void HandleEntityPlaced(RplId prefabID, SCR_EditableEntityComponent entity, int playerID)
	{
		// for DE traders, instead of applying supply cost to provider
		// we want to apply cash cost to player's cash container
		// and increase their rep based on trader profit
		array<ref SCR_EntityBudgetValue> budgets = {};
		entity.GetEntityAndChildrenBudgetCost(budgets, entity.GetOwner());
		
		UUID playerUuid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerID);
		SCR_EditableEntityUIInfo uiInfo = SCR_EditableEntityUIInfo.Cast(entity.GetInfo());
		
		foreach (SCR_EntityBudgetValue budget : budgets)
		{
			if (budget.GetBudgetType() != EEditableEntityBudget.CAMPAIGN)
				continue;
			
			DE_VehicleTraderEntity vehicleTrader = DE_VehicleTraderEntity.Cast(trader);
			
			int supplies = budget.GetBudgetValue();
			float cashCost = economySystem.SupplyToCashValue(supplies * vehicleTrader.GetCashValueMult(uiInfo), trader.traderMargin);
			
			SCR_ResourceConsumer bankConsumer;
			SCR_ResourceConsumer walletConsumer;
			economySystem.GetPlayerCashConsumers(playerID, bankConsumer, walletConsumer);
			
			// select bank consumer if player cant afford wallet payment and trader accepts card
			SCR_ResourceConsumer selectedConsumer = walletConsumer;
			if (walletConsumer.GetAggregatedResourceValue() < cashCost && trader.cardPayment)
				selectedConsumer = bankConsumer;
			
			selectedConsumer.RequestConsumtion(cashCost);
			SCR_ResourceGenerator generator = selectedConsumer.GetComponent().GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
			
			// increase player rep based on trader's profit
			trader.GrantRep(playerUuid, supplies);
			
			SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
			pc.NotifyRepChange(Replication.FindId(trader), trader.GetRep(playerUuid));
			pc.NotifyBankDataChange(Replication.FindId(generator.GetOwner()), generator.GetComponent().GetContainer(EResourceType.CASH).GetResourceValue());
			pc.NotifyPlayerDataChange(-cashCost);
			break;
		}
	}
	
	override bool IsThereEnoughBudgetToSpawn(IEntityComponentSource entitySource)
	{
		if (!trader)
			return super.IsThereEnoughBudgetToSpawn(entitySource);
		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return true;
		
		// ignore supply budget for DE traders
		array<ref SCR_EntityBudgetValue> budgetCosts = {};
		m_BudgetManager.GetBudgetCosts(entitySource, budgetCosts);	
		
		return providerComponent.IsThereEnoughBudgetToSpawn(budgetCosts);
	}
	
	/*override void OnEntityCreatedServer(array<SCR_EditableEntityComponent> entities)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return super.OnEntityCreatedServer(entities);
		
		trader = DE_TraderEntity.Cast(providerComponent.GetOwner());
		if (!trader)
			return super.OnEntityCreatedServer(entities);
		
		economySystem = DE_EconomySystem.GetInstance();
		
		int userId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (!userId)
			return;
		
		foreach (SCR_EditableEntityComponent spawnedEntity : entities)
		{

		}
	}*/
}
