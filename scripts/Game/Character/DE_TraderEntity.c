typedef map<ref UUID, ref float> DE_TraderRepMap;

class DE_TraderEntityClass : GenericEntityClass
{
}

class DE_TraderEntity : GenericEntity
{
	DE_EconomySystem economySystem;
	
	[RplProp(onRplName: "OnTraderNameChanged")]
	string traderName;
	
	[RplProp()]
	bool cardPayment;
	
	[RplProp()]
	float traderMargin = -1;
	
	[RplProp()]
	float fallbackSupplyCost;
	
	[RplProp()]
	SCR_EArsenalItemType types;
	
	[RplProp()]
	SCR_EArsenalItemMode modes;
	
	[RplProp()]
	ref array<EEditableEntityLabel> labels;	
	
	[RplProp()]
	ref array<EEditableEntityLabel> labelsBlacklist;
	
	[RplProp(onRplName: "OnTraderTraitsChanged")]
	ref array<EEditableEntityLabel> traits;	
	
	[RplProp()]
	ref array<EEditableEntityLabel> traitsBlacklist;
	
	[RplProp()]
	FactionKey factionKey;
	
	[RplProp()]
	ref array<ResourceName> itemWhitelist = {};
	
	// map of player UUID -> rep value
	// full map is only held on server, clients instead have their own reps for each trader replicated into economySystem.localRepMap
	ref DE_TraderRepMap repMap = new DE_TraderRepMap();
	float GetRep(UUID playerUuid)
	{
		float rep = 0;
		if (playerUuid && repMap.Contains(playerUuid))
			rep = repMap.Get(playerUuid);
		
		return rep;
	}
	
	// grant player rep based on transaction supply value
	float GrantRep(UUID playerUuid, float supplyValue)
	{
		if (!repMap.Contains(playerUuid))
			repMap.Insert(playerUuid, 0);
		
		float newValue = repMap.Get(playerUuid) + (supplyValue * 0.05 * economySystem.traderRepMultiplier);
		repMap.Set(playerUuid, newValue);
		return newValue;
	}
	
	float GetLocalPlayerRep()
	{
		return economySystem.localRepMap.Get(Replication.FindId(this));
	}
	
	void DE_TraderEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		economySystem = DE_EconomySystem.GetInstance();
		if (!economySystem || !owner.GetParent())
			return;
		
		SCR_ArsenalComponent arsenal = SCR_ArsenalComponent.Cast(owner.FindComponent(SCR_ArsenalComponent));
		UniversalInventoryStorageComponent storage = UniversalInventoryStorageComponent.Cast(owner.FindComponent(UniversalInventoryStorageComponent));
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(owner.FindComponent(SCR_ResourceComponent));
		SCR_ResourceGenerator generator = resource.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.CASH);
		SCR_ResourceConsumer consumer = resource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.CASH);

		DE_TraderComponent traderComp = DE_TraderComponent.Cast(owner.GetParent().FindComponent(DE_TraderComponent));
		traderName = traderComp.traderName;
		owner.SetName("Trader-" + traderName);
		OnTraderNameChanged();
		
		if (traderComp.cardPayment)
			cardPayment = traderComp.cardPayment;
		
		if (traderComp.fallbackSupplyCost != -1)
			fallbackSupplyCost = traderComp.fallbackSupplyCost;
		else
			fallbackSupplyCost = economySystem.fallbackSupplyCost;

		if (traderComp.labels && traderComp.labels.Count() > 0)
			labels = traderComp.labels;
		
		if (traderComp.labelsBlacklist && traderComp.labelsBlacklist.Count() > 0)
			labelsBlacklist = traderComp.labelsBlacklist;
		
		if (traderComp.traits && traderComp.traits.Count() > 0)
			traits = traderComp.traits;	
			
		if (traderComp.traitsBlacklist && traderComp.traitsBlacklist.Count() > 0)
			traitsBlacklist = traderComp.traitsBlacklist;
		
		if (traderComp.factionKey)
			factionKey = traderComp.factionKey;
		
		if (traderComp.traderMargin != -1)
			traderMargin = traderComp.traderMargin;
		else
			traderMargin = economySystem.traderMargin;
		
		consumer.SetBuyMultiplier(1 + traderMargin, true);
		consumer.SetSellMultiplier(1 - traderMargin, true);
		
		if (traderComp.modes)
		{
			modes = traderComp.modes;
			arsenal.SetSupportedArsenalItemTypes(traderComp.modes);
		}
		
		if (traderComp.types)
		{
			types = traderComp.types;
			arsenal.SetSupportedArsenalItemModes(traderComp.types);
		}
		
		if (traderComp.itemWhitelist)
			itemWhitelist = traderComp.itemWhitelist;
		
		RegisterTrader(owner);
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetParent());
		if (!character)
			return;
		
		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		if (!characterControllerComponent)
			return;
		
		characterControllerComponent.GetOnPlayerDeathWithParam().Insert(OnCharacterDeath);
	}
	
	void RegisterTrader(IEntity owner)
	{
		economySystem = DE_EconomySystem.GetInstance();
		if (!economySystem)
		{
			PrintFormat("DE: Unable to find DE_EconomySystem, trader cannot be registered!", level: LogLevel.ERROR);
			return;
		}
		
		DE_TraderComponent traderComp = DE_TraderComponent.Cast(owner.GetParent().FindComponent(DE_TraderComponent));
		if (!traderComp)
		{
			PrintFormat("DE: Unable to find DE_TraderComponent for %1, trader cannot be registered!", this, level: LogLevel.ERROR);
			return;
		}
		
		traderComp.trader = this;
		economySystem.traders.Insert(this);
	}
	
	void OnCharacterDeath(SCR_CharacterControllerComponent characterControllerComponent, IEntity killerEntity, Instigator killer)
	{
		delete this;
	}
	
	void OnTraderNameChanged()
	{
		// ensure trader is registered on local client system on first RPL
		if (!Replication.IsServer() && economySystem && !economySystem.traders.Contains(this))
			RegisterTrader(this);
		
		// set character name if trader is a character entity
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetParent());
		if (character)
		{
			SCR_CharacterIdentityComponent identity = SCR_CharacterIdentityComponent.Cast(character.FindComponent(SCR_CharacterIdentityComponent));
			identity.GetIdentity().SetName("");
			identity.GetIdentity().SetSurname("");
			identity.GetIdentity().SetAlias(traderName);
		}
	}	
	
	void OnTraderTraitsChanged()
	{
		SCR_CampaignBuildingProviderComponent provider = SCR_CampaignBuildingProviderComponent.Cast(FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!provider)
			PrintFormat("DE: Unable to find building provider for %1!", this, level: LogLevel.WARNING);
		
		provider.SetAvailableTraits(traits);
	}
	
	// get additional trader cash value multipliers
	float GetCashValueMult(SCR_EditableEntityUIInfo entityUIInfo)
	{
		float cashValueMult = 1;
		
		if (!DE_VehicleTraderEntity.Cast(this))
			return cashValueMult;
		
		if (
			entityUIInfo.HasEntityLabel(EEditableEntityLabel.VEHICLE_HELICOPTER)
			|| entityUIInfo.HasEntityLabel(EEditableEntityLabel.VEHICLE_AIRPLANE)
		)
			cashValueMult = economySystem.vehicleTraderAircraftCashValueMultiplier;
		
		if (
			entityUIInfo.HasEntityLabel(EEditableEntityLabel.VEHICLE_APC)
		)
			cashValueMult = economySystem.vehicleTraderArmorCashValueMultiplier;
		
		if (
			entityUIInfo.HasEntityLabel(EEditableEntityLabel.FACTION_CIV)
		)
			cashValueMult = economySystem.vehicleTraderCivCashValueMultiplier;
		
		// add global vehicle mult on top of any type-specific one
		return cashValueMult * economySystem.vehicleTraderValueMultiplier;
	}
	
	// get additional trader rep value multipliers
	float GetRepValueMult(SCR_EditableEntityUIInfo entityUIInfo)
	{
		float repValueMult = 1;
		
		if (!DE_VehicleTraderEntity.Cast(this))
			return repValueMult;
		
		if (
			entityUIInfo.HasEntityLabel(EEditableEntityLabel.FACTION_CIV)
		)
			repValueMult *= economySystem.vehicleTraderCivRepValueMultiplier;
		
		// add global vehicle mult on top of any type-specific one
		return repValueMult * economySystem.vehicleTraderRepValueMultiplier;
	}
}

class DE_TraderComponentClass : ScriptComponentClass
{
}

class DE_TraderComponent : ScriptComponent
{
	[Attribute("Trader", UIWidgets.Auto, desc: "Character name for this trader shown in UI", category: "Dynamic Economy - Traders")]
	string traderName;
	
	[Attribute("false", UIWidgets.Auto, desc: "Whether trader accepts card payment (bank) or false for cash-only (wallet)", category: "Dynamic Economy - Traders")]
	bool cardPayment;	
	
	[Attribute("-1", UIWidgets.Auto, desc: "Profit margin for this trader, leave as -1 to use system default", category: "Dynamic Economy - Traders")]
	float traderMargin;
	
	[Attribute("-1", UIWidgets.Auto, desc: "Supply cost for items w/o one set, leave as -1 to use system default, set to 0 to allow free items", category: "Dynamic Economy - Traders")]
	float fallbackSupplyCost;
	
	[Attribute(desc: "Sets Faction Key on underlying SCR_ArsenalComponent, if specified trader will only sell items from matching faction catalogs.", category: "Dynamic Economy - Item Traders")]
	FactionKey factionKey;
	
	[Attribute(desc: "If set, defines what arsenal item types will be available from trader. Leave unset for all", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType), category: "Dynamic Economy - Item Traders")]
	SCR_EArsenalItemType types;

	[Attribute(desc: "If set, defines what arsenal item modes will be available from trader. Leave unset for all", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode), category: "Dynamic Economy - Item Traders")]
	SCR_EArsenalItemMode modes;
	
	[Attribute(desc: "Optional whitelist of ResourceNames to filter items by for e.g. unique traders that only sell specific items like a Safe Zone trader", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Dynamic Economy - Item Traders")]
	ref array<ResourceName> itemWhitelist;
	
	[Attribute(desc: "If set, defines what editable entity labels will be available in trader. Leave unset for all", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Dynamic Economy - Vehicle Traders")]
	ref array<EEditableEntityLabel> labels;
	
	[Attribute(desc: "If set, defines what editable entity labels will be prohibited to appear in trader. Leave unset for all", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Dynamic Economy - Vehicle Traders")]
	ref array<EEditableEntityLabel> labelsBlacklist;
	
	[Attribute(desc: "If set, defines what editable entity traits (labels) will be required to appear in trader. Leave unset for all vehicle types", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Dynamic Economy - Vehicle Traders")]
	ref array<EEditableEntityLabel> traits;	
	
	[Attribute(desc: "If set, defines what editable entity traits (labels) will be prohibited to appear in trader", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Dynamic Economy - Vehicle Traders")]
	ref array<EEditableEntityLabel> traitsBlacklist;
	
	// holds reference to trader entity created by system
	DE_TraderEntity trader;
	
	// gets persisted then set on trader entity upon recreation
	ref DE_TraderRepMap repMap = new DE_TraderRepMap();
	
	void DE_TraderComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetEventMask(ent, ent.GetEventMask() | EntityEvent.INIT);
		if (parent)
			parent.SetEventMask(parent.GetEventMask() | EntityEvent.INIT);
	}
	
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