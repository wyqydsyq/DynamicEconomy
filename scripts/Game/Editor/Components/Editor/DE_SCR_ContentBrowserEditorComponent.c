modded class SCR_ContentBrowserEditorComponent : SCR_BaseEditorComponent
{
	void AddBlackListedLabels(array<EEditableEntityLabel> blacklist)
	{
		m_eBlackListedLabels.Copy(blacklist);
	}
}

