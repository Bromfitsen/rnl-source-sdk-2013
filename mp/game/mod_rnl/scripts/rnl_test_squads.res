"SquadInfo"
{
	//********************************************************
	//
	// Allied Squad and Class Definition
	//
	//********************************************************
	"Allies"
	{
		"kits"
		{
			"Rifleman"
			{
				"title"		"#RnL_Allied_Rifle"
				"type"		"infantry"
				"model"
				{
					"file"		"models/player/pir/505_rifleman01.mdl"
					"skin"		"0"
				}
				"weapons"
				{
					"primary"	"garand"
					"secondary"	"colt"
					"grenade2"	"mk2grenade"
					"tnt"		"alliedexplosives"
				}
			}

		}

		"squads"
		{
			"squad1fireteam1"
			{
				"name"	"#RnL_Allied_Squad1_Fireteam1"
				"slots"
				{
					"Rifleman"	"2"
				}
			}
				
			"squad1fireteam2"
			{
				"name"	"#RnL_Allied_Squad1_Fireteam2"
				"slots"
				{
					"Rifleman"		"3"
				}
	    	}
	    	
			"squad2fireteam1"
			{
				"name"	"#RnL_Allied_Squad2_Fireteam1"
				"slots"
				{
					"Rifleman"	"4"
				}
			}
				
			"squad2fireteam2"
			{
				"name"	"#RnL_Allied_Squad2_Fireteam2"
				"slots"
				{
					"Rifleman"		"5"
				}
			}
		}
	}
	
	//********************************************************
	//
	// Axis Squad and Class Definition
	//
	//********************************************************
	"Axis"
	{
		"kits"
		{
			"Officer"
			{
				"title"		"#RnL_Axis_NCO"
				"type"		"infantry"
				"model"
				{
					"file"		"models/player/heer/heer_nco01.mdl"
					"skin"		"0"
				}
				"weapons"
				{
					"primary"	"mp40"
					"secondary"	"walther"
					"grenade"	"sticky39grenade"
					"grenade2"	"sticky24grenade"
				}
			}
			
			"Rifleman"
			{
				"title"		"#RnL_Axis_Rifle"
				"type"		"infantry"
				"model"
				{
					"file"		"models/player/heer/heer_rifleman01.mdl"
					"skin"		"0"
				}
				"weapons"
				{
					"primary"	"k98k"
					"secondary"	"walther"
					"grenade"	"sticky24grenade"
				}
			}
			
			"Radioman"
			{
				"title"		"#RnL_Axis_Radio"
				"type"		"support"
				"model"
				{
					"file"		"models/player/heer/heer_rifleman01.mdl"
					"skin"		"0"
				}
				"weapons"
				{
					"primary"	"gewehr43"
					"secondary"	"walther"
					"grenade"	"sticky24grenade"
				}
			}
			
			"Machinegunner"
			{
				"title"		"#RnL_Axis_MG"
				"type"		"support"
				"model"
				{
					"file"		"models/player/heer/heer_rifleman01.mdl"
					"skin"		"0"
				}
				"weapons"
				{
					"primary"	"mg42"
					"secondary"	"walther"
					"grenade"	"sticky24grenade"
				}
			}
		}
		"squads"
		{
			"squad1"
			{
	        	"name"		"#RnL_Axis_Squad1"
	        	"slots"
				{
					"Officer"		"1"
					"Machinegunner"	"1"
					"Radioman"		"1"
					"Rifleman"		"5"
				}
	    	}

			"squad2"
			{
				"name"		"#RnL_Axis_Squad2"
				"slots"
				{
					"Officer"		"1"
					"Machinegunner"	"1"
					"Radioman"		"1"
					"Rifleman"		"5"
				}
	    	}
		}
	}
}