modded class SCR_CampaignBuildingBudgetEditorComponent : SCR_BudgetEditorComponent
{
	override protected void FilterAvailableBudgets(inout notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		SCR_EntityBudgetValue supplyBudget;
		SCR_EntityBudgetValue budget;
		for (int i = budgetCosts.Count() - 1; i >= 0; i--)
		{
			budget = budgetCosts[i];
			
			// remove & cache supply budget to generate cash budget from
			if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
			{
				supplyBudget = budget;
				continue;
			}
			
			if (!IsBudgetAvailable(budget.GetBudgetType()))
				budgetCosts.Remove(i);
		}
		
		// inject cash budget based on supply if not set
		if (supplyBudget)
		{
			SCR_EntityBudgetValue cashBudget = new SCR_EntityBudgetValue(EEditableEntityBudget.CASH, DE_EconomySystem.GetInstance().SupplyToCashValue(supplyBudget.GetBudgetValue()));
			budgetCosts.Insert(cashBudget);
		}
	}
	
	override bool GetMaxBudgetValue(EEditableEntityBudget type, out int maxBudget)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		DE_TraderEntity trader = DE_TraderEntity.Cast(providerComponent.GetOwner());
		if (type != EEditableEntityBudget.CASH || !trader)
			return super.GetMaxBudgetValue(type, maxBudget);
		
		maxBudget = 1;

		return true;
	}
	
	override bool GetMaxBudget(EEditableEntityBudget type, out SCR_EntityBudgetValue budget)
	{
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		DE_TraderEntity trader = DE_TraderEntity.Cast(providerComponent.GetOwner());
		if (type != EEditableEntityBudget.CASH || !trader)
			return super.GetMaxBudget(type, budget);
		
		budget.SetBudgetValue(1);
		
		return true;
	}

	override int GetCurrentBudgetValue(EEditableEntityBudget type)
	{
		if (type != EEditableEntityBudget.CASH)
			return super.GetCurrentBudgetValue(type);
		
		return 1;
	}
}
