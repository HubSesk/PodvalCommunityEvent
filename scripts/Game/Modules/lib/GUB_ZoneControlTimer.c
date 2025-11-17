[BaseContainerProps()]
class GUB_ZoneControlTimer
{
	[Attribute(defvalue: "60.0", desc: "How much time is needed for the condition to be fully met (sec)")]
	protected float timeToComplete;

	protected float timeNow = 0;

	bool AddTime(float time)
	{
		timeNow += time;
		if (timeNow >= timeToComplete)
			return true;
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