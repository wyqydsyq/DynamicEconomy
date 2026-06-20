class DE_EconomySystemData : PersistentState
{
	float cashSupplyExchangeRate = 50;
}

class DE_EconomySystemSerializer : ScriptedStateSerializer
{
	override static typename GetTargetType()
	{
		return DE_EconomySystemData;
	}
	
	override ESerializeResult Serialize(notnull Managed instance, notnull SaveContext context)
	{
		DE_EconomySystem system = DE_EconomySystem.GetInstance();
		DE_EconomySystemData data = DE_EconomySystemData.Cast(instance);
		data.cashSupplyExchangeRate = system.cashSupplyExchangeRate;
		context.WriteValueDefault("cashSupplyExchangeRate", data.cashSupplyExchangeRate, 50);
		return ESerializeResult.OK;
	}
	
	override bool Deserialize(notnull Managed instance, notnull LoadContext context)
	{
		DE_EconomySystem system = DE_EconomySystem.GetInstance();
		DE_EconomySystemData data = DE_EconomySystemData.Cast(instance);
		context.ReadValueDefault("cashSupplyExchangeRate", data.cashSupplyExchangeRate, 50);
		system.cashSupplyExchangeRate = data.cashSupplyExchangeRate;
		return true;
	}
}
