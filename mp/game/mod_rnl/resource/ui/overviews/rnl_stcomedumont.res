// rnl NCO Map description file for rnl_stcomedumont
"mapvalues"
{
	"alliesinfo"
	{
		"material"	"overview/rnl_stcomedumont_allies"	// texture file
		"offset_x"	"-30"
		"offset_y"	"-46"
		"scale"		"1.26"
		"rotate"	"0"
	}
	"axisinfo"
	{
		"material"	"overview/rnl_stcomedumont_axis"	// texture file
		"offset_x"	"-30"
		"offset_y"	"-46"
		"scale"		"1.26"
		"rotate"	"0"
	}
	"tasks"
	{
		"task_dead_mans_corner"
		{
			"displayname"	"Dead Man's Corner"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"area"
			{
				"vert" "-2440.0 -7768.0 0.0"
				"vert" "-2520.0 -4695.0 0.0"
				"vert" "-1968.0 -3832.0 0.0"
				"vert" "-1640.0 -3512.0 0.0"
				"vert" "-872.0 	-3512.0 0.0"
				"vert" "464.0 	-3592.0 0.0"
				"vert" "464.0 	-3768.0 0.0"
				"vert" "1216.0 	-4351.0 0.0"
				"vert" "1408.0 	-5952.0 0.0"
				"vert" "-384.0 	-6785.0 0.0"
			}
		}
		"task_field"
		{
			"displayname"	"Trenches"
			"textposition"  "0" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"task_village_01"
		{
			"displayname"	"Village West"
			"textposition"  "0" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"task_village_02"
		{
			"displayname"	"Village East"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone

			"area"
			{
				"vert" "-136.0 	5408.0 0.0"
				"vert" "-136.0 	3300.0 0.0"
				"vert" "-1141.0 3300.0 0.0"
				"vert" "-1141.0 3832.0 0.0"
				"vert" "-1888.0 3832.0 0.0"
				"vert" "-1888.0 4960.0 0.0"
				"vert" "-1960.0 4960.0 0.0"
				"vert" "-1960.0 5408.0 0.0"
			}
		}
	}
	"objectives"
	{
		"arrow"
		{
			"size"				"6" // Size of the pointer arrows
		}
		"objective_allies_base"
		{
			"displayname"		"Allied Base"
			"displayposition"	"-770 -9300 112"
		}
		"objective_dmc"
		{
			"displayname"		"Dead Man's Corner"
			"displayposition"	"-400 -4948 112"
		}
		"objective_trench"
		{
			"displayname"		"Trenches"
			"displayposition"	"-2928 1048 224"
		}
		"objective_town"
		{
			"displayname"		"St. Come Du Mont"
			"displayposition"	"-2181 4437 224"
		}
	}
}