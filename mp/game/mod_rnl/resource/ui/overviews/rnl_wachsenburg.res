// rnl NCO Map description file for rnl_wachsenburg
"mapvalues"
{
	"alliesinfo"
	{
		"material"	"overview/rnl_stcomedumont_allies"	// texture file
		"offset_x"	"0"
		"offset_y"	"0"
		"scale"		"1"
		"rotate"	"0"
	}
	"axisinfo"
	{
		"material"	"overview/rnl_stcomedumont_axis"	// texture file
		"offset_x"	"0"
		"offset_y"	"0"
		"scale"		"1"
		"rotate"	"0"
	}
	"tasks"
	{
		"task_cap_shack"
		{
			"displayname"	"Allied Base"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"task_cap_outpost"
		{
			"displayname"	"Outpost"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"task_cap_wachsenburg"
		{
			"displayname"	"Wachsenburg"
			"textposition"  "0" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"task_destruction_01"
		{
			"displayname"		"Flak 1"
			"displayposition"	"-2437 -2240 224"
		}
		"task_destruction_02"
		{
			"displayname"		"Flak 2"
			"displayposition"	"-3230 -482 224"
		}
		"task_destruction_03"
		{
			"displayname"		"Flak 3"
			"displayposition"	"-1001 99 224"
		}
	}
	"objectives"
	{
		"objective_capture_shack"
		{
			"displayname"		"Shack"
			"displayposition"	"7208 -1736 112"
		}
		"objective_capture_outpost"
		{
			"displayname"		"Outpost"
			"displayposition"	"1696 -2672 -120"
		}
		"objective_capture_wachsenburg"
		{
			"displayname"		"Wachsenburg"
			"displayposition"	"-1786 1144 1138"
		}
	}
}