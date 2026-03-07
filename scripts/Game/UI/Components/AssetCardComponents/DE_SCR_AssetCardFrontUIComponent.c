modded class SCR_AssetCardFrontUIComponent : SCR_ScriptedWidgetComponent
{
	float cashCost;
	float repRequirement;
	
	ResourceName traderCostsPrefab = "{5BC579F07D64C156}UI/layouts/Menus/DE_TraderCompositionCost.layout";
	HorizontalLayoutWidget traderCostsWidget;
	DE_EconomySystem economySystem;
	DE_TraderEntity trader;
	
	override void InitCard(int prefabID, SCR_UIInfo info, ResourceName prefab, SCR_UIInfo blockingBudgetInfo = null)
	{
		SCR_EditorManagerCore editorCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = editorCore.GetEditorManager();
		trader = editorManager.trader;
		
		super.InitCard(prefabID, info, prefab, blockingBudgetInfo);
		
		if (!trader)
			return;
		
		m_BudgetCostLayout = m_wWidget.FindAnyWidget(m_sBudgetCostLayoutName);
		if (!m_BudgetCostLayout)
			return;
		
		m_BudgetCostLayout.SetVisible(true);
		
		economySystem = DE_EconomySystem.GetInstance();
		traderCostsWidget = HorizontalLayoutWidget.Cast(GetGame().GetWorkspace().CreateWidgets(traderCostsPrefab));
		m_BudgetCostLayout.AddChild(traderCostsWidget);
		
		VerticalLayoutWidget repCostContainer = VerticalLayoutWidget.Cast(traderCostsWidget.FindAnyWidget("RepCost"));
		if (repCostContainer)
			repCostContainer.SetVisible(false);
		
		VerticalLayoutWidget cashCostContainer = VerticalLayoutWidget.Cast(traderCostsWidget.FindAnyWidget("CashCost"));
		if (!cashCostContainer)
			cashCostContainer.SetVisible(false);
		
		OverlayWidget supplyCostContainer = OverlayWidget.Cast(m_BudgetCostLayout.FindAnyWidget("Cost"));
		if (supplyCostContainer)
			supplyCostContainer.SetVisible(false);
	}
	
	void UpdateRepCost(SCR_EntityBudgetValue entityBudgetCost = null)
	{
		VerticalLayoutWidget repCostContainer = VerticalLayoutWidget.Cast(traderCostsWidget.FindAnyWidget("RepCost"));
		if (!repCostContainer)
			return;
		
		if (!entityBudgetCost || !entityBudgetCost.GetBudgetValue() || entityBudgetCost.GetBudgetValue() == -1)
			return;
		
		repCostContainer.SetVisible(true);
		TextWidget valueWidget = TextWidget.Cast(repCostContainer.FindAnyWidget("CostValue"));
		valueWidget.SetText(FormatFloat(entityBudgetCost.GetBudgetValue() / economySystem.intPrecisionFactor));
	}
	
	void UpdateCashCost(SCR_EntityBudgetValue entityBudgetCost = null)
	{
		VerticalLayoutWidget cashCostContainer = VerticalLayoutWidget.Cast(traderCostsWidget.FindAnyWidget("CashCost"));
		if (!cashCostContainer)
			return;
		
		if (!entityBudgetCost)
			return;
		
		cashCostContainer.SetVisible(true);
		TextWidget valueWidget = TextWidget.Cast(cashCostContainer.FindAnyWidget("CostValue"));
		valueWidget.SetText(FormatFloat(entityBudgetCost.GetBudgetValue() / economySystem.intPrecisionFactor));
	}
	
	override void UpdateBudgetCost(SCR_EntityBudgetValue entityBudgetCost = null)
	{
		if (!trader)
			return super.UpdateBudgetCost(entityBudgetCost);
		
		m_BudgetCostLayout = m_wWidget.FindAnyWidget(m_sBudgetCostLayoutName);
		if (!m_BudgetCostLayout)
			return;
		
		m_BudgetCostLayout.SetVisible(true);
	}
}