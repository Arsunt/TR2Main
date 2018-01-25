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
#include "game/health.h"
#include "game/text.h"
#include "global/vars.h"

void __cdecl DisplayModeInfo(char *modeString) {
	if( modeString == NULL ) {
		T_RemovePrint(DisplayModeStringInfo);
		DisplayModeStringInfo = NULL;
		return;
	}

	if( DisplayModeStringInfo != NULL ) {
		T_ChangeText(DisplayModeStringInfo, modeString);
	} else {
		DisplayModeStringInfo = T_Print(-16, -16, 0, modeString);
		T_RightAlign(DisplayModeStringInfo, 1);
		T_BottomAlign(DisplayModeStringInfo, 1);
	}

	DisplayModeInfoTimeout = 75;
}

/*
 * Inject function
 */
void Inject_Health() {
//	INJECT(0x00421980, FlashIt);
//	INJECT(0x004219B0, DrawAssaultTimer);
//	INJECT(0x00421B00, DrawGameInfo);
//	INJECT(0x00421B50, DrawHealthBar);
//	INJECT(0x00421C00, DrawAirBar);
//	INJECT(0x00421CA0, MakeAmmoString);
//	INJECT(0x00421CD0, DrawAmmoInfo);
//	INJECT(0x00421E20, InitialisePickUpDisplay);
//	INJECT(0x00421E40, DrawPickups);
//	INJECT(0x00421F40, AddDisplayPickup);

	INJECT(0x00421FB0, DisplayModeInfo);

//	INJECT(0x00422030, DrawModeInfo);
}
