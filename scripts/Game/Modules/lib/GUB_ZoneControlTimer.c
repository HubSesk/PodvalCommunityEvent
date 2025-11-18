[BaseContainerProps()]
class GUB_ZoneControlTimer
{
	[Attribute(defvalue: "60.0", desc: "How much time is needed for the condition to be fully met (sec)")]
	protected float timeToComplete;

	protected float timeNow = 0;

	// true, если пересек timeToComplete
	bool AddTime(float time)
	{
		timeNow += time;
		if (timeNow >= timeToComplete)
			return true;
		return false;
	}

	// true, если пересек 0
	bool MinusTime(float time)
	{
		timeNow -= time;
		if (timeNow <= 0)
		{
			timeNow = -1 * timeNow;
			return true;
		}
		return false;
	}

	float GetCapturePercentage()
	{
		return timeNow / timeToComplete;
	}

	float GetTimeToComplete()
	{
		return timeToComplete;
	}
}