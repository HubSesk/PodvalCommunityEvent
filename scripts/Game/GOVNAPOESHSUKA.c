modded class SCR_AIGroup
{
	[Attribute()]
	ref array<ref PointInfo> m_aSpawnPosition;
	
	[Attribute()]
	ref array<ref PS_MoveToVehicle> m_aSpawnVehicle;
	
	override bool SpawnGroupMember(bool snapToTerrain, int index, ResourceName res, bool editMode, bool isLast)
	{
		
		if (!GetGame().GetAIWorld().CanLimitedAIBeAdded() && GetGame().InPlayMode())
		{
			if (isLast)
				Event_OnInit.Invoke(this);
			
			//Event_OnLastGroupMemberSpawned.Invoke(this);
			
			return true;
		}
		BaseWorld world = GetWorld();
		AIFormationDefinition formationDefinition;
		AIFormationComponent formationComponent = AIFormationComponent.Cast(this.FindComponent(AIFormationComponent));
		if (formationComponent)
			formationDefinition = formationComponent.GetFormation();
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.TransformMode = ETransformMode.WORLD;
		GetWorldTransform(spawnParams.Transform);
		float quat[4];
		Math3D.MatrixToQuat(spawnParams.Transform, quat);
		vector pos = spawnParams.Transform[3];
		
		if (m_aSpawnPosition.Count() <= index)
		{
			if (formationDefinition)
				pos = CoordToParent(formationDefinition.GetOffsetPosition(index));
			else
				pos = CoordToParent(Vector(index, 0, 0));
			if (!GetGame().InPlayMode())
			{
				WorldEditorAPI api = _WB_GetEditorAPI();
				if (api)
				{
					IEntitySource groupSource = api.EntityToSource(this);
					if (groupSource)
					{
						int componentsCount = groupSource.GetComponentCount();
						IEntityComponentSource formationComponentSource;
						for (int i = 0; i < componentsCount; i++)
						{
							IEntityComponentSource componentSource = groupSource.GetComponent(i);
							if (componentSource.GetClassName() == "AIFormationComponent")
							{
								formationComponentSource = componentSource;
								break;
							}
						}
						if (formationComponentSource)
						{
							string groupFormationName;
							formationComponentSource.Get("DefaultFormation", groupFormationName);
							IEntitySource aiWorldSource = api.EntityToSource(GetGame().GetAIWorld());
							if (aiWorldSource)
							{
								BaseContainerList formations = aiWorldSource.GetObjectArray("Formations");
								int count = formations.Count();
								for (int i = 0; i < count; i++)
								{
									BaseContainer formation = formations.Get(i);
									string formationName;
									formation.Get("Name", formationName);
									if (formationName != groupFormationName)
										continue;
									string formationOffsets;
									formation.Get("Offsets", formationOffsets);
									array<string> offsetsStrings = {};
									formationOffsets.Split(",", offsetsStrings, false);
									if (offsetsStrings.Count() <= index)
										break;
									string offsetString = offsetsStrings[index];
									vector offset = offsetString.ToVector();
									
									offset = SCR_Math3D.QuatMultiply(quat, offset);
									pos = CoordToParent(Vector(0, 0, 0));
									pos += offset;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			PointInfo position = m_aSpawnPosition[index];
			vector mat[4];
			position.GetLocalTransform(mat);
			pos = CoordToParent(mat[3]);
			Math3D.MatrixMultiply4(spawnParams.Transform, mat, spawnParams.Transform);
		}
		
		if (snapToTerrain)
		{
			float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
			pos[1] = surfaceY;
		}
		
		
		//Snap to the nearest navmesh point
		AIPathfindingComponent pathFindindingComponent = AIPathfindingComponent.Cast(this.FindComponent(AIPathfindingComponent));
		
		if (!editMode && GetGame().InPlayMode())
		{
			NavmeshWorldComponent navmesh = pathFindindingComponent.GetNavmeshComponent();
			if (navmesh)
			{	 
				if (navmesh.IsTileRequested(pos))
				{
					return false;	
				}
				if (!navmesh.IsTileLoaded(pos))
				{
					navmesh.LoadTileIn(pos);
					return false;
				}
			}
		}
		
		if (pathFindindingComponent && pathFindindingComponent.GetClosestPositionOnNavmesh(pos, "10 10 10", pos) && GetGame().InPlayMode() && m_aSpawnPosition.Count() <= index)
		{
			float groundHeight = world.GetSurfaceY(pos[0], pos[2]);
			if (pos[1] < groundHeight)
				pos[1] = groundHeight;
			vector outWaterSurfacePoint;
			EWaterSurfaceType waterSurfaceType;
			vector transformWS[4];
			vector obbExtents;
			if (ChimeraWorldUtils.TryGetWaterSurface(GetWorld(), pos, outWaterSurfacePoint, waterSurfaceType, transformWS, obbExtents))
			{
				pos = outWaterSurfacePoint;
			}
		}
		
		spawnParams.Transform[3] = pos;
		
		IEntity member = GetGame().SpawnEntityPrefabEx(res, true, world, spawnParams);
		if (!member)
			return true;
		
		// Move in to vehicle 
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(member.FindComponent(SCR_EditableEntityComponent));
		
		
		if (editMode)
			m_aSceneGroupUnitInstances.Insert(member);
		
		// Even same null-check is above, in some situations, member can get deleted and it would result in VME
		if (!member)
			return true;
		
		if (m_aSpawnVehicle && m_aSpawnVehicle.Count() > index)
		{
			PS_MoveToVehicle moveToVehicle = m_aSpawnVehicle[index];
			if (moveToVehicle && moveToVehicle.m_sVehicleName != "")
			{
				Vehicle vehicle = Vehicle.Cast(world.FindEntityByName(moveToVehicle.m_sVehicleName));
				if (vehicle)
				{
					SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(member);
					CompartmentAccessComponent compartmentAccessComponent = character.GetCompartmentAccessComponent();
					GetGame().GetCallqueue().Call(PS_MoveToVehicle, vehicle, moveToVehicle, compartmentAccessComponent);
				}
			}
		}
		
		AddAIEntityToGroup(member);
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(member.FindComponent(FactionAffiliationComponent));
		
		if (factionAffiliation)
			factionAffiliation.SetAffiliatedFactionByKey(m_faction);
	
		if (isLast)
			Event_OnInit.Invoke(this);
		return true;
	}
	
	void PS_MoveToVehicle(Vehicle vehicle, PS_MoveToVehicle moveToVehicle, CompartmentAccessComponent compartmentAccessComponent)
	{
		BaseCompartmentManagerComponent compartmentManagerComponent = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		array<BaseCompartmentSlot> outCompartments = {};
		compartmentManagerComponent.GetCompartments(outCompartments);
		compartmentAccessComponent.GetInVehicle(vehicle, outCompartments[moveToVehicle.m_iCompartmentIndex], true, -1, ECloseDoorAfterActions.INVALID, true);
	}
}

[BaseContainerProps()]
class PS_MoveToVehicle
{
	[Attribute()]
	string m_sVehicleName;
	[Attribute()]
	int m_iCompartmentIndex;
}




















