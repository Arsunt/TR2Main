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
#include "game/skidoo.h"
#include "3dsystem/phd_math.h"
#include "game/items.h"
#include "specific/game.h"
#include "specific/output.h"
#include "global/vars.h"

void __cdecl DoSnowEffect(ITEM_INFO *item) {
	__int16 fxID;
	FX_INFO *fx;
	int displacement;

	fxID = CreateEffect(item->roomNumber);
	if (fxID != -1) {
		fx = &Effects[fxID];
		displacement = 260 * (GetRandomDraw() - 16384) >> 14;
		fx->pos.x = item->pos.x - ((500 * phd_sin(item->pos.rotY) + displacement * phd_cos(item->pos.rotY)) >> W2V_SHIFT);
		fx->pos.y = item->pos.y + (500 * phd_sin(item->pos.rotX) >> W2V_SHIFT);
		fx->pos.z = item->pos.z - ((500 * phd_cos(item->pos.rotY) - displacement * phd_sin(item->pos.rotY)) >> W2V_SHIFT);
		fx->room_number = item->roomNumber;
		fx->frame_number = 0;
		fx->object_number = ID_SNOW_SPRITE;
		fx->speed = 0;
		if (item->speed < 64) {
			fx->fallspeed = (ABS(item->speed) - 64) * GetRandomDraw() >> 15;
		} else {
			fx->fallspeed = 0;
		}
		PhdMatrixPtr->_23 = 0;
		S_CalculateLight(fx->pos.x, fx->pos.y, fx->pos.z, fx->room_number);
		fx->shade = LsAdder - 512;
		CLAMPL(fx->shade, 0);
	}
}

/*
 * Inject function
 */
void Inject_Skidoo() {
//	INJECT(0x0043CEE0, InitialiseSkidoo);
//	INJECT(0x0043CF20, SkidooCheckGeton);
//	INJECT(0x0043D010, SkidooCollision);
//	INJECT(0x0043D110, SkidooBaddieCollision);
//	INJECT(0x0043D310, TestHeight);
//	INJECT(0x0043D3D0, DoShift);
//	INJECT(0x0043D650, DoDynamics);
//	INJECT(0x0043D6B0, GetCollisionAnim);

	INJECT(0x0043D740, DoSnowEffect);

//	INJECT(0x0043D880, SkidooDynamics);
//	INJECT(0x0043DD20, SkidooUserControl);
//	INJECT(0x0043DEE0, SkidooCheckGetOffOK);
//	INJECT(0x0043DFF0, SkidooAnimation);
//	INJECT(0x0043E2D0, SkidooExplode);
//	INJECT(0x0043E350, SkidooCheckGetOff);
//	INJECT(0x0043E590, SkidooGuns);
//	INJECT(0x0043E6B0, SkidooControl);
//	INJECT(0x0043EB10, DrawSkidoo);
//	INJECT(0x0043EDF0, InitialiseSkidman);
//	INJECT(0x0043EE80, SkidManControl);
//	INJECT(0x0043F280, SkidmanPush);
//	INJECT(0x0043F3A0, SkidmanCollision);
}
