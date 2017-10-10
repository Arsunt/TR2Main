/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
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
#include "game/draw.h"
#include "game/hair.h"
#include "game/setup.h"
#include "global/vars.h"

void __cdecl InitialiseLevelFlags() {
	memset(&SaveGame.statistics, 0, sizeof(STATISTICS_INFO));
}

void __cdecl InitialiseObjects() {
	for( int i = 0; i < ID_NUMBER_OBJECTS; ++i ) {
		Objects[i].flags &= ~0xBEu;
		Objects[i].initialise = NULL;
		Objects[i].collision = NULL;
		Objects[i].control = NULL;
		Objects[i].drawRoutine = DrawAnimatingItem;
		Objects[i].ceiling = NULL;
		Objects[i].floor = NULL;
		Objects[i].pivotLength = 0;
		Objects[i].radius = 10;
		Objects[i].shadowSize = 0;
		Objects[i].hitPoints = 0xC000; // non targettable
	}
	BaddyObjects();
	TrapObjects();
	ObjectObjects();
	InitialiseHair();
}

/*
 * Inject function
 */
void Inject_Setup() {
	INJECT(0x0043A500, InitialiseLevelFlags);
	INJECT(0x0043C7C0, InitialiseObjects);
}
