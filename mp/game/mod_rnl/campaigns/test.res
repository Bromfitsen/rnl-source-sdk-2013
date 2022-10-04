"campaign"
{
	"section1"
	{
		"title"		"#RnL_Campaign_Default"
		"map"		"rnl_street"
		"script"	"rnl_default_squads"
		"timelimit" "45"
		"win"
		{
			"allies"	"section2"
			"axis"		"section1"
			"draw"		"section1"
		}
	}
	"section2"
	{
		"title"		"#RnL_Campaign_Default"
		"map"		"rnl_field"
		"script"	"rnl_test_squads"
		"timelimit" "30"
		"win"
		{
			"allies"	"section1"
			"axis"		"section1"
			"draw"		"section2"
		}
	}
}