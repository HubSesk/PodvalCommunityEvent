[BaseContainerProps()]
class GUB_ZoneControlConditionAbstract
{
	bool Evaluate(map<string, int> FactionCounts) {return false; }
	string FillDescription();

	protected int GetCountForFaction(map<string, int> FactionCounts, FactionKey key)
	{
		string skey = key;
		if (skey == "")
			return 0;

		int v = FactionCounts.Get(skey);
		return v;
	}
}