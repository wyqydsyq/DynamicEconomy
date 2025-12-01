modded class SCR_ArsenalInventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
{
	ref array<ResourceName> prefabsToSpawn_Cached;
	override protected void FillInitialPrefabsToStore(out array<ResourceName> prefabsToSpawn)
	{
		DL_LootSystem lootSystem = DL_LootSystem.GetInstance();
		if (!lootSystem)
			return;
		
		if (lootSystem.lootDataReady)
			return super.FillInitialPrefabsToStore(prefabsToSpawn);

		prefabsToSpawn_Cached = prefabsToSpawn;
		lootSystem.Event_LootCatalogsReady.Insert(HandleLootCatalogsReady);
	}
	
	void HandleLootCatalogsReady()
	{
		if (prefabsToSpawn_Cached)
			FillInitialPrefabsToStore(prefabsToSpawn_Cached);
	}
}
