"campaign"
{
	"section1"
	{
		"title"		"#RnL_Campaign_Default"
		"map"		"rnl_eglise"
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
		"map"		"rnl_stcomedumont"
		"script"	"rnl_default_squads"
		"timelimit" "30"
		"win"
		{
			"allies"	"section3"
			"axis"		"section1"
			"draw"		"section2"
		}
	}
	"section3"
	{
		"title"		"#RnL_Campaign_Default"
		"map"		"rnl_stemariedumont"
		"script"	"rnl_default_squads"
		"timelimit" "30"
		"win"
		{
			"allies"	"section4"
			"axis"		"section1"
			"draw"		"section3"
		}
	}
	"section4"
	{
		"title"		"#RnL_Campaign_Default"
		"map"		"rnl_lafiere"
		"script"	"rnl_default_squads"
		"timelimit" "30"
		"win"
		{
			"allies"	"section1"
			"axis"		"section1"
			"draw"		"section4"
		}
	}
}