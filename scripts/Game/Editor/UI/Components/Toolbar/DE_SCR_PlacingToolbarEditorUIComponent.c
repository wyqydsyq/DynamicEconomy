modded class SCR_PlacingToolbarEditorUIComponent : SCR_BaseToolbarEditorUIComponent
{
	override protected Widget CreateItem(int index)
	{
		SCR_EditorManagerCore editorCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = editorCore.GetEditorManager();
		DE_TraderEntity trader = editorManager.trader;
		if (!trader)
			return super.CreateItem(index);
		
		int prefabID = m_ContentBrowserManager.GetFilteredPrefabID(index);
		if (prefabID < 0)
			return null;
		
		SCR_EditableEntityUIInfo info = m_ContentBrowserManager.GetInfo(prefabID);
		
		m_ItemLayout = m_DefaultLayout;
		if (info)
		{
			foreach (SCR_ContentBrowserEditorCard itemLayoutCandidate: m_aCardPrefabs)
			{
				if (itemLayoutCandidate.m_EntityType == info.GetEntityType())
				{
					m_ItemLayout = itemLayoutCandidate.m_sPrefab;
					break;
				}
			}
		}
		
		Widget itemWidget;
		SCR_BaseToolbarItemEditorUIComponent item;
		if (!CreateItem(itemWidget, item))
			return null;
		
		SCR_UIInfo blockingBudgetInfo;
		array<ref SCR_EntityBudgetValue> entityBudgetCosts = { };
		m_ContentBrowserManager.CanPlace(prefabID, entityBudgetCosts, blockingBudgetInfo);		
		
		SCR_AssetCardFrontUIComponent assetCard = SCR_AssetCardFrontUIComponent.Cast(itemWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		assetCard.GetOnHover().Insert(OnCardHover);
		assetCard.InitCard(prefabID, info, m_ContentBrowserManager.GetResourceNamePrefabID(prefabID), blockingBudgetInfo);
		
		SCR_EntityBudgetValue budgetCost;
		if (m_bShowBudgetCost && !entityBudgetCosts.IsEmpty())
			foreach (SCR_EntityBudgetValue budgetValue: entityBudgetCosts)
			{
				if (trader && budgetValue.GetBudgetType() == EEditableEntityBudget.CASH)
					assetCard.UpdateCashCost(budgetValue);
			
				if (trader && budgetValue.GetBudgetType() == EEditableEntityBudget.REP)
					assetCard.UpdateRepCost(budgetValue);
			
				if (budgetValue.GetBudgetType() == m_eBudgetToShow)
					assetCard.UpdateBudgetCost(budgetValue);
			}
		
		ButtonActionComponent.GetOnAction(itemWidget, true, 0).Insert(OnCardLMB);
		
		SCR_ButtonBaseComponent buttonComponent = SCR_ButtonBaseComponent.Cast(itemWidget.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComponent)
			buttonComponent.SetMouseOverToFocus(false);
		
		return itemWidget;
	}
}
