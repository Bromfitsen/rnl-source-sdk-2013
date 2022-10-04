"Resource/UI/ScoreBoard.res"
{
	"scores"
	{
		"ControlName"	"CClientScoreBoardDialog"
		"fieldName"		"scores"
		"xpos"			"30"
		"ypos"			"32"
		"wide"			"560" //444 
		"tall"			"400" //360 
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"0"
		"enabled"		"1"
		"tabPosition"	"0"
		"avatar_width"	"34"	[$WIN32]
		"avatar_width"	"0"		[$X360]
	}
	"ServerName"
	{
		"ControlName"	"Label"
		"fieldName"		"ServerName"
		"xpos"			"3"
		"ypos"			"2"
		"wide"			"250"
		"tall"			"24"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"labelText"		""
		"textAlignment"	"north-west"
		"dulltext"		"0"
		"brighttext"	"1"
	}
	"PlayerList"
	{
		"ControlName"	"SectionedListPanel"
		"fieldName"		"PlayerList"
		"xpos"			"0"
		"ypos"			"20"
		"wide"			"280"
		"tall"			"400"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"	"0"
		"linespacing"	"17"
	}
	"AxisPlayerList"
	{
		"ControlName"	"SectionedListPanel"
		"fieldName"		"AxisPlayerList"
		"xpos"			"275"
		"ypos"			"20"
		"wide"			"280"
		"tall"			"400"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"	"0"
		"linespacing"	"17"
	}
}
