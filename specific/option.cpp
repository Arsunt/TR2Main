/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
 * Original game is written by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Square Enix Ltd.
 *
 * This file is part of TR2Main.
 *
 * TR2Main is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TR2Main is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TR2Main.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global/precompiled.h"
#include "specific/option.h"
#include "game/invtext.h"
#include "game/sound.h"
#include "game/text.h"
#include "global/vars.h"

#define LN_WIDTH_L	(160)
#define LN_WIDTH_M	(LN_WIDTH_L - 4)
#define LN_WIDTH_S	(LN_WIDTH_L - 12)

#define LN_HEIGHT	(25)

#define LN_Y_TITLE	(-30)
#define LN_Y_SPACE	(LN_Y_TITLE - 2)
#define LN_Y_LINE1	(LN_HEIGHT * 0)
#define LN_Y_LINE2	(LN_HEIGHT * 1)
#define LN_Y_LINE3	(LN_HEIGHT * 2)
#define LN_Y_LINE4	(LN_HEIGHT * 3)
#define LN_Y_LINE5	(LN_HEIGHT * 4)

#define LN_NEARZ	(8)
#define LN_FARZ		(16)

static LPCSTR ControlKeysText[0x110] = {
	NULL,   "ESC",   "1",     "2",     "3",     "4",     "5",     "6",
	"7",    "8",     "9",     "0",     "-",     "+",     "BKSP",  "TAB",
	"Q",    "W",     "E",     "R",     "T",     "Y",     "U",     "I",
	"O",    "P",     "<",     ">",     "RET",   "CTRL",  "A",     "S",
	"D",    "F",     "G",     "H",     "J",     "K",     "L",     ";",
	"'",    "`",     "SHIFT", "#",     "Z",     "X",     "C",     "V",
	"B",    "N",     "M",     ",",     ".",     "/",     "SHIFT", "PADx",
	"ALT",  "SPACE", "CAPS",  NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    "NMLK",  NULL,    "PAD7",
	"PAD8", "PAD9",  "PAD-",  "PAD4",  "PAD5",  "PAD6",  "PAD+",  "PAD1",
	"PAD2", "PAD3",  "PAD0",  "PAD.",  NULL,    NULL,    "\\",    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    "ENTER", "CTRL",  NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    "SHIFT", NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    "PAD/",  NULL,    NULL,
	"ALT",  NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    "HOME",
	"UP",   "PGUP",  NULL,    "LEFT",  NULL,    "RIGHT", NULL,    "END",
	"DOWN", "PGDN",  "INS",   "DEL",   NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	NULL,   NULL,    NULL,    NULL,    NULL,    NULL,    NULL,    NULL,
	"JOY1", "JOY2",  "JOY3",  "JOY4",  "JOY5",  "JOY6",  "JOY7",  "JOY8",
	"JOY9", "JOY10", "JOY11", "JOY12", "JOY13", "JOY14", "JOY15", "JOY16",
};

void __cdecl do_inventory_options(INVENTORY_ITEM *item) {
	switch( item->objectID ) {
	// passport
		case ID_PASSPORT_OPTION:
			do_passport_option(item);
			break;
	// gamma
		case ID_GAMMA_OPTION:
			do_gamma_option(item);
			break;
	// detail
		case ID_DETAIL_OPTION:
			do_detail_option(item);
			break;
	// sound
		case ID_SOUND_OPTION:
			do_sound_option(item);
			break;
	// control
		case ID_CONTROL_OPTION:
			do_control_option(item);
			break;
	// statistics
		case ID_COMPASS_OPTION:
			do_compass_option(item);
			break;
	// weapons
		case ID_PISTOL_OPTION:
		case ID_SHOTGUN_OPTION:
		case ID_MAGNUM_OPTION:
		case ID_UZI_OPTION:
		case ID_HARPOON_OPTION:
		case ID_M16_OPTION:
		case ID_GRENADE_OPTION:
	// medipacks
		case ID_SMALL_MEDIPACK_OPTION:
		case ID_LARGE_MEDIPACK_OPTION:
	// puzzles
		case ID_PUZZLE_OPTION1:
		case ID_PUZZLE_OPTION2:
		case ID_PUZZLE_OPTION3:
		case ID_PUZZLE_OPTION4:
	// keys
		case ID_KEY_OPTION1:
		case ID_KEY_OPTION2:
		case ID_KEY_OPTION3:
		case ID_KEY_OPTION4:
	// pickups
		case ID_PICKUP_OPTION1:
		case ID_PICKUP_OPTION2:
			InputDB |= IN_SELECT;
			break;
	// ammo
		case ID_PISTOL_AMMO_OPTION:
		case ID_SHOTGUN_AMMO_OPTION:
		case ID_MAGNUM_AMMO_OPTION:
		case ID_UZI_AMMO_OPTION:
		case ID_HARPOON_AMMO_OPTION:
		case ID_M16_AMMO_OPTION:
		case ID_GRENADE_AMMO_OPTION:
			break;
	// other
		default:
			if( CHK_ANY(InputDB, IN_SELECT|IN_DESELECT) ) {
				item->goalFrame = 0;
				item->animDirection = -1;
			}
			break;
	}
}

void __cdecl do_gamma_option(INVENTORY_ITEM *item) {
// null function
}

void __cdecl do_detail_option(INVENTORY_ITEM *item) {
	int i;

	if( DetailTextInfo[0] == NULL ) {
		DetailTextInfo[4] = T_Print(0, LN_Y_TITLE, 0, GF_GameStringTable[GSI_Detail_SelectDetail]);
		DetailTextInfo[3] = T_Print(0, LN_Y_SPACE, 0, " ");
		DetailTextInfo[2] = T_Print(0, LN_Y_LINE1, 0, GF_GameStringTable[GSI_Detail_High]);
		DetailTextInfo[1] = T_Print(0, LN_Y_LINE2, 0, GF_GameStringTable[GSI_Detail_Medium]);
		DetailTextInfo[0] = T_Print(0, LN_Y_LINE3, 0, GF_GameStringTable[GSI_Detail_Low]);

		T_AddBackground(DetailTextInfo[4], LN_WIDTH_M, 0, 0, 0, LN_NEARZ, ICLR_Black, NULL, 0);
		T_AddOutline(DetailTextInfo[4], TRUE, ICLR_Orange, NULL, 0);

		T_AddBackground(DetailTextInfo[DetailLevel], LN_WIDTH_S, 0, 0, 0, LN_NEARZ, ICLR_Black, NULL, 0);
		T_AddOutline(DetailTextInfo[DetailLevel], TRUE, ICLR_Orange, NULL, 0);

		T_AddBackground(DetailTextInfo[3], LN_WIDTH_L, (LN_HEIGHT * 4 + 7), 0, 0, LN_FARZ, ICLR_Black, NULL, 0);
		T_AddOutline(DetailTextInfo[3], TRUE, ICLR_Blue, NULL, 0);

		for( i=0; i<5 ; ++i ) {
			T_CentreH(DetailTextInfo[i], 1);
			T_CentreV(DetailTextInfo[i], 1);
		}
	}

	if( CHK_ANY(InputDB, IN_BACK) && DetailLevel > 0 ) {
		T_RemoveOutline(DetailTextInfo[DetailLevel]);
		T_RemoveBackground(DetailTextInfo[DetailLevel]);
		--DetailLevel;
		T_AddOutline(DetailTextInfo[DetailLevel], TRUE, ICLR_Orange, NULL, 0);
		T_AddBackground(DetailTextInfo[DetailLevel], LN_WIDTH_S, 0, 0, 0, LN_NEARZ, ICLR_Black, NULL, 0);
	}

	if ( CHK_ANY(InputDB, IN_FORWARD) && DetailLevel < 2 ) {
		T_RemoveOutline(DetailTextInfo[DetailLevel]);
		T_RemoveBackground(DetailTextInfo[DetailLevel]);
		++DetailLevel;
		T_AddOutline(DetailTextInfo[DetailLevel], TRUE, ICLR_Orange, NULL, 0);
		T_AddBackground(DetailTextInfo[DetailLevel], LN_WIDTH_S, 0, 0, 0, LN_NEARZ, ICLR_Black, NULL, 0);
	}

	switch( DetailLevel ) {
		case 2 :
			// NOTE: maybe supposed to be SW_DETAIL_HIGH? Anyway this menu is disabled by Core
			PerspectiveDistance = SW_DETAIL_ULTRA;
			break;
		case 1 :
			PerspectiveDistance = SW_DETAIL_MEDIUM;
			break;
		case 0 :
		default :
			PerspectiveDistance = SW_DETAIL_LOW;
			break;
	}

	if( CHK_ANY(InputDB, IN_SELECT|IN_DESELECT) ) {
		for( i=0; i<5 ; ++i ) {
			T_RemovePrint(DetailTextInfo[i]);
			DetailTextInfo[i] = NULL;
		}
	}
}

void __cdecl do_compass_option(INVENTORY_ITEM *item) {
	if( CurrentLevel == 0 ) {
		// level is Assault (Lara's Home)
		ShowGymStatsText();
	} else {
		// normal level
		int hours, minutes, seconds;
		char timeString[32] = {0};

		seconds = SaveGame.statistics.timer / 30 % 60;
		minutes = SaveGame.statistics.timer / 30 / 60 % 60;
		hours   = SaveGame.statistics.timer / 30 / 60 / 60;
		sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);
		ShowStatsText(timeString, TRUE);
	}

	if( CHK_ANY(InputDB, IN_SELECT|IN_DESELECT) ) {
		item->animDirection = 1;
		item->goalFrame = item->framesTotal - 1;
	}

	PlaySoundEffect(113, NULL, SFX_ALWAYS); // ticking clock sound
}

void __cdecl FlashConflicts() {
	UINT16 key;

	for( int i=0; i<14; ++i ) {
		key = Layout[LayoutPage].key[i];
		T_FlashText(CtrlTextB[i], 0, 0);

		for( int j=0; j<14; ++j ) {
			if( (i != j) && (key == Layout[LayoutPage].key[j]) ) {
				T_FlashText(CtrlTextB[i], 1, 20);
				break;
			}
		}
	}
}

void __cdecl DefaultConflict() {
	for( int i=0; i<14; ++i ) {
		ConflictLayout[i] = 0;

		for( int j=0; j<14; ++j ) {
			if( Layout[CTRL_Custom].key[j] == Layout[CTRL_Default].key[i] ) {
				ConflictLayout[i] = 1;
				break;
			}
		}
	}
}

void __cdecl S_ChangeCtrlText() {
	UINT16 key;
	char headerStr[40];

	if( LayoutPage == CTRL_Default ) {
		T_ChangeText(ControlTextInfo[0], GF_SpecificStringTable[SSI_DefaultKeys]);
	} else {
		// NOTE: it seems Core wanted to use several custom control layout pages
		sprintf(headerStr, GF_SpecificStringTable[SSI_UserKeys], LayoutPage);
		T_ChangeText(ControlTextInfo[0], headerStr);
	}

	for( int i=0; i<14; ++i ) {
		key = Layout[LayoutPage].key[i];
		if( ControlKeysText[key] != 0 )
			T_ChangeText(CtrlTextB[i], ControlKeysText[key]);
		else
			T_ChangeText(CtrlTextB[i], "BAD");
	}
}

void __cdecl S_RemoveCtrlText() {
	for( int i=0; i<14; ++i ) {
		T_RemovePrint(CtrlTextA[i]);
		T_RemovePrint(CtrlTextB[i]);
		CtrlTextA[i] = NULL;
		CtrlTextB[i] = NULL;
	}
}

/*
 * Inject function
 */
void Inject_Option() {
	INJECT(0x0044EE80, do_inventory_options);

//	INJECT(0x0044EF90, do_passport_option);

//	INJECT(----------, do_gamma_option); // NOTE: this is null in the original code
	INJECT(0x0044F5E0, do_detail_option);

//	INJECT(0x0044F8C0, do_sound_option);

	INJECT(0x0044FD60, do_compass_option);
	INJECT(0x0044FE20, FlashConflicts);
	INJECT(0x0044FEA0, DefaultConflict);

//	INJECT(0x0044FEE0, do_control_option);
//	INJECT(0x004505F0, S_ShowControls);

	INJECT(0x00450AC0, S_ChangeCtrlText);
	INJECT(0x00450B60, S_RemoveCtrlText);
}
