/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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
#include "game/boat.h"
#include "3dsystem/phd_math.h"
#include "game/items.h"
#include "specific/game.h"
#include "specific/output.h"
#include "global/vars.h"

void __cdecl DoWakeEffect(ITEM_INFO *item) {
	__int16 frame_number, fxID;
	int i;
	FX_INFO *fx;

	PhdMatrixPtr->_23 = 0;
	S_CalculateLight(item->pos.x, item->pos.y, item->pos.z, item->roomNumber);
	frame_number = Objects[ID_WATER_SPRITE].nMeshes * GetRandomDraw() >> 15;
	for (i = 0; i < 3; ++i) {
		fxID = CreateEffect(item->roomNumber);
		if (fxID != -1) {
			fx = &Effects[fxID];
			fx->pos.x = item->pos.x + ((-700 * phd_sin(item->pos.rotY) + 300 * (i - 1) * phd_cos(item->pos.rotY)) >> W2V_SHIFT);
			fx->pos.y = item->pos.y;
			fx->pos.z = item->pos.z + ((-700 * phd_cos(item->pos.rotY) - 300 * (i - 1) * phd_sin(item->pos.rotY)) >> W2V_SHIFT);
			fx->pos.rotY = 16384 * (i - 1) + item->pos.rotY;
			fx->room_number = item->roomNumber;
			fx->frame_number = frame_number;
			fx->counter = 20;
			fx->object_number = ID_WATER_SPRITE;
			fx->speed = item->speed >> 2;
			if (item->speed < 64) {
				fx->fallspeed = (ABS(item->speed) - 64) * GetRandomDraw() >> 15;
			} else {
				fx->fallspeed = 0;
			}
			fx->shade = LsAdder - 768;
			CLAMPL(fx->shade, 0);
		}
	}
}

/*
 * Inject function
 */
void Inject_Boat() {
//	INJECT(0x0040CB10, InitialiseBoat);
//	INJECT(0x0040CB50, BoatCheckGeton);
//	INJECT(0x0040CCC0, BoatCollision);
//	INJECT(0x0040CE20, TestWaterHeight);
//	INJECT(0x0040CF20, DoBoatShift);

	INJECT(0x0040D0F0, DoWakeEffect);

//	INJECT(0x0040D270, DoBoatDynamics);
//	INJECT(0x0040D2C0, BoatDynamics);
//	INJECT(0x0040D7A0, BoatUserControl);
//	INJECT(0x0040D930, BoatAnimation);
//	INJECT(0x0040DAA0, BoatControl);
//	INJECT(0x0040E0D0, GondolaControl);
}
