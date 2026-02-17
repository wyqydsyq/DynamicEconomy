class DE_TraderComponentPersistenceData {
	ref array<UUID> playerIDs = {};
	ref array<float> repValues = {};
	
	void DE_TraderComponentPersistenceData(DE_TraderEntity trader = null)
	{
		if (!trader)
			return;
		
		// unzip repMap to individual arrays for keys and values
		// for easy persistence
		foreach (UUID playerID, float playerRep : trader.repMap)
		{
			playerIDs.Insert(playerID);
			repValues.Insert(playerRep);
		}
	}
	
	// zip split arrays back into a map
	DE_TraderRepMap Zip()
	{
		DE_TraderRepMap result = new DE_TraderRepMap();
		
		foreach (int idx, UUID playerID : playerIDs)
		{
			float playerRep = repValues[idx];
			result.Insert(playerID, playerRep);
		}
		
		return result;
	}
}

class DE_TraderComponentSerializer : ScriptedComponentSerializer
{
	override static typename GetTargetType()
	{
		return DE_TraderComponent;
	}

	// find trader attached to component and serialize it's current rep list
	override ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const DE_TraderComponent traderComp = DE_TraderComponent.Cast(component);
		const DE_TraderEntity trader = DE_TraderEntity.Cast(traderComp.trader);
		const DE_TraderComponentPersistenceData data = new DE_TraderComponentPersistenceData(trader);

		context.WriteValue("version", 1);
		context.WriteValue("data", data);
		
		return ESerializeResult.OK;
	}

	// apply saved rep list to component so it can be propagated to created trader entity
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		const DE_TraderComponent traderComp = DE_TraderComponent.Cast(component);
		const DE_TraderComponentPersistenceData data = new DE_TraderComponentPersistenceData();
		
		context.ReadValue("data", data);
		if (!data)
			return false;
		
		DE_TraderRepMap repMap = data.Zip();
		foreach (UUID playerID, float playerRep : repMap)
			traderComp.repMap.Insert(playerID, playerRep);
		
		return true;
	}
}
