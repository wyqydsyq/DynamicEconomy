typedef func OnArsenalItemRefundedMethod;
typedef ScriptInvokerBase<OnArsenalItemRefundedMethod> OnArsenalItemRefundedInvoker;

modded class SCR_ResourcePlayerControllerInventoryComponent : ScriptComponent
{
	ref static OnArsenalItemRefundedInvoker s_OnArsenalItemRefunded;
	static OnArsenalItemRefundedInvoker GetOnArsenalItemRefunded()
	{
		if (!s_OnArsenalItemRefunded)
			s_OnArsenalItemRefunded = new OnArsenalItemRefundedInvoker();

		return s_OnArsenalItemRefunded;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void RpcAsk_ArsenalRequestItem_(RplId rplIdResourceComponent, RplId rplIdInventoryManager, RplId rplIdStorageComponent, ResourceName resourceNameItem, EResourceType resourceType)
	{
		if (!rplIdInventoryManager.IsValid())
			return;
		
		SCR_InventoryStorageManagerComponent inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(Replication.FindItem(rplIdInventoryManager));
		if (!inventoryManagerComponent)
			return;
		
		if (!rplIdStorageComponent.IsValid())
			return;
		
		BaseInventoryStorageComponent storageComponent = BaseInventoryStorageComponent.Cast(Replication.FindItem(rplIdStorageComponent));
		if (!storageComponent)
			return;
		
		if (!rplIdResourceComponent.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponent));
		if (!resourceComponent)
			return;
		
		IEntity resourcesOwner = resourceComponent.GetOwner();
		if (!resourcesOwner)
			return;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(resourcesOwner);
		DE_TraderEntity trader = DE_TraderEntity.Cast(resourcesOwner);
		if (!trader)
			return super.RpcAsk_ArsenalRequestItem_(rplIdResourceComponent, rplIdInventoryManager, rplIdStorageComponent, resourceNameItem, resourceType);
		
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		if (!consumer)
			return;
		
		float resourceCost = 0;

		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return;
		
		if (!inventoryManagerComponent.ValidateStorageRequest(resourcesOwner))
			return;
		
		SCR_Faction faction;
		if (arsenalComponent)
			faction = arsenalComponent.GetAssignedFaction();
		
		DL_LootSystem lootSystem = DL_LootSystem.GetInstance();
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		SCR_EntityCatalogEntry entry = lootSystem.lootCatalog.GetEntryWithPrefab(resourceNameItem);
		if (!entry)
			return;
		
		SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!data)
			return;
		
		if (arsenalComponent)
			resourceCost = data.GetSupplyCost(arsenalComponent.GetSupplyCostType());
		else 
			resourceCost = data.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT);
		
		if (economySystem.fallbackSupplyCost > 0 && resourceCost <= 0)
			resourceCost = economySystem.fallbackSupplyCost;
		
		resourceCost *= economySystem.cashSupplyExchangeRate;
		resourceCost *= consumer.GetBuyMultiplier();
				
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetOwner());
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		SCR_ResourceConsumer playerConsumer = playerResource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(pc.GetPlayerId()));
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		SCR_ResourceConsumer charConsumer = charResource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		
		IEntity fundsHolder = char;
		
		// if player can't afford from default funds consumer (wallet)
		if (charConsumer.GetAggregatedResourceValue() < resourceCost)
		{
			// set fund consumer to bank account if trader allows card payment and player can afford
			if (trader.cardPayment && playerConsumer.GetAggregatedResourceValue() >= resourceCost)
				fundsHolder = pc;
			else
			{
				SCR_NotificationsComponent.SendToPlayer(pc.GetPlayerId(), DE_EconomySystem.GetInstance().insufficientNotification, charConsumer.GetAggregatedResourceValue());
				return;
			}
		}
		
		SCR_ResourceComponent fundsResource = SCR_ResourceComponent.Cast(fundsHolder.FindComponent(SCR_ResourceComponent));
		SCR_ResourceConsumer fundsConsumer = fundsResource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		SCR_ResourceContainer fundsContainer = fundsResource.GetContainer(EResourceType.CASH);
		
		//if (!TryPerformResourceConsumption(fundsConsumer, resourceCost))
		//	return;
		fundsContainer.SetResourceValue(fundsContainer.GetResourceValue() - resourceCost);
		
		pc.NotifyBankDataChange(Replication.FindId(fundsHolder), fundsContainer.GetResourceValue());
		pc.NotifyPlayerDataChange(- resourceCost);

		if (inventoryManagerComponent.TrySpawnPrefabToStorage(resourceNameItem, storageComponent, cb: new SCR_PrefabSpawnCallback(storageComponent)) && s_OnArsenalItemRequested)
			GetOnArsenalItemRequested().Invoke(resourceComponent, resourceNameItem, pc, storageComponent, resourceType, - resourceCost);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void RpcAsk_ArsenalRefundItem_(RplId rplIdResourceComponent, RplId rplIdInventoryItem, EResourceType resourceType)
	{
		if (!rplIdInventoryItem.IsValid())
			return;
		
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(Replication.FindItem(rplIdInventoryItem));
		
		if (!inventoryItemComponent)
			return;
		
		if (!rplIdResourceComponent.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponent));
		if (!resourceComponent)
			return;
		
		IEntity resourcesOwner = resourceComponent.GetOwner();
		if (!resourcesOwner)
			return;
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetOwner());
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		DE_TraderEntity trader = DE_TraderEntity.Cast(resourcesOwner);
		
		if (!trader)
			return super.RpcAsk_ArsenalRefundItem_(rplIdResourceComponent, rplIdInventoryItem, resourceType);
		
		SCR_ResourceGenerator generator = playerResource.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		if (!generator)
			return;
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(pc.GetPlayerId()));
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		SCR_ResourceGenerator charGenerator = charResource.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		if (charGenerator)
			generator = charGenerator;
		
		IEntity inventoryItemEntity	= inventoryItemComponent.GetOwner();
		if (!inventoryItemEntity)
			return;
		
		ResourceName resourceNameItem = inventoryItemEntity.GetPrefabData().GetPrefabName();
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(resourcesOwner);
		
		SCR_Faction faction = arsenalComponent.GetAssignedFaction();
		SCR_EntityCatalogEntry entry;
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		
		DL_LootSystem lootSystem = DL_LootSystem.GetInstance();
		entry = lootSystem.lootCatalog.GetEntryWithPrefab(resourceNameItem);
		if (!entry)
			return;
		
		SCR_ArsenalManagerComponent.OnItemRefunded_S(inventoryItemEntity, pc, arsenalComponent);
		IEntity parentEntity = inventoryItemEntity.GetParent();
		
		SCR_InventoryStorageManagerComponent inventoryManagerComponent;
		if (parentEntity)
			inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(parentEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (inventoryManagerComponent && !inventoryManagerComponent.TryDeleteItem(inventoryItemEntity))
			return;
		else if (!inventoryManagerComponent)
			RplComponent.DeleteRplEntity(inventoryItemEntity, false);
		
		SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		float resourceCost = data.GetSupplyRefundAmount(arsenalComponent.GetSupplyCostType());
		
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		if (economySystem.fallbackSupplyCost > 0 && resourceCost <= 0)
			resourceCost = economySystem.fallbackSupplyCost;
		
		if (resourceCost <= 0)
			return;

		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		resourceCost *= economySystem.cashSupplyExchangeRate;
		resourceCost *= consumer.GetSellMultiplier();
		generator.RequestGeneration(resourceCost, generator);
		
		pc.NotifyBankDataChange(Replication.FindId(generator.GetOwner()), generator.GetComponent().GetContainer(EResourceType.CASH).GetResourceValue());
		pc.NotifyPlayerDataChange(resourceCost);
		
		GetOnArsenalItemRefunded().Invoke(resourceComponent, resourceNameItem, GetOwner(), null, resourceType, resourceCost);
	}
}
