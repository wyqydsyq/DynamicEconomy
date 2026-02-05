class DE_EconomySystem : WorldSystem
{
	float lastTickTime = 0;
	ref ScriptCallQueue callQueue = new ScriptCallQueue();
	
	[Attribute("0.1", UIWidgets.Auto, desc: "Dynamic Economy system tick rate, only affects initialization of traders and banks.", category: "Dynamic Economy")]
	float tickInterval;
	
	[Attribute("0.3", UIWidgets.Auto, desc: "Default profit margin for traders, can be overridden on specific DE_TraderComponent", category: "Dynamic Economy - Traders")]
	float traderMargin;
	
	[Attribute("0.0001", UIWidgets.Auto, desc: "Fraction of trader profit margin that gets added to rep, 1 = gain as much rep as trader gains in profit", category: "Dynamic Economy - Traders")]
	float traderRepMultiplier;
	
	[Attribute(desc: "Sets base rep requirements based on supply cost thresholds used for items without one set", category: "Dynamic Economy - Traders")]
	ref array<ref RepSupplyCostRule> repSupplyCostRules;
	
	[Attribute("0.25", UIWidgets.Auto, desc: "Default supply cost for items w/o one set, set to 0 to allow free items", category: "Dynamic Economy - Traders")]
	float fallbackSupplyCost;
	
	[Attribute("1", UIWidgets.Auto, desc: "Fraction of exchange rate change to apply, lower number makes trade value impact exchange rate more", category: "Dynamic Economy - Cash")]
	float exchangeRateChangeScale;
	
	[RplProp(onRplName: "ExchangeRateChanged"), Attribute("50", UIWidgets.Auto, desc: "Cash:Supply exchange rate, how much cash 1 unit of supply costs", category: "Dynamic Economy - Cash")]
	float cashSupplyExchangeRate;
	
	[Attribute("0", UIWidgets.Auto, desc: "Minimum randomized wallet value of AI characters", category: "Dynamic Economy - AI")]
	float minAiWalletValue;
	
	[Attribute("2500", UIWidgets.Auto, desc: "Maximum randomized wallet value of AI characters", category: "Dynamic Economy - AI")]
	float maxAiWalletValue;
	
	[Attribute("10", UIWidgets.Auto, desc: "% of max wallet value that can drop in non-jackpot wallets", category: "Dynamic Economy - AI")]
	float jackpotWalletMultiplier;
	
	[Attribute("0.05", UIWidgets.Auto, desc: "Likelihood of AI dropping a jackpot wallet", category: "Dynamic Economy - AI")]
	float jackpotWalletRate;
	
	[Attribute("{C6EA723C0E2C52E7}Prefabs/Items/Core/DE_Item_Cash.et", UIWidgets.Auto, desc: "Cash item prefab dropped on character death", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Dynamic Economy - Cash")]
	ResourceName cashPrefab;	
	
	[Attribute("{7037D4A3456F324B}Prefabs/DE_TraderEntity.et", UIWidgets.Auto, desc: "Prefab initialized for DE_TraderComponent instances", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Dynamic Economy - Cash")]
	ResourceName traderEntityPrefab;	
	
	[Attribute("{476C7609DA6F1F6E}Prefabs/DE_VehicleTraderEntity.et", UIWidgets.Auto, desc: "Prefab initialized for DE_VehicleTraderComponent instances", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Dynamic Economy - Cash")]
	ResourceName vehicleTraderEntityPrefab;
	
	[Attribute(ENotification.DE_SELL_NOTIFICATION.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	ENotification sellNotification;
	
	[Attribute(ENotification.DE_BUY_NOTIFICATION.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	ENotification buyNotification;
		
	[Attribute(ENotification.DE_DEPOSIT_NOTIFICATION.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	ENotification depositNotification;
	
	[Attribute(ENotification.DE_WITHDRAW_NOTIFICATION.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	ENotification withdrawNotification;	
	
	[Attribute(ENotification.DE_INSUFFICIENT_FUNDS_NOTIFICATION.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	ENotification insufficientNotification;
	
	ref array<DE_TraderComponent> traderComponents = {};
	ref array<IEntity> traderOwners = {};
	ref array<DE_TraderEntity> traders = {};
		
	ref array<DE_BankComponent> bankComponents = {};
	ref array<IEntity> bankOwners = {};
	ref array<DE_BankEntity> banks = {};
	
	void DE_EconomySystem()
	{
		PrintFormat("DE_EconomySystem: Constructed");
	}
	
    override static void InitInfo(WorldSystemInfo outInfo)
    {
		super.InitInfo(outInfo);
        outInfo
            .SetAbstract(false)
            .SetLocation(ESystemLocation.Both)
            .AddPoint(ESystemPoint.Frame)
			.AddExecuteAfter(DL_LootSystem, WorldSystemPoint.Frame);
    }
	
	override void OnInit()
	{
		PrintFormat("DE_EconomySystem: OnInit");
		if (!Replication.IsServer())
			return;
	}
	
	static DE_EconomySystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return DE_EconomySystem.Cast(world.FindSystem(DE_EconomySystem));
	}
	
	override void OnUpdate(ESystemPoint point)
	{
		callQueue.Tick(point);
		
		if (!Replication.IsServer()) // only calculate updates on server, changes are broadcast to clients
			return;
		
		float time = GetGame().GetWorld().GetFixedTimeSlice();
		lastTickTime += time;
		if (lastTickTime < tickInterval)
			return;
		lastTickTime = 0;
		
		if (!DL_LootSystem.GetInstance().lootDataReady)
			return;
		
		for (int i = 0; i < Math.Min(100, bankComponents.Count()); i++)
		{
			DE_BankComponent bankComp = bankComponents[i];
			if (!bankComp)
				continue;
			
			IEntity owner = bankComp.GetOwner();
			if (!owner)
				return;
			
			DE_BankEntity bankEnt = DE_BankEntity.Cast(GetGame().SpawnEntityPrefabEx("{2EA890EED4EDE0C3}Prefabs/DE_BankEntity.et", true));

			int attachIdx = -1;
			if (SCR_ChimeraCharacter.Cast(owner))
				attachIdx = owner.GetAnimation().GetBoneIndex("Neck1");
			else
				attachIdx = owner.GetAnimation().GetBoneIndex("Bone_Keypad"); // if not a character, assume ATM
			
			owner.AddChild(bankEnt, attachIdx);
			banks.Insert(bankEnt);
			bankOwners.Insert(owner);
			bankComponents.Remove(i);
		}
		
		for (int i = 0; i < Math.Min(100, traderComponents.Count()); i++)
		{
			DE_TraderComponent traderComp = traderComponents[i];
			ResourceName prefab = traderEntityPrefab;
			if (!traderComp)
			{
				traderComponents.Remove(i);
				continue;
			}
			
			IEntity owner = traderComp.GetOwner();
			if (!owner)
			{
				traderComponents.Remove(i);
				continue;
			}
			
			if (DE_VehicleTraderComponent.Cast(traderComp))
				prefab = vehicleTraderEntityPrefab;
			
			EntitySpawnParams params = new EntitySpawnParams();
			params.Parent = owner;
			DE_TraderEntity traderEnt = DE_TraderEntity.Cast(GetGame().SpawnEntityPrefabEx(prefab, true, null, params));
			
			owner.AddChild(traderEnt, owner.GetAnimation().GetBoneIndex("Neck1"));
			traders.Insert(traderEnt);
			traderOwners.Insert(owner);
			traderComponents.Remove(i);
		}
	}
	
	float CalculateRateChange(float resourceCost)
	{
		float rateChange = (resourceCost / cashSupplyExchangeRate) / (cashSupplyExchangeRate * cashSupplyExchangeRate) / (100 * exchangeRateChangeScale);
		cashSupplyExchangeRate += rateChange;
		//PrintFormat("DE: CalculateRateChange(%1): %2 -> %3", resourceCost, rateChange, cashSupplyExchangeRate);
		ExchangeRateChanged();
		Replication.BumpMe();
		return rateChange;
	}
	
	void ExchangeRateChanged()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		MenuBase menu = menuManager.GetTopMenu();
		if (!menu)
			return;
		
		SCR_InventoryMenuUI inventoryUI = SCR_InventoryMenuUI.Cast(menu);
		if (!inventoryUI)
			return;
		
		inventoryUI.RefreshLootUIListener();
	}
	
	RepSupplyCostRule GetRepRequirement(SCR_EntityCatalogEntry entry)
	{
		SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!data)
			return null;
		
		float supplyCost = data.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT);
		RepSupplyCostRule highestRule;
		
		foreach (RepSupplyCostRule rule : repSupplyCostRules)
		{
			if (
				supplyCost >= rule.supplyCost
				&& (
					!highestRule
					|| highestRule.supplyCost < rule.supplyCost
				)
			)
				highestRule = rule;
		}
		
		return highestRule;
	}
	
	void GetPlayerCashConsumers(int playerId, out SCR_ResourceConsumer bankConsumer, out SCR_ResourceConsumer walletConsumer)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!pc)
			return;
		
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		bankConsumer = playerResource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(pc.GetPlayerId()));
		if (!char)
			return;
		
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		walletConsumer = charResource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
	}
	
	float SupplyToCashValue(float supplyCost, float margin = -1)
	{
		float cashValue = supplyCost;
		if (fallbackSupplyCost > 0 && supplyCost <= 0)
			cashValue = fallbackSupplyCost;
		
		// multiply supply cost by cash-supply exchange rate
		cashValue *= cashSupplyExchangeRate;
		
		// apply margin
		if (margin != -1)
			cashValue *= margin;
		
		return cashValue;
	}
}

string FormatFloat(float v, int decimals = 2)
{
	string str = v.ToString(lenDec: decimals);
	string result = "";
	array<string> parts = {};
	
	if (decimals > 0)
		str.Split(".", parts, true);
	else
		parts.Insert(str);
	
	int offset = 0;
	int separateOffset = 3;
	for (int i = 1; i <= parts[0].Length(); i++)
	{
		string char = parts[0][parts[0].Length() - i];
		if (char == "." || char == ",")
			break;
		
		result = char + result;
		offset++;
		if (offset >= separateOffset && i <= parts[0].Length() - 1)
		{
			result = "," + result;
			offset = 0;
		}
	}
	
	if (parts.Count() > 1)
		return result + "." + parts[1];
	else
		return result;
}

[BaseContainerProps()]
class RepSupplyCostRule {
	[Attribute("100", UIWidgets.Auto, desc: "Supply Cost threshold items worth equal or more than will have rep requirement applied")]
	float supplyCost;
	
	[Attribute("1", UIWidgets.Auto, desc: "Rep required for any items above supply cost")]
	float repRequired;
}