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
#include "game/items.h"
#include "global/vars.h"

#ifdef FEATURE_VIDEOFX_IMPROVED
extern DWORD AlphaBlendMode;
#endif // FEATURE_VIDEOFX_IMPROVED

__int16 __cdecl GunShot(int x, int y, int z, __int16 speed, __int16 rotY, __int16 room_number) {
#ifdef FEATURE_VIDEOFX_IMPROVED
	if( AlphaBlendMode ) {
		__int16 fx_id = CreateEffect(room_number);
		if( fx_id >= 0) {
			FX_INFO *fx = &Effects[fx_id];
			fx->pos.x = x;
			fx->pos.y = y;
			fx->pos.z = z;
			fx->room_number = room_number;
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
	__int16 fx_id = CreateEffect(room_number);
	if( fx_id >= 0 ) {
		FX_INFO *fx = &Effects[fx_id];
		fx->pos.x = x;
		fx->pos.y = y;
		fx->pos.z = z;
		fx->room_number = room_number;
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

/*
 * Inject function
 */
void Inject_People() {
//	INJECT(0x00435EB0, Targetable);
//	INJECT(0x00435F40, ControlGlow);
//	INJECT(0x00435F80, ControlGunShot);

	INJECT(0x00435FD0, GunShot);

//	INJECT(0x00436040, GunHit);
//	INJECT(0x00436100, GunMiss);
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
