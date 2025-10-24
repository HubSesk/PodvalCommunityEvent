[BaseContainerProps()]
class GUB_ZoneControlCondition : GUB_ZoneControlConditionAbstract
{
	[Attribute(defvalue: "USSR", desc: "Faction Key")]
	FactionKey m_sFactionKey;

	[Attribute("0", UIWidgets.ComboBox, "Compare operator (=,<,>,<=,>=)", "", ParamEnumArray.FromEnum(DRG_ConditionOp))]
	int m_iOperator;

	[Attribute(defvalue: "0", desc: "Required count", uiwidget: UIWidgets.EditBox)]
	int m_iCount;

	override bool Evaluate(map<string, int> FactionCounts)
	{
		int FactionCount = GetCountForFaction(FactionCounts, m_sFactionKey);

		switch (m_iOperator)
		{
			case DRG_ConditionOp.EQUALS:            return FactionCount == m_iCount;
			case DRG_ConditionOp.LESS:              return FactionCount <  m_iCount;
			case DRG_ConditionOp.GREATER:           return FactionCount >  m_iCount;
			case DRG_ConditionOp.LESS_OR_EQUALS:    return FactionCount <= m_iCount;
			case DRG_ConditionOp.GREATER_OR_EQUALS: return FactionCount >= m_iCount;
		}
		return false;
	}

	override string FillDescription()
	{
		FactionManager factionManager = GetGame().GetFactionManager();				
		Faction faction = factionManager.GetFactionByKey(m_sFactionKey);
		
		string factionName = faction.GetFactionName();
						
		string operatorStr;			
		if (m_iOperator == DRG_ConditionOp.EQUALS)
		{
			operatorStr = "==";				
		} else if (m_iOperator == DRG_ConditionOp.LESS){
			operatorStr = "<";				
		} else if (m_iOperator == DRG_ConditionOp.GREATER){
			operatorStr = ">";
		} else if (m_iOperator == DRG_ConditionOp.LESS_OR_EQUALS){
			operatorStr = "<=";
		} else if (m_iOperator == DRG_ConditionOp.GREATER_OR_EQUALS){
			operatorStr = ">=";
		}
				
		string desc =" • " + factionName + " " + "<color hex=\"0xFFE2A74F\">" + operatorStr + "<color name>" + " " + m_iCount.ToString() + "\n";
		return desc;
	}
}