modded class SCR_CampaignBuildingStartUserAction : ScriptedUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		DE_TraderEntity trader = DE_TraderEntity.Cast(GetOwner());
		if (!trader)
			return super.CanBePerformedScript(user);
		
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		DE_TraderEntity trader = DE_TraderEntity.Cast(GetOwner());
		if (!trader)
			return super.CanBeShownScript(user);
		
		return true;
	}
}
