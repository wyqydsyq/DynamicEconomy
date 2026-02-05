modded class SCR_CampaignBuildingProviderComponent : SCR_MilitaryBaseLogicComponent
{
	void SetDisplayName(string name)
	{
		m_sProviderDisplayName = name;
	}
	
	override bool UseMasterProvider()
	{
		DE_TraderEntity trader = DE_TraderEntity.Cast(GetOwner());
		if (!trader)
			return super.UseMasterProvider();
		
		return false;
	}
	
	override int GetBudgetValue(EEditableEntityBudget type, out SCR_CampaignBuildingProviderComponent componentToUse)
	{
		if (type != EEditableEntityBudget.CASH)
			return super.GetBudgetValue(type, componentToUse);
		
		return 1;
	}
	
	override int GetMaxBudgetValue(EEditableEntityBudget budget)
	{
		if (budget != EEditableEntityBudget.CASH)
			return super.GetMaxBudgetValue(budget);
		
		return 1;
	}
	
	override bool IsThereEnoughBudgetToSpawn(notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		DE_TraderEntity trader = DE_TraderEntity.Cast(GetOwner());
		if (!trader)
			return super.IsThereEnoughBudgetToSpawn(budgetCosts);
		
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		
		float margin = trader.traderMargin;
		if (margin == -1)
			margin = economySystem.traderMargin;
		
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
			const int budgetIncrease = economySystem.SupplyToCashValue(budget.GetBudgetValue(), margin);

			if (budgetType == EEditableEntityBudget.CAMPAIGN)	
			{
				if (!gameMode.IsResourceTypeEnabled(EResourceType.SUPPLIES))
					continue;
				
				const bool enoughSupplies = realProvider.IsThereEnoughSupplies(currentBudgetValue, budgetIncrease, accumulatedBudgetChanges);
				if (!enoughSupplies)
					return false;
				
				continue;
			}
			
			if (budgetType == EEditableEntityBudget.CASH)	
			{
				SCR_ResourceConsumer bankConsumer;
				SCR_ResourceConsumer walletConsumer;
				economySystem.GetPlayerCashConsumers(SCR_PlayerController.GetLocalPlayerId(), bankConsumer, walletConsumer);

				bool canAfford = budgetIncrease <= walletConsumer.GetAggregatedResourceValue();
				if (!canAfford && trader.cardPayment)
					canAfford = budgetIncrease <= bankConsumer.GetAggregatedResourceValue();
				
				if (!canAfford)
					return false;
				
				continue;
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
		
		if (supplyBudgetIdx != -1 && hasCashBudget)
			budgets.Remove(supplyBudgetIdx);
		
		return budgets.Count();
	}
}
