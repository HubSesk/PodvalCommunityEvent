[BaseContainerProps()]
class GUB_ZoneController
{
	[Attribute(defvalue: "", desc: "Entity name of zone (ShapeEntity) that has PS_PolyZone component")]
	string m_sZoneEntityName;

	[Attribute("1", UIWidgets.CheckBox, "Count alive only (DEAD/DESTROYED excluded)", "")]
	bool m_bAliveOnly;
	[Attribute("0", UIWidgets.CheckBox, "Debug logging (print counts each tick)", "")]
	bool m_bDebug;

    protected IEntity m_ZoneEntity;                 // сущность зоны (ShapeEntity / PolylineShapeEntity)
	protected PS_PolyZone m_ZoneComp;               // компонент полигона
	protected BaseGameTriggerEntity m_Trigger;      // вспомогательный сферический триггер
	protected bool m_bQueryInFlight;

    protected ref map<string, int> m_mFactionCounts; // счётчики по фракциям

	// Префаб сферического триггера (как в Seizing)
	protected const ResourceName RN_SPHERE_TRIGGER = "{59A6F1EBC6C64F79}Prefabs/Logic/SeizingTrigger.et";

    void Init(map<string, int> FactionCounts)
    {
		m_mFactionCounts = FactionCounts;
    	m_bQueryInFlight = false;

        ResolveZoneAndPrepare();
    }

    // Поиск зоны и подготовка триггера
	protected void ResolveZoneAndPrepare()
	{
		if (m_ZoneComp && m_Trigger)
			return;

		if (m_sZoneEntityName == "")
		{
			Debug.Error("GUB_ZoneController: m_sZoneEntityName is Null");
			return;
		}

		IEntity ent = GetGame().GetWorld().FindEntityByName(m_sZoneEntityName);
		if (!ent)
		{
			if (m_bDebug)
				Print(string.Format("GUB_ZoneControlBetweenFactions: zone entity '%1' not found (retry)", m_sZoneEntityName));
			return;
		}

		m_ZoneEntity = ent;
		m_ZoneComp = PS_PolyZone.Cast(ent.FindComponent(PS_PolyZone));
		if (!m_ZoneComp && ent.GetParent())
			m_ZoneComp = PS_PolyZone.Cast(ent.GetParent().FindComponent(PS_PolyZone));

		if (!m_ZoneComp)
		{
			if (m_bDebug)
				Print(string.Format("GUB_ZoneControlBetweenFactions: PS_PolyZone not found on '%1' or parent (retry)", m_sZoneEntityName));
			return;
		}

		Resource res = Resource.Load(RN_SPHERE_TRIGGER);
		if (!res)
		{
			Print("GUB_ZoneControlBetweenFactions: failed to load SeizingTrigger prefab", LogLevel.ERROR);
			return;
		}

		m_Trigger = BaseGameTriggerEntity.Cast(GetGame().SpawnEntityPrefabLocal(res, GetGame().GetWorld()));
		if (!m_Trigger)
		{
			Print("GUB_ZoneControlBetweenFactions: failed to spawn trigger", LogLevel.ERROR);
			return;
		}

		// радиус вычисляем автоматически по точкам шейпа
		float autoR = ComputeAutoRadius();
		m_Trigger.SetSphereRadius(autoR);

		// центрируем триггер на зоне
		m_ZoneEntity.AddChild(m_Trigger, -1);
	}

    // Вычисляем радиус сферы, чтобы она накрывала полигон (по XZ)
	protected float ComputeAutoRadius()
	{
		ShapeEntity shape = ShapeEntity.Cast(m_ZoneEntity);
		if (!shape)
			return 150.0;

		array<vector> pts = {};
		shape.GetPointsPositions(pts);
		if (pts.IsEmpty())
			return 150.0;

		vector origin = m_ZoneEntity.GetOrigin();
		float r = 0.0;

		for (int i = 0; i < pts.Count(); i++)
		{
			vector pw = pts[i] + origin;
			float dx = pw[0] - origin[0];
			float dz = pw[2] - origin[2];
			float d  = Math.Sqrt(dx * dx + dz * dz);

			if (d > r)
				r = d;
		}

		r = r * 1.10;          // запас 10%
		if (r < 5.0) r = 5.0;  // минимальный радиус

		return r;
	}

    // Один шаг опроса
	void EvaluateOnce()
	{
		if (!m_ZoneComp || !m_Trigger)
			ResolveZoneAndPrepare();

		m_Trigger.GetOnQueryFinished().Insert(OnTriggerQueryFinished);
		m_bQueryInFlight = true;
		m_Trigger.QueryEntitiesInside();
	}

    // Обработка результата триггера
	protected void OnTriggerQueryFinished(BaseGameTriggerEntity trigger)
	{
		m_bQueryInFlight = false;
		m_Trigger.GetOnQueryFinished().Remove(OnTriggerQueryFinished);

		m_mFactionCounts.Clear();

		array<IEntity> inside = {};
		int cnt = m_Trigger.GetEntitiesInside(inside);

		for (int i = 0; i < cnt; i++)
		{
			IEntity e = inside[i];

			// считаем только SCR_ChimeraCharacter (игроки + ИИ пехота)
			SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(e);
			if (!ch)
				continue;

			// жив?
			if (m_bAliveOnly)
			{
				CharacterControllerComponent ctrl = ch.GetCharacterController();
				if (!ctrl)
					continue;

				if (ctrl.GetLifeState() != ECharacterLifeState.ALIVE)
					continue;
			}

			// действительно в ПОЛИГОНЕ
			vector pos = ch.GetOrigin();
			if (!m_ZoneComp.IsInsidePolygon(pos))
				continue;

			// фракция
			string fkey = ch.GetFactionKey();
			if (fkey == "")
				continue;

			int cur = m_mFactionCounts.Get(fkey);
			if (cur == 0)
				m_mFactionCounts.Insert(fkey, 1);
			else
				m_mFactionCounts.Set(fkey, cur + 1);
		}

		// Отладка — перебор map по индексам
		if (m_bDebug)
		{
			int totalKeys = m_mFactionCounts.Count();
			for (int k = 0; k < totalKeys; k++)
			{
				string keyStr = m_mFactionCounts.GetKey(k);
				int    val    = m_mFactionCounts.GetElement(k);
				Print(string.Format("[GUB_ZoneControlBetweenFactions] Zone=%1 Faction=%2 Count=%3", m_sZoneEntityName, keyStr, val));
			}
		}
	}

    void ~GUB_ZoneController()
    {
        if (m_Trigger)
		{
			delete m_Trigger;
			m_Trigger = null;
		}
    }
}