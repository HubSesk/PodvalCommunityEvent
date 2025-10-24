[BaseContainerProps()]
class GUB_ZoneControlBetweenFactionsCondition : GUB_ZoneControlConditionAbstract
{
	[Attribute(defvalue: "USSR", desc: "Faction Key")]
	FactionKey m_sFirstFactionKey;

	[Attribute(defvalue: "US", desc: "Other Faction Key to compare with")]
	FactionKey m_sSecondFactionKey;

	[Attribute("1.0", UIWidgets.Slider, "Comparison multiplier (e.g., 5.0 means 5 times more)", "0.1 10 0.1")]
	float m_fComparisonMultiplier;

	[Attribute("0", UIWidgets.ComboBox, "Compare operator (=,<,>,<=,>=)", "", ParamEnumArray.FromEnum(DRG_ConditionOp))]
	int m_iOperator;

	override bool Evaluate(map<string, int> FactionCounts)
	{
		// Применяем коэффициент к количеству другой фракции
		int firstFactionCount = GetCountForFaction(FactionCounts, m_sFirstFactionKey);
		int secondFactionCount = GetCountForFaction(FactionCounts, m_sSecondFactionKey);
		
		float adjustedSecondFactionCount = secondFactionCount * m_fComparisonMultiplier;
		
		switch (m_iOperator)
		{
			case DRG_ConditionOp.EQUALS:            return firstFactionCount == adjustedSecondFactionCount;
			case DRG_ConditionOp.LESS:              return firstFactionCount <  adjustedSecondFactionCount;
			case DRG_ConditionOp.GREATER:           return firstFactionCount >  adjustedSecondFactionCount;
			case DRG_ConditionOp.LESS_OR_EQUALS:    return firstFactionCount <= adjustedSecondFactionCount;
			case DRG_ConditionOp.GREATER_OR_EQUALS: return firstFactionCount >= adjustedSecondFactionCount;
		}
		return false;
	}

	override string FillDescription()
	{
		FactionManager factionManager = GetGame().GetFactionManager();				
		Faction firstFaction = factionManager.GetFactionByKey(m_sFirstFactionKey);
		Faction secondFaction = factionManager.GetFactionByKey(m_sSecondFactionKey);
		
		string factionName = firstFaction.GetFactionName();
		string otherFactionName = secondFaction.GetFactionName();
						
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
		
		string multiplierText = "";
		if (m_fComparisonMultiplier != 1.0)
		{
			multiplierText = " (x" + m_fComparisonMultiplier.ToString() + ")";
		}
				
		string desc = desc + " • " + factionName + " " + "<color hex=\"0xFFE2A74F\">" + operatorStr + "<color name>" + " " + otherFactionName + multiplierText + "\n";
		return desc;
	}
}