modded class SCR_EntityCatalogManagerComponent : SCR_BaseGameModeComponent
{
	bool GetArsenalItems_P(out array<SCR_ArsenalItem> arsenalItems, notnull SCR_EntityCatalog itemCatalog, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, SCR_EArsenalGameModeType arsenalGameModeType = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		return GetArsenalItems(arsenalItems, itemCatalog, typeFilter, modeFilter, arsenalGameModeType, requiresDisplayType);
	}
	
	SCR_EntityCatalogEntry GetEntryWithPrefabFromGivenCatalog(SCR_EntityCatalog catalog, ResourceName prefabToFind)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabToFind))
			return null;

		return catalog.GetEntryWithPrefab(prefabToFind);
	}
}
