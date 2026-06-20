modded class SCR_PlacingEditorComponent : SCR_BaseEditorComponent
{
	/*!
	Set currently placed prefab.
	\param prefab Prefab. Must be registered!
	\return true when prefab can be selected (fits budget + prefab data available)
	*/
	override bool SetSelectedPrefab(ResourceName prefab = "", bool onConfirm = false, bool showBudgetMaxNotification = true, set<SCR_EditableEntityComponent> recipients = null, SCR_BaseEditorAction sourceAction = null)
	{
		if (prefab == m_SelectedPrefab) return true;

		//--- Check if entity is within budget
		EEditableEntityBudget blockingBudget;
		bool isPrefabEmpty = prefab.IsEmpty();
		if (!isPrefabEmpty && !CanSelectEntityPrefab(prefab, blockingBudget, showBudgetMaxNotification))
		{
			Print(string.Format("DE: %1 can not be selected! blockingBudget: %2", prefab, blockingBudget), LogLevel.WARNING);
			return false;
		}
		
		//--- Place instantly
		if (m_InstantPlacingParam && !isPrefabEmpty)
		{
			//--- This will prevent m_StatesManager.SetIsWaiting(false) from failing in CreateEntityOwner()
			m_StatesManager.SetSafeDialog(true);
			m_SelectedPrefab = prefab;
			m_Recipients = recipients;
			CreateEntity();
			return true;
		}
		
		//--- Check if the prefab is registered	
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (!isPrefabEmpty && prefabData.GetPrefabID(prefab) == -1)
		{
			Print(string.Format("Cannot initiate placing of prefab @\"%1\", it's not registered in placeable entities!", prefab), LogLevel.WARNING);
			return false;
		}
		
		if (isPrefabEmpty || HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER))
		{
			m_BudgetManager.ResetPreviewCost();
		}
		
		if (m_StatesManager && isPrefabEmpty)
			m_StatesManager.UnsetState(EEditorState.PLACING);
		
		//--- Track if recipients are deleted (stop placing when no recipient remains)
		if (!isPrefabEmpty && recipients)
		{
			m_editableEntityCore.Event_OnEntityUnregistered.Insert(OnEntityUnregistered);
		}
		else if (isPrefabEmpty && m_Recipients)
		{
			m_editableEntityCore.Event_OnEntityUnregistered.Remove(OnEntityUnregistered);
		}
		
		//--- Set the prefab
		ResourceName prefabPrev = m_SelectedPrefab;
		m_SelectedPrefab = prefab;
		m_Recipients = recipients;
		
		//--- Create preview entity
		int instanceCount = 1;
		if (recipients) instanceCount = recipients.Count();
		IEntity previewEntity = m_PreviewManager.CreatePreview(prefab, GetOffsets(instanceCount));//, fixedTransform);
		
		if (previewEntity)
		{
			if (m_StatesManager && !m_StatesManager.SetState(EEditorState.PLACING)) 
			{
				m_PreviewManager.DeletePreview();
				return false; //--- Exit when cannot set placing state
			}
				
			
			//--- Teleport camera to fixed position
			if (m_PreviewManager.IsFixedPosition())
			{
				SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
				if (camera)
				{
					SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
					if (teleportComponent) teleportComponent.TeleportCamera(previewEntity.GetOrigin(), true, true);
				}
			}
			m_CompatiblePlacingFlags = GetCompatiblePlacingFlags(Resource.Load(m_SelectedPrefab));
		}
		else
		{
			SetPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER, false);
		}

		m_iActionInfo = -1;
		m_PlacingFlags &= ~EEditorPlacingFlags.PLACING_ACTION;
		if (sourceAction)
		{
			m_iActionInfo = SCR_EditorPreviewParams.PackActionInfo(sourceAction, m_Owner);
			if (m_iActionInfo >= 0)
				SetPlacingFlag(EEditorPlacingFlags.PLACING_ACTION, true);
		}

		//--- Call event handlers
		Event_OnSelectedPrefabChange.Invoke(prefab, prefabPrev);
		
		if (prefab && !prefabPrev)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceStart(), this);
		else if (!prefab && prefabPrev && !onConfirm)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceCancel(), this);
		
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	override void CreateEntityOwner(int prefabID, array<RplId> entityIds, int entityIndex, int isQueue, bool hasRecipients, RplId currentLayerID, int attempt)
	{
		//~ Remove placing flag player
		if (HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER))
			SetPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER, false);
		
		//~ Todo: Make sure crew and Passenger que placing and placing again budget is updated correctly. 
		//~ Now if placing flag is active on start placing the budget is not correct. Same goes for queing
		if (HasPlacingFlag(EEditorPlacingFlags.VEHICLE_CREWED))
			SetPlacingFlag(EEditorPlacingFlags.VEHICLE_CREWED, false);
		if (HasPlacingFlag(EEditorPlacingFlags.VEHICLE_PASSENGER))
			SetPlacingFlag(EEditorPlacingFlags.VEHICLE_PASSENGER, false);
		
		//--- Delete the ghost preview which was waiting for server callback
		IEntity waitingPreview;
		if (m_WaitingPreviews.Find(entityIndex, waitingPreview))
		{
			m_WaitingPreviews.Remove(entityIndex);
			delete waitingPreview;
		}
		
		if (m_StatesManager && !m_StatesManager.SetIsWaiting(false))
		{
			SetSelectedPrefab();
			return;
		}
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		SCR_EditableEntityComponent entity;
		for (int i, count = entityIds.Count(); i < count; i++)
		{
			entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityIds[i]));
			if (entity)
				entities.Insert(entity);
		}
		
		//--- Wait for entities to be streamed in
		if (entities.IsEmpty())
		{
			if (attempt < 30)
			{
				GetGame().GetCallqueue().CallLater(CreateEntityOwner, 1, false, prefabID, entityIds, entityIndex, isQueue, hasRecipients, currentLayerID, attempt + 1);
			}
			else
			{
				SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
				Print(string.Format("Error when creating entity from prefab '%1' (id = %2)!", prefabData.GetPrefab(prefabID), prefabID), LogLevel.ERROR);
			}

			return;
		}
		
		//--- Select placed entities
		SCR_BaseEditableEntityFilter selectedFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		if (selectedFilter)
			selectedFilter.Replace(entities);
		
		//--- Set current layer to be the parent of newly created entity, to make sure its icon is visible
		SCR_EditableEntityComponent currentLayer = SCR_EditableEntityComponent.Cast(Replication.FindItem(currentLayerID));
		if (!currentLayerID.IsValid() || currentLayer)
		{
			SCR_LayersEditorComponent layerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
			if (layerManager)
				layerManager.SetCurrentLayer(currentLayer);
		}
		
		//--- Delayed check if entity is within budget (character budget update is delayed)
		if (isQueue)
		{
			GetGame().GetCallqueue().CallLater(CheckBudgetOwner, 100, false);
		}
		else
		{
			m_BudgetManager.ResetPreviewCost();
		}
		
		//--- Call event
		Event_OnPlaceEntity.Invoke(prefabID, entity);
		
		//--- Activate effects
		vector pos;
		entities[0].GetPos(pos);
		
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData) SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceConfirm(), this, pos, entities);
	}
};
