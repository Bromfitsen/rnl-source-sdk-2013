// rnl NCO Map description file for rnl_dieppe
"mapvalues"
{
	"alliesinfo"
	{
		"material"	"overview/rnl_dieppe_allies"	// texture file
		"offset_x"	"-30"
		"offset_y"	"-10"
		"scale"		"1.15"
		"rotate"	"90"
	}
	"axisinfo"
	{
		"material"	"overview/rnl_dieppe_axis"	// texture file
		"offset_x"	"0"
		"offset_y"	"0"
		"scale"		"1"
		"rotate"	"90"
	}
	"tasks"
	{
		"task_capture_hotel"
		{
			"displayname"	"Hotel"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"task_capture_church"
		{
			"displayname"	"Church"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			
			"area"
			{
				"vert" "32.0 -352.0 0.0"
				"vert" "32.0 -128.0 0.0"
				"vert" "-136.0 -128.0 0.0"
				"vert" "-136.0 128.0 0.0"
				"vert" "32.0 128.0 0.0"
				"vert" "32.0 352.0 0.0"
				"vert" "880.0 352.0 0.0"
				"vert" "880.0 616.0 0.0"
				"vert" "1136.0 	316.0 0.0"
				"vert" "1136.0 	352.0 0.0"
				"vert" "1376.0 	352.0 0.0"
				"vert" "1376.0 	-352.0 0.0"
				"vert" "1136.0 	-352.0 0.0"
				"vert" "1136.0 -616.0 0.0"
				"vert" "880.0 -616.0 0.0"
				"vert" "880.0 -352.0 0.0"
			}
		}
		"task_destruction_01"
		{
			"displayname"		"Flak 1"
			"displayposition"	"871 4266 224"
		}
		"task_destruction_02"
		{
			"displayname"		"Flak 2"
			"displayposition"	"1406 4297 224"
		}
		"task_destruction_03"
		{
			"displayname"		"Bridge 1"
			"displayposition"	"-6819 958 224"
		}
		"task_destruction_04"
		{
			"displayname"		"Bridge 2"
			"displayposition"	"-6819 958 224"
		}
		"task_destruction_05"
		{
			"displayname"		"Flak 3"
			"displayposition"	"181 -5458 224"
		}
		"task_destruction_06"
		{
			"displayname"		"Flak 4"
			"displayposition"	"3998 -3878 224"
		}
		"task_destruction_07"
		{
			"displayname"		"Bridge 3"
			"displayposition"	"-6496 -6116 224"
		}
		"task_destruction_08"
		{
			"displayname"		"Bridge 4"
			"displayposition"	"-6496 -6495 224"
		}
	}
	"objectives"
	{
		"objective_capture_hotel"
		{
			"displayname"		"Hotel"
			"displayposition"	"7488 1232 112"
		}
		"objective_capture_outpost"
		{
			"displayname"		"Church"
			"displayposition"	"-1024 0 -120"
		}
		"objective_destroy_88s"
		{
			"displayname"		"Flak fortifications"
			"displayposition"	"2240 -4896 -120"
		}
		"objective_bridge1"
		{
			"displayname"		"Bridge"
			"displayposition"	"-7256 800 -120"
		}
		"objective_bridge2"
		{
			"displayname"		"Railway Bridge"
			"displayposition"	"-7035 -6308 -120"
		}
	}
}