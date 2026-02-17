/*
 Based on SCR_ResourceEntityRefundAction, changed to give player cash and rep with nearest trader instead of supply
 had to create a new class for this since there is no way to do nested super calls e.g. DE_TraderSellAction -> SCR_ResourceEntityRefundAction -> SCR_ScriptedUserAction
 to override stuff like CanBeShownScript to skip SCR_ResourceEntityRefundAction logic and call SCR_ScriptedUserAction logic after our custom stuff
*/
class DE_TraderSellAction : SCR_ScriptedUserAction
{
	DL_LootSystem lootSystem;
	DE_EconomySystem economySystem;
	DE_TraderEntity trader;
	IEntity owner;
	
	SCR_EditableEntityUIInfo uiInfo;
	SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	float supplyCost = -1;

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		lootSystem = DL_LootSystem.GetInstance();
		economySystem = DE_EconomySystem.GetInstance();
		owner = pOwnerEntity;
		trader = FindTrader();
		
		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
		
		SCR_EditableEntityComponent editableComp = SCR_EditableEntityComponent.Cast(owner.FindComponent(SCR_EditableEntityComponent));
		if (editableComp)
			uiInfo = SCR_EditableEntityUIInfo.Cast(editableComp.GetInfo(owner));

		if (lootSystem.vehicleDataReady)
			GetSupplyCost();
		else
			lootSystem.Event_VehicleCatalogsReady.Insert(GetSupplyCost);
	}
	
	void GetSupplyCost()
	{
		// find prefab in DL's merged entity catalog that contains all factions vehicles
		ResourceName prefabName = owner.GetPrefabData().GetPrefabName();
		SCR_EntityCatalogEntry entry = lootSystem.vehicleCatalog.GetEntryWithPrefab(prefabName);
		if (!entry)
			return;
		
		uiInfo = SCR_EditableEntityUIInfo.Cast(entry.GetEntityUiInfo());
		if (uiInfo)
		{
			array<ref SCR_EntityBudgetValue> budgets = {};
			uiInfo.GetEntityAndChildrenBudgetCost(budgets);
			
			foreach (SCR_EntityBudgetValue budget : budgets)
			{
				if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
					supplyCost += budget.GetBudgetValue();
			}
		}
		
		if (supplyCost != -1)
			return;
		
		SCR_EntityCatalogSpawnerData data = SCR_EntityCatalogSpawnerData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!data)
			return;
		
		supplyCost = data.GetSupplyCost();
	}

	DE_TraderEntity FindTrader()
	{
		if (!owner)
		{
			owner = GetOwner();
			if (!owner)
				return null;
		}
		
		ref DE_TraderEntity nearestTrader;
		vector vehicleOrigin = owner.GetOrigin();
		float nearestTraderDistance = 25; // initial min check radius
		
		foreach (ref DE_TraderEntity foundTrader : economySystem.traders)
		{
			float traderDistance = vector.Distance(vehicleOrigin, foundTrader.GetOrigin());
			if (traderDistance < nearestTraderDistance)
			{
				nearestTraderDistance = traderDistance;
				nearestTrader = foundTrader;
			}
		}
		
		trader = nearestTrader;
		return trader;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		RplComponent rplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rplComp || !rplComp.IsMaster())
			return;
		
		if (m_CompartmentManager && m_CompartmentManager.AnyCompartmentsOccupiedOrLocked())
			return;
		
		// need to get reference to trader and system again on server as it hasn't run Init on the action
		economySystem = DE_EconomySystem.GetInstance();
		FindTrader();
		
		float cashGain = economySystem.SupplyToCashValue(supplyCost * trader.GetCashValueMult(uiInfo), -trader.traderMargin);
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		
		SCR_ResourceConsumer walletConsumer;
		economySystem.GetPlayerCashConsumers(playerId, null, walletConsumer);
		
		SCR_ResourceContainer walletContainer = walletConsumer.GetComponent().GetContainer(EResourceType.CASH);
		walletContainer.IncreaseResourceValue(cashGain);
		
		// increase rep for this player based on trader profit
		UUID playerUuid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		trader.GrantRep(playerUuid, supplyCost);
		
		pc.NotifyRepChange(Replication.FindId(trader), trader.GetRep(playerUuid));
		pc.NotifyBankDataChange(Replication.FindId(trader), walletContainer.GetResourceValue());
		pc.NotifyPlayerDataChange(cashGain);
		
		RplComponent.DeleteRplEntity(GetOwner(), false);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		FindTrader();
		if (!trader)
			return false;
		
		if (supplyCost > 0)
		{
			ActionNameParams[0] = trader.traderName;
			ActionNameParams[1] = FormatFloat(economySystem.SupplyToCashValue(supplyCost * trader.GetCashValueMult(uiInfo), -trader.traderMargin));

			outName = "Sell to %1: $%2";
			return true;
		}
		else
		{
			ActionNameParams[0] = trader.traderName;
			outName = "%1 is not interested in buying this";
			return true;
		}
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (!FindTrader())
		{
			m_sCannotPerformReason = "No trader nearby";
			return false;
		}
		
		if (m_CompartmentManager && m_CompartmentManager.AnyCompartmentsOccupiedOrLocked())
		{
			m_sCannotPerformReason = "#AR-Supplies_Refund_Action_Vehicle_Occupied";
			return false;
		}
		
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (!FindTrader())
			return false;
		
		return super.CanBeShownScript(user);
	}
}
