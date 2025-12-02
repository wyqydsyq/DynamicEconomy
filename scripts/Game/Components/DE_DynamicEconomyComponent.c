class DE_DynamicEconomyComponentClass : SCR_BaseGameModeComponentClass
{
}

class DE_DynamicEconomyComponent : SCR_BaseGameModeComponent
{
	bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return rplComponent && rplComponent.IsProxy();
	}
	
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		super.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);
		
		if (!requestComponent || !entity || IsProxy())
			return;
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(requestComponent.GetPlayerController());
		if (!pc)
			return;

		PrintFormat("DE: Queuing player data update for %1", pc.GetPlayerId());
		DE_EconomySystem.GetInstance().callQueue.CallLater(UpdatePlayerData, 1000, param1: pc, param2: 0);
	}
	
	// notifies existing player their bank data has changed from prefab defaults
	void UpdatePlayerData(SCR_PlayerController pc, int attempts = 0)
	{
		// bank account lives on PC
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer playerContainer = playerResource.GetContainer(EResourceType.CASH);
		
		// wallet lives on SCR_ChimeraCharacter
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		
		// char not spawned/posessed yet, try again after 1sec up to 10 times
		if (!char || !char.FindComponent(SCR_ResourceComponent))
		{
			PrintFormat("DE: Unable to find character for %1, attempts: %2", pc.GetPlayerId(), attempts);
			if (attempts < 10)
				return DE_EconomySystem.GetInstance().callQueue.CallLater(UpdatePlayerData, 1000, param1: pc, param2: attempts + 1);
			else
				return;
		}
		
		PrintFormat("DE: Found character for %1, pushing bank data...", pc.GetPlayerId());
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer charContainer = charResource.GetContainer(EResourceType.CASH);

		pc.NotifyBankDataChange(Replication.FindId(pc), playerContainer.GetResourceValue());
		pc.NotifyBankDataChange(Replication.FindId(char), charContainer.GetResourceValue());
	}
	
	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnControllableDestroyed(instigatorContextData);
		
		if (IsProxy())
			return;
		
		IEntity victim = instigatorContextData.GetVictimEntity();
		if (!victim)
			return;
		
		DE_EconomySystem.GetInstance().callQueue.Call(OnCharacterKilled, victim);
	}
	
	void OnCharacterKilled(IEntity victim)
	{
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		if (!economySystem)
			return;
		
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
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.Parent = char;
		IEntity cashEnt = GetGame().SpawnEntityPrefabEx(economySystem.cashPrefab, true, null, params);
		DE_CashComponent cash = DE_CashComponent.Cast(cashEnt.FindComponent(DE_CashComponent));
		if (!cash)
			return;
		
		float value = wallet.GetResourceValue();
		
		// randomize AI character wallet values
		if (!EntityUtils.IsPlayer(char))
		{
			float maxValue = economySystem.maxAiWalletValue;
			if (Math.RandomFloat(0, 1) > economySystem.jackpotWalletRate)
				maxValue *= economySystem.jackpotWalletThreshold;
			
			value = Math.RandomFloat(economySystem.minAiWalletValue, maxValue);
		}
		
		cash.value = value;
		inv.TryInsertItem(cashEnt, EStoragePurpose.PURPOSE_ANY);
	}
}
