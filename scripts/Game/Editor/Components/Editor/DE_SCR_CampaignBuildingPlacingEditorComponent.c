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
			if (walletConsumer.GetComponent().GetContainer(EResourceType.CASH).GetResourceValue() < cashCost && trader.cardPayment)
				selectedConsumer = bankConsumer;
			
			selectedConsumer.RequestConsumtion(cashCost);
			SCR_ResourceGenerator generator = selectedConsumer.GetComponent().GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
			
			// increase player rep based on trader's profit
			trader.GrantRep(playerUuid, supplies);
			
			SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
			pc.NotifyRepChange(Replication.FindId(trader), trader.GetRep(playerUuid));
			pc.NotifyBankDataChange(Replication.FindId(generator.GetOwner()), generator.GetComponent().GetContainer(EResourceType.CASH).GetResourceValue());
			pc.NotifyPlayerTransaction(-cashCost);
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
	
	//------------------------------------------------------------------------------------------------
	//! Method used by the server to validate if labels from entity that was requested to spawn are matching with labels that are available with current building mode provider
	//! \param[in] entityLabels list of labels which should be validated
	//! \return true if validation was successful, otherwise false
	override bool AreLabelsMatching(notnull array<EEditableEntityLabel> entityLabels)
	{
		Print(string.Format("DE: checking labels match..."), LogLevel.NORMAL);
		if (!m_Provider)
			return false;

		
		// DE: CUSTOM
		// only do faction check when provider isn't a DE Trader
		if (!DE_TraderEntity.Cast(m_Provider) && !DE_TraderEntity.Cast(m_Provider.GetParent()))
		{
			Print(string.Format("DE: %1 is not a trader, checking faction...", m_Provider), LogLevel.NORMAL);
			FactionAffiliationComponent fac = FactionAffiliationComponent.Cast(m_Provider.FindComponent(FactionAffiliationComponent));
			if (!fac)
			{
				// in case of construction trucks, the provider is the back of the truck,
				// while faction affiliation component is only on the truck itself,
				// which is a parent, thus we need to check that as well
				IEntity parent = m_Provider.GetParent();
				while (parent && fac == null)
				{
					fac = FactionAffiliationComponent.Cast(parent.FindComponent(FactionAffiliationComponent));
					parent = parent.GetParent();
				}
	
				if (!fac)
					return false;
			}
	
			SCR_Faction providerFaction = SCR_Faction.Cast(fac.GetAffiliatedFaction());
			if (!providerFaction)
			{
				// in case of vehicles, they might be empty, and thus they are no affiliated with any faction,
				// but they provide structures from their default faction
				providerFaction = SCR_Faction.Cast(fac.GetDefaultAffiliatedFaction());
				if (!providerFaction)
					return false;
			}
	
			if (!entityLabels.Contains(providerFaction.GetFactionLabel()))
				return false;
		}
		
		// DE: CUSTOM_END
		
		SCR_CampaignBuildingProviderComponent providerComp = SCR_CampaignBuildingProviderComponent.Cast(m_Provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComp)
			return false;

		array<EEditableEntityLabel> providerLabels = providerComp.GetAvailableTraits();
		if (providerLabels.Contains(EEditableEntityLabel.SERVICE_HQ) && !SCR_CampaignBuildingProviderComponent.CanBeUsedToEstablishBase(m_Provider, m_Manager.GetPlayerID()))
			providerLabels.RemoveItem(EEditableEntityLabel.SERVICE_HQ);

		bool matchingLabel;
		foreach (EEditableEntityLabel providerLabel : providerLabels)
		{
			if (!entityLabels.Contains(providerLabel)) // at least one of these needs to match
				continue;

			matchingLabel = true;
			break;
		}

		if (!matchingLabel)
			return false;

		SCR_ContentBrowserEditorComponent browserComp = SCR_ContentBrowserEditorComponent.Cast(m_Owner.FindComponent(SCR_ContentBrowserEditorComponent));
		if (!browserComp)
			return false;

		array<EEditableEntityLabel> validBlackListLabels = {};
		browserComp.GetValidBlackListedLabels(validBlackListLabels);
		foreach (EEditableEntityLabel blacklistedLabel : validBlackListLabels)
		{
			if (entityLabels.Contains(blacklistedLabel)) // none of these can match
				return false;
		}

		return true;
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
