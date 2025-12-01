modded class SCR_ArsenalComponent : ScriptComponent
{
	override bool GetFilteredArsenalItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		DE_TraderEntity trader = DE_TraderEntity.Cast(GetOwner());
		if (!trader)
			return super.GetFilteredArsenalItems(filteredArsenalItems, requiresDisplayType);

		if (m_OverwriteArsenalConfig)
		{
			GetFilteredOverwriteArsenalItems(filteredArsenalItems, requiresDisplayType);
			return !filteredArsenalItems.IsEmpty();
		}

		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
			return false;
		
		DL_LootSystem lootSystem = DL_LootSystem.GetInstance();
		
		SCR_EntityCatalog catalog;
		if (trader.factionKey)
			catalog = catalogManager.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM, trader.factionKey);
		else
			catalog = lootSystem.lootCatalog;
		
		array<SCR_EntityCatalogEntry> filteredEntityList = {};
		catalog.GetFullFilteredEntityListWithLabels(filteredEntityList, trader.labels);
		
		foreach (SCR_EntityCatalogEntry entry : filteredEntityList)
		{
			SCR_ArsenalItem item = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
			if (!item)
				continue;
			
			if (trader.types && !SCR_Enum.HasPartialFlag(item.GetItemType(), trader.types))
				continue;
			
			if (trader.modes && !SCR_Enum.HasPartialFlag(item.GetItemMode(), trader.modes))
				continue;
			
			if (requiresDisplayType != -1 && !item.GetDisplayDataOfType(requiresDisplayType))
				continue;
			
			filteredArsenalItems.Insert(item);
		}
		
		//catalogManager.GetArsenalItems_P(filteredArsenalItems, lootSystem.lootCatalog, m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, -1, requiresDisplayType);
		//filteredArsenalItems = catalogManager.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, SCR_ArsenalManagerComponent.GetArsenalGameModeType_Static(), null, requiresDisplayType);
		return !filteredArsenalItems.IsEmpty();
	}
	
	void SetOverwriteArsenalConfig(SCR_ArsenalItemListConfig config)
	{
		m_OverwriteArsenalConfig = config;
	}
}
