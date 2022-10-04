// rnl NCO Map description file rnl_stmariedumont
"mapvalues"
{

	"alliesinfo"
	{
		"material"	"overview/rnl_stemariedumont_allies"	// texture file
		"offset_x"	"-70" // -8
		"offset_y"	"-120" // -78
		"scale"		"0.985"  // 1.78
		"reverse_y"	"0"   // 0
	}

	"axisinfo"
	{
		"material"	"overview/rnl_stemariedumont_axis"	// texture file
		"offset_x"	"-70" // -8
		"offset_y"	"-111.5" // -78
		"scale"		"0.985"  // 1.78
		"reverse_y"	"0"   // 0
	}
	
	"tasks"
	{
		"task_hotel"
		{
			"displayname"	""
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}

		"task_church"
		{
			"displayname"	"Town"
			"textposition"  "0" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"area"
			{
				"vert" "-2944 -9752 0"
				"vert" "-2944 -7832 0"
				"vert" "-1792 -7832 0"
				"vert" "-724 -8148 0"
				"vert" "296 -8148 0"
				"vert" "296 -9752 0"
			}
		}

		"task_apple"
		{
			"displayname"	"Cider Farm"
			"textposition"  "0" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "0"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}

		"task_corn"
		{
			"displayname"	"Corn Farm"
			"textposition"  "4" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"area"
			{
				"vert" "-2918 2426 0"
				"vert" "-2918 1914 0"
				"vert" "-3094 1914 0"
				"vert" "-3094 2426 0"
			}
			"area"
			{
				"vert" "-2854 1914 0"
				"vert" "-2854 1510 0"
				"vert" "-3178 1510 0"
				"vert" "-3178 1914 0"
			}
			"area"
			{
				"vert" "-3178 1966 0"
				"vert" "-3178 1674 0"
				"vert" "-3606 1674 0"
				"vert" "-3606 1966 0"
			}
		}

		"task_pond"
		{
			"displayname"	"Pond House"
			"textposition"  "3" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"area"
			{
				"vert" "484 5924 0"
				"vert" "484 5684 0"
				"vert" "-140 5684 0"
				"vert" "-140 5924 0"
			}
			"area"
			{
				"vert" "448 5684 0"
				"vert" "448 5308 0"
				"vert" "200 5308 0"
				"vert" "200 5684 0"
			}
		}

		"task_trench"
		{
			"displayname"	"Trench"
			"textposition"  "3" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"area"
			{
				"vert" "4728 6276 0"
				"vert" "3232 6276 0"
				"vert" "2688 6996 0"
				"vert" "4728 6996 0"
			}
			"area"
			{
				"vert" "3584 6996 0"
				"vert" "2688 6996 0"
				"vert" "2688 8084 0"
				"vert" "3584 8084 0"
			}
		}

		"task_barn"
		{
			"displayname"	"Barn"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
	}
	"objectives"
	{
		"arrow"
		{
			"size"				"4" // Size of the pointer arrows
		}
		"node_barn"
		{
			"displayname"		""
			"displayposition"	"7352 11284 616"
		}
		"node_trench"
		{
			"displayname"		""
			"displayposition"	"5428 7260 252"
		}
		"node_pond"
		{
			"displayname"		""
			"displayposition"	"1372 5616 412"
		}
		"node_corn"
		{
			"displayname"		""
			"displayposition"	"-2030 1968 489"
		}	
		"node_apple"
		{
			"displayname"		""
			"displayposition"	"940 -2452 346"
		}	
		"node_church"
		{
			"displayname"		""
			"displayposition"	"924 -8620 290"
		}	
	}
}