modded class SCR_CampaignBuildingEditorComponent : SCR_BaseEditorComponent
{
	ref ScriptInvoker Event_OnResourcesChanged = new ScriptInvoker;
	void OnResourcesChanged()
	{
		Event_OnResourcesChanged.Invoke();
	}
	
	//---- REFACTOR NOTE START: Has a hot fix for content browser as for some reason it would show an empty entry. So we skip an entry from the catalog and simply hide it ----
	//------------------------------------------------------------------------------------------------
	//! Make the area around where is possible to build composition visible for player
	override protected void EOnEditorActivate()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent(true);
		if (!providerComponent)
			return;
		
		if (!System.IsConsoleApp() && GetGame().GetPlayerController())
		{
			m_BuildingAreaTrigger = SpawnClientTrigger();

			if (m_BuildingAreaTrigger)
			{
				m_BuildingAreaTrigger.SetSphereRadius(providerComponent.GetBuildingRadius());

				SCR_CampaignBuildingAreaMeshComponent areaMeshComponent = SCR_CampaignBuildingAreaMeshComponent.Cast(m_BuildingAreaTrigger.FindComponent(SCR_CampaignBuildingAreaMeshComponent));
				if (areaMeshComponent && areaMeshComponent.ShouldEnableFrameUpdateDuringEditor())
				{
					areaMeshComponent.ActivateEveryFrame();
					areaMeshComponent.GenerateAreaMesh();
				}

				m_BuildingAreaTrigger.SetFlags(EntityFlags.VISIBLE, false);
			}
		}

		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));

		// custom fix wrapping below in condition to prevent trader faction labels being overridden
		DE_TraderEntity trader = DE_TraderEntity.Cast(providerComponent.GetOwner());
		if (!trader)
		{
			FactionAffiliationComponent factionComponent = GetProviderFactionComponent();
			if (!factionComponent)
				return;
				
			Faction buildingFaction;
			if (SCR_VehicleFactionAffiliationComponent.Cast(factionComponent))
				buildingFaction = factionComponent.GetDefaultAffiliatedFaction();
			else
			{
				buildingFaction = factionComponent.GetAffiliatedFaction();
	
				if (!buildingFaction)
					buildingFaction = factionComponent.GetDefaultAffiliatedFaction();
			}
	
			if (buildingFaction)
				AddRemoveFactionLabel(SCR_Faction.Cast(buildingFaction), true);
		}
		else
		{
			// copy trader labels to content browser manager
			if (trader.labels && trader.labels.Count())
			{
				foreach (EEditableEntityLabel label : trader.labels)
				{
					if (!trader.labelsBlacklist || !trader.labelsBlacklist.Count() || !trader.labelsBlacklist.Contains(label))
						m_ContentBrowserManager.AddRemoveLabelOfPersistentBrowserState(label, true);
				}
			}
			
			// remove any blacklisted labels (incl. vanilla and modded ones)
			if (trader.labelsBlacklist && trader.labelsBlacklist.Count())
				m_ContentBrowserManager.AddBlackListedLabels(trader.labelsBlacklist);
		}
		// end custom

		array<SCR_EditorContentBrowserSaveStateDataUI> contentBrowserStates = {};
		int tabsCount = m_ContentBrowserManager.GetContentBrowserTabStates(contentBrowserStates);

		for (int i = 0; i < tabsCount; i++)
		{
			if (!contentBrowserStates[i])
				continue;
			
			//~ Todo: Fix first tab being broken
			//~ Hotfix for first tab being broken
			if (i == 0 || !CanBeShown(contentBrowserStates[i]))
				m_ContentBrowserManager.SetStateTabVisible(i, false);
		}

		ToggleBuildingTool(false);

		// if we were to call it immediately the editor wouldnt technically be opened yet hence we need to delay the call
		// we also want to check periodically to look for if a new enemy has entered the range.
		if (providerComponent.ObstrucViewWhenEnemyInRange())
			GetGame().GetCallqueue().CallLater(TestForEnemies, TEST_DELAY, true);

		SCR_CampaignBuildingProviderComponent nonMasterProviderComponent = GetProviderComponent(false);
		if (!nonMasterProviderComponent)
			return;

		if (!nonMasterProviderComponent.UseAllAvailableProviders())
			return;

		array<SCR_MilitaryBaseComponent> bases = {};
		int basesCount = nonMasterProviderComponent.GetBases(bases);
		if (basesCount > 0)
		{
			bases[0].GetOnMilitaryBaseRegistered().Insert(OnMilitaryBaseRegistrationChanged);
			bases[0].GetOnMilitaryBaseUnregistered().Insert(OnMilitaryBaseRegistrationChanged);
		}
	}
	//---- REFACTOR NOTE END ----
}
