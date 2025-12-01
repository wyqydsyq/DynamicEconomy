class DE_BankEntityClass : GenericEntityClass
{
}

class DE_BankEntity : GenericEntity
{
	[RplProp(onRplName: "OnBankerNameChanged")]
	string bankerName;
	
	float currentAmount = 0;
	int baseIncrement = 100;
	int incrementModifier = 0;
	
	void DE_BankEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		DL_LootSystem sys = DL_LootSystem.GetInstance();
		if (!sys || !owner.GetParent())
			return;
		
		DE_BankComponent bankComp = DE_BankComponent.Cast(owner.GetParent().FindComponent(DE_BankComponent));
		bankerName = bankComp.bankerName;
		OnBankerNameChanged();
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetParent());
		if (!character)
			return;
		
		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		if (!characterControllerComponent)
			return;
		
		characterControllerComponent.GetOnPlayerDeathWithParam().Insert(OnCharacterDeath);
	}
	
	void RequestDeposit(int playerId, float amount)
	{
		Rpc(DoDeposit, playerId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void DoDeposit(int playerId, float amount)
	{
		// bank account lives on PC
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer playerContainer = playerResource.GetContainer(EResourceType.CASH);
		
		// wallet lives on SCR_ChimeraCharacter
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer charContainer = charResource.GetContainer(EResourceType.CASH);
		
		if (charContainer.GetResourceValue() < amount)
		{
			SCR_NotificationsComponent.SendToPlayer(playerId, DE_EconomySystem.GetInstance().insufficientNotification, charContainer.GetResourceValue());
			return;
		}
		
		charContainer.DecreaseResourceValue(amount);
		charResource.ReplicateEx();
		
		playerContainer.IncreaseResourceValue(amount);
		playerResource.ReplicateEx();
		
		if (amount > 0)
			SCR_NotificationsComponent.SendToPlayer(playerId, DE_EconomySystem.GetInstance().depositNotification, amount);
		
		pc.NotifyPlayerDataChange();
	}	
	
	void RequestWithdraw(int playerId, float amount)
	{
		Rpc(DoWithdraw, playerId, amount);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void DoWithdraw(int playerId, float amount)
	{
		// bank account lives on PC
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		SCR_ResourceComponent playerResource = SCR_ResourceComponent.Cast(pc.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer playerContainer = playerResource.GetContainer(EResourceType.CASH);
		
		// wallet lives on SCR_ChimeraCharacter
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer charContainer = charResource.GetContainer(EResourceType.CASH);
		
		if (playerContainer.GetResourceValue() < amount)
		{
			SCR_NotificationsComponent.SendToPlayer(playerId, DE_EconomySystem.GetInstance().insufficientNotification, playerContainer.GetResourceValue());
			return;
		}
		
		playerContainer.DecreaseResourceValue(amount);
		playerResource.ReplicateEx();
		
		charContainer.IncreaseResourceValue(amount);
		charResource.ReplicateEx();
		
		if (amount > 0)
			SCR_NotificationsComponent.SendToPlayer(playerId, DE_EconomySystem.GetInstance().withdrawNotification, amount);
	}
	
	void OnCharacterDeath(SCR_CharacterControllerComponent characterControllerComponent, IEntity killerEntity, Instigator killer)
	{
		delete this;
	}
	
	void OnBankerNameChanged()
	{
		// set character name if banker is a character entity
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetParent());
		if (character)
		{
			SCR_CharacterIdentityComponent identity = SCR_CharacterIdentityComponent.Cast(character.FindComponent(SCR_CharacterIdentityComponent));
			identity.GetIdentity().SetName("");
			identity.GetIdentity().SetSurname("");
			identity.GetIdentity().SetAlias(bankerName);
		}
	}
}

class DE_BankComponentClass : ScriptComponentClass
{
}

class DE_BankComponent : ScriptComponent
{
	[Attribute("Banker", UIWidgets.Auto, desc: "Name for this banker shown in UI", category: "Dynamic Economy")]
	string bankerName;
	
	void DE_BankComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
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
			sys.bankComponents.Insert(this);
	}
}

class DE_BankAmountAction : SCR_AdjustSignalAction
{
	DE_BankEntity bank;
	InputManager input;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		bank = DE_BankEntity.Cast(pOwnerEntity);
		input = GetGame().GetInputManager();
		
		input.AddActionListener("BankIncrementModifier10x", EActionTrigger.DOWN, OnIncrement10x);
		input.AddActionListener("BankIncrementModifier100x", EActionTrigger.DOWN, OnIncrement100x);
		input.AddActionListener("BankIncrementModifier1000x", EActionTrigger.DOWN, OnIncrement1000x);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		input.ActivateContext("BankContext");
		bank.currentAmount = m_fTargetValue;
		outName = string.Format("Amount: $%1", FormatFloat(bank.currentAmount, 0));
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		input.ActivateContext("BankContext");
		return super.CanBeShownScript(user);
	}
	
	void OnIncrement10x()
	{
		bank.incrementModifier += 10;
		input.AddActionListener("BankIncrementModifier10x", EActionTrigger.UP, OnIncrement10xRelease);
	}	
	
	void OnIncrement10xRelease()
	{
		bank.incrementModifier -= 10;
		input.RemoveActionListener("BankIncrementModifier10x", EActionTrigger.UP, OnIncrement10xRelease);
	}
	
	void OnIncrement100x()
	{
		bank.incrementModifier += 100;
		input.AddActionListener("BankIncrementModifier100x", EActionTrigger.UP, OnIncrement100xRelease);
	}	
	
	void OnIncrement100xRelease()
	{
		bank.incrementModifier -= 100;
		input.RemoveActionListener("BankIncrementModifier100x", EActionTrigger.UP, OnIncrement100xRelease);
	}
	
	void OnIncrement1000x()
	{
		bank.incrementModifier += 1000;
		input.AddActionListener("BankIncrementModifier1000x", EActionTrigger.UP, OnIncrement1000xRelease);
	}	
	
	void OnIncrement1000xRelease()
	{
		bank.incrementModifier -= 1000;
		input.RemoveActionListener("BankIncrementModifier1000x", EActionTrigger.UP, OnIncrement1000xRelease);
	}
	
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		input.ActivateContext("BankContext");
		return super.GetActionProgressScript(fProgress, timeSlice);
	}
	
	override protected void HandleAction(float value)
	{
		input.ActivateContext("BankContext");
		
		if (value == 0)
			return;
		
		if (m_bManualAdjustment)
			value /= Math.AbsFloat(value);
		
		int modifier = bank.incrementModifier;
		if (modifier == 0)
			modifier = 1;
		
		value *= bank.baseIncrement * Math.Max(modifier, 0.1);
		m_fTargetValue += value;
		
		if (m_bLoopAction)
		{
			if (value > 0 && float.AlmostEqual(SCR_GetCurrentValue(), SCR_GetMaximumValue()))
				m_fTargetValue = SCR_GetMinimumValue();
			else if (value < 0 && float.AlmostEqual(SCR_GetCurrentValue(), SCR_GetMinimumValue()))
				m_fTargetValue = SCR_GetMaximumValue();
		}

		//m_fTargetValue = Math.Round(m_fTargetValue / value) * value;
		m_fTargetValue = Math.Clamp(m_fTargetValue, SCR_GetMinimumValue(), SCR_GetMaximumValue());

		if (!float.AlmostEqual(m_fTargetValue, SCR_GetCurrentValue()))
			SetSendActionDataFlag();
	}
}

class DE_DepositAllAction : SCR_ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId()));
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		if (!charResource)
			return false;
		
		SCR_ResourceContainer charContainer = charResource.GetContainer(EResourceType.CASH);
		
		outName = string.Format("Deposit All ($%1)", FormatFloat(charContainer.GetResourceValue(), 0));
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		DE_BankEntity bank = DE_BankEntity.Cast(pOwnerEntity);
		if (!bank)
			return;
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(pUserEntity);
		SCR_ResourceComponent charResource = SCR_ResourceComponent.Cast(char.FindComponent(SCR_ResourceComponent));
		if (!charResource)
			return;
		
		SCR_ResourceContainer charContainer = charResource.GetContainer(EResourceType.CASH);
		
		bank.DoDeposit(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity), charContainer.GetResourceValue());
	}
}

class DE_DepositAction : SCR_ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Deposit";
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		DE_BankEntity bank = DE_BankEntity.Cast(pOwnerEntity);
		if (!bank)
			return;
		
		bank.DoDeposit(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity), bank.currentAmount);
	}
}

class DE_WithdrawAction : SCR_ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Withdraw";
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		DE_BankEntity bank = DE_BankEntity.Cast(pOwnerEntity);
		if (!bank)
			return;
		
		bank.DoWithdraw(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity), bank.currentAmount);
	}
}
