modded class SCR_CampaignBuildingEditorComponent : SCR_BaseEditorComponent
{
	ref ScriptInvoker Event_OnResourcesChanged = new ScriptInvoker;
	void OnResourcesChanged()
	{
		Event_OnResourcesChanged.Invoke();
	}
	
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
		
		if (providerComponent.ObstrucViewWhenEnemyInRange())
			SetOnEnterEvent();

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
		// end custom fix

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
}
