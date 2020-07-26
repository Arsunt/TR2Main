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
#include "game/people.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/sound.h"
#include "game/sphere.h"
#include "specific/game.h"
#include "global/vars.h"

#ifdef FEATURE_VIDEOFX_IMPROVED
extern DWORD AlphaBlendMode;
#endif // FEATURE_VIDEOFX_IMPROVED

__int16 __cdecl GunShot(int x, int y, int z, __int16 speed, __int16 rotY, __int16 roomNumber) {
#ifdef FEATURE_VIDEOFX_IMPROVED
	if( AlphaBlendMode ) {
		__int16 fx_id = CreateEffect(roomNumber);
		if( fx_id >= 0) {
			FX_INFO *fx = &Effects[fx_id];
			fx->pos.x = x;
			fx->pos.y = y;
			fx->pos.z = z;
			fx->room_number = roomNumber;
			fx->counter = 4;
			fx->speed = 0x400;
			fx->frame_number = 0x200; // this is sprite scale
			fx->fallspeed = 0;
			fx->object_number = ID_GLOW;
			fx->shade = 0x800;
			// NOTE: Core's hacky way to store the sprite flags in the rotation fields
			DWORD flags = GLOW_GUNSHOT_COLOR;
			flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SHADE|SPR_SCALE|SPR_SEMITRANS|SPR_ABS;
			fx->pos.rotX=(UINT16)flags;
			fx->pos.rotY=(UINT16)(flags >> 16);
		}
	}
#endif // FEATURE_VIDEOFX_IMPROVED
	__int16 fx_id = CreateEffect(roomNumber);
	if( fx_id >= 0 ) {
		FX_INFO *fx = &Effects[fx_id];
		fx->pos.x = x;
		fx->pos.y = y;
		fx->pos.z = z;
		fx->room_number = roomNumber;
		fx->pos.rotZ = 0;
		fx->pos.rotX = 0;
		fx->pos.rotY = rotY;
		fx->counter = 3;
		fx->frame_number = 0;
		fx->object_number = ID_GUN_FLASH;
		fx->shade = 0x1000;
	}
	return fx_id;
}

__int16 __cdecl GunHit(int x, int y, int z, __int16 speed, __int16 rotY, __int16 roomNumber) {
#ifdef FEATURE_CHEAT
	GAME_VECTOR pos = {0, 0, 0, roomNumber, 0};
	GetJointAbsPosition(LaraItem, (PHD_VECTOR *)&pos, GetRandomControl() * 25 / 0x7FFF);
	if( Lara.water_status == LWS_Cheat ) {
		Richochet(&pos);
	} else {
		DoBloodSplat(pos.x, pos.y, pos.z, LaraItem->speed, LaraItem->pos.rotY, LaraItem->roomNumber);
		PlaySoundEffect(50, &LaraItem->pos, 0);
	}
#else // FEATURE_CHEAT
	PHD_VECTOR pos = {0, 0, 0};
	GetJointAbsPosition(LaraItem, &pos, GetRandomControl() * 25 / 0x7FFF);
	DoBloodSplat(pos.x, pos.y, pos.z, LaraItem->speed, LaraItem->pos.rotY, LaraItem->roomNumber);
	PlaySoundEffect(50, &LaraItem->pos, 0);
#endif // FEATURE_CHEAT
	return GunShot(x, y, z, speed, rotY, roomNumber);
}

__int16 __cdecl GunMiss(int x, int y, int z, __int16 speed, __int16 rotY, __int16 roomNumber) {
	GAME_VECTOR pos;
	pos.x = LaraItem->pos.x + (GetRandomDraw() - 0x4000) * 0x200 / 0x7FFF;
	pos.y = LaraItem->floor;
	pos.z = LaraItem->pos.z + (GetRandomDraw() - 0x4000) * 0x200 / 0x7FFF;
	pos.roomNumber = LaraItem->roomNumber;
	Richochet(&pos);
	return GunShot(x, y, z, speed, rotY, roomNumber);
}

/*
 * Inject function
 */
void Inject_People() {
//	INJECT(0x00435EB0, Targetable);
//	INJECT(0x00435F40, ControlGlow);
//	INJECT(0x00435F80, ControlGunShot);

	INJECT(0x00435FD0, GunShot);
	INJECT(0x00436040, GunHit);
	INJECT(0x00436100, GunMiss);

//	INJECT(0x004361B0, ShotLara);
//	INJECT(0x00436380, InitialiseCult1);
//	INJECT(0x004363D0, Cult1Control);
//	INJECT(0x00436800, InitialiseCult3);
//	INJECT(0x00436850, Cult3Control);
//	INJECT(0x00436DC0, Worker1Control);
//	INJECT(0x004371C0, Worker2Control);
//	INJECT(0x00437620, BanditControl);
//	INJECT(0x00437960, Bandit2Control);
//	INJECT(0x00437DA0, WinstonControl);
}
