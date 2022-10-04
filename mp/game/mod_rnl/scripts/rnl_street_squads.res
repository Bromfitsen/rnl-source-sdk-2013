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
			"Officer"
			{
				"title"			"#RnL_Allied_NCO"
				"type"			"infantry"
				"squadleader"	"true"
				"extraammo"		"AMMO_BROWNING"
				"model"
				{
					"file"		"models/player/pir/505_nco01.mdl"
					"skin"		"0"
					
					"bodygroups"
					{
						"helmet" "2"
						"gear"	"1"
						"colt_holster" "1"
						"ammo_pouches" "5"
						"ammo_pouches_secondary" "1"
						"map"   "1"
						"frags" "2"
						"smokes" "2"
						"knife" "1"
						"upperbody" "0"
					}
				}
				
				"weapons"
				{
					"primary"	"thompson"
					"secondary"	"colt"
					"grenade1"	"m18grenade"
					"grenade2"	"mk2grenade"
				}
			}

			"Fireteam_Leader"
			{
				"title"			"#RnL_Allied_Fireteam_Leader"
				"type"			"infantry"
				"squadleader"	"true"
				"extraammo"		"AMMO_BROWNING"
				"model"
				{
					"file"		"models/player/pir/505_nco01.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"gear"		"2"
						"helmet"	"2"
						"colt_holster"	"1"
						"ammo_pouches"	"3"
						"ammo_pouches_secondary"	"1"
						"frags"		"2"
						"smokes"	"2"
						"knife"		"1"
						"upperbody"	"0"
					}
				}

				"weapons"
				{
					"primary"	"carbine"
					"secondary"	"colt"
					"grenade1"	"m18grenade"
					"grenade2"	"mk2grenade"
				}
			}

			"Rifleman"
			{
				"title"		"#RnL_Allied_Rifle"
				"type"		"infantry"
				"extraammo"	"AMMO_BROWNING"
				"model"
				{
					"file"		"models/player/pir/505_rifleman02.mdl"
					"skin"		"0"
					
					"bodygroups"
					{
						"helmet" "1"
						"gear"	"1"
						"colt_holster"	"1"
						"ammo_pouches" "1"
						"frags" "2"
						"knife" "1"
						"upperbody" "0"
					}
				}
				"weapons"
				{
					"primary"	"garand"
					"secondary"	"colt"
					"grenade2"	"mk2grenade"
				}
			}

			"Radioman"
			{
				"title"		"#RnL_Allied_Radio"
				"type"		"support"
				"extraammo"	"AMMO_BROWNING"
				"model"
				{
					"file"		"models/player/pir/505_rifleman02.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "2"
						"gear"	"2"
						"colt_holster"	"1"
						"ammo_pouches" "2"
						"ammo_pouches_secondary" "1"
						"frags" "2"
						"knife" "1"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"carbine"
					"secondary"	"colt"
					"grenade2"	"mk2grenade"
				}
			}

			"Rifleman-BAR"
			{
				"title"		"#RnL_Allied_Rifle_BAR"		
				"type"		"support"
				"extraammo"	"AMMO_BROWNING"
				"model"
				{
					"file"		"models/player/pir/505_rifleman02.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "2"
						"gear"	"1"
						"colt_holster"	"1"
						"ammo_pouches" "1"
						"frags" "2"
						"knife" "1"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"bar"
					"secondary"	"colt"
					"grenade2"	"mk2grenade"
				}
			}

			"Machinegunner"
			{
				"title"		"#RnL_Allied_MG"
				"type"		"support"
				"model"
				{
					"file"		"models/player/pir/505_rifleman02.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "2"
						"gear"	"1"
						"colt_holster"	"1"
						"ammo_pouches" "5"
						"ammo_pouches_secondary" "1"
						"frags" "2"
						"knife" "1"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"browning"
					"secondary"	"colt"
					"grenade2"	"mk2grenade"
				}
			}

			"Engineer"
			{
				"title"		"#RnL_Allied_Engineer"
				"type"		"support"
				"extraammo"	"AMMO_BROWNING"
				"model"
				{
					"file"		"models/player/pir/505_rifleman02.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "2"
						"gear"	"1"
						"colt_holster"	"1"
						"ammo_pouches" "3"
						"ammo_pouches_secondary" "1"
						"frags" "2"
						"knife" "1"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"carbine"
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
					"Officer"		"1"
					"Engineer"		"1"
					"Radioman"		"1"
					"Rifleman"		"1"
				}
			}

			"squad1fireteam2"
			{
				"name"	"#RnL_Allied_Squad1_Fireteam2"
				"slots"
				{
					"Fireteam_Leader"		"1"
					"Radioman"		"1"
					"Machinegunner" "1"
					"Rifleman-BAR"	"1"
				}
	    	}

			"squad2fireteam1"
			{
				"name"	"#RnL_Allied_Squad2_Fireteam1"
				"slots"
				{
					"Officer"	"1"
					"Engineer"	"1"
					"Radioman"	"1"
					"Rifleman"	"1"
				}
			}

			"squad2fireteam2"
			{
				"name"	"#RnL_Allied_Squad2_Fireteam2"
				"slots"
				{
					"Fireteam_Leader"		"1"
					"Rifleman"		"1"
					"Rifleman-BAR"	"1"
					"Machinegunner" "1"
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
				"title"			"#RnL_Axis_NCO"
				"type"			"infantry"
				"squadleader"	"true"
				"extraammo"		"AMMO_MG42"
				"model"
				{
					"file"		"models/player/heer/heer_nco01.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "4"
						"gear"	"2"
						"map"   "1"
						"p38_holster"	"1"
						"ammo_pouches" "1"
						"smokes" "2"
						"frags" "2"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"mp40"
					"secondary"	"walther"
					"grenade"	"sticky39grenade"
					"grenade2"	"sticky24grenade"
				}
			}

			"Squad_Leader"
			{
				"title"			"#RnL_Axis_Squad_Leader"
				"type"			"infantry"
				"squadleader"	"true"
				"extraammo"		"AMMO_MG42"
				"model"
				{
					"file"		"models/player/heer/heer_nco01.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet"	"4"
						"gear"		"2"
						"map"		"1"
						"p38_holster"	"1"
						"ammo_pouches"	"4"
						"smokes"	"2"
						"frags"		"2"
						"upperbody"	"0"
					}
				}

				"weapons"
				{
					"primary"	"gewehr43"
					"secondary"	"walther"
					"grenade"	"sticky39grenade"
					"grenade2"	"sticky24grenade"
				}
			}

			"Rifleman"
			{
				"title"		"#RnL_Axis_Rifle"
				"type"		"infantry"
				"extraammo"	"AMMO_MG42"
				"model"
				{
					"file"		"models/player/heer/heer_rifleman01.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "1"
						"gear"	"2"
						"p38_holster"	"2"
						"ammo_pouches" "3"
						"frags" "2"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"k98k"
					"secondary"	"walther"
					"grenade"	"sticky24grenade"
				}
			}

			"Rifleman-FG42"
			{
				"title"		"#RnL_Axis_Rifle_FG42"
				"type"		"infantry"
				"extraammo"	"AMMO_MG42"
				"model"
				{
					"file"		"models/player/heer/heer_rifleman01.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "4"
						"gear"	"4"
						"p38_holster"	"2"
						"ammo_pouches" "4"
						"frags" "2"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"fg42"
					"secondary"	"walther"
					"grenade"	"sticky24grenade"
				}
			}

			"Radioman"
			{
				"title"		"#RnL_Axis_Radio"
				"type"		"support"
				"extraammo"	"AMMO_MG42"
				"model"
				{
					"file"		"models/player/heer/heer_rifleman01.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "3"
						"gear"	"3"
						"p38_holster"	"2"
						"ammo_pouches" "3"
						"frags" "2"
						"upperbody" "0"
					}
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

					"bodygroups"
					{
						"helmet" "4"
						"gear"	"2"
						"p38_holster"	"2"
						"ammo_pouches" "3"
						"frags" "2"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"mg42"
					"secondary"	"walther"
					"grenade"	"sticky24grenade"
				}
			}

			"Engineer"
			{
				"title"		"#RnL_Axis_Engineer"
				"type"		"support"
				"extraammo"	"AMMO_MG42"
				"model"
				{
					"file"		"models/player/heer/heer_rifleman01.mdl"
					"skin"		"0"

					"bodygroups"
					{
						"helmet" "2"
						"gear"	"3"
						"p38_holster"	"2"
						"ammo_pouches" "3"
						"frags" "2"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"gewehr43"
					"secondary"	"walther"
					"grenade"	"sticky24grenade"
					"tnt"		"axisexplosives"
				}
			}

			"Sharpshooter"
			{
				"title"		"#RnL_Axis_Sharpshooter"
				"type"		"support"
				"extraammo"	"AMMO_MG42"
				"model"
				{
					"file"		"models/player/heer/heer_rifleman01.mdl"
					"skin"		"3"

					"bodygroups"
					{
						"helmet" "2"
						"gear"	"3"
						"p38_holster"	"1"
						"ammo_pouches" "4"
						"frags" "2"
						"upperbody" "0"
					}
				}

				"weapons"
				{
					"primary"	"k98scoped"
					"secondary"	"walther"
				}
			}
		}

		"squads"
		{
			"squad1fireteam1"
			{
	        	"name"		"#RnL_Axis_Squad1_Fireteam1"
	        	"slots"
				{
					"Officer"		"1"
					"Machinegunner"	"1"
					"Radioman"		"1"
					"Sharpshooter"	"1"
				}
			}

			"squad1fireteam2"
			{
				"name"		"#RnL_Axis_Squad1_Fireteam2"
				"slots"
				{
					"Squad_Leader"	"1"
					"Engineer"		"1"
					"Rifleman"		"2"
				}
	    	}

			"squad2fireteam1"
			{
				"name"		"#RnL_Axis_Squad2_Fireteam1"
				"slots"
				{
					"Officer"		"1"
					"Machinegunner"	"1"
					"Radioman"		"1"
					"Rifleman-FG42"	"1"
				}
			}

			"squad2fireteam2"
			{
				"name"		"#RnL_Axis_Squad2_Fireteam2"
				"slots"
				{
					"Squad_Leader"	"1"
					"Engineer"		"1"
					"Rifleman"		"2"
				}
	    	}
		}
	}
}