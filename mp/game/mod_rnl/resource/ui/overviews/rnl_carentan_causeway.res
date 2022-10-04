// rnl NCO Map description file for rnl_carantan_causeway
"mapvalues"
{	
	"alliesinfo"
	{
		"material"	"overview/rnl_carentan_causeway_allies"	// texture file
		"offset_x"	"-60"
		"offset_y"	"-55"
		"scale"		"0.75" 	
		//"reverse_y"	"1"
	}

	"axisinfo"
	{
		"material"	"overview/rnl_carentan_causeway_axis"	// texture file
		"offset_x"	"-60"
		"offset_y"	"-55"
		"scale"		"0.75" 	
		//"reverse_y"	"1"
	}

	"tasks"
	{
		"task_capture_farm01"
		{
			"displayname"	"Ingouf Farm"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			
			"area"
			{
				"vert" "-4866.0 11501.0 0.0"
				"vert" "1983.0 11520.0 0.0"
				"vert" "1983.0 8768.0 0.0"
				"vert" "1024.0 8768.0 0.0"
				"vert" "-673.0 7248.0 0.0"
				"vert" "-2304.0 6688.0 0.0"
				"vert" "-4864.0 6688.0 0.0"
			}
		}
		"task_capture_orchard"
		{
			"displayname"	"Orchard"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"task_capture_farm02"
		{
			"displayname"	"Farm"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
		"task_capture_farm03"
		{
			"displayname"	"Axis Base"
			"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
			"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		}
	}
	"objectives"
	{
		"objective_capture_farm01"
		{
			"displayname"		"Ingouf Farm"
			"displayposition"	"-1472 9728 0"
		}
		"objective_capture_orchard"
		{
			"displayname"		"Orchard"
			"displayposition"	"6152 11776 0"
		}
		"objective_capture_farm02"
		{
			"displayname"		"Farm"
			"displayposition"	"8704 5472 0"
		}
		"objective_capture_farm03"
		{
			"displayname"		"Axis Base"
			"displayposition"	"14640 2048 0"
		}
	}
}