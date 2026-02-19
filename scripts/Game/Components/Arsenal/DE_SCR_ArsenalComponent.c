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
		ref array<SCR_EntityCatalogEntry> filteredEntityList = {};
		ref SCR_EntityCatalog catalog;
		
		if (trader.factions && trader.factions.Count())
		{
			catalog = new SCR_EntityCatalog();
			catalog.SetCatalogType(EEntityCatalogType.ITEM);
			catalog.SetEntityList({});
				
			array<Faction> factions = {};
			GetGame().GetFactionManager().GetFactionsList(factions);
			foreach (Faction f : factions)
			{
				SCR_Faction fact = SCR_Faction.Cast(f);
				if (!fact || !trader.factions.Contains(f.GetFactionKey()))
					continue;
				
				SCR_EntityCatalog factionCatalog = fact.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM);
				catalog.MergeEntityListRef(factionCatalog.GetEntityListRef(), EEntityCatalogType.ITEM, f.GetFactionKey(), false);
			}
		}
		else
			catalog = lootSystem.lootCatalog;
		
		catalog.GetFullFilteredEntityListWithLabels(filteredEntityList, trader.labels);
		
		foreach (SCR_EntityCatalogEntry entry : filteredEntityList)
		{
			SCR_ArsenalItem item = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
			if (!item)
				continue;
			
			// item has trader data requirements set, filter out item if they are not met
			DE_ArsenalItemTraderData traderData = DE_ArsenalItemTraderData.Cast(entry.GetEntityDataOfType(DE_ArsenalItemTraderData));
			float repRequirement = DE_ArsenalItemTraderData.GetRepRequirement(entry);
			if (traderData && traderData.secret && repRequirement != -1)	
			{
				// get player rep for this trader and check it meets requirement
				float playerRep = trader.repMap.Get(SCR_PlayerIdentityUtils.GetPlayerIdentityId(SCR_PlayerController.GetLocalPlayerId()));
				if (!playerRep || playerRep < repRequirement)
					continue;
			}
			
			if (trader.itemWhitelist.Count() > 0 && !trader.itemWhitelist.Contains(item.GetItemResourceName()))
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
