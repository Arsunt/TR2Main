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
#include "game/missile.h"
#include "3dsystem/phd_math.h"
#include "game/control.h"
#include "game/draw.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/sound.h"
#include "game/traps.h"
#include "specific/game.h"
#include "global/vars.h"

void __cdecl ControlMissile(__int16 fxID) {
	FX_INFO *fx = &Effects[fxID];

	if( Effects[fxID].object_number == ID_MISSILE_HARPOON
		&& !CHK_ANY(RoomInfo[fx->room_number].flags, ROOM_UNDERWATER)
		&& fx->pos.rotX > -PHD_90*3/4 )
	{
		fx->pos.rotX -= PHD_DEGREE;
	}
	int speed = fx->speed * phd_cos(fx->pos.rotX) >> W2V_SHIFT;
	fx->pos.y += fx->speed * phd_sin(-fx->pos.rotX) >> W2V_SHIFT;
	fx->pos.z += speed * phd_cos(fx->pos.rotY) >> W2V_SHIFT;
	fx->pos.x += speed * phd_sin(fx->pos.rotY) >> W2V_SHIFT;

	__int16 roomNumber = fx->room_number;
	FLOOR_INFO *floor = GetFloor(fx->pos.x, fx->pos.y, fx->pos.z, &roomNumber);
	if( fx->pos.y >= GetHeight(floor, fx->pos.x, fx->pos.y, fx->pos.z)
		|| (fx->pos.y <= GetCeiling(floor, fx->pos.x, fx->pos.y, fx->pos.z)) )
	{
		if( fx->object_number == ID_MISSILE_KNIFE || fx->object_number == ID_MISSILE_HARPOON ) {
			fx->speed = 0;
			fx->frame_number = -GetRandomControl()/11000;
			fx->counter = 6;
			fx->object_number = ID_RICOCHET;
			PlaySoundEffect(258, &fx->pos, 0);
		} else if( fx->object_number == ID_MISSILE_FLAME ) {
			AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 14, 11);
			KillEffect(fxID);
		}
		return;
	}

	if( roomNumber != fx->room_number ) {
		EffectNewRoom(fxID, roomNumber);
	}
	if( fx->object_number == ID_MISSILE_FLAME ) {
		if( ItemNearLara(&fx->pos, 350) ) {
			LaraItem->hitPoints -= 3;
			LaraItem->hit_status = 1;
			LaraBurn();
			return;
		}
	} else if( ItemNearLara(&fx->pos, 200) ) {
		if( fx->object_number == ID_MISSILE_KNIFE || fx->object_number == ID_MISSILE_HARPOON ) {
			LaraItem->hitPoints -= 50;
			fx->object_number = ID_BLOOD;
			PlaySoundEffect(317, &fx->pos, 0);
		}
		LaraItem->hit_status = 1;
		fx->pos.rotY = LaraItem->pos.rotY;
		fx->counter = 0;
		fx->speed = LaraItem->speed;
		fx->frame_number = 0;
	}

	if( fx->object_number == ID_MISSILE_HARPOON && CHK_ANY(RoomInfo[fx->room_number].flags, ROOM_UNDERWATER) ) {
		CreateBubble(&fx->pos, fx->room_number);
	} else if( fx->object_number == ID_MISSILE_FLAME && !fx->counter-- ) {
		AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 14, 11);
		PlaySoundEffect(305, &fx->pos, 0);
		KillEffect(fxID);
	} else if( fx->object_number == ID_MISSILE_KNIFE ) {
		fx->pos.rotZ += 30*PHD_DEGREE;
	}
}

/*
 * Inject function
 */
void Inject_Missile() {
	INJECT(0x00433090, ControlMissile);

//	INJECT(0x00433360, ShootAtLara);
//	INJECT(0x00433410, ExplodingDeath);
//	INJECT(0x004337A0, ControlBodyPart);
}
