[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Trader Data", "DISABLED - Trader Data", 1)]
class DE_ArsenalItemTraderData : SCR_BaseEntityCatalogData
{
	[Attribute("-1", UIWidgets.Auto, desc: "Reputation requirement for this item to become available, leave -1 for no rep requirement.\n\nTakes precedence over global Rep supply cost rules", category: "Dynamic Economy")]
	float repRequirement;
	
	[Attribute("0", UIWidgets.Auto, desc: "Secret items are completely hidden until rep requirement is met, otherwise they will be visible but disabled so players can see how to unlock them", category: "Dynamic Economy")]
	bool secret;
	
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		
	}
	
	static float GetRepRequirement(SCR_EntityCatalogEntry entry)
	{
		float repRequirement;
		DE_ArsenalItemTraderData data = DE_ArsenalItemTraderData.Cast(entry.GetEntityDataOfType(DE_ArsenalItemTraderData));
		
		if (data && data.repRequirement != -1)
			repRequirement = data.repRequirement;
		else
		{
			RepSupplyCostRule foundRule = DE_EconomySystem.GetInstance().GetRepRequirement(entry);
			if (!foundRule)
				return -1;
			
			repRequirement = foundRule.repRequired;
		}
		
		return repRequirement;
	}
}
