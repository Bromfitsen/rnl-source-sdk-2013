///////////////////////////////////////////////////////////
// Tracker scheme resource file
//
// sections:
//		Colors			- all the colors used by the scheme
//		BaseSettings	- contains settings for app to use to draw controls
//		Fonts			- list of all the fonts used by app
//		Borders			- description of all the borders
//
///////////////////////////////////////////////////////////
Scheme
{
	//////////////////////// COLORS ///////////////////////////
	Colors
	{
		// base colors
		"Orange"				"255 176 0 255"
		"OrangeDim"				"255 176 0 120"
		"LightOrange"			"188 112 0 128"
		"White"					"255 255 255 255"
		"WhiteDim"				"255 255 255 192"
		"Red"					"192 28 0 140"
		"Black"					"0 0 0 255"
		"TransparentBlack"		"0 0 0 196"
		"TransparentLightBlack"	"0 0 0 90"
		"Blank"					"0 0 0 0"

		// rnl base colors
		"BaseColour1"			"165 160 130 255"
		"BaseColour2"			"221 221 221 255"
		"BaseColour3"			"255 255 255 255"
		"BaseColour3Dim"		"218 217 195 190"
		"BaseColour4"			"27 39 27 255"
		"BaseColour4Dim"		"27 39 27 190"
		"BaseColour5"			"80 87 79 255"
		"BaseColour6"			"146 144 113 255"
		"BaseColour6a"			"146 144 113 0"
		"BaseColour6Dim"		"146 144 113 190"
		"BaseColour7"			"197 192 137 255"
		"ListBack"				"32 32 25 255"

		// test colors
		"ForTesting"			"255 0 0 32"
		"ForTesting_Magenta"	"255 0 255 255"
		"ForTesting_MagentaDim"	"255 0 255 120"

		// menu colors
		"MenuTitleBackground"		"146 144 113 190"
		"MenuSubtitleBackground"	"80 87 79 190"
		"MenuContentBackground"		"216 214 171 190"
		"SquadPanelBackground"		"218 217 195 255"
		"MenuLine"					"136 134 103 190"
		"SlotClassName"				"27 39 27 255"
		"SlotClassNameMouseOver"	"255 0 0 255"
		"SlotClassDescription"		"128 128 128 255"
		"SlotPlayerName"			"128 0 0 255"
		"SlotFree"					"0 128 0 255"
		"SlotDeactivated"			"0 0 128 255"
		"LoadoutItem"				"0 0 0 255"
		"LoadoutItemMouseOver"		"255 0 0 255"
		"LoadoutPanelBackground"	"218 217 195 255"
	}

	///////////////////// BASE SETTINGS ////////////////////////
	//
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		"FgColor"					"255 220 0 100"
		"BgColor"					"0 0 0 255"

		"Panel.FgColor"				"BaseColour3"
		"Panel.BgColor"				"BaseColour6a"

		"BrightFg"					"BaseColour3"

		"DamagedBg"					"180 0 0 200"
		"DamagedFg"					"180 0 0 230"
		"BrightDamagedFg"			"255 0 0 255"

		// weapon selection colors
		"SelectionNumberFg"			"255 220 0 255"
		"SelectionTextFg"			"255 220 0 255"
		"SelectionEmptyBoxBg" 		"0 0 0 80"
		"SelectionBoxBg" 			"0 0 0 80"
		"SelectionSelectedBoxBg" 	"0 0 0 80"
		"ZoomReticleColor"			"255 255 255 255"

		// HL1-style HUD colors
		"Yellowish"					"255 160 0 255"
		"Normal"					"255 208 64 255"
		"Caution"					"255 48 0 255"

		// vgui_controls color specifications
		Border.Bright				"BaseColour3"		// the lit side of a control
		Border.Dark					"BaseColour3"		// the dark/unlit side of a control
		Border.Selection			"BaseColour3"		// the additional border color for displaying the default/selected button

		Button.TextColor			"BaseColour4"
		Button.BgColor				"BaseColour3"
		Button.ArmedTextColor		"BaseColour4Dim"
		Button.ArmedBgColor			"BaseColour3Dim"
		Button.DepressedTextColor	"BaseColour4"
		Button.DepressedBgColor		"BaseColour3"
		Button.DisabledBgColor		"BaseColour6"

		CheckButton.TextColor			"Orange"
		CheckButton.SelectedTextColor	"Orange"
		CheckButton.BgColor				"TransparentBlack"
		CheckButton.Border1  			"Border.Dark" 		// the left checkbutton border
		CheckButton.Border2  			"Border.Bright"		// the right checkbutton border
		CheckButton.Check				"Orange"			// color of the check itself

		ComboBoxButton.ArrowColor		"BaseColour3"
		ComboBoxButton.ArmedArrowColor	"BaseColour3"
		ComboBoxButton.BgColor			"BaseColour5"
		ComboBoxButton.DisabledBgColor	"Blank"

		Frame.BgColor						"Blank"
		Frame.OutOfFocusBgColor				"BaseColour5"
		Frame.FocusTransitionEffectTime		"0.0"	// time it takes for a window to fade in/out on focus/out of focus
		Frame.TransitionEffectTime			"0.0"	// time it takes for a window to fade in/out on open/close
		Frame.AutoSnapRange					"0"
		FrameGrip.Color1					"Blank"
		FrameGrip.Color2					"Blank"
		FrameTitleButton.FgColor			"Blank"
		FrameTitleButton.BgColor			"Blank"
		FrameTitleButton.DisabledFgColor	"Blank"
		FrameTitleButton.DisabledBgColor	"Blank"
		FrameSystemButton.FgColor			"Blank"
		FrameSystemButton.BgColor			"Blank"
		FrameSystemButton.Icon				""
		FrameSystemButton.DisabledIcon		""
		FrameTitleBar.TextColor				"Orange"
		FrameTitleBar.BgColor				"Blank"
		FrameTitleBar.DisabledTextColor		"Orange"
		FrameTitleBar.DisabledBgColor		"Blank"

		GraphPanel.FgColor					"Orange"
		GraphPanel.BgColor					"TransparentBlack"

		Label.TextDullColor					"WhiteDim"
		Label.TextColor						"White"
		Label.TextBrightColor				"White"
		Label.SelectedTextColor				"Orange"
		Label.BgColor						"Blank"
		Label.DisabledFgColor1				"Blank"
		Label.DisabledFgColor2				"LightOrange"

		ListPanel.TextColor					"White"
		ListPanel.BgColor					"ListBack"
		ListPanel.SelectedTextColor			"LightOrange"
		ListPanel.SelectedBgColor			"ListBack"
		ListPanel.SelectedOutOfFocusBgColor	"ListBack"
		ListPanel.EmptyListInfoTextColor	"White"

		Menu.TextColor						"BaseColour3"
		Menu.BgColor						"TransparentBlack"
		Menu.ArmedTextColor					"BaseColour4"
		Menu.ArmedBgColor					"BaseColour3"
		Menu.TextInset						"6"

		Chat.TypingText						"BaseColour3"

		Panel.FgColor						"BaseColour6Dim"
		Panel.BgColor						"BaseColour3"

		ProgressBar.FgColor					"BaseColour3"
		ProgressBar.BgColor					"BaseColour4"

		PropertySheet.TextColor				"Orange"
		PropertySheet.SelectedTextColor		"Orange"
		PropertySheet.TransitionEffectTime	"0.25"	// time to change from one tab to another

		RadioButton.TextColor				"Orange"
		RadioButton.SelectedTextColor		"Orange"

		RichText.TextColor					"BaseColour3"
		RichText.BgColor					"BaseColour5"
		RichText.SelectedTextColor			"BaseColour3"
		RichText.SelectedBgColor			"Blank"

		ScrollBarButton.FgColor				"White"
		ScrollBarButton.BgColor				"ListBack"
		ScrollBarButton.ArmedFgColor		"Orange"
		ScrollBarButton.ArmedBgColor		"ListBack"
		ScrollBarButton.DepressedFgColor	"Orange"
		ScrollBarButton.DepressedBgColor	"ListBack"

		ScrollBarSlider.FgColor				"White"		// nob color
		ScrollBarSlider.BgColor				"ListBack"		// slider background color

		ScrollBar.BgColor					"ListBack"
		ScrollBar.Wide						"0"

		HTML.BgColor						"BaseColour3"

		SectionedListPanel.HeaderTextColor				"Orange"
		SectionedListPanel.HeaderBgColor				"Blank"
		SectionedListPanel.DividerColor					"Black"
		SectionedListPanel.TextColor					"Orange"
		SectionedListPanel.BrightTextColor				"Orange"
		SectionedListPanel.BgColor						"TransparentLightBlack"
		SectionedListPanel.SelectedTextColor			"Black"
		SectionedListPanel.SelectedBgColor				"Red"
		SectionedListPanel.OutOfFocusSelectedTextColor	"Black"
		SectionedListPanel.OutOfFocusSelectedBgColor	"255 255 255 32"

		Slider.NobColor						"108 108 108 255"
		Slider.TextColor					"127 140 127 255"
		Slider.TrackColor					"31 31 31 255"
		Slider.DisabledTextColor1			"117 117 117 255"
		Slider.DisabledTextColor2			"30 30 30 255"

		TextEntry.TextColor					"BaseColour3"
		TextEntry.BgColor					"TransparentBlack"
		TextEntry.CursorColor				"BaseColour3"
		TextEntry.DisabledTextColor			"BaseColour6"
		TextEntry.DisabledBgColor			"Blank"
		TextEntry.SelectedTextColor			"BaseColour3"
		TextEntry.SelectedBgColor			"TransparentBlack"
		TextEntry.OutOfFocusSelectedBgColor	"Blank"
		TextEntry.FocusEdgeColor			"TransparentBlack"

		ToggleButton.SelectedTextColor		"Orange"

		Tooltip.TextColor				"TransparentBlack"
		Tooltip.BgColor					"Red"

		TreeView.BgColor				"TransparentBlack"

		WizardSubPanel.BgColor			"Blank"
		

		// CHudMenu
		"ItemColor"		"255 167 42 200"	// default 255 167 42 255
		"MenuColor"		"233 208 173 255"
		"MenuBoxBg"		"0 0 0 100"

		// Top-left corner of the "Half-Life 2" on the main screen
		"Main.Title1.X"		"76"
		"Main.Title1.Y"		"184"
		"Main.Title1.Color"	"255 255 255 0"

		// Top-left corner of the menu on the main screen
		"Main.Menu.X"		"76"
		"Main.Menu.Y"		"245"

		// Blank space to leave beneath the menu on the main screen
		"Main.BottomBorder"	"32"
	}

	//////////////////////// FONTS /////////////////////////////
	//
	// describes all the fonts
	Fonts
	{
		// fonts are used in order that they are listed
		// fonts are used in order that they are listed
		"DebugFixed"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"400"
				"antialias" "1"
			}
		}
		// fonts are used in order that they are listed
		"DebugFixedSmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"400"
				"antialias" "1"
			}
		}
		// fonts listed later in the order will only be used if they fulfill a range not already filled
		// if a font fails to load then the subsequent fonts will replace
		Default
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"11"
				"weight"	"700"
				"antialias" "1"
				"yres"	"1 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"700"
				"antialias" "1"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"16"
				"weight"	"900"
				"antialias" "1"
				"yres"	"768 1023"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"22"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1024 1199"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"26"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1200 10000"
			}
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"1200 6000"
				"antialias"	"1"
			}
			"6"
			{
				"name"		"Arial"
				"tall"		"12"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1200 6000"
				"antialias"	"1"
			}
			"6"
			{
				"name"		"Verdana"
				"tall"		"12"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
			"7"
			{
				"name"		"Arial"
				"tall"		"11"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
		}
		WeaponIcons
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"64"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		WeaponIconsSelected
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"64"
				"weight"	"0"
				"antialias" "1"
				"blur"		"5"
				"scanlines"	"2"
				"additive"	"1"
				"custom"	"1"
			}
		}
		QuickInfo
		{
			"1"
			{
				"name"		"HL2cross"
				"tall"		"28"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		HudNumbers
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"32"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		HudNumbersGlow
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"32"
				"weight"	"0"
				"blur"		"4"
				"scanlines" "2"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		HudNumbersSmall
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"16"
				"weight"	"1000"
				"additive"	"1"
				"antialias" "1"
				"custom"	"1"
			}
		}
		HudSelectionNumbers
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"11"
				"weight"	"700"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudHintTextLarge
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"1000"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudHintTextSmall
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"11"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudSelectionText
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"8"
				"weight"	"700"
				"antialias" "1"
				"yres"	"1 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"10"
				"weight"	"700"
				"antialias" "1"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"900"
				"antialias" "1"
				"yres"	"768 1023"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"16"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1024 1199"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"17"
				"weight"	"1000"
				"antialias" "1"
				"yres"	"1200 10000"
			}
		}
		BudgetLabel
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"400"
				"outline"	"1"
			}
		}
		DebugOverlay
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"400"
				"outline"	"1"
			}
		}
		"CloseCaption_Normal"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"26"
				"weight"	"500"
			}
		}
		"CloseCaption_Italic"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"26"
				"weight"	"500"
				"italic"	"1"
			}
		}
		"CloseCaption_Bold"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"26"
				"weight"	"900"
			}
		}
		"CloseCaption_BoldItalic"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"26"
				"weight"	"900"
				"italic"	"1"
			}
		}
		"MenuTitle"
		{
			"1"
			{
				"name"		"Courier New"
				"tall"		"14"
				"weight"	"700"
			}
		}
		// this is the symbol font
		"Marlett"
		{
			"1"
			{
				"name"		"Marlett"
				"tall"		"14"
				"weight"	"0"
				"symbol"	"1"
			}
		}
		"Trebuchet24"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"900"
				"range"		"0x0000 0x007F"	//	Basic Latin
				"antialias" "1"
				"additive"	"1"
			}
		}
		"Trebuchet18"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"18"
				"weight"	"900"
			}
		}
		"ChatFont"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"700"
				"yres"	"480 599"
				"dropshadow"	"1"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"700"
				"yres"	"600 767"
				"dropshadow"	"1"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"700"
				"yres"	"768 1023"
				"dropshadow"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"700"
				"yres"	"1024 1199"
				"dropshadow"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"700"
				"yres"	"1200 10000"
				"dropshadow"	"1"
			}
		}
		ClientTitleFont
		{
			"1"
			{
				"name"  "Courier New"
				"tall"  "46"
				"weight" "30"
				"additive" "0"
				"antialias" "1"
			}
		}
		CreditsLogo
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"128"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsText
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
			}
		}
		CreditsOutroLogos
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"48"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsOutroText
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"9"
				"weight"	"900"
				"antialias" "1"
			}
		}
		CenterPrintText
		{
			// note that this scales with the screen resolution
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"18"
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
			}
		}
		LoadoutMenu
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"32"
				"weight"	"500"
				"antialias"	"1"
			}
		}
		ClassLabel
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"900"
				"antialias" "1"
				"yres"		"1 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"22"
				"weight"	"900"
				"antialias" "1"
				"yres"		"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"900"
				"antialias" "1"
				"yres"		"768 1023"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"26"
				"weight"	"900"
				"antialias" "1"
				"yres"		"1024 1199"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"32"
				"weight"	"900"
				"antialias" "1"
				"yres"		"1200 10000"
			}
		}
		joeHand
		{
			"1"
			{
				"name"		"joeHand"
				"tall"		"28"
				"weight"	"700"
				"yres"	"480 599"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"joeHand"
				"tall"		"32"
				"weight"	"700"
				"yres"	"600 767"
				"antialias"	"1"
			}
			"3"
			{
				"name"		"joeHand"
				"tall"		"36"
				"weight"	"700"
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"joeHand"
				"tall"		"40"
				"weight"	"700"
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"joeHand"
				"tall"		"48"
				"weight"	"700"
				"yres"	"1200 10000"
				"antialias"	"1"
			}
		}
		joeHandBig
		{
			"1"
			{
				"name"		"joeHand"
				"tall"		"54"
				"weight"	"700"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"joeHand"
				"tall"		"72"
				"weight"	"700"
				"antialias"	"1"
			}
		}
		verArmy
		{
			"1"
			{
				"name"		"Ver Army"
				"tall"		"12"
				"weight"	"700"
				"antialias"	"1"
			}
		}
		QuicksandBold-Regular
		{
			"1"
			{
				"name"		"QuicksandBold-Regular"
				"tall"		"16"
				"weight"	"700"
				"yres"		"480 599"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"QuicksandBold-Regular"
				"tall"		"20"
				"weight"	"700"
				"yres"		"600 767"
				"antialias"	"1"
			}
			"3"
			{
				"name"		"QuicksandBold-Regular"
				"tall"		"24"
				"weight"	"700"
				"yres"		"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"QuicksandBold-Regular"
				"tall"		"28"
				"weight"	"700"
				"yres"		"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"QuicksandBold-Regular"
				"tall"		"32"
				"weight"	"700"
				"yres"		"1200 10000"
				"antialias"	"1"
			}
		}
	}

		//////////////////// BORDERS //////////////////////////////
	//
	// describes all the border types
	Borders
	{
		BaseBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}

		TitleButtonBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		TitleButtonDisabledBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BgColor"
					"offset" "1 0"
				}
			}
			Top
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 0"
				}
			}
		}

		TitleButtonDepressedBorder
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}

		ScrollBarButtonBorder
		{
			"inset" "1 0 0 0"
			Left
			{
				"0"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"0"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"0"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"0"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		ScrollBarButtonDepressedBorder
		{
			"inset" "2 2 0 0"
			Left
			{
				"0"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"0"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"0"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"0"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}

		ButtonBorder
		{
			"inset" "0 0 0 0"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		FrameBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "ControlBG"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "ControlBG"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "ControlBG"
					"offset" "0 1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "ControlBG"
					"offset" "0 0"
				}
			}
		}

		TabBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}

		TabActiveBorder
		{
			"inset" "0 0 1 0"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "ControlBG"
					"offset" "6 2"
				}
			}
		}


		ToolTipBorder
		{
			"inset" "0 0 1 0"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		// this is the border used for default buttons (the button that gets pressed when you hit enter)
		ButtonKeyFocusBorder
		{
			"inset" "0 0 0 0"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		ButtonDepressedBorder
		{
			"inset" "0 0 0 0"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		ComboBoxBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}

		MenuBorder
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}
		BrowserBorder
		{
			"inset" "0 0 0 1"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}
	}


	//////////////////////// CUSTOM FONT FILES /////////////////////////////
	//
	// specifies all the custom (non-system) font files that need to be loaded to service the above described fonts
	CustomFontFiles
	{
		"1"		"resource/HALFLIFE2.ttf"
		"2"		"resource/HL2crosshairs.ttf"
		"3"		"resource/joeHand.ttf"
		"4"		"resource/QuicksandBold-Regular.ttf"
	}

}