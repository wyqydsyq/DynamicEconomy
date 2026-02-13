class DE_EconomySystem : WorldSystem
{
	float lastTickTime = 0;
	ref ScriptCallQueue callQueue = new ScriptCallQueue();
	
	[Attribute("0.1", UIWidgets.Auto, desc: "Dynamic Economy system tick rate, only affects initialization of traders and banks.", category: "Dynamic Economy")]
	float tickInterval;	
	
	[Attribute("100", UIWidgets.Auto, desc: "Multiplier/divisor applied to floats that need to pass through functions that only accept ints to preserve decimal places", category: "Dynamic Economy")]
	int intPrecisionFactor;
	
	[Attribute("0.3", UIWidgets.Auto, desc: "Default profit margin for traders, can be overridden on specific DE_TraderComponent", category: "Dynamic Economy - Traders")]
	float traderMargin;
	
	[Attribute("0.005", UIWidgets.Auto, desc: "Fraction of trader margin that gets added to rep gain, 1 = gain as much rep as trader gains in profit (in supply)", category: "Dynamic Economy - Traders")]
	float traderRepMultiplier;
	
	[Attribute("1.5", UIWidgets.Auto, desc: "Multiplier applied to cash value for armor (APCs, Tanks) at vehicle traders, useful for boosting or lowering their cash value as a whole", category: "Dynamic Economy - Traders")]
	float vehicleTraderArmorCashValueMultiplier;
	
	[Attribute("2", UIWidgets.Auto, desc: "Multiplier applied to cash value for aircraft at vehicle traders, useful for boosting or lowering their cash value as a whole", category: "Dynamic Economy - Traders")]
	float vehicleTraderAircraftCashValueMultiplier;	
	
	[Attribute("0.85", UIWidgets.Auto, desc: "Multiplier applied to cash value for Civ vehicles at vehicle traders, useful for boosting or lowering their cash value as a whole", category: "Dynamic Economy - Traders")]
	float vehicleTraderCivCashValueMultiplier;	
	
	[Attribute("0.6", UIWidgets.Auto, desc: "Multiplier applied to rep value for Civ vehicles at vehicle traders, useful for boosting or lowering their rep value as a whole", category: "Dynamic Economy - Traders")]
	float vehicleTraderCivRepValueMultiplier;
	
	[Attribute("5", UIWidgets.Auto, desc: "Multiplier applied to cash value for vehicle traders, useful for boosting or lowering their cash value as a whole", category: "Dynamic Economy - Traders")]
	float vehicleTraderValueMultiplier;
	
	[Attribute("0.175", UIWidgets.Auto, desc: "Multiplier applied to rep cost for vehicle traders, useful for boosting or lowering their rep value as a whole", category: "Dynamic Economy - Traders")]
	float vehicleTraderRepValueMultiplier;
	
	[Attribute("0.25", UIWidgets.Auto, desc: "Default supply cost for items w/o one set, set to 0 to allow free items", category: "Dynamic Economy - Traders")]
	float fallbackSupplyCost;
	
	[Attribute(desc: "Sets rep requirements based on supply cost thresholds for items without one set directly", category: "Dynamic Economy - Traders")]
	ref array<ref RepSupplyCostRule> repSupplyCostRules;
	
	[Attribute("{7037D4A3456F324B}Prefabs/DE_TraderEntity.et", UIWidgets.Auto, desc: "Prefab initialized for DE_TraderComponent instances", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Dynamic Economy - Traders")]
	ResourceName traderEntityPrefab;
	
	[Attribute("{476C7609DA6F1F6E}Prefabs/DE_VehicleTraderEntity.et", UIWidgets.Auto, desc: "Prefab initialized for DE_VehicleTraderComponent instances", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Dynamic Economy - Traders")]
	ResourceName vehicleTraderEntityPrefab;
	
	[Attribute("0.01", desc: "Fraction of trade value to adjust exchange rate by, lower number makes trades impact exchange rate less", category: "Dynamic Economy - Cash")]
	float exchangeRateChangeScale;
	
	[RplProp(onRplName: "ExchangeRateChanged"), Attribute("50", desc: "Cash:Supply exchange rate, how much cash 1 unit of supply costs", category: "Dynamic Economy - Cash")]
	float cashSupplyExchangeRate;
	
	[Attribute("0", desc: "Minimum randomized wallet value of AI characters", category: "Dynamic Economy - AI")]
	float minAiWalletValue;
	
	[Attribute("2500", desc: "Maximum randomized wallet value of AI characters", category: "Dynamic Economy - AI")]
	float maxAiWalletValue;
	
	[Attribute("10", desc: "% of max wallet value that can drop in non-jackpot wallets", category: "Dynamic Economy - AI")]
	float jackpotWalletMultiplier;
	
	[Attribute("0.05", desc: "Likelihood of AI dropping a jackpot wallet", category: "Dynamic Economy - AI")]
	float jackpotWalletRate;
	
	[Attribute("{C6EA723C0E2C52E7}Prefabs/Items/Core/DE_Item_Cash.et", desc: "Cash item prefab dropped on character death", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Dynamic Economy - Cash")]
	ResourceName cashPrefab;
	
	[Attribute("{04CC877B9D9EA4AD}UI/Textures/Editor/ContentBrowser/ContentBrowser_Theme_Commercial.edds", desc: "Icon used to represent traders with card (bank) payment available", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds", category: "Dynamic Economy - UI")]
	ResourceName cashIcon;
	
	[Attribute("{59A4D48F95F0B476}UI/Textures/InventoryIcons/InventorySlot-Size_v2_UI.edds", desc: "Icon used to represent traders with card (bank) payment available", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds", category: "Dynamic Economy - UI")]
	ResourceName cardPaymentIcon;
	
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
	
	// local player rep map of trader rplId -> rep value
	ref map<RplId, float> localRepMap = new map<RplId, float>();
	
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
		
		foreach (DE_BankComponent bankComp : bankComponents)
		{
			IEntity owner = bankComp.GetOwner();
			if (!owner)
				continue;
			
			EntitySpawnParams params = new EntitySpawnParams();
			params.Parent = owner;
			DE_BankEntity bankEnt = DE_BankEntity.Cast(GetGame().SpawnEntityPrefabEx("{2EA890EED4EDE0C3}Prefabs/DE_BankEntity.et", true, null, params));

			TNodeId targetBoneIdx = -1;
			Animation anim = owner.GetAnimation();
			if (anim)
			{
				if (SCR_ChimeraCharacter.Cast(owner))
					targetBoneIdx = anim.GetBoneIndex("Neck1");
				else
					targetBoneIdx = anim.GetBoneIndex("Bone_Keypad"); // if not a character, assume ATM
			}
			
			owner.AddChild(bankEnt, targetBoneIdx);
			banks.Insert(bankEnt);
			bankOwners.Insert(owner);
		}
		bankComponents.Clear();
	
		foreach (DE_TraderComponent traderComp : traderComponents)
		{
			IEntity owner = traderComp.GetOwner();
			if (!owner || traderComp.trader)
				continue;
			
			ResourceName prefab = traderEntityPrefab;
			if (DE_VehicleTraderComponent.Cast(traderComp))
				prefab = vehicleTraderEntityPrefab;
			
			EntitySpawnParams params = new EntitySpawnParams();
			params.Parent = owner;
			DE_TraderEntity traderEnt = DE_TraderEntity.Cast(GetGame().SpawnEntityPrefabEx(prefab, true, null, params));
			
			// hydrate trader repMap from persisted data
			if (traderComp.repMap.Count())
				traderEnt.repMap = traderComp.repMap;
			
			TNodeId targetBoneIdx = -1;
			Animation anim = owner.GetAnimation();
			if (anim)
				targetBoneIdx = anim.GetBoneIndex("Neck1");
				
			owner.AddChild(traderEnt, targetBoneIdx);
			traders.Insert(traderEnt);
			traderOwners.Insert(owner);
			traderComp.trader = traderEnt;
		}
		traderComponents.Clear();
	}
	
	float CalculateRateChange(float resourceCost)
	{
		float rateChange = resourceCost / 1000 * exchangeRateChangeScale * exchangeRateChangeScale;
		cashSupplyExchangeRate += rateChange;
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
		
		// @TODO force-update build menu budget editor component if open?
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
	
	RepSupplyCostRule GetRepRequirement(float supplyCost)
	{
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
			cashValue += cashValue * margin; // 0.3 margin = 130%/1.3x total cost
		
		return cashValue;
	}
}

string FormatFloat(float v, int decimals = 2, bool preserveZeroDecimals = false)
{
	string suffix;
	if (v >= 1000000)
	{
		v /= 1000000;
		suffix = "m";
	}
	else if (v >= 1000)
	{
		v /= 1000;
		suffix = "k";
	}
	
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
	
	if (parts.Count() > 1 && (parts[1] != "00" || preserveZeroDecimals))
		return result + "." + parts[1] + suffix;
	else
		return result + suffix;
}

[BaseContainerProps()]
class RepSupplyCostRule {
	[Attribute("100", UIWidgets.Auto, desc: "Supply Cost threshold items worth equal or more than will have rep requirement applied")]
	float supplyCost;
	
	[Attribute("1", UIWidgets.Auto, desc: "Rep required for any items above supply cost")]
	float repRequired;
}