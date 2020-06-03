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
#include "game/traps.h"
#include "game/control.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/sound.h"
#include "game/sphere.h"
#include "global/vars.h"

void __cdecl FlameEmitterControl(__int16 item_id) {
	ITEM_INFO *item;
	__int16 fxID;
	FX_INFO *fx;

	item = &Items[item_id];
	if (TriggerActive(item)) {
		if (!item->data) {
			fxID = CreateEffect(item->roomNumber);
			if (fxID != -1) {
				fx = &Effects[fxID];
				fx->pos.x = item->pos.x;
				fx->pos.y = item->pos.y;
				fx->pos.z = item->pos.z;
				fx->frame_number = 0;
				fx->object_number = ID_FLAME;
				fx->counter = 0;
			}
			item->data = (LPVOID) (fxID + 1);
		}
	} else {
		if (item->data) {
			KillEffect((int) item->data - 1);
			item->data = (LPVOID) 0;
		}
	}
}

void __cdecl FlameControl(__int16 fx_id) {
	int y;
	FX_INFO *fx;

	fx = &Effects[fx_id];
	--fx->frame_number;
	if (fx->frame_number <= Objects[ID_FLAME].nMeshes)
		fx->frame_number = 0;
	if (fx->counter < 0) {
		fx->pos.x = 0;
		fx->pos.y = 0;
		if (fx->counter == -1) {
			fx->pos.z = -100;
		} else {
			fx->pos.z = 0;
		}
		GetJointAbsPosition(LaraItem, (PHD_VECTOR *) &fx->pos, -1 - fx->counter);
		if (LaraItem->roomNumber != fx->room_number)
			EffectNewRoom(fx_id, LaraItem->roomNumber);
		y = GetWaterHeight(fx->pos.x, fx->pos.y, fx->pos.z, fx->room_number);
		if (y != -32512 && fx->pos.y > y) {
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
		if (fx->counter) {
			--fx->counter;
		} else {
			if (ItemNearLara(&fx->pos, 600)) {
				LaraItem->hitPoints -= 5;
				LaraItem->hit_status = 1;
				if (SQR(LaraItem->pos.x - fx->pos.x) + SQR(LaraItem->pos.z - fx->pos.z) < SQR(450)) {
					fx->counter = 100;
					LaraBurn();
				}
			}
		}
	}
}

void __cdecl LaraBurn() {
	__int16 fxID;
	FX_INFO *fx;

	if (!Lara.burn) {
		fxID = CreateEffect(LaraItem->roomNumber);
		if (fxID != -1) {
			fx = &Effects[fxID];
			fx->frame_number = 0;
			fx->object_number = ID_FLAME;
			fx->counter = -1;
			Lara.burn = 1;
		}
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

	INJECT(0x00442BE0, FlameEmitterControl);
	INJECT(0x00442C70, FlameControl);
	INJECT(0x00442DE0, LaraBurn);

//	INJECT(0x00442E30, LavaBurn);
//	INJECT(0x00442F20, LavaSpray);
//	INJECT(0x00442FF0, ControlLavaBlob);
}
