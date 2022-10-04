// rnl NCO Map description file for rnl_ardennes
"mapvalues"
{
	"alliesinfo"
	{
		"material"	"overview/rnl_ardennes_allies"	// texture file
		"offset_x"	"-60"
		"offset_y"	"-60"
		"scale"		"1.70"
		"reverse_y"	"0"
		"angle"		"90"
	}
	"axisinfo"
	{
		"material"	"overview/rnl_ardennes_axis"	// texture file
		"offset_x"	"-60"
		"offset_y"	"-60"
		"scale"		"1.70"
		"reverse_y"	"0"
		"angle"		"90"
	}
	"tasks"
	{
		"area_capture_south"
		{
			"displayname"	"Southern Zone"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"area"
			{
				"vert" "-3514 1321 0.0"
				"vert" "-2006 804 0.0"
				"vert" "-2052 -731 0.0"
				"vert" "-3463 -3 0.0"
			}
			
		}
		"area_capture_center"
		{
			"displayname"	"City Center"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"area_capture_church"
		{
			"displayname"	"Northern Zone"
			"textposition"  "0" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"area"
			{
				"vert" "883 966 0.0"
				"vert" "2833 1066 0.0"
				"vert" "2848 -620.0"
				"vert" "891 -470 0.0"
			}
		}
		"task_destruction_axis_tank"
		{
			"displayname"		"Axis Tank"
			"displayposition"	"173 456 -190"
			"textposition"  "2" 	
		}
		"task_destruction_allied_tank"
		{
			"displayname"		"Allied Tank"
			"displayposition"	"-1325 457 -185"
		}
	}
	"objectives"
	{
		"arrow"
		{
			"size"				"7" // Size of the pointer arrows
		}
		"objective_south"
		{
			"displayname"		""
			"displayposition"	"-2761 294 53"
		}
		"objective_center"
		{
			"displayname"		""
			"displayposition"	"-624 -721 -170"
		}
		"objective_church"
		{
			"displayname"		""
			"displayposition"	"1866 194 0"
			
		}
		"objective_allied_base"
		{
			"displayname"		"Allied Base"
			"displayposition"	"-6272 978 62"
		}
		"objective_axis_base"
		{
			"displayname"		"Axis Base"
			"displayposition"	"5388 340 238"
		}
	}
}