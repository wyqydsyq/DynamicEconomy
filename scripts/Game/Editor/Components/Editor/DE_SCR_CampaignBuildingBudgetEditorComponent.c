modded class SCR_CampaignBuildingBudgetEditorComponent : SCR_BudgetEditorComponent
{
	DE_TraderEntity trader;
	int currentSupplyBudgetChange;
	
	ref array<ref EEditableEntityBudget> traderBudgets = {EEditableEntityBudget.CASH, EEditableEntityBudget.REP};
	
	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		trader = DE_TraderEntity.Cast(providerComponent.GetOwner());
	}
	
	void FilterAvailableBudgetsEntity(inout notnull array<ref SCR_EntityBudgetValue> budgetCosts, SCR_EditableEntityUIInfo entityUIInfo)
	{
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		if (!trader || !economySystem)
			return super.FilterAvailableBudgets(budgetCosts);
		
		float repRequirement = 0;
		float cashValueMult = 1;
		int supplyBudgetValue = 0;
		for (int i = budgetCosts.Count() - 1; i >= 0; i--)
		{
			SCR_EntityBudgetValue budget = budgetCosts[i];
			
			// cache supply budget to generate cash budget from
			if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
			{
				supplyBudgetValue = budget.GetBudgetValue();
				continue;
			}
			
			// read item-specific rep requirement
			if (budget.GetBudgetType() == EEditableEntityBudget.REP)
				repRequirement = budget.GetBudgetValue();
			
			if (!IsBudgetAvailable(budget.GetBudgetType()))
				budgetCosts.Remove(i);
		}
		
		// inject cash and rep budgets for any item with supply budget
		if (supplyBudgetValue)
		{
			DE_VehicleTraderEntity vehicleTrader = DE_VehicleTraderEntity.Cast(trader);
			float traderMult = vehicleTrader.GetCashValueMult(entityUIInfo);
			float cashValue = economySystem.SupplyToCashValue(supplyBudgetValue * traderMult, trader.traderMargin);
			
			int budgetValue = cashValue * economySystem.intPrecisionFactor;
			ref SCR_EntityBudgetValue cashBudget = new SCR_EntityBudgetValue(EEditableEntityBudget.CASH, budgetValue);
			budgetCosts.Insert(cashBudget);
			
			// if no item-specific rep requirement set, check for rule to apply based on supply cost
			if (!repRequirement)
			{
				float repMult = economySystem.vehicleTraderRepValueMultiplier;
				float repBudgetValue = supplyBudgetValue * repMult;
				ref RepSupplyCostRule foundRule = economySystem.GetRepRequirement(repBudgetValue);
				if (foundRule)
					repRequirement = foundRule.repRequired;
			}
			int repValue = repRequirement * economySystem.intPrecisionFactor;
			ref SCR_EntityBudgetValue repBudget = new SCR_EntityBudgetValue(EEditableEntityBudget.REP, repValue);
			budgetCosts.Insert(repBudget);
		}
	}
	
	// get player cash or rep balances
	override bool GetMaxBudgetValue(EEditableEntityBudget type, out int maxBudget)
	{
		if (!trader || !traderBudgets.Contains(type))
			return super.GetMaxBudgetValue(type, maxBudget);
		
		maxBudget = 0;

		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		if (!economySystem)
			return true;
		
		if (type == EEditableEntityBudget.CASH)
		{
			ref SCR_ResourceConsumer bankConsumer;
			ref SCR_ResourceConsumer walletConsumer;
			economySystem.GetPlayerCashConsumers(SCR_PlayerController.GetLocalPlayerId(), bankConsumer, walletConsumer);
			
			if (!trader.cardPayment)
				maxBudget = walletConsumer.GetAggregatedResourceValue() * economySystem.intPrecisionFactor;
			else if (trader.cardPayment)
				maxBudget = bankConsumer.GetAggregatedResourceValue() * economySystem.intPrecisionFactor;
		}
		if (type == EEditableEntityBudget.REP)
		{
			maxBudget = trader.GetLocalPlayerRep() * economySystem.intPrecisionFactor;
		}
		
		return true;
	}
	
	override bool GetMaxBudget(EEditableEntityBudget type, out SCR_EntityBudgetValue budget)
	{
		if (!trader || !traderBudgets.Contains(type))
			return super.GetMaxBudget(type, budget);
		
		budget.SetBudgetValue(1);
		
		return true;
	}

	override int GetCurrentBudgetValue(EEditableEntityBudget type)
	{
		if (!trader || !traderBudgets.Contains(type))
			return super.GetCurrentBudgetValue(type);
		
		return 0;
	}
	
	override bool GetEntityPreviewBudgetCosts(SCR_EditableEntityUIInfo entityUIInfo, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if (!trader)
			return super.GetEntityPreviewBudgetCosts(entityUIInfo, budgetCosts);
		
		if (!entityUIInfo)
			return false;
		
		SCR_EditableGroupUIInfo groupUIInfo = SCR_EditableGroupUIInfo.Cast(entityUIInfo);
		if (groupUIInfo)
		{
			entityUIInfo.GetEntityAndChildrenBudgetCost(budgetCosts);
				
			if (!entityUIInfo.GetEntityBudgetCost(budgetCosts))
				GetEntityTypeBudgetCost(entityUIInfo.GetEntityType(), budgetCosts);
		}
		else
		{
			array<ref SCR_EntityBudgetValue> entityChildrenBudgetCosts = {};
			
			if (!entityUIInfo.GetEntityBudgetCost(budgetCosts))
				GetEntityTypeBudgetCost(entityUIInfo.GetEntityType(), budgetCosts);
			
			entityUIInfo.GetEntityChildrenBudgetCost(entityChildrenBudgetCosts);
	
			SCR_EntityBudgetValue.MergeBudgetCosts(budgetCosts, entityChildrenBudgetCosts);
		}
		
		FilterAvailableBudgetsEntity(budgetCosts, entityUIInfo);
		
		return true;
	}
}
