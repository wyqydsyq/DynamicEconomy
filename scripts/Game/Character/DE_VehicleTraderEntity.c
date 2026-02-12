class DE_VehicleTraderEntityClass : DE_TraderEntityClass
{
}

class DE_VehicleTraderEntity : DE_TraderEntity
{
	override void EOnInit(IEntity owner)
	{
		economySystem = DE_EconomySystem.GetInstance();
		if (!economySystem || !owner.GetParent())
			return;
		
		SCR_CatalogEntitySpawnerComponent spawner = SCR_CatalogEntitySpawnerComponent.Cast(owner.FindComponent(SCR_CatalogEntitySpawnerComponent));
		SCR_CampaignBuildingProviderComponent provider = SCR_CampaignBuildingProviderComponent.Cast(owner.FindComponent(SCR_CampaignBuildingProviderComponent));
		UniversalInventoryStorageComponent storage = UniversalInventoryStorageComponent.Cast(owner.FindComponent(UniversalInventoryStorageComponent));
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(owner.FindComponent(SCR_ResourceComponent));
		SCR_ResourceGenerator generator = resource.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		SCR_ResourceConsumer consumer = resource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);

		DE_VehicleTraderComponent traderComp = DE_VehicleTraderComponent.Cast(owner.GetParent().FindComponent(DE_VehicleTraderComponent));
		traderName = traderComp.traderName;
		provider.SetDisplayName(traderName);
		owner.SetName("VehicleTrader-" + traderName);
		OnTraderNameChanged();
		
		if (traderComp.cardPayment)
			cardPayment = traderComp.cardPayment;
		
		if (traderComp.fallbackSupplyCost != -1)
			fallbackSupplyCost = traderComp.fallbackSupplyCost;
		else
			fallbackSupplyCost = economySystem.fallbackSupplyCost;

		if (traderComp.labels && traderComp.labels.Count() > 0)
			labels = traderComp.labels;
		
		if (traderComp.factionKey)
			factionKey = traderComp.factionKey;
		
		if (traderComp.traderMargin != -1)
			traderMargin = traderComp.traderMargin;
		else
			traderMargin = economySystem.traderMargin;
		
		consumer.SetBuyMultiplier(1 + traderMargin, true);
		consumer.SetSellMultiplier(1 - traderMargin, true);
		
		if (labels)
			spawner.SetAllowedLabels(labels);
		
		if (traderComp.itemWhitelist)
			itemWhitelist = traderComp.itemWhitelist;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetParent());
		if (!character)
			return;
		
		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		if (!characterControllerComponent)
			return;
		
		characterControllerComponent.GetOnPlayerDeathWithParam().Insert(OnCharacterDeath);
	}
}

class DE_VehicleTraderComponentClass : DE_TraderComponentClass
{
}

class DE_VehicleTraderComponent : DE_TraderComponent
{
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (!Replication.IsServer())
			return;
		
		DE_EconomySystem sys = DE_EconomySystem.GetInstance();
		if (sys)
			sys.traderComponents.Insert(this);
	}
}