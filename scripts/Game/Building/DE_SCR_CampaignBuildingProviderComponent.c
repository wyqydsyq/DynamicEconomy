modded class SCR_CampaignBuildingProviderComponent : SCR_MilitaryBaseLogicComponent
{
	DE_TraderEntity trader;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		trader = DE_TraderEntity.Cast(GetOwner());
	}
	
	void SetDisplayName(string name)
	{
		m_sProviderDisplayName = name;
	}
	
	void SetAvailableTraits(array<EEditableEntityLabel> traits)
	{
		m_aAvailableTraits = traits;
	}
	
	override array<EEditableEntityLabel> GetAvailableTraits()
	{
		if (!trader)
			return super.GetAvailableTraits();
		
		if (!trader.traitsBlacklist || !trader.traitsBlacklist.Count())
			return super.GetAvailableTraits();
		
		array<EEditableEntityLabel> filteredTraits = {};
		array<EEditableEntityLabel> traits = super.GetAvailableTraits();
		foreach (EEditableEntityLabel trait : traits)
		{
			// filter out blacklisted traits
			if (trader.traitsBlacklist.Contains(trait))
				continue;
			
			filteredTraits.Insert(trait);
		}
		
		return filteredTraits;
	}
	
	override int GetBudgetValue(EEditableEntityBudget type, out SCR_CampaignBuildingProviderComponent componentToUse)
	{
		if (!trader)
			return super.GetBudgetValue(type, componentToUse);
		
		// disable supply for traders
		if (type == EEditableEntityBudget.CAMPAIGN)
			return 0;
		
		if (type == EEditableEntityBudget.CASH || type == EEditableEntityBudget.REP)
			return 1;
		
		return super.GetBudgetValue(type, componentToUse);
	}
	
	override int GetMaxBudgetValue(EEditableEntityBudget budget)
	{
		if (!trader)
			return super.GetMaxBudgetValue(budget);
		
		// disable supply for traders
		if (budget == EEditableEntityBudget.CAMPAIGN)
			return 0;
		
		if (budget == EEditableEntityBudget.CASH || budget == EEditableEntityBudget.REP)
			return 1;
		
		return 1;
	}
	
	override bool IsThereEnoughBudgetToSpawn(notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		trader = DE_TraderEntity.Cast(GetOwner());
		if (!trader)
			return super.IsThereEnoughBudgetToSpawn(budgetCosts);
		
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		
		if (budgetCosts.IsEmpty())
			return true;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		foreach (SCR_EntityBudgetValue budget : budgetCosts)
		{
			const EEditableEntityBudget budgetType = budget.GetBudgetType();
			SCR_CampaignBuildingBudgetToEvaluateData data = GetBudgetData(budgetType);
			if (!data)
				continue;
			
			SCR_CampaignBuildingProviderComponent realProvider = this;
			const int currentBudgetValue = GetBudgetValue(budgetType, realProvider);
			const int accumulatedBudgetChanges = realProvider.GetAccumulatedBudgetChanges(budgetType);
			const int budgetIncrease = budget.GetBudgetValue();
			
			if (budgetType == EEditableEntityBudget.CASH)	
			{
				float cashIncrease = budgetIncrease / economySystem.intPrecisionFactor;
				
				SCR_ResourceConsumer bankConsumer;
				SCR_ResourceConsumer walletConsumer;
				economySystem.GetPlayerCashConsumers(SCR_PlayerController.GetLocalPlayerId(), bankConsumer, walletConsumer);

				bool canAfford = cashIncrease <= walletConsumer.GetComponent().GetContainer(EResourceType.CASH).GetResourceValue();
				if (!canAfford && trader.cardPayment)
					canAfford = cashIncrease <= bankConsumer.GetComponent().GetContainer(EResourceType.CASH).GetResourceValue();
				
				if (!canAfford)
					return false;
				
				continue;
			}
			
			if (budgetType == EEditableEntityBudget.REP)
			{
				if (trader.GetLocalPlayerRep() < budget.GetBudgetValue())
					return false;	
			}
			
			const int maxBudgetValue = GetMaxBudgetValueFromMasterIfNeeded(budgetType);
			if (maxBudgetValue == -1)
				continue;

			if (budgetIncrease + accumulatedBudgetChanges + currentBudgetValue > maxBudgetValue)
				return false;
			
			realProvider.AccumulateBudgetChange(budgetType, budget.GetBudgetValue());
		}
		
		return true;
	}
	
	override int GetBudgetTypesToEvaluate(notnull out array<ref EEditableEntityBudget> budgets)
	{
		bool hasCashBudget;
		int supplyBudgetIdx = -1;
		foreach (SCR_CampaignBuildingBudgetToEvaluateData budgetData : m_aBudgetsToEvaluate)
		{
			if (budgetData.GetBudget() == EEditableEntityBudget.CAMPAIGN)
				supplyBudgetIdx = m_aBudgetsToEvaluate.Find(budgetData);
			
			if (budgetData.GetBudget() == EEditableEntityBudget.CASH)
				hasCashBudget = true;
			
			budgets.Insert(budgetData.GetBudget());
		}
		
		if (supplyBudgetIdx != -1 && hasCashBudget && trader)
			budgets.Remove(supplyBudgetIdx);
		
		return budgets.Count();
	}
	
	override void RequestEnterBuildingMode(int playerID, bool userActionUsed)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;
		
		trader = DE_TraderEntity.Cast(GetOwner());
		editorManager.SetTrader(trader);

		SetOnPlayerConsciousnessChanged();
		SetOnPlayerTeleported(playerID);
		editorManager.GetOnOpenedServer().Insert(BuildingModeCreated);
		editorManager.GetOnClosed().Insert(OnModeClosed);
		networkComponent.RequestEnterBuildingMode(GetOwner(), playerID, m_bUserActionActivationOnly, userActionUsed);
	}
	
	override Faction GetEntityFaction(notnull IEntity ent)
	{
		if (!trader)
			return super.GetEntityFaction(ent);
		
		return null;
	}
	
	override void EntitySpawnedByProvider(int prefabID, SCR_EditableEntityComponent editableEntity)
	{
		if (!trader)
			return super.EntitySpawnedByProvider(prefabID, editableEntity);
		
		// skip regular conflict spawner logic for DE traders we will implement our own
		return;
	}
}
