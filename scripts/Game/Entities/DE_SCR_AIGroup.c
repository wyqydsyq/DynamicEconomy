modded class SCR_AIGroup : ChimeraAIGroup
{
	override bool AddAIEntityToGroup(IEntity entity)
	{
		bool result = super.AddAIEntityToGroup(entity);
		float value = 0;
		
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		if (!DE_EconomySystem)
			return true;
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(entity);
		if (!char)
			return true;
		
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		if (!resource)
			return true;
		
		SCR_ResourceContainer wallet = resource.GetContainer(EResourceType.CASH);
		if (!wallet)
			return true;
		
		float maxValue = economySystem.maxAiWalletValue;
		if (Math.RandomFloat(0, 1) <= economySystem.jackpotWalletRate)
			maxValue *= economySystem.jackpotWalletMultiplier;
		
		value = Math.RandomFloat(economySystem.minAiWalletValue, maxValue);
		
		wallet.SetResourceValue(value);
		
		return result;
	}
}

