modded class SCR_ArsenalInventorySlotUI : SCR_InventorySlotUI
{
	override float GetTotalResources()
	{
		m_fSupplyCost = 0;
		
		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		DE_TraderEntity trader = DE_TraderEntity.Cast(storageEnt);
		if (!trader)
			return super.GetTotalResources();
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(storageEnt.FindComponent(SCR_ArsenalComponent));
		if (arsenalComponent && !arsenalComponent.IsArsenalUsingSupplies() && !DE_TraderEntity.Cast(storageEnt))
		{
			m_fSupplyCost = -1;
			return m_fSupplyCost;
		}
		
		if (!m_pItem || !m_pItem.GetOwner())
			return 0;
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;
		
		SCR_Faction faction;
		if (arsenalComponent)
			faction = arsenalComponent.GetAssignedFaction();
		
		Resource resource = Resource.Load(m_pItem.GetOwner().GetPrefabData().GetPrefabName());
		DL_LootSystem lootSystem = DL_LootSystem.GetInstance();
		SCR_EntityCatalogEntry entry = lootSystem.lootCatalog.GetEntryWithPrefab(resource.GetResource().GetResourceName());
		if (!entry)
			return 0;
		
		SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!data)
			return 0;
		
		if (arsenalComponent)
			m_fSupplyCost = data.GetSupplyCost(arsenalComponent.GetSupplyCostType());
		else 
			m_fSupplyCost = data.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT);
		
		if (!m_ArsenalResourceComponent)
			return m_fSupplyCost;
		
		SCR_ResourceConsumer consumer = m_ArsenalResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		if (!consumer)
			return m_fSupplyCost;
		
		DE_EconomySystem system = DE_EconomySystem.GetInstance();
		if (system.fallbackSupplyCost > 0 && m_fSupplyCost <= 0)
			m_fSupplyCost = system.fallbackSupplyCost;
		
		m_fSupplyCost *= system.cashSupplyExchangeRate;
		m_fSupplyCost *= consumer.GetBuyMultiplier();
		
		return m_fSupplyCost;
	}
	
	override void UpdateTotalResources(float totalResources)
	{
		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		DE_TraderEntity trader = DE_TraderEntity.Cast(storageEnt);
		if (!trader)
			return super.UpdateTotalResources(totalResources);
		
		if (!m_CostResourceHolderText || !m_CostResourceHolder)
			return;
		
		if (totalResources < 0)
		{
			SetItemAvailability(true);
			m_CostResourceHolder.SetVisible(false);
			return;
		}
		else 
		{
			m_CostResourceHolder.SetVisible(true);
		}
		
		m_CostResourceHolderText.SetText(FormatFloat(totalResources));
		
		if (!m_ArsenalResourceComponent)
		{
			SetItemAvailability(true);
			return;
		}
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(SCR_PlayerController.GetLocalPlayerId()));
		SCR_ResourceComponent traderResource = SCR_ResourceComponent.Cast(trader.FindComponent(SCR_ResourceComponent));
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		if (!playerResource || !traderResource)
		{
			SetItemAvailability(true);
			return;
		}
		
		bool playerCanAfford = false;
		SCR_ResourceContainer container;
		SCR_ResourceContainer playerContainer = playerResource.GetContainer(EResourceType.CASH);
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(pc.GetPlayerId()));
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer charContainer = charResource.GetContainer(EResourceType.CASH);
		
		if (totalResources < charContainer.GetResourceValue())
			playerCanAfford = true;
		
		if (!playerCanAfford && trader.cardPayment && totalResources < playerContainer.GetResourceValue())
			playerCanAfford = true;
		
		//SCR_ResourceConsumer traderConsumer = traderResource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		SetItemAvailability(playerCanAfford);
	}
	
	override protected void CheckPersonalResources(int cost)
	{
		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		DE_TraderEntity trader = DE_TraderEntity.Cast(storageEnt);
		if (!trader)
			return super.CheckPersonalResources(cost);
	}
	
	override protected void CheckRequiredRank()
	{
		ImageWidget rankIcon = ImageWidget.Cast(m_widget.FindAnyWidget(RANK_ICON_WIDGET_NAME));
		if (!rankIcon)
			return;

		rankIcon.SetVisible(false);
		
		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		DE_TraderEntity trader = DE_TraderEntity.Cast(storageEnt);
		if (trader)
			return;
		
		return super.CheckRequiredRank();
	}
};