modded class SCR_InventoryMenuUI : ChimeraMenuBase
{
	ref SCR_SupplyCostItemHintUIInfo repRequirementUIInfo;
	DE_TraderEntity trader;
	TextWidget repWidget;
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		UpdatePlayerData();
	}
	
	override void OnMenuClose()
	{
		super.OnMenuClose();
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		BaseInventoryStorageComponent storage = m_pStorageLootUI.GetCurrentNavigationStorage();
		if (!storage)
			 return;
		
		trader = DE_TraderEntity.Cast(storage.GetOwner());
		if (!trader)
			return;
	
		Widget header = m_wLootStorage.FindAnyWidget("Header");
		if (header)
			header.SetVisible(false);
		
		TextWidget title = TextWidget.Cast(m_wLootStorage.FindAnyWidget("TextC"));
		if (title)
			title.SetText(trader.traderName);
		
		Widget previewWidget = m_wLootStorage.FindAnyWidget("itemStorage");
		if (previewWidget)
			delete previewWidget;
		
		Widget closeWidget = m_wLootStorage.FindAnyWidget("ButtonTraverseBack");
		if (closeWidget)
			closeWidget.SetVisible(false);
		
		Widget resourcesWidget = m_wLootStorage.FindAnyWidget("ResourceDisplayAvailable");
		if (resourcesWidget)
		{
			resourcesWidget.SetVisible(true);
			ImageWidget iconWidget = ImageWidget.Cast(resourcesWidget.FindAnyWidget("Image0"));
			if (iconWidget)
				iconWidget.LoadImageFromSet(0, "{37204ADEBD67C8A6}UI/Imagesets/Notifications/NotificationIcons.imageset", "Notification_Squad_Leader");
			
			repWidget = TextWidget.Cast(resourcesWidget.FindAnyWidget("ResourceText"));
			if (repWidget)
				repWidget.SetText(string.Format("Rep: %1", trader.GetLocalPlayerRep()));
		}
		
		Widget storedWidget = m_wLootStorage.FindAnyWidget("ResourceDisplayStored");
		if (!trader.cardPayment)
			storedWidget.SetVisible(false);
		else if (storedWidget)
		{
			storedWidget.SetVisible(true);
			ImageWidget iconWidget = ImageWidget.Cast(storedWidget.FindAnyWidget("Image0"));
			if (iconWidget)
				iconWidget.LoadImageTexture(0, DE_EconomySystem.GetInstance().cardPaymentIcon);
			
			TextWidget textWidget = TextWidget.Cast(storedWidget.FindAnyWidget("ResourceText"));
			if (textWidget)
				textWidget.SetText("Card payment available");
		}
		
		UpdatePlayerData();
	}
	
	override void ShowVicinity(bool compact = false)
	{
		if (!m_pVicinity)
		{
			Print("No vicnity component on character!", LogLevel.DEBUG);
			return;
		}

		if (m_wLootStorage)
		{
			m_wLootStorage.RemoveHandler(m_wLootStorage.FindHandler(SCR_InventoryStorageLootUI));
			m_wLootStorage.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget("StorageLootSlot");
		m_wLootStorage = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, parent);
		if (!m_wLootStorage)
			return;

		if (compact)
			m_wLootStorage.AddHandler(new SCR_InventoryStorageLootUI(null, null, this, 0, null, m_Player, 4, 6));
		else
			m_wLootStorage.AddHandler(new SCR_InventoryStorageLootUI(null, null, this, 0, null, m_Player, 10, 6));
		
		m_pStorageLootUI = SCR_InventoryStorageBaseUI.Cast(m_wLootStorage.FindHandler(SCR_InventoryStorageLootUI));
	}
	
	void UpdatePlayerData()
	{
		DL_PlayerInfoWidgetData data = new DL_PlayerInfoWidgetData();
		data.InitWidgets(m_widget);
		data.Update();
		
		if (repWidget)
			repWidget.SetText(string.Format("Rep: %1", trader.GetLocalPlayerRep()));
	}
	
	override protected bool MoveBetweenFromVicinity_VirtualArsenal()
	{
		SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
		if (m_pSelectedSlotUI)
			 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		
		if (!m_pSelectedSlotUI || !arsenalInventorySlotUI)
			return false;
		
		if (!arsenalInventorySlotUI.IsAvailable())
			return true;
		
		SCR_InventoryStorageManagerComponent invManagerTo = m_InventoryManager;
		IEntity slotEntity = arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner();
		BaseInventoryStorageComponent storageTo = m_InventoryManager.FindStorageForItem(slotEntity, EStoragePurpose.PURPOSE_ANY);
		IEntity arsenalEntity = SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		ResourceName resourceName = arsenalInventorySlotUI.GetItemResource();
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		// if arsenal entity is a trader, use cash resource type
		EResourceType resourceMode = EResourceType.SUPPLIES;
		if (DE_TraderEntity.Cast(arsenalEntity))
			resourceMode = EResourceType.CASH;
		
		if (invManagerTo.CanInsertItemInStorage(slotEntity, storageTo))
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, resourceMode);
		
		return true;
	}
	
	override protected bool MoveBetweenToVicinity_VirtualArsenal()
	{
		BaseInventoryStorageComponent storageComponent = m_pStorageLootUI.GetCurrentNavigationStorage();
 		if (!storageComponent && m_aOpenedStoragesUI)
			storageComponent = GetOpenArsenalStorage();
		
		if (!storageComponent || !IsStorageArsenal(storageComponent) || DoesSlotContainNonRefundableItems(m_pSelectedSlotUI))
			return false;
		
		IEntity arsenalEntity = storageComponent.GetOwner();
		if (arsenalEntity)
		{
			SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(arsenalEntity.FindComponent(SCR_ArsenalComponent));
			
			if (arsenalComp && !arsenalComp.IsArsenalEnabled())
				return true;
		}
		
		InventoryItemComponent inventoryItemComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		if (!resourceInventoryComponent)
			return false;
		
		// if arsenal entity is a trader, use cash resource type
		EResourceType resourceMode = EResourceType.SUPPLIES;
		if (DE_TraderEntity.Cast(arsenalEntity))
			resourceMode = EResourceType.CASH;
		
		resourceInventoryComponent.RpcAsk_ArsenalRefundItem(Replication.FindId(resourceComponent), Replication.FindId(inventoryItemComponent), resourceMode);
		
		return true;
	}
	
	override protected bool MoveItemToStorageSlot_VirtualArsenal(out bool operationFailed = false)
	{
		SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
		if (m_pSelectedSlotUI)
			 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		else
			return false;
		
		if (!arsenalInventorySlotUI)
		{
			// If the slot was dragged onto a non arsenal slot then do not process virtual arsenal.
			if (!SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI))
				return false;
			
			// Check if slot contains any nonrefundable items
			if (DoesSlotContainNonRefundableItems(m_pSelectedSlotUI))
				return false;
			
			//! Perform refund logic.
			BaseInventoryStorageComponent storageComponent = m_pStorageLootUI.GetCurrentNavigationStorage();
			
			if (!storageComponent) //! Relevant for OpenStorage classes
				storageComponent = m_pActiveHoveredStorageUI.GetStorage();
			
			if (!storageComponent || !IsStorageArsenal(storageComponent) )	
				return false;
			
			IEntity arsenalEntity = storageComponent.GetOwner();
			if (arsenalEntity)
			{
				SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(arsenalEntity.FindComponent(SCR_ArsenalComponent));
				
				//~ Arsenal is disabled so cannot refund
				if (arsenalComp && !arsenalComp.IsArsenalEnabled())
				{
					operationFailed = true;
					return true;
				}
			}
				
			InventoryItemComponent inventoryItemComponent	= m_pSelectedSlotUI.GetInventoryItemComponent();
			SCR_ResourceComponent resourceComponent			= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
			SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
			
			// if arsenal entity is a trader, use cash resource type
			EResourceType resourceMode = EResourceType.SUPPLIES;
			if (DE_TraderEntity.Cast(arsenalEntity))
				resourceMode = EResourceType.CASH;
		
			resourceInventoryComponent.RpcAsk_ArsenalRefundItem(Replication.FindId(resourceComponent), Replication.FindId(inventoryItemComponent), resourceMode);
			
			return true;
		}
		
		/*
		Do nothing but exit as successful because of dragging arsenal virtual item into another
			arsenal virtual item.
		*/
		if (SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI))
		{
			operationFailed = true;
			return true;
		}
		
		SCR_InventoryStorageManagerComponent invManagerTo	= m_pFocusedSlotUI.GetStorageUI().GetInventoryManager();
		BaseInventoryStorageComponent storageTo				= m_pFocusedSlotUI.GetAsStorage();
		if (!storageTo)
			storageTo = m_pFocusedSlotUI.GetStorageUI().GetStorage();

		IEntity arsenalEntity								= SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
		SCR_ResourceComponent resourceComponent				= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		ResourceName resourceName							= arsenalInventorySlotUI.GetItemResource();
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (storageTo.IsInherited(ClothNodeStorageComponent))
		{
			storageTo = invManagerTo.FindActualStorageForItemResource(resourceName, storageTo);
			if (!storageTo)
			{
				operationFailed = true;
				return true;
			}
		}

		// if arsenal entity is a trader, use cash resource type
		EResourceType resourceMode = EResourceType.SUPPLIES;
		if (DE_TraderEntity.Cast(arsenalEntity))
			resourceMode = EResourceType.CASH;
	
		if (invManagerTo.CanInsertItemInStorage(arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner(), storageTo))
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, resourceMode);
		else
			operationFailed = true;
		
		return true;
	}
	
	override protected void GetGeneralItemHintsInfos(out notnull array<SCR_InventoryItemHintUIInfo> hintsInfo)
	{
		DL_LootSystem lootSystem = DL_LootSystem.GetInstance();
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		
		BaseInventoryStorageComponent storage = m_pStorageLootUI.GetCurrentNavigationStorage();
		if (!storage)
			return super.GetGeneralItemHintsInfos(hintsInfo);
		
		DE_TraderEntity trader = DE_TraderEntity.Cast(storage.GetOwner());
		if (!trader)
			return super.GetGeneralItemHintsInfos(hintsInfo);
		
		m_SupplyCostUIInfo = SCR_SupplyCostItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab("{975B61E72A80C207}Configs/Inventory/ItemHints/CashCost_ItemHint.conf"));
		m_SupplyRefundUIInfo = SCR_SupplyRefundItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab("{A5C5F8503B38DCE6}Configs/Inventory/ItemHints/CashSell_ItemHint.conf"));
		repRequirementUIInfo = SCR_SupplyCostItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab("{1E522AF737B22820}Configs/Inventory/ItemHints/RepRequirement_ItemHint.conf"));
		
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(trader.FindComponent(SCR_ResourceComponent));
		SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(trader.FindComponent(SCR_ArsenalComponent));
		
		ResourceName resourceNameItem = m_pFocusedSlotUI.GetItemResource();
		
		SCR_EntityCatalogEntry entry = lootSystem.lootCatalog.GetEntryWithPrefab(resourceNameItem);
		if (!entry)
			return;
		
		if (m_SupplyCostUIInfo && IsStorageArsenal(storage) && arsenalSlot)
		{
				m_SupplyCostUIInfo.SetSupplyCost(economySystem.SupplyToCashValue(arsenalSlot.GetItemSupplyCost(), trader.traderMargin));
				hintsInfo.InsertAt(m_SupplyCostUIInfo, 0);
				
				if (repRequirementUIInfo)
				{
					float repRequirement = DE_ArsenalItemTraderData.GetRepRequirement(entry);
					if (repRequirement != -1)
					{
						if (trader.GetLocalPlayerRep() >= repRequirement)
							return;				
						
						repRequirementUIInfo.SetSupplyCost(repRequirement);
						hintsInfo.InsertAt(repRequirementUIInfo, 1);
					}
				}
		}
		
		if (m_SupplyRefundUIInfo && IsStorageArsenal(storage))
		{
			float resourceCost = 0;
			bool isNonRefundable;
			bool nonrefundableItemInStorage = false;
			if (DoesSlotContainNonRefundableItems(m_pFocusedSlotUI, nonrefundableItemInStorage))
			{
				m_NonrefundableUIInfo.SetContainsNonrefundableItem(nonrefundableItemInStorage);
				hintsInfo.InsertAt(m_NonrefundableUIInfo, 0);
				isNonRefundable = true;
			}
			
			if (!isNonRefundable)
			{
				SCR_Faction faction = arsenalComponent.GetAssignedFaction();
				SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
				SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
				if (data)
				{
					if (arsenalComponent)
						resourceCost = data.GetSupplyRefundAmount(arsenalComponent.GetSupplyCostType());
					else 
						resourceCost = data.GetSupplyRefundAmount(SCR_EArsenalSupplyCostType.DEFAULT);
					
					if (economySystem.fallbackSupplyCost > 0 && resourceCost <= 0)
						resourceCost = economySystem.fallbackSupplyCost;
				}
				
				if (resourceCost >= 0)
				{
					m_SupplyRefundUIInfo.SetSupplyRefund(economySystem.SupplyToCashValue(resourceCost, -trader.traderMargin), true);
					hintsInfo.InsertAt(m_SupplyRefundUIInfo, 0);
				}
			}
		}
		
		if (!m_mItemFactionUIInfos.IsEmpty())
		{
			InventoryItemComponent itemInventory = m_pFocusedSlotUI.GetInventoryItemComponent();
			if (itemInventory)
			{
				SCR_ItemOutfitFactionComponent outfitFaction = SCR_ItemOutfitFactionComponent.Cast(itemInventory.GetOwner().FindComponent(SCR_ItemOutfitFactionComponent));
				if (outfitFaction && outfitFaction.IsValid())
				{
					array<SCR_OutfitFactionData> outfitValues = {};
					outfitFaction.GetOutfitFactionDataArray(outfitValues);
					
					SCR_FactionOutfitItemHintUIInfo foundInfo;
					
					foreach (SCR_OutfitFactionData data : outfitValues)
					{
						if (!m_mItemFactionUIInfos.Find(data.GetAffiliatedFactionKey(), foundInfo))
							continue;
						
						foundInfo.SetFaction(data.GetAffiliatedFaction(), data);
						hintsInfo.Insert(foundInfo);
					}
				}
			}
		}
		
		SCR_IdentityInventoryItemComponent identityInventoryItem = SCR_IdentityInventoryItemComponent.Cast(m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner().FindComponent(SCR_IdentityInventoryItemComponent));
		if (identityInventoryItem && identityInventoryItem.HasValuableIntel(true))
		{
			m_ValuableIntelUIInfo.SetIntel(identityInventoryItem);
			hintsInfo.Insert(m_ValuableIntelUIInfo);
		}
	}
}

modded class SCR_InvCallBack : ScriptedInventoryOperationCallback
{
	override void OnComplete()
	{
		if (!m_pItem)
			return;
		
		return super.OnComplete();
	}
}

class DL_PlayerInfoWidgetData
{
	TextWidget widgetPlayerName;
	TextWidget widgetPlayerData;
	TextWidget widgetBankValue;
	TextWidget widgetWalletValue;
	
	void InitWidgets(notnull Widget root)
	{
		widgetPlayerName = TextWidget.Cast(root.FindAnyWidget("PlayerName"));
		widgetPlayerData = TextWidget.Cast(root.FindAnyWidget("PlayerData"));
		widgetBankValue = TextWidget.Cast(root.FindAnyWidget("BankValue"));
		widgetWalletValue = TextWidget.Cast(root.FindAnyWidget("WalletValue"));
	}
	
	void Update()
	{
		float bankAmount = 0;
		float walletAmount = 0;
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(SCR_PlayerController.GetLocalPlayerId()));
		widgetPlayerName.SetText(GetGame().GetPlayerManager().GetPlayerName(pc.GetPlayerId()));
		
		string playerDataText = "";
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManager)
		{
			SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(pc.GetPlayerId());
			if (playerGroup)
			{
				if (playerGroup.GetCustomName())
					playerDataText = playerGroup.GetCustomName();
				else
				{
					string company, platoon, squad, character, format;
					playerGroup.GetCallsigns(company, platoon, squad, character, format);
					playerDataText = WidgetManager.Translate(format, company, platoon, squad, character);
				}
			}
		}
		widgetPlayerData.SetText(playerDataText);
		
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer playerContainer = playerResource.GetContainer(EResourceType.CASH);
		if (playerContainer)
			bankAmount = playerContainer.GetResourceValue();
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(pc.GetMainEntity());
		if (!char)
			return;
		
		SCR_ResourceComponent characterResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		if (characterResource)
		{
			SCR_ResourceContainer characterContainer = characterResource.GetContainer(EResourceType.CASH);
			if (characterContainer)
				walletAmount = characterContainer.GetResourceValue();
		}
		
		widgetBankValue.SetText("$" + FormatFloat(bankAmount));
		widgetWalletValue.SetText("$" + FormatFloat(walletAmount));
	}
}
