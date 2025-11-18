[BaseContainerProps()]
class GUB_ZoneControlLogicAbstract
{
	void Init(PS_GameModeCoop mode)
    {
        Debug.Error("Don't use abstract class!");
    }

    string FillDescription()
    {
        return "";
    };

    string FormatTime(int seconds) {
		// Проверка на отрицательное время
		if (seconds < 0) {
			return "00:00:00";
		}
		
		int hours = seconds / 3600;
		int minutes = (seconds % 3600) / 60;
		int secs = seconds % 60;
		
		string result = string.Format("%1:%2:%3", hours, minutes, secs);
		return result;
	}
}