class DE_CashComponentClass : ScriptComponentClass
{}

class DE_CashComponent : ScriptComponent
{
	[RplProp(), Attribute("0", UIWidgets.Auto, desc: "Cash value contained in stack", category: "Dynamic Economy")]
	float value;
}