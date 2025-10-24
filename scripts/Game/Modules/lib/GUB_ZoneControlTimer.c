class GUB_ZoneControlTimer
{
	protected ref array<ref GUB_ZoneControlConditionAbstract> conditions;
	protected ref map<string, int> factionCounts;
	protected float timeToComplete;
	protected bool continuously;
	protected float timeNow = 0;

    void SetParams(array<ref GUB_ZoneControlConditionAbstract> Conditions, 
		map<string, int> FactionCounts, float TimeToComplete, bool Continuously)
		{
			conditions = Conditions;
			factionCounts = FactionCounts;
			timeToComplete = TimeToComplete;
			continuously = Continuously;
		}

	void ResetTimeIfContinuously() 
	{
		if (continuously)
			timeNow = 0; 
	}
	bool AddTime(float time)
	{
		timeNow += time;
		if (timeNow >= timeToComplete)
			return true;
		return false;
	}
	bool Check(float TimePassed)
	{
		if (!conditions || conditions.Count() == 0)
			return false;

		for (int i = 0; i < conditions.Count(); i++)
		{
			GUB_ZoneControlConditionAbstract cond = conditions[i];
			if (!cond)
			{
				ResetTimeIfContinuously();
				return false;
			}

			bool ok = cond.Evaluate(factionCounts);
			if (!ok)
			{
				ResetTimeIfContinuously();
				return false;
			}
		}

		return AddTime(TimePassed);
	}
	float GetCapturePercentage()
	{
		return timeNow / timeToComplete;
	}
}