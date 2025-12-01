[BaseContainerProps(configRoot: true)]
modded class SCR_ResourceGenerator : SCR_ResourceInteractor
{
	void SetResourceMultiplier(float mult)
	{
		m_fResourceMultiplier = mult;
	}
	
	//------------------------------------------------------------------------------------------------
	override static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, SCR_ResourceGenerator.CODEC_GENERATOR_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	override static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return snapshot.Serialize(packet, SCR_ResourceGenerator.CODEC_GENERATOR_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	override static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, SCR_ResourceGenerator.CODEC_GENERATOR_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	override static bool PropCompare(SCR_ResourceGenerator instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return instance.PropCompareNetworkedVariables(snapshot, ctx);
	}
	
	//------------------------------------------------------------------------------------------------
	override static bool Extract(SCR_ResourceGenerator instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return instance.ExtractNetworkedVariables(snapshot, ctx);
	}
	
	//------------------------------------------------------------------------------------------------
	override static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_ResourceGenerator instance)
	{
		return instance.InjectNetworkedVariables(snapshot, ctx);
	}
}