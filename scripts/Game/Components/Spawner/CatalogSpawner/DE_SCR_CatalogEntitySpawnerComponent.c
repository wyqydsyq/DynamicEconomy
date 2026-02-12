modded class SCR_CatalogEntitySpawnerComponent : SCR_SlotServiceComponent
{
	array<EEditableEntityLabel> SetAllowedLabels(array<EEditableEntityLabel> labels)
	{
		m_aAllowedLabels.Clear();
		m_aAllowedLabels.Copy(labels);
		return m_aAllowedLabels;
	}	
	
	array<EEditableEntityLabel> SetIgnoredLabels(array<EEditableEntityLabel> labels)
	{
		m_aIgnoredLabels.Clear();
		m_aIgnoredLabels.Copy(labels);
		return m_aIgnoredLabels;
	}
}
