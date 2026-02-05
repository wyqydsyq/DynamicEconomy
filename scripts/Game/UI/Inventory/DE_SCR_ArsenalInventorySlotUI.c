modded class SCR_ArsenalInventorySlotUI : SCR_InventorySlotUI
{
	float requiredRep = -1;
	
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
		
		return DE_EconomySystem.GetInstance().SupplyToCashValue(m_fSupplyCost, consumer.GetBuyMultiplier());
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
		CheckRequiredRank();
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
		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		DE_TraderEntity trader = DE_TraderEntity.Cast(storageEnt);
		if (!trader)
			return super.CheckRequiredRank();
		
		ImageWidget rankIcon = ImageWidget.Cast(m_widget.FindAnyWidget(RANK_ICON_WIDGET_NAME));
		if (!rankIcon)
			return;

		rankIcon.SetVisible(false);
	
		ResourceName resourceName = m_pItem.GetOwner().GetPrefabData().GetPrefabName();
		ref SCR_EntityCatalogEntry entry = DL_LootSystem.GetInstance().lootCatalog.GetEntryWithPrefab(resourceName);
		if (!entry)
			return;
		
		DE_ArsenalItemTraderData data = DE_ArsenalItemTraderData.Cast(entry.GetEntityDataOfType(DE_ArsenalItemTraderData));
		float repRequirement = DE_ArsenalItemTraderData.GetRepRequirement(entry);

		UUID playerUuid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(SCR_PlayerController.GetLocalPlayerId());
		if (repRequirement == -1 || (trader.repMap.Contains(playerUuid) && trader.repMap.Get(playerUuid) >= repRequirement))
			return;
		
		SetItemAvailability(false);
		rankIcon.SetVisible(true);
		
		if (m_wMSARIcon)
			m_wMSARIcon.SetVisible(false);
		
		rankIcon.LoadImageFromSet(0, "{37204ADEBD67C8A6}UI/Imagesets/Notifications/NotificationIcons.imageset", "Notification_Squad_Leader");
	}
};