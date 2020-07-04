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
#include "game/traps.h"
#include "game/control.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/sound.h"
#include "game/sphere.h"
#include "specific/game.h"
#include "global/vars.h"

void __cdecl FlameControl(__int16 fx_id) {
	FX_INFO *fx = &Effects[fx_id];
	if( --fx->frame_number <= Objects[ID_FLAME].nMeshes ) {
		fx->frame_number = 0;
	}
	if( fx->counter < 0 ) {
		fx->pos.x = 0;
		fx->pos.y = 0;
		fx->pos.z = ( fx->counter == -1 ) ? -100 : 0;
		GetJointAbsPosition(LaraItem, (PHD_VECTOR *)&fx->pos, -1 - fx->counter);
		if( LaraItem->roomNumber != fx->room_number ) {
			EffectNewRoom(fx_id, LaraItem->roomNumber);
		}
		int height = GetWaterHeight(fx->pos.x, fx->pos.y, fx->pos.z, fx->room_number);
		if( height != NO_HEIGHT && fx->pos.y > height ) {
			fx->counter = 0;
			KillEffect(fx_id);
			Lara.burn = 0;
		} else {
			PlaySoundEffect(150, &fx->pos, 0);
			LaraItem->hitPoints -= 7;
			LaraItem->hit_status = 1;
		}
	} else {
		PlaySoundEffect(150, &fx->pos, 0);
		if( fx->counter ) {
			--fx->counter;
		} else if( ItemNearLara(&fx->pos, 600) ) {
			LaraItem->hitPoints -= 5;
			LaraItem->hit_status = 1;
			int dx = LaraItem->pos.x - fx->pos.x;
			int dz = LaraItem->pos.z - fx->pos.z;
			if( SQR(dx) + SQR(dz) < SQR(450) ) {
				fx->counter = 100;
				LaraBurn();
			}
		}
	}
}

void __cdecl LaraBurn() {
	if( Lara.burn ) {
		return;
	}

	__int16 fx_id = CreateEffect(LaraItem->roomNumber);
	if( fx_id < 0 ) {
		return;
	}
	FX_INFO *fx = &Effects[fx_id];
	fx->object_number = ID_FLAME;
	fx->frame_number = 0;
	fx->counter = -1;
	Lara.burn = 1;
}

void __cdecl LavaBurn(ITEM_INFO *item) {
	if( item->hitPoints < 0 ) {
		return;
	}

	__int16 room_number = item->roomNumber;
	FLOOR_INFO *floor = GetFloor(item->pos.x, 32000, item->pos.z, &room_number);
	if( item->floor != GetHeight(floor, item->pos.x, 32000, item->pos.z) ) {
		return;
	}

	item->hit_status = 1;
	item->hitPoints = -1;
	for( int i = 0; i < 10; ++i ) {
		__int16 fx_id = CreateEffect(item->roomNumber);
		if( fx_id < 0 ) continue;
		FX_INFO *fx = &Effects[fx_id];
		fx->object_number = ID_FLAME;
		fx->frame_number = Objects[ID_FLAME].nMeshes * GetRandomControl() / 0x7FFF;
		fx->counter = -1 - 24 * GetRandomControl() / 0x7FFF;
	}
}

/*
 * Inject function
 */
void Inject_Traps() {
//	INJECT(0x00440FC0, MineControl);
//	INJECT(0x004411C0, ControlSpikeWall);
//	INJECT(0x00441300, ControlCeilingSpikes);
//	INJECT(0x00441420, HookControl);
//	INJECT(0x004414B0, PropellerControl);
//	INJECT(0x00441640, SpinningBlade);
//	INJECT(0x004417C0, IcicleControl);
//	INJECT(0x004418C0, InitialiseBlade);
//	INJECT(0x00441900, BladeControl);
//	INJECT(0x004419A0, InitialiseKillerStatue);
//	INJECT(0x004419F0, KillerStatueControl);
//	INJECT(0x00441B00, SpringBoardControl);
//	INJECT(0x00441BE0, InitialiseRollingBall);
//	INJECT(0x00441C20, RollingBallControl);
//	INJECT(0x00441F70, RollingBallCollision);
//	INJECT(0x004421C0, SpikeCollision);
//	INJECT(0x00442320, TrapDoorControl);
//	INJECT(0x00442370, TrapDoorFloor);
//	INJECT(0x004423B0, TrapDoorCeiling);
//	INJECT(0x004423F0, OnTrapDoor);
//	INJECT(0x004424A0, Pendulum);
//	INJECT(0x004425B0, FallingBlock);
//	INJECT(0x004426C0, FallingBlockFloor);
//	INJECT(0x00442700, FallingBlockCeiling);
//	INJECT(0x00442750, TeethTrap);
//	INJECT(0x00442810, FallingCeiling);
//	INJECT(0x004428F0, DartEmitterControl);
//	INJECT(0x00442A30, DartsControl);
//	INJECT(0x00442B90, DartEffectControl);
//	INJECT(0x00442BE0, FlameEmitterControl);

	INJECT(0x00442C70, FlameControl);
	INJECT(0x00442DE0, LaraBurn);
	INJECT(0x00442E30, LavaBurn);

//	INJECT(0x00442F20, LavaSpray);
//	INJECT(0x00442FF0, ControlLavaBlob);
}
