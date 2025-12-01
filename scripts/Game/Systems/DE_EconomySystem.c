class DE_EconomySystem : WorldSystem
{
	float lastTickTime = 0;
	
	[Attribute("0.1", UIWidgets.Auto, desc: "Dynamic Economy system tick rate, only affects initialization of traders and banks.", category: "Dynamic Economy")]
	float tickInterval;
	
	[Attribute("0.3", UIWidgets.Auto, desc: "Default profit margin for traders, can be overridden on specific DE_TraderComponent", category: "Dynamic Economy")]
	float traderMargin;	
	
	[Attribute("0.25", UIWidgets.Auto, desc: "Default supply cost for items w/o one set, set to 0 to allow free items", category: "Dynamic Economy")]
	float fallbackSupplyCost;
	
	[Attribute("50", UIWidgets.Auto, desc: "Cash:Supply exchange rate, how much cash 1 unit of supply costs", category: "Dynamic Economy")]
	float cashSupplyExchangeRate;
	
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
			
			EntitySpawnParams params = new EntitySpawnParams();
			params.Parent = owner;
			DE_BankEntity bankEnt = DE_BankEntity.Cast(GetGame().SpawnEntityPrefabEx("{2EA890EED4EDE0C2}Prefabs/DE_BankEntity.et", true, null, params));
			owner.AddChild(bankEnt, owner.GetAnimation().GetBoneIndex("Neck1"));
			banks.Insert(bankEnt);
			bankOwners.Insert(owner);
			bankComponents.Remove(i);
		}
		
		for (int i = 0; i < Math.Min(100, traderComponents.Count()); i++)
		{
			DE_TraderComponent traderComp = traderComponents[i];
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
			
			EntitySpawnParams params = new EntitySpawnParams();
			params.Parent = owner;
			DE_TraderEntity traderEnt = DE_TraderEntity.Cast(GetGame().SpawnEntityPrefabEx("{7037D4A3456F324A}Prefabs/DE_TraderEntity.et", true, null, params));
			owner.AddChild(traderEnt, owner.GetAnimation().GetBoneIndex("Neck1"));
			traders.Insert(traderEnt);
			traderOwners.Insert(owner);
			traderComponents.Remove(i);
		}
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