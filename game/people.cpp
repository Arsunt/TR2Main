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
#include "3dsystem/phd_math.h"
#include "game/box.h"
#include "game/control.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/larafire.h"
#include "game/sound.h"
#include "game/sphere.h"
#include "specific/game.h"
#include "global/vars.h"

#ifdef FEATURE_VIDEOFX_IMPROVED
extern DWORD AlphaBlendMode;
#endif // FEATURE_VIDEOFX_IMPROVED

int __cdecl Targetable(ITEM_INFO *item, AI_INFO *info) {
	CREATURE_INFO *creature;
	ITEM_INFO *enemy;
	GAME_VECTOR source, destination;

	creature = (CREATURE_INFO *) item->data;
	enemy = creature->enemy;
	if (enemy->hitPoints > 0 && info->ahead && info->distance < SQR(8192)) {
		source.x = item->pos.x;
		source.y = item->pos.y - 768;
		source.z = item->pos.z;
		source.roomNumber = item->roomNumber;
		destination.x = enemy->pos.x;
		destination.y = enemy->pos.y - 768;
		destination.z = enemy->pos.z;
		return LOS(&source, &destination);
	}
	return 0;
}

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
	PHD_VECTOR pos = {0, 0, 0};
	GetJointAbsPosition(LaraItem, &pos, GetRandomControl() * 25 / 0x7FFF);
#ifdef FEATURE_CHEAT
	if( Lara.water_status == LWS_Cheat ) {
		__int16 fxID = CreateEffect(roomNumber);
		if( fxID >= 0 ) {
			FX_INFO *fx = &Effects[fxID];
			fx->pos.x = pos.x;
			fx->pos.y = pos.y;
			fx->pos.z = pos.z;
			fx->counter = 4;
			fx->object_number = ID_RICOCHET;
			fx->pos.rotY = LaraItem->pos.rotY;
			fx->speed = LaraItem->speed;
			fx->frame_number = -3 * GetRandomDraw() / 0x8000;
		}
		PlaySoundEffect(10, &LaraItem->pos, 0);
	} else {
		DoBloodSplat(pos.x, pos.y, pos.z, LaraItem->speed, LaraItem->pos.rotY, LaraItem->roomNumber);
		PlaySoundEffect(50, &LaraItem->pos, 0);
	}
#else // FEATURE_CHEAT
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

int __cdecl ShotLara(ITEM_INFO *item, AI_INFO *info, BITE_INFO *bite, __int16 rotation, int damage) {
	CREATURE_INFO *creature;
	ITEM_INFO *enemy;
	int val;
	BOOL hit, shot;
	__int16 fxID, itemID;
	GAME_VECTOR source, destination;

	creature = (CREATURE_INFO *) item->data;
	enemy = creature->enemy;
	if (info->distance <= SQR(8192) && Targetable(item, info)) {
		val = SQR(8192) * (enemy->speed * phd_sin(info->enemy_facing) >> W2V_SHIFT) / 300;
		if (SQR(val) + info->distance > SQR(8192)) {
			hit = FALSE;
		} else {
			hit = GetRandomControl() < (SQR(8192) - info->distance) / 3276 + 8192;
		}
		shot = TRUE;
	} else {
		shot = FALSE;
		hit = FALSE;
	}
	fxID = -1;
	if (enemy == LaraItem) {
		if (hit) {
			fxID = CreatureEffect(item, bite, GunHit);
			LaraItem->hitPoints -= damage;
			LaraItem->hit_status = 1;
		} else {
			if (shot)
				fxID = CreatureEffect(item, bite, GunMiss);
		}
	} else {
		fxID = CreatureEffect(item, bite, GunShot);
		if (hit) {
			enemy->hitPoints -= damage / 10;
			enemy->hit_status = 1;
		}
	}
	if (fxID != -1)
		Effects[fxID].pos.rotY += rotation;
	source.x = item->pos.x;
	source.y = item->pos.y - 768;
	source.z = item->pos.z;
	source.roomNumber = item->roomNumber;
	destination.x = enemy->pos.x;
	destination.y = enemy->pos.y - 768;
	destination.z = enemy->pos.z;
	itemID = ObjectOnLOS(&source, &destination);
	if (itemID != -1)
		SmashItem(itemID, LGT_Unarmed);
	return shot;
}

void __cdecl WinstonControl(__int16 itemID) {
	ITEM_INFO *item;
	CREATURE_INFO *winston;
	AI_INFO info;
	__int16 angle;

	if (CreatureActive(itemID)) {
		item = &Items[itemID];
		winston = (CREATURE_INFO *) item->data;
		CreatureAIInfo(item, &info);
		CreatureMood(item, &info, TRUE);
		angle = CreatureTurn(item, winston->maximum_turn);
		if (item->currentAnimState == 1) {
			if ((info.distance > SQR(1536) || !info.ahead) && item->goalAnimState != 2) {
				item->goalAnimState = 2;
				PlaySoundEffect(345, &item->pos, 0);
			}
		} else {
			if (info.distance <= SQR(1536)) {
				if (info.ahead) {
					item->goalAnimState = 1;
					if (CHK_ANY(winston->flags, 1))
						--winston->flags;
				} else {
					if (!CHK_ANY(winston->flags, 1)) {
						PlaySoundEffect(344, &item->pos, 0);
						PlaySoundEffect(347, &item->pos, 0);
						winston->flags |= 1;
					}
				}
			}
		}
		if (item->touchBits && !CHK_ANY(winston->flags, 2)) {
			PlaySoundEffect(346, &item->pos, 0);
			PlaySoundEffect(347, &item->pos, 0);
			winston->flags |= 2;
		} else {
			if (!item->touchBits && CHK_ANY(winston->flags, 2))
				winston->flags -= 2;
		}
		if (GetRandomDraw() < 256)
			PlaySoundEffect(347, &item->pos, 0);
		CreatureAnimation(itemID, angle, 0);
	}
}

/*
 * Inject function
 */
void Inject_People() {
	INJECT(0x00435EB0, Targetable);

//	INJECT(0x00435F40, ControlGlow);
//	INJECT(0x00435F80, ControlGunShot);

	INJECT(0x00435FD0, GunShot);
	INJECT(0x00436040, GunHit);
	INJECT(0x00436100, GunMiss);
	INJECT(0x004361B0, ShotLara);

//	INJECT(0x00436380, InitialiseCult1);
//	INJECT(0x004363D0, Cult1Control);
//	INJECT(0x00436800, InitialiseCult3);
//	INJECT(0x00436850, Cult3Control);
//	INJECT(0x00436DC0, Worker1Control);
//	INJECT(0x004371C0, Worker2Control);
//	INJECT(0x00437620, BanditControl);
//	INJECT(0x00437960, Bandit2Control);

	INJECT(0x00437DA0, WinstonControl);
}
