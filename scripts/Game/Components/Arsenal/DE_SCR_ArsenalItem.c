[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Arsenal Data", "DISABLED - Arsenal Data", 1)]
modded class SCR_ArsenalItem : SCR_BaseEntityCatalogData
{	
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		m_EntryParent = entry;
		
		m_ItemResource = Resource.Load(m_EntryParent.GetPrefab());
		
		// BI forgot to null check
		if (m_aArsenalAlternativeCostData && !m_aArsenalAlternativeCostData.IsEmpty())
		{
			m_mArsenalAlternativeCostData = new map<SCR_EArsenalSupplyCostType, ref SCR_ArsenalAlternativeCostData>();
		
			foreach (SCR_ArsenalAlternativeCostData data : m_aArsenalAlternativeCostData)
			{
				//~ Ignore default as that is m_iSupplyCost defined in the arsenal item
				if (data.m_eAlternativeCostType == SCR_EArsenalSupplyCostType.DEFAULT)
					continue;
				
				m_mArsenalAlternativeCostData.Insert(data.m_eAlternativeCostType, data);
			}
			
			//~ Delete array
			m_aArsenalAlternativeCostData = null;
		}
	}
}
