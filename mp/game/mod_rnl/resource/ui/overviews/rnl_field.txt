// rnl NCO Map description file rnl_field
"mapvalues"
{
	"alliesinfo"
	{
		"material"	"overview/rnl_field"	// texture file
		"offset_x"	"0"
		"offset_y"	"60"
		"scale"		"5.0"
		"reverse_y"	"1"
		"angle"		"180"
	}

	"axisinfo"
	{
		"material"	"overview/rnl_field_axis"	// texture file
		"offset_x"	"0"
		"offset_y"	"60"
		"scale"		"5.0"
		"reverse_y"	"1"
		"angle"		"180"
	}

	"node0"
	{
		"displayname"	"Allies Start"
		"textposition"  "0" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		"iconposition"  "1"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone

		//For Drawing the area on the map
		//comes in form of "vert" "X Y Z U V" where X Y and Z are world coordinates
		"area"
		{
			"vert" "0.0 512.0 0.0"
			"vert" "-100.0 512.0 0.0"
			"vert" "0.0 1024.0 0.0"
			"vert" "255.0 1024.0 0.0"
			"vert" "512.0 768.0 0.0"
		}
	}

	"node1"
	{
		"displayname"	"Hole 2"
		"textposition"  "2" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		"iconposition"  "0"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		"area"
		{
			"vert" "0.0 1536.0 0.0"
			"vert" "0.0 2048.0 0.0"
			"vert" "255.0 2048.0 0.0"
			"vert" "255.0 1536.0 0.0"
		}
	}

	"node2"
	{
		"displayname"	"Hole 1"
		"textposition"  "4" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		"iconposition"  "2"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		"area"
		{
			"vert" "100.0 0.0 0.0"
			"vert" "0.0 250.0 0.0"
			"vert" "0.0 360.0 0.0"
			"vert" "100.0 512.0 0.0"
			"vert" "255.0 512.0 0.0"
			"vert" "300.0 400.0 0.0"
		}
	}

	"node3"
	{
		"textposition"  "3" 					// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		"iconposition"  "4"						// 0 = above, 1 = center of, 2 = below, 3 = left of, 4 = right of capture zone
		"displayname"	"Axis Start"
		"area"
		{
			"vert" "0.0 1024.0 0.0"
			"vert" "0.0 1536.0 0.0"
			"vert" "255.0 1536.0 0.0"
		}
	}
}