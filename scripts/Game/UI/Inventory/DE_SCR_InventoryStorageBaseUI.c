modded class SCR_InventoryStorageBaseUI : ScriptedWidgetComponent
{
	override void GetAllItems(out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null)
	{
		if (!pStorage)
			return super.GetAllItems(pItemsInStorage, pStorage);
		
		DE_TraderEntity trader = DE_TraderEntity.Cast(pStorage.GetOwner());
		if (!trader)
			return super.GetAllItems(pItemsInStorage, pStorage);
		
		if (s_OnArsenalEnter)
			s_OnArsenalEnter.Clear();
		
		super.GetAllItems(pItemsInStorage, pStorage);
	}
};