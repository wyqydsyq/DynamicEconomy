[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class DE_SetCashValueEditorAttribute : SCR_BaseValueListEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		if (!economySystem)
			return null;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		if (!CanDisplay(editableEntity))
			return null;
		
		DE_CashComponent cash = DE_CashComponent.Cast(editableEntity.GetOwner().FindComponent(DE_CashComponent));
		if (!cash)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateFloat(Math.Round(cash.value / 1000));
	}
	
	protected bool CanDisplay(SCR_EditableEntityComponent editableEntity)
	{
		return true;
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		DE_EconomySystem economySystem = DE_EconomySystem.GetInstance();
		if (!economySystem)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return;
		
		DE_CashComponent cash = DE_CashComponent.Cast(editableEntity.GetOwner().FindComponent(DE_CashComponent));
		if (!cash)
			return;
		
		cash.value = var.GetFloat() * 1000;
		cash.ForceRplBump();
	}
};