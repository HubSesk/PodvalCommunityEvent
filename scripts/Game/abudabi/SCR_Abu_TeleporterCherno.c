class EntityCollectorCherno
{
    private ref array<IEntity> m_Entities;

    void EntityCollectorCherno()
    {
        m_Entities = new array<IEntity>();
    }

    bool OnEntity(IEntity ent)
    {
        m_Entities.Insert(ent);
        return true; // Continue querying
    }

    array<IEntity> GetEntities()
    {
        return m_Entities;
    }
}	

class SCR_Abu_TeleporterCherno
{
	[Attribute("US", desc: "Winner by trigger")]
	string factionkey;
    // ======================================================
    // Adjustable Variables
    // ======================================================

      // --- SurfaceIsFlat ---
    // Purpose: Defines the maximum allowable slope angle (in degrees) for a surface to be considered flat.
    // Used in: SurfaceIsFlat
    static const float MAX_SLOPE_ANGLE = 1.0; // Maximum slope angle in degrees for flat surfaces

    // --- RandomizeNextPosition ---
    // Purpose: Controls the minimum distance between entities and the maximum random offset for generating new positions.
    // Used in: RandomizeNextPosition
    static const float MIN_DISTANCE_BETWEEN_ENTITIES = 10.0; // Minimum distance between entities 
	static const float SEARCH_STEP = 10; // Step size for searching in X and Z directions
    static const int MAX_SEARCH_RADIUS = 1000; // Maximum search radius in meters

    // --- Random Position Generation ---
    // Purpose: Defines the bounds for randomizing X and Z coordinates of attack positions.
    // Used in: GenerateRandomSpawnPointAttackSide
    static const float MIN_X_COORDINATE = 89.0;   // Minimum X coordinate for attack positions
    static const float MAX_X_COORDINATE = 12710.0; // Maximum X coordinate for attack positions
    static const float MIN_Z_COORDINATE = 186.0;  // Minimum Z coordinate for attack positions
    static const float MAX_Z_COORDINATE = 12640.0; // Maximum Z coordinate for attack positions

    // --- Max Attempts for Position Generation ---
    // Purpose: Limits the number of attempts to find a valid position before giving up.
    // Used in: GenerateRandomSpawnPointAttackSide, RandomizeNextPosition
    static const int MAX_ATTEMPTS_ATTACK_POSITION = 2147483646; // Max attempts for attack position generation
    static const int MAX_ATTEMPTS_RANDOM_POSITION = 2147483646; // Max attempts for randomizing entity positions

    // --- Number of Entities ---
    // Purpose: Defines the number of entities to teleport for both Defend and Attack groups.
    // Used in: TeleportGroups
    static const int NUM_DEFEND_ENTITIES = 50; // Number of Defend entities (Def_0 to Def_15)
    static const int NUM_ATTACK_ENTITIES = 50; // Number of Attack entities (Group_0 to Group_15)

    // --- RandomizeNextPosition Retry Mechanism ---
    // Purpose: Defines the maximum number of retries for finding a valid position.
    // Used in: RandomizeNextPosition
    static const int MAX_RETRIES = 2; // Maximum number of retries before giving up
	
	// --- GenerateRandomSpawnPointAttackSide ---
    // Purpose: Defines the minimum and maximum distance between attack and defend spawn points.
    // Used in: GenerateRandomSpawnPointAttackSide
    float MIN_DISTANCE_ATTACK_DEFEND = 2500.0; // Minimum distance between attack and defend spawns
    float MAX_DISTANCE_ATTACK_DEFEND = 4000.0; // Maximum distance between attack and defend spawns
		

    // ======================================================
    // Methods
    // ======================================================	
	
		
	int PrintEntitiesInSphere(vector center, float radius)
	{
	    // Get the world instance
	    BaseWorld world = GetGame().GetWorld();
	    if (!world)
	    {
	        Print("World not found!");
	        return(200);
	    }
	
	    // Create collector and perform query
	    ref EntityCollectorCherno collector = new EntityCollectorCherno();
	    bool queryResult = world.QueryEntitiesBySphere(center, radius, collector.OnEntity, null, EQueryEntitiesFlags.ALL);
	
	    // Retrieve and print results
	    array<IEntity> entities = collector.GetEntities();

		int counter=0;
	    foreach (IEntity entity : entities)
	    {
			counter++;
			string entityName = entity.GetName();
			//SCR_EntityHelper.DeleteEntityAndChildren(entity);
			
	        //string entityName = entity.GetName();
	        //EntityID entityID = entity.GetID();
			
			
	        //Print(string.Format("Entity Found: %1 (ID: %2)", entityName, entityID.ToString()));
			
	    }
		return (counter);
	}
	
	void DeleteEntitiesInCylinder(vector center, float radius, float height)
	{
	    // Get the world instance
	    BaseWorld world = GetGame().GetWorld();
	    if (!world)
	    {
	        Print("World not found!");
	        return;
	    }
	
	    // Calculate the sphere radius to cover the cylindrical area
	    float sphereRadius = Math.Sqrt(radius * radius + height * height);
	
	    // Create collector and perform sphere query
	    ref EntityCollectorCherno collector = new EntityCollectorCherno();
	    bool queryResult = world.QueryEntitiesBySphere(center, sphereRadius, collector.OnEntity, null, EQueryEntitiesFlags.ALL);
	
	    // Retrieve and filter entities
	    array<IEntity> entities = collector.GetEntities();
	    foreach (IEntity entity : entities)
	    {
	        vector entityPos = entity.GetOrigin();
	        vector delta = entityPos - center;
	
	        // Calculate horizontal distance (ignore Y-axis)
	        float horizontalDistance = Math.Sqrt(delta[0] * delta[0] + delta[2] * delta[2]);
	        
	        // Check if within horizontal radius and vertical height range
	        if (horizontalDistance <= radius && delta[1] >= 0 && delta[1] <= height)
	        {
	            SCR_EntityHelper.DeleteEntityAndChildren(entity);
	        }
	    }
	}
	
	void UnfreezeCars() //On freezetime end
	{
		PS_GameModeCoop gameMode = PS_GameModeCoop.Cast(GetGame().GetGameMode());
		//Print("IsFreezeTimeEnd "+gameMode.IsFreezeTimeEnd());
		if(gameMode.IsFreezeTimeEnd())
		{
			SetVehiclesUnBroke();
			// Cancel the repeated call to UnfreezeCars
        	GetGame().GetCallqueue().Remove(UnfreezeCars);
		};
	}
	
	bool KSHM_Destroyed = false;
	
	void MCU_Trcuks_Trigger()
	{
	    bool Truck0 = false, Truck1 = false, Truck2 = false;
	    IEntity Target0 = GetGame().GetWorld().FindEntityByName("Target_0");
	    IEntity Target1 = GetGame().GetWorld().FindEntityByName("Target_1");
	    IEntity Target2 = GetGame().GetWorld().FindEntityByName("Target_2");
	    
	    // Check if all targets exist
	    if (!Target0 || !Target1 || !Target2)
	        return;
	        
	    DamageManagerComponent Target0dmg = DamageManagerComponent.Cast(Target0.FindComponent(DamageManagerComponent));
	    DamageManagerComponent Target1dmg = DamageManagerComponent.Cast(Target1.FindComponent(DamageManagerComponent));
	    DamageManagerComponent Target2dmg = DamageManagerComponent.Cast(Target2.FindComponent(DamageManagerComponent));
	    
	    // Check if all damage components exist
	    if (!Target0dmg || !Target1dmg || !Target2dmg)
	        return;
	    
	    if (KSHM_Destroyed == false && Target0dmg.GetHealth() <= 0 && Target1dmg.GetHealth() <= 0 && Target2dmg.GetHealth() <= 0)
	    {
	        // Create the hint
	        const string hintTitle = "Победа!";
	        const string hintDescription = "Все КШМ уничтожены, победа США";
	        const float duration = 10; // Duration in seconds
	        EHint type = EHint.UNDEFINED; // Hint type
	        bool isTimerVisible = true; // Show timer bar
	        EFieldManualEntryId fieldManualEntry = EFieldManualEntryId.NONE; // No field manual link
	
	        // Create the hint info
	        SCR_HintUIInfo hintInfo = SCR_HintUIInfo.CreateInfo(hintDescription, hintTitle, duration, type, fieldManualEntry, isTimerVisible);
	
	        // Show the hint
	        SCR_HintManagerComponent.ShowHint(hintInfo);
	        KSHM_Destroyed = true;
			//GameStateTransitions.RequestScenarioRestart();
	        //GetGame().RequestClose();
	    }
	}
	
	void CountSupremacyInTrigger()
	{

		//IEntity Trigger0 = GetGame().GetWorld().FindEntityByName("Target_0");
	}
	
	
	int CountPlayersOnServer()
	{
	    PlayerManager playerManager = GetGame().GetPlayerManager();
	    array<int> players = {};
	    playerManager.GetPlayers(players);
	    return players.Count();
	}
	
	void changeStartingDistanceOnPlayerCount()
	{
		int num=CountPlayersOnServer();
		if (num>=0 && num <=20){
			MIN_DISTANCE_ATTACK_DEFEND = 2500;
			MAX_DISTANCE_ATTACK_DEFEND = 4000;
			}
		if (num>20 && num <=40){
			MIN_DISTANCE_ATTACK_DEFEND = 2500;
			MAX_DISTANCE_ATTACK_DEFEND = 4000;
			}
		if (num>40 && num <=60){
			MIN_DISTANCE_ATTACK_DEFEND = 2500;
			MAX_DISTANCE_ATTACK_DEFEND = 4000;
			}
		if (num>60 && num <=80){
			MIN_DISTANCE_ATTACK_DEFEND = 2500;
			MAX_DISTANCE_ATTACK_DEFEND = 4000;
			}
		if (num>80 && num <=100){
			MIN_DISTANCE_ATTACK_DEFEND = 2500;
			MAX_DISTANCE_ATTACK_DEFEND = 4000;
			}
		if (num>100 && num <=130){
			MIN_DISTANCE_ATTACK_DEFEND = 2500;
			MAX_DISTANCE_ATTACK_DEFEND = 4000;
			}
	
	}
	
	void SetVehiclesBrake() 
	{
	    array<IEntity> EntitiesALL = {};
	    
	    // Get the instance of SCR_EditableEntityCore
	    SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
	    if (!core) {
	        Print("Failed to get SCR_EditableEntityCore instance!", LogLevel.ERROR);
	        return;
	    }
	    
	    // Retrieve all editable entities
	    set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
	    core.GetAllEntities(entities, false, true);
	    
	    // Iterate through the entities and add them to the array
	    foreach (SCR_EditableEntityComponent comp : entities) {
	        IEntity entity = comp.GetOwner();
	        if (entity) {
	            EntitiesALL.Insert(entity);
	            typename type = entity.Type();
	            //Print(type.ToString());
	            
	            // Try to get the VehicleWheeledSimulation component from the entity
	            VehicleWheeledSimulation veh = VehicleWheeledSimulation.Cast(entity.FindComponent(VehicleWheeledSimulation));
	            if (veh) {
	                // Set the gear to 2 (assuming 2 is the first gear)
					veh.SetBreak(1,1);
					veh.SetGear(2);
					//veh.EngineStop();
	                
					//veh.BaseVehicleControllerComponent.SetEngineStartupChance
	            }
	        }	
	    }
	    
	    // Now EntitiesALL contains all the entities retrieved from SCR_EditableEntityCore
	}
	
	void SetVehiclesBroke() 
	{
	    array<IEntity> EntitiesALL = {};
	    
	    // Get the instance of SCR_EditableEntityCore
	    SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
	    if (!core) {
	        Print("Failed to get SCR_EditableEntityCore instance!", LogLevel.ERROR);
	        return;
	    }
	    
	    // Retrieve all editable entities
	    set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
	    core.GetAllEntities(entities, false, true);
	    
	    // Iterate through the entities and add them to the array
	    foreach (SCR_EditableEntityComponent comp : entities) {
	        IEntity entity = comp.GetOwner();
	        if (entity) {
	            EntitiesALL.Insert(entity);
	            typename type = entity.Type();
	            // Try to get the BaseVehicleControllerComponent from the entity
	                BaseVehicleControllerComponent controller = BaseVehicleControllerComponent.Cast(entity.FindComponent(BaseVehicleControllerComponent));
	                if (controller) {
	                    // Set the engine startup chance to 0
	                    controller.SetEngineStartupChance(0);
	                }
	            }
	        }	
	    	    
	}
	
	void SetVehiclesUnBroke() 
	{
	    array<IEntity> EntitiesALL = {};
	    
	    // Get the instance of SCR_EditableEntityCore
	    SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
	    if (!core) {
	        Print("Failed to get SCR_EditableEntityCore instance!", LogLevel.ERROR);
	        return;
	    }
	    
	    // Retrieve all editable entities
	    set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
	    core.GetAllEntities(entities, false, true);
	    
	    // Iterate through the entities and add them to the array
	    foreach (SCR_EditableEntityComponent comp : entities) {
	        IEntity entity = comp.GetOwner();
	        if (entity) {
	            EntitiesALL.Insert(entity);
	            typename type = entity.Type();
	            // Try to get the BaseVehicleControllerComponent from the entity
	                BaseVehicleControllerComponent controller = BaseVehicleControllerComponent.Cast(entity.FindComponent(BaseVehicleControllerComponent));
	                if (controller) {
	                    // Set the engine startup chance to 0
	                    controller.SetEngineStartupChance(100);
	                }
	            }
	        }	
	    	    
	}
    

    // Get the current local time in seconds
    int GetLocalTimeInSeconds()
    {
        string localTime = SCR_DateTimeHelper.GetTimeLocal(); // Format: "hh:ii:ss"
        int hour = localTime.Substring(0, 2).ToInt();
        int minute = localTime.Substring(3, 2).ToInt();
        int second = localTime.Substring(6, 2).ToInt();
        return SCR_DateTimeHelper.GetSecondsFromHourMinuteSecond(hour, minute, second);
    }


    // Check if the terrain surface is below the water surface
    static bool SurfaceIsWater(vector pos)
    {
        pos[1] = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
        vector outWaterSurfacePoint;
        EWaterSurfaceType outType;
        vector transformWS[4];
        vector obbExtents;
        ChimeraWorldUtils.TryGetWaterSurface(GetGame().GetWorld(), pos, outWaterSurfacePoint, outType, transformWS, obbExtents);
        return outType != EWaterSurfaceType.WST_NONE;
    }

    // Check if the surface is terrain
    static bool SurfaceIsTerrain(vector pos)
    {
        ResourceName material = GetSurfaceMaterial(pos);
        return material.IndexOf("Terrains") >= 0;
    }

    // Get the terrain slope in degrees for the given position
    static float GetSlope(vector pos)
    {
        vector normal = SCR_TerrainHelper.GetTerrainNormal(pos);
        return Math.RAD2DEG * Math.Acos(vector.Dot(Vector(0, 1, 0), normal));
    }

    // Check if the terrain slope at the given position is below the specified angle
    static bool SurfaceIsFlat(vector pos, float maxAngle = MAX_SLOPE_ANGLE)
    {
        return GetSlope(pos) < maxAngle;
    }

    // Get the surface material at the given position
    static ResourceName GetSurfaceMaterial(vector pos)
    {
        pos[1] = SCR_TerrainHelper.GetTerrainY(pos);
        TraceParam params = new TraceParam();
        params.Flags = TraceFlags.WORLD;
        params.Start = pos + 0.01 * vector.Up;
        params.End = pos - 0.01 * vector.Up;
        GetGame().GetWorld().TraceMove(params, null);
        return params.TraceMaterial;
    }

    // Calculate the distance between two positions
    float CalculateDistance(vector pos1, vector pos2)
    {
        vector diff = pos2 - pos1;
        return Math.Sqrt(Math.Pow(diff[0], 2) + Math.Pow(diff[1], 2) + Math.Pow(diff[2], 2));
    }

	// Generate a random spawn point for the defend side
	vector GenerateRandomSpawnPointDefendSide()
    {
		Math.Randomize(-1);
        array<vector> basePositions = {};
        for (int i = 0; i <= 87; i++)
        {
            string baseName = "Base_" + i;
            IEntity baseEntity = GetGame().GetWorld().FindEntityByName(baseName);
            if (baseEntity)
                basePositions.Insert(baseEntity.GetOrigin());
        }

        if (basePositions.IsEmpty())
        {
            Print("No valid Base entities found. Returning default position.");
			GenerateRandomSpawnPointDefendSide();
            //return Vector(0, 0, 0);
        }

        //int totalSeconds = GetLocalTimeInSeconds();
		//Print("totalSeconds "+totalSeconds);
        Math.Randomize(-1);
        int randomIndex = Math.RandomInt(0, basePositions.Count() - 1);
        return basePositions[randomIndex];
    }
	
		// Exclude islands
	static bool IsPositionInRectangle(vector position, vector corner1, vector corner2)
	{
	    float minX = Math.Min(corner1[0], corner2[0]);
	    float maxX = Math.Max(corner1[0], corner2[0]);
	    float minZ = Math.Min(corner1[2], corner2[2]);
	    float maxZ = Math.Max(corner1[2], corner2[2]);
	
	    return (position[0] >= minX && position[0] <= maxX &&
	            position[2] >= minZ && position[2] <= maxZ);
	}

	vector GenerateRandomSpawnPointAttackSide(vector defendPosition)
	{
	    int totalSeconds = GetLocalTimeInSeconds();
	    Math.Randomize(totalSeconds);
	
	    vector randomPosition;
	    float distance;
	    int attempts = 0;
	
	    // Disable ISLANDS SPAWN
	    array<vector> excludedRectangles = {
	        {10893.14, 0, 2345.53}, {14422.48, 0, 3723.44},  // Rect1
    		{1988.68, 0, 1018.30}, {3575.57, 0, 2121.94}   // Rect2			
	    };
	
	    while (attempts < MAX_ATTEMPTS_ATTACK_POSITION)
	    {
	        randomPosition[0] = Math.RandomFloat(MIN_X_COORDINATE, MAX_X_COORDINATE);
	        randomPosition[2] = Math.RandomFloat(MIN_Z_COORDINATE, MAX_Z_COORDINATE);
	        randomPosition[1] = GetGame().GetWorld().GetSurfaceY(randomPosition[0], randomPosition[2]);
	
	        distance = CalculateDistance(defendPosition, randomPosition);
	
	        // Check if the position is within any of the excluded rectangles
	        bool isExcluded = false;
	        for (int i = 0; i < excludedRectangles.Count(); i += 2)
	        {
	            if (IsPositionInRectangle(randomPosition, excludedRectangles[i], excludedRectangles[i + 1]))
	            {
	                isExcluded = true;
	                break;
	            }
	        }
	
	        if (!isExcluded &&
	            distance >= MIN_DISTANCE_ATTACK_DEFEND && distance <= MAX_DISTANCE_ATTACK_DEFEND &&
	            !SurfaceIsWater(randomPosition) && SurfaceIsFlat(randomPosition) &&
				 (PrintEntitiesInSphere(randomPosition, 5)==0))
	        {
	            break;
	        }
	
	        attempts++;
	    }
	
	    if (attempts >= MAX_ATTEMPTS_ATTACK_POSITION)
	    {
	        Print("Failed to find a valid attack position.");
	        return Vector(0, 0, 0);
	    }
	
	    return randomPosition;
	}

	vector RandomizeNextPosition(vector lastPosition, array<vector> existingPositions, array<vector> defendPositions, bool isDefender = false)
    {
        // Define search parameters


        // Start searching from the lastPosition and expand outward
        for (int radius = 0; radius <= MAX_SEARCH_RADIUS; radius += SEARCH_STEP)
        {
            // Iterate over a square grid around the lastPosition
            for (float offsetX = -radius; offsetX <= radius; offsetX += SEARCH_STEP)
            {
                for (float offsetZ = -radius; offsetZ <= radius; offsetZ += SEARCH_STEP)
                {
                    // Skip the center position (already checked or invalid)
                    if (offsetX == 0 && offsetZ == 0)
                        continue;

                    // Calculate the candidate position
                    vector candidatePosition = lastPosition + Vector(offsetX, 0, offsetZ);
                    candidatePosition[1] = GetGame().GetWorld().GetSurfaceY(candidatePosition[0], candidatePosition[2]);

                    // Check if the candidate position is valid
                    bool isValid = true;

                    // Check if the candidate position is too close to any existing position
                    foreach (vector existingPosition : existingPositions)
                    {
                        if (CalculateDistance(candidatePosition, existingPosition) < MIN_DISTANCE_BETWEEN_ENTITIES)
                        {
                            isValid = false;
                            break;
                        }
                    }

                    // If this is a defender entity, ensure it maintains MIN_DISTANCE_BETWEEN_ENTITIES with other defenders
                    if (isDefender)
                    {
                        foreach (vector defendPosition : defendPositions)
                        {
                            if (CalculateDistance(candidatePosition, defendPosition) < MIN_DISTANCE_BETWEEN_ENTITIES)
                            {
                                isValid = false;
                                break;
                            }
                        }
                    }
                    else
                    {
                        // For attack entities, ensure they maintain MIN_DISTANCE_ATTACK_DEFEND from defenders
                        foreach (vector defendPosition : defendPositions)
                        {
                            if (CalculateDistance(candidatePosition, defendPosition) < MIN_DISTANCE_ATTACK_DEFEND)
                            {
                                isValid = false;
                                break;
                            }
                        }
                    }

                    // Additional checks for surface conditions
                    //Print(string.Format("Candidate before snap Position: %1", candidatePosition.ToString()));
                    // Snap candidatePosition to the ground
                    // candidatePosition[1] = GetGame().GetWorld().GetSurfaceY(candidatePosition[0], candidatePosition[2]);
                    //Print(string.Format("Candidate after snap Position: %1", candidatePosition.ToString()));
                    if (isValid &&
                        !SurfaceIsWater(candidatePosition) &&
                        SurfaceIsFlat(candidatePosition) &&
                        SCR_WorldTools.TraceCylinder(candidatePosition, 15, 15) &&
						(PrintEntitiesInSphere(candidatePosition, 5)==0)
						//SCR_WorldTools.FindEmptyTerrainPosition(
						//SCR_CampaignBuildingPlacingObstructionEditorComponent.TraceEntityOnPosition(candidatePosition,null,15)
					)
                    {
                        return candidatePosition; // Return the valid position
                    }
                }
            }
        }

        // No valid position found within the search radius
        Print("Failed to find a valid position within the search radius. Retrying...");
		//GameStateTransitions.RequestScenarioRestart();
        // Retry mechanism
        static int retryCount = 0;
        if (retryCount < MAX_RETRIES)
        {
            retryCount++;
            Print(string.Format("Retry attempt: %1", retryCount));
            return RandomizeNextPosition(lastPosition, existingPositions, defendPositions, isDefender);
        }
        else
        {
            retryCount = 0;
            Print("Max retries reached. Restarting mission.");
			//GameStateTransitions.RequestScenarioRestart();
			GetGame().RequestClose();
			return vector.Zero;
        }
		
    }
	

    // Teleport groups of entities to randomized positions
    void TeleportGroups()
    {
		// Store default positions before any transformations
        //StoreDefaultPositions();
		//PS_GameModeCoop.GetFreezeTime();
		
		//SetVehiclesBrake();//no rolling 
		//SetVehiclesBroke();//no starter
		//GetGame().GetCallqueue().CallLater(UnfreezeCars,5000,true);//starter HEALTHY after freeze		
		changeStartingDistanceOnPlayerCount();
		GetGame().GetCallqueue().CallLater(MCU_Trcuks_Trigger,10000,true);//Check URAL KSHM's
		
        vector defendPosition = GenerateRandomSpawnPointDefendSide();
        vector attackPosition = GenerateRandomSpawnPointAttackSide(defendPosition);
		
		//IEntity Trigger0 = GetGame().GetWorld().FindEntityByName("Trigger_Dominance");
		//Trigger0.SetOrigin(defendPosition);
		//Trigger0.SetTransform(defendPosition);

        array<vector> defPositions = {};
        array<vector> groupPositions = {};

        // Teleport Defend entities
        for (int i = 0; i <= NUM_DEFEND_ENTITIES; i++)
        {
            string defName = "Def_" + i;
            IEntity defEntity = GetGame().GetWorld().FindEntityByName(defName);
            if (defEntity)
            {
                vector defPosition = RandomizeNextPosition(defendPosition, defPositions, defPositions, true); // isDefender = true
                if (defPosition != vector.Zero)
                {
                    defPositions.Insert(defPosition);
                    vector transform[4] = { "1 0 0", "0 1 0", "0 0 1", defPosition };
                    SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(defEntity.FindComponent(SCR_EditableEntityComponent));
                    if (editableComponent)
						//Print(PrintEntitiesInSphere(transform[3],5));	
						//DeleteEntitiesInSphere(transform[3], 5);
						//PrintEntitiesInSphere(transform[3], 5);					
						//DeleteEntitiesInCylinder(transform[3],5,15);
                        editableComponent.SetTransformWithChildren(transform);
						
                }
            }
        }

        // Teleport Attack entities
        for (int i = 0; i <= NUM_ATTACK_ENTITIES; i++)
        {
            string groupName = "Group_" + i;
            IEntity groupEntity = GetGame().GetWorld().FindEntityByName(groupName);
            if (groupEntity)
            {
                vector groupPosition = RandomizeNextPosition(attackPosition, groupPositions, defPositions); // isDefender = false
                if (groupPosition != vector.Zero)
                {
                    groupPositions.Insert(groupPosition);
                    vector transform[4] = { "1 0 0", "0 1 0", "0 0 1", groupPosition };
                    SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(groupEntity.FindComponent(SCR_EditableEntityComponent));
                    if (editableComponent)			
						//Print(PrintEntitiesInSphere(transform[3],5));			
						//DeleteEntitiesInSphere(transform[3], 5);
						//PrintEntitiesInSphere(transform[3], 5);
						//DeleteEntitiesInCylinder(transform[3],5,15);
                        editableComponent.SetTransformWithChildren(transform);
						 
                }
            }
        }
    }
};