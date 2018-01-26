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
#include "game/text.h"
#include "global/vars.h"

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

void __cdecl do_gamma_option(INVENTORY_ITEM *item) {
// null function
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
//	INJECT(0x0044EE80, do_inventory_options);
//	INJECT(0x0044EF90, do_passport_option);

//	INJECT(----------, do_gamma_option); // NOTE: this is null in the original code

//	INJECT(0x0044F5E0, do_detail_option);
//	INJECT(0x0044F8C0, do_sound_option);
//	INJECT(0x0044FD60, do_compass_option);

	INJECT(0x0044FE20, FlashConflicts);
	INJECT(0x0044FEA0, DefaultConflict);

//	INJECT(0x0044FEE0, do_control_option);
//	INJECT(0x004505F0, S_ShowControls);

	INJECT(0x00450AC0, S_ChangeCtrlText);
	INJECT(0x00450B60, S_RemoveCtrlText);
}
