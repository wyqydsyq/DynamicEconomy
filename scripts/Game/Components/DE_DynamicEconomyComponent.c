class DE_DynamicEconomyComponentClass : SCR_BaseGameModeComponentClass
{
}

class DE_DynamicEconomyComponent : SCR_BaseGameModeComponent
{
	DE_EconomySystem economySystem;
	
	bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return rplComponent && rplComponent.IsProxy();
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		economySystem = DE_EconomySystem.GetInstance();
	}
	
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		super.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);
		
		if (!requestComponent || !entity || IsProxy())
			return;
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(requestComponent.GetPlayerController());
		if (!pc)
			return;
		
		economySystem = DE_EconomySystem.GetInstance();
		economySystem.callQueue.CallLater(UpdatePlayerData, 1000, param1: pc.GetPlayerId(), param2: 0);
	}
	
	override void OnPlayerAuditSuccess(int playerId)
	{
		economySystem = DE_EconomySystem.GetInstance();
		economySystem.callQueue.CallLater(UpdatePlayerData, 1000, param1: playerId, param2: 0);
	}
	
	override void OnPlayerConnected(int playerId)
	{
		economySystem = DE_EconomySystem.GetInstance();
		economySystem.callQueue.CallLater(UpdatePlayerData, 1000, param1: playerId, param2: 0);
	}
	
	// notifies existing player their bank data has changed from prefab defaults
	void UpdatePlayerData(int playerId, int attempts = 0)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!pc)
			return;
		
		// bank account lives on PC
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer playerContainer = playerResource.GetContainer(EResourceType.CASH);
		
		// wallet lives on SCR_ChimeraCharacter
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		
		// char not spawned/posessed yet, try again after 1sec up to 10 times
		if (!char || !char.FindComponent(SCR_ResourceComponent))
		{
			if (attempts < 10)
				return economySystem.callQueue.CallLater(UpdatePlayerData, 1000, param1: playerId, param2: attempts + 1);
			else
				return;
		}
		
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer charContainer = charResource.GetContainer(EResourceType.CASH);

		pc.NotifyBankDataChange(Replication.FindId(pc), playerContainer.GetResourceValue());
		pc.NotifyBankDataChange(Replication.FindId(char), charContainer.GetResourceValue());
		
		UUID playerUuid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		
		// sync player's rep from each trader
		foreach (DE_TraderEntity trader : economySystem.traders)
		{
			float rep = trader.GetRep(playerUuid);
			if (rep)
				pc.NotifyRepChange(Replication.FindId(trader), rep);
		}
	}
	
	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnControllableDestroyed(instigatorContextData);
		
		if (IsProxy())
			return;
		
		IEntity victim = instigatorContextData.GetVictimEntity();
		if (!victim)
			return;
		
		economySystem = DE_EconomySystem.GetInstance();
		economySystem.callQueue.Call(OnCharacterKilled, victim);
	}
	
	void OnCharacterKilled(IEntity victim)
	{
		economySystem = DE_EconomySystem.GetInstance();
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(victim);
		if (!char)
			return;
		
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(char.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;
		
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		if (!resource)
			return;
		
		SCR_ResourceContainer wallet = resource.GetContainer(EResourceType.CASH);
		if (!wallet)
			return;
		
		float value = wallet.GetResourceValue();
		if (value <= 0)
			return;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.Parent = char;
		IEntity cashEnt = GetGame().SpawnEntityPrefabEx(economySystem.cashPrefab, true, null, params);
		DE_CashComponent cash = DE_CashComponent.Cast(cashEnt.FindComponent(DE_CashComponent));
		if (!cash)
			return;
		
		cash.value = value;
		inv.TryInsertItem(cashEnt, EStoragePurpose.PURPOSE_ANY);
	}
}
