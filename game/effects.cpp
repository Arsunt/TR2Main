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
#include "game/effects.h"
#include "3dsystem/3d_gen.h"
#include "3dsystem/phd_math.h"
#include "game/control.h"
#include "game/draw.h"
#include "game/hair.h"
#include "game/invtext.h"
#include "game/items.h"
#include "game/sound.h"
#include "game/sphere.h"
#include "specific/game.h"
#include "specific/output.h"
#include "specific/sndpc.h"
#include "global/vars.h"

int __cdecl ItemNearLara(PHD_3DPOS *pos, int distance) {
	int dx, dy, dz;
	__int16 *frame;

	dx = pos->x - LaraItem->pos.x;
	dy = pos->y - LaraItem->pos.y;
	dz = pos->z - LaraItem->pos.z;
	if (ABS(dx) <= distance && ABS(dz) <= distance && ABS(dy) <= 3072 && SQR(dx) + SQR(dz) <= SQR(distance)) {
		frame = GetBoundsAccurate(LaraItem);
		if (dy >= frame[2] && dy <= frame[3] + 100)
			return 1;
	}
	return 0;
}

void __cdecl SoundEffects() {
	DWORD i;

	for (i = 0; i < SoundFxCount; ++i) {
		if ((FlipStatus && CHK_ANY(SoundFx[i].flags, 0x40)) || (!FlipStatus && CHK_ANY(SoundFx[i].flags, 0x80)))
			PlaySoundEffect(SoundFx[i].data, (PHD_3DPOS *) &SoundFx[i].x, 0);
	}
	if (FlipEffect != -1)
		(*SfxFunctions[FlipEffect])(NULL);
	SOUND_EndScene();
}

__int16 __cdecl DoBloodSplat(int x, int y, int z, __int16 speed, __int16 direction, __int16 roomID) {
	__int16 fxID;
	FX_INFO *fx;

	fxID = CreateEffect(roomID);
	if (fxID != -1) {
		fx = &Effects[fxID];
		fx->pos.x = x;
		fx->pos.y = y;
		fx->pos.z = z;
		fx->pos.rotY = direction;
		fx->speed = speed;
		fx->frame_number = 0;
		fx->object_number = ID_BLOOD;
		fx->counter = 0;
	}
	return fxID;
}

void __cdecl DoLotsOfBlood(int x, int y, int z, __int16 speed, __int16 direction, __int16 roomID, int number) {
	int i;

	for (i = 0; i < number; ++i)
		DoBloodSplat(x - 512 * GetRandomDraw() / 32768 + 256,
					y - 512 * GetRandomDraw() / 32768 + 256,
					z - 512 * GetRandomDraw() / 32768 + 256,
					speed,
					direction,
					roomID);
}

void __cdecl ControlBlood1(__int16 fxID) {
	FX_INFO *fx;

	fx = &Effects[fxID];
	fx->pos.x += fx->speed * phd_sin(fx->pos.rotY) >> W2V_SHIFT;
	fx->pos.z += fx->speed * phd_cos(fx->pos.rotY) >> W2V_SHIFT;
	++fx->counter;
	if (fx->counter == 4) {
		--fx->frame_number;
		fx->counter = 0;
		if (fx->frame_number <= Objects[fx->object_number].nMeshes)
			KillEffect(fxID);
	}
}

void __cdecl ControlExplosion1(__int16 fxID) {
	FX_INFO *fx;

	fx = &Effects[fxID];
	++fx->counter;
	if (fx->counter == 2) {
		--fx->frame_number;
		fx->counter = 0;
		if (fx->frame_number <= Objects[fx->object_number].nMeshes) {
			KillEffect(fxID);
		} else {
			AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 13, 11);
		}
	} else {
		AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 12, 10);
	}
}

void __cdecl Richochet(GAME_VECTOR *pos) {
	__int16 fxID;
	FX_INFO *fx;

	fxID = CreateEffect(pos->roomNumber);
	if (fxID != -1) {
		fx = &Effects[fxID];
		fx->pos.x = pos->x;
		fx->pos.y = pos->y;
		fx->pos.z = pos->z;
		fx->counter = 4;
		fx->object_number = ID_RICOCHET;
		fx->frame_number = -3 * GetRandomDraw() / 32768;
		PlaySoundEffect(10, &fx->pos, 0);
	}
}

void __cdecl ControlRichochet1(__int16 fxID) {
	FX_INFO *fx;

	fx = &Effects[fxID];
	--fx->counter;
	if (!fx->counter)
		KillEffect(fxID);
}

void __cdecl CreateBubble(PHD_3DPOS *pos, __int16 roomID) {
	__int16 fxID;
	FX_INFO *fx;

	fxID = CreateEffect(roomID);
	if (fxID != -1) {
		fx = &Effects[fxID];
		fx->pos.x = pos->x;
		fx->pos.y = pos->y;
		fx->pos.z = pos->z;
		fx->speed = (6 * GetRandomDraw() >> 15) + 10;
		fx->object_number = ID_BUBBLES;
		fx->frame_number = -(3 * GetRandomDraw() >> 15);
	}
}

void __cdecl LaraBubbles(ITEM_INFO *item) {
	int i, count;
	PHD_VECTOR pos;

	count = 3 * GetRandomDraw() / 32768;
	if (count) {
		PlaySoundEffect(37, &item->pos, SFX_UNDERWATER);
		pos.x = 0;
		pos.y = 0;
		pos.z = 50;
		GetJointAbsPosition(item, &pos, 14);
		for (i = 0; i < count; ++i)
			CreateBubble((PHD_3DPOS *) &pos, item->roomNumber);
	}
}

void __cdecl ControlBubble1(__int16 fxID) {
	FX_INFO *fx;
	int x, y, z, ceiling;
	__int16 roomID;
	FLOOR_INFO *floor;

	fx = &Effects[fxID];
	fx->pos.rotY += 9 * PHD_DEGREE;
	fx->pos.rotX += 13 * PHD_DEGREE;
	x = fx->pos.x + (11 * phd_sin(fx->pos.rotY) >> W2V_SHIFT);
	y = fx->pos.y - fx->speed;
	z = fx->pos.z + (8 * phd_cos(fx->pos.rotX) >> W2V_SHIFT);
	roomID = fx->room_number;
	floor = GetFloor(x, y, z, &roomID);
	if (floor && CHK_ANY(RoomInfo[roomID].flags, ROOM_UNDERWATER)) {
		ceiling = GetCeiling(floor, x, y, z);
		if (ceiling != -32512 && y > ceiling) {
			if (fx->room_number != roomID)
				EffectNewRoom(fxID, roomID);
			fx->pos.x = x;
			fx->pos.y = y;
			fx->pos.z = z;
			return;
		}
	}
	KillEffect(fxID);
}

void __cdecl Splash(ITEM_INFO *item) {
	int y, i;
	__int16 roomID, fxID;
	FX_INFO *fx;

	y = GetWaterHeight(item->pos.x, item->pos.y, item->pos.z, item->roomNumber);
	roomID = item->roomNumber;
	GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomID);
	for (i = 0; i < 10; ++i) {
		fxID = CreateEffect(roomID);
		if (fxID != -1) {
			fx = &Effects[fxID];
			fx->pos.x = item->pos.x;
			fx->pos.y = y;
			fx->pos.z = item->pos.z;
			fx->pos.rotY = 2 * GetRandomDraw() - 32768;
			fx->frame_number = 0;
			fx->object_number = ID_SPLASH;
			fx->speed = GetRandomDraw() / 256;
		}
	}
}

void __cdecl ControlSplash1(__int16 fxID) {
	FX_INFO *fx;

	fx = &Effects[fxID];
	--fx->frame_number;
	if (fx->frame_number <= Objects[fx->object_number].nMeshes) {
		KillEffect(fxID);
	} else {
		fx->pos.z += fx->speed * phd_cos(fx->pos.rotY) >> W2V_SHIFT;
		fx->pos.x += fx->speed * phd_sin(fx->pos.rotY) >> W2V_SHIFT;
	}
}

void __cdecl ControlWaterSprite(__int16 fxID) {
	FX_INFO *fx;

	fx = &Effects[fxID];
	--fx->counter;
	if (!CHK_ANY(fx->counter, 3)) {
		--fx->frame_number;
		if (fx->frame_number <= Objects[fx->object_number].nMeshes)
			fx->frame_number = 0;
	}
	if (fx->counter && fx->fallspeed <= 0) {
		fx->pos.z += fx->speed * phd_cos(fx->pos.rotY) >> W2V_SHIFT;
		fx->pos.x += fx->speed * phd_sin(fx->pos.rotY) >> W2V_SHIFT;
		if (fx->fallspeed) {
			fx->pos.y += fx->fallspeed;
			fx->fallspeed += 6;
		}
	} else {
		KillEffect(fxID);
	}
}

void __cdecl ControlSnowSprite(__int16 fxID) {
	FX_INFO *fx;

	fx = &Effects[fxID];
	--fx->frame_number;
	if (fx->frame_number <= Objects[fx->object_number].nMeshes) {
		KillEffect(fxID);
	} else {
		fx->pos.z += fx->speed * phd_cos(fx->pos.rotY) >> W2V_SHIFT;
		fx->pos.x += fx->speed * phd_sin(fx->pos.rotY) >> W2V_SHIFT;
		if (fx->fallspeed) {
			fx->pos.y += fx->fallspeed;
			fx->fallspeed += 6;
		}
	}
}

void __cdecl ControlHotLiquid(__int16 fxID) {
	FX_INFO *fx;
	__int16 roomID;
	int height;

	fx = &Effects[fxID];
	--fx->frame_number;
	if (fx->frame_number <= Objects[ID_HOT_LIQUID].nMeshes)
		fx->frame_number = 0;
	fx->pos.y += fx->fallspeed;
	fx->fallspeed += 6;
	roomID = fx->room_number;
	height = GetHeight(GetFloor(fx->pos.x, fx->pos.y, fx->pos.z, &roomID), fx->pos.x, fx->pos.y, fx->pos.z);
	if (fx->pos.y >= height) {
		PlaySoundEffect(285, &fx->pos, 0);
		fx->object_number = ID_SPLASH;
		fx->pos.y = height;
		fx->pos.rotY = 2 * GetRandomDraw();
		fx->fallspeed = 0;
		fx->speed = 50;
	} else {
		if (fx->room_number != roomID)
			EffectNewRoom(fxID, roomID);
		PlaySoundEffect(284, &fx->pos, 0);
	}
}

void __cdecl WaterFall(__int16 itemID) {
	ITEM_INFO *item;
	__int16 fxID;
	FX_INFO *fx;

	item = &Items[itemID];
	if (ABS(item->pos.x - LaraItem->pos.x) <= 10240 &&
		ABS(item->pos.z - LaraItem->pos.z) <= 10240 &&
		ABS(item->pos.y - LaraItem->pos.y) <= 10240)
	{
		S_CalculateLight(item->pos.x, item->pos.y, item->pos.z, item->roomNumber);
		fxID = CreateEffect(item->roomNumber);
		if (fxID != -1) {
			fx = &Effects[fxID];
			fx->pos.x = item->pos.x + 1024 * (GetRandomDraw() - 16384) / 32767;
			fx->pos.z = item->pos.z + 1024 * (GetRandomDraw() - 16384) / 32767;
			fx->pos.y = item->pos.y;
			fx->speed = 0;
			fx->frame_number = 0;
			fx->shade = LsAdder;
			fx->object_number = ID_SPLASH;
		}
		PlaySoundEffect(79, &item->pos, 0);
	}
}

void __cdecl finish_level_effect(ITEM_INFO *item) {
	IsLevelComplete = TRUE;
}

void __cdecl turn180_effect(ITEM_INFO *item) {
	item->pos.rotX = -item->pos.rotX;
	item->pos.rotY += PHD_180;
}

void __cdecl floor_shake_effect(ITEM_INFO *item) {
	int dx, dy, dz;

	dx = item->pos.x - Camera.pos.x;
	dy = item->pos.y - Camera.pos.y;
	dz = item->pos.z - Camera.pos.z;
	if (ABS(dx) < 16384 && ABS(dy) < 16384 && ABS(dz) < 16384)
		Camera.bounce = 100 * (SQR(1024) - (SQR(dx) + SQR(dy) + SQR(dz)) / 256) / SQR(1024);
}

void __cdecl lara_normal_effect(ITEM_INFO *item) {
	item->currentAnimState = AS_STOP;
	item->goalAnimState = AS_STOP;
	item->animNumber = 11;
	item->frameNumber = Anims[item->animNumber].frameBase;
	Camera.type = CAM_Chase;
	AlterFOV(80 * PHD_DEGREE);
}

void __cdecl BoilerFX(ITEM_INFO *item) {
	PlaySoundEffect(338, NULL, 0);
	FlipEffect = -1;
}

void __cdecl FloodFX(ITEM_INFO *item) {
	PHD_3DPOS pos;

	if (FlipTimer > 120) {
		FlipEffect = -1;
	} else {
		pos.x = LaraItem->pos.x;
		if (FlipTimer < 30) {
			pos.y = 100 * (30 - FlipTimer) + Camera.target.y;
		} else {
			pos.y = 100 * (FlipTimer - 30) + Camera.target.y;
		}
		pos.z = LaraItem->pos.z;
		PlaySoundEffect(79, &pos, 0);
	}
	++FlipTimer;
}

void __cdecl RubbleFX(ITEM_INFO *item) {
	PlaySoundEffect(24, NULL, 0);
	Camera.bounce = -350;
	FlipEffect = -1;
}

void __cdecl ChandelierFX(ITEM_INFO *item) {
	PlaySoundEffect(278, NULL, 0);
	++FlipTimer;
	if (FlipTimer > 30)
		FlipEffect = -1;
}

void __cdecl ExplosionFX(ITEM_INFO *item) {
	PlaySoundEffect(105, NULL, 0);
	Camera.bounce = -75;
	FlipEffect = -1;
}

void __cdecl PistonFX(ITEM_INFO *item) {
	PlaySoundEffect(190, NULL, 0);
	FlipEffect = -1;
}

void __cdecl CurtainFX(ITEM_INFO *item) {
	PlaySoundEffect(191, NULL, 0);
	FlipEffect = -1;
}

void __cdecl StatueFX(ITEM_INFO *item) {
	PlaySoundEffect(331, NULL, 0);
	FlipEffect = -1;
}

void __cdecl SetChangeFX(ITEM_INFO *item) {
	PlaySoundEffect(330, NULL, 0);
	FlipEffect = -1;
}

void __cdecl ControlDingDong(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (CHK_ALL(item->flags, IFL_CODEBITS)) {
		PlaySoundEffect(334, &item->pos, 0);
		item->flags -= IFL_CODEBITS;
	}
}

void __cdecl ControlLaraAlarm(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (CHK_ALL(item->flags, IFL_CODEBITS))
		PlaySoundEffect(335, &item->pos, 0);
}

void __cdecl ControlAlarmSound(__int16 itemID) {
	ITEM_INFO *item;
	int counter;

	item = &Items[itemID];
	if (CHK_ALL(item->flags, IFL_CODEBITS)) {
		PlaySoundEffect(332, &item->pos, 0);
		counter = (int) item->data + 1;
		if (counter > 6) {
			AddDynamicLight(item->pos.x, item->pos.y, item->pos.z, 12, 11);
			if (counter > 12)
				counter = 0;
		}
		item->data = (LPVOID) counter;
	}
}

void __cdecl ControlBirdTweeter(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (item->objectID == ID_BIRD_TWEETER2) {
		if (GetRandomDraw() < 1024)
			PlaySoundEffect(316, &item->pos, 0);
	} else {
		if (GetRandomDraw() < 256)
			PlaySoundEffect(329, &item->pos, 0);
	}
}

void __cdecl DoChimeSound(ITEM_INFO *item) {
	PHD_3DPOS pos;

	pos.x = LaraItem->pos.x + ((item->pos.x - LaraItem->pos.x) >> 6);
	pos.y = LaraItem->pos.y + ((item->pos.y - LaraItem->pos.y) >> 6);
	pos.z = LaraItem->pos.z + ((item->pos.z - LaraItem->pos.z) >> 6);
	PlaySoundEffect(208, &pos, 0);
}

void __cdecl ControlClockChimes(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (item->timer) {
		if (item->timer % 60 == 59)
			DoChimeSound(item);
		--item->timer;
		if (!item->timer) {
			DoChimeSound(item);
			item->timer = -1;
			RemoveActiveItem(itemID);
			item->status = ITEM_INACTIVE;
			item->flags &= ~IFL_CODEBITS;
		}
	}
}

void __cdecl SphereOfDoomCollision(__int16 itemID, ITEM_INFO *laraItem, COLL_INFO *coll) {
	ITEM_INFO *item;
	int dx, dz, distance, angle;

	if (!CHK_ANY(RoomInfo[laraItem->roomNumber].flags, ROOM_UNDERWATER)) {
		item = &Items[itemID];
		dx = laraItem->pos.x - item->pos.x;
		dz = laraItem->pos.z - item->pos.z;
		distance = 5 * item->timer >> 1;
		if (SQR(dx) + SQR(dz) < SQR(distance)) {
			angle = phd_atan(dz, dx);
			if (ABS(laraItem->pos.rotY - angle) < PHD_90) {
				laraItem->speed = 150;
				laraItem->pos.rotY = angle;
			} else {
				laraItem->speed = -150;
				laraItem->pos.rotY = angle + PHD_180;
			}
			laraItem->gravity = 1;
			laraItem->fallSpeed = -50;
			laraItem->pos.z = item->pos.z + (phd_cos(angle) * (distance + 50) >> W2V_SHIFT);
			laraItem->pos.x = item->pos.x + (phd_sin(angle) * (distance + 50) >> W2V_SHIFT);
			laraItem->pos.rotZ = 0;
			laraItem->pos.rotX = 0;
			LaraItem->animNumber = 34;
			LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = AS_FORWARDJUMP;
			LaraItem->goalAnimState = AS_FORWARDJUMP;
		}
	}
}

void __cdecl SphereOfDoom(__int16 itemID) {
	ITEM_INFO *item;
	int dx, dy, dz, distance, difference;
	PHD_3DPOS pos;

	item = &Items[itemID];
	item->timer += 64;
	item->pos.rotY += item->objectID == ID_SPHERE_OF_DOOM2 ? 10 * PHD_DEGREE : -10 * PHD_DEGREE;
	dx = item->pos.x - LaraItem->pos.x;
	dy = item->pos.y - LaraItem->pos.y;
	dz = item->pos.z - LaraItem->pos.z;
	distance = 5 * item->timer >> 1;
	difference = phd_sqrt(SQR(dx) + SQR(dy) + SQR(dz)) - distance;
	pos.x = LaraItem->pos.x + difference * dx / distance;
	pos.y = LaraItem->pos.y + difference * dy / distance;
	pos.z = LaraItem->pos.z + difference * dz / distance;
	PlaySoundEffect(341, &pos, 0);
	if (item->timer > 3840)
		KillItem(itemID);
}

void __cdecl DrawSphereOfDoom(ITEM_INFO *item) {
	int clip;

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
	phd_RotY(item->pos.rotY);
	PhdMatrixPtr->_00 = item->timer * PhdMatrixPtr->_00 >> 8;
	PhdMatrixPtr->_01 = item->timer * PhdMatrixPtr->_01 >> 8;
	PhdMatrixPtr->_02 = item->timer * PhdMatrixPtr->_02 >> 8;
	PhdMatrixPtr->_10 = item->timer * PhdMatrixPtr->_10 >> 8;
	PhdMatrixPtr->_11 = item->timer * PhdMatrixPtr->_11 >> 8;
	PhdMatrixPtr->_12 = item->timer * PhdMatrixPtr->_12 >> 8;
	PhdMatrixPtr->_20 = item->timer * PhdMatrixPtr->_20 >> 8;
	PhdMatrixPtr->_21 = item->timer * PhdMatrixPtr->_21 >> 8;
	PhdMatrixPtr->_22 = item->timer * PhdMatrixPtr->_22 >> 8;
	clip = S_GetObjectBounds(Anims[item->animNumber].framePtr);
	if (clip) {
		CalculateObjectLighting(item, Anims[item->animNumber].framePtr);
		phd_PutPolygons(MeshPtr[Objects[item->objectID].meshIndex], clip);
	}
	phd_PopMatrix();
}

void __cdecl lara_hands_free(ITEM_INFO *item) {
	Lara.gun_status = LGS_Armless;
}

void __cdecl flip_map_effect(ITEM_INFO *item) {
	FlipMap();
}

void __cdecl draw_right_gun(ITEM_INFO *item) {
	__int16 *tmp;

	SWAP(Lara.mesh_ptrs[4], MeshPtr[Objects[ID_LARA_PISTOLS].meshIndex + 4], tmp);
	SWAP(Lara.mesh_ptrs[10], MeshPtr[Objects[ID_LARA_PISTOLS].meshIndex + 10], tmp);
}

void __cdecl draw_left_gun(ITEM_INFO *item) {
	__int16 *tmp;

	SWAP(Lara.mesh_ptrs[1], MeshPtr[Objects[ID_LARA_PISTOLS].meshIndex + 1], tmp);
	SWAP(Lara.mesh_ptrs[13], MeshPtr[Objects[ID_LARA_PISTOLS].meshIndex + 13], tmp);
}

void __cdecl swap_meshes_with_meshswap1(ITEM_INFO *item) {
	int i;
	__int16 *tmp;

	for (i = 0; i < Objects[item->objectID].nMeshes; ++i)
		SWAP(MeshPtr[Objects[item->objectID].meshIndex + i], MeshPtr[Objects[ID_WORKER_RESERVED1].meshIndex + i], tmp);
}

void __cdecl swap_meshes_with_meshswap2(ITEM_INFO *item) {
	int i;
	__int16 *tmp;

	for (i = 0; i < Objects[item->objectID].nMeshes; ++i)
		SWAP(MeshPtr[Objects[item->objectID].meshIndex + i], MeshPtr[Objects[ID_WORKER_RESERVED2].meshIndex + i], tmp);
}

void __cdecl swap_meshes_with_meshswap3(ITEM_INFO *item) {
	int i;
	__int16 *tmp;

	for (i = 0; i < Objects[item->objectID].nMeshes; ++i) {
		if (item == LaraItem)
			Lara.mesh_ptrs[i] = MeshPtr[Objects[ID_LARA_SWAP].meshIndex + i];
		SWAP(MeshPtr[Objects[item->objectID].meshIndex + i], MeshPtr[Objects[ID_LARA_SWAP].meshIndex + i], tmp);
	}
}

void __cdecl invisibility_on(ITEM_INFO *item) {
	item->status = ITEM_INVISIBLE;
}

void __cdecl invisibility_off(ITEM_INFO *item) {
	item->status = ITEM_ACTIVE;
}

void __cdecl dynamic_light_on(ITEM_INFO *item) {
	item->dynamic_light = 1;
}

void __cdecl dynamic_light_off(ITEM_INFO *item) {
	item->dynamic_light = 0;
}

void __cdecl reset_hair(ITEM_INFO *item) {
	InitialiseHair();
}

void __cdecl AssaultStart(ITEM_INFO *item) {
	SaveGame.statistics.timer = 0;
	IsAssaultTimerActive = TRUE;
	IsAssaultTimerDisplay = TRUE;
	FlipEffect = -1;
}

void __cdecl AssaultStop(ITEM_INFO *item) {
	IsAssaultTimerActive = FALSE;
	IsAssaultTimerDisplay = TRUE;
	FlipEffect = -1;
}

void __cdecl AssaultReset(ITEM_INFO *item) {
	IsAssaultTimerActive = FALSE;
	IsAssaultTimerDisplay = FALSE;
	FlipEffect = -1;
}

void __cdecl AssaultFinished(ITEM_INFO *item) {
	if (IsAssaultTimerActive) {
		AddAssaultTime(SaveGame.statistics.timer);
		if (AssaultBestTime < 0) {
			if (SaveGame.statistics.timer < 3000) {
				S_CDPlay(22, FALSE);
				AssaultBestTime = SaveGame.statistics.timer;
			} else {
				S_CDPlay(24, FALSE);
				AssaultBestTime = 3000;
			}
		} else {
			if (SaveGame.statistics.timer < (DWORD) AssaultBestTime) {
				S_CDPlay(22, FALSE);
				AssaultBestTime = SaveGame.statistics.timer;
			} else {
				S_CDPlay(SaveGame.statistics.timer >= (DWORD) AssaultBestTime + 150 ? 21 : 23, FALSE);
			}
		}
		IsAssaultTimerActive = FALSE;
	}
	FlipEffect = -1;
}

/*
 * Inject function
 */
void Inject_Effects() {
	INJECT(0x0041C4B0, ItemNearLara);
	INJECT(0x0041C540, SoundEffects);
	INJECT(0x0041C5B0, DoBloodSplat);
	INJECT(0x0041C610, DoLotsOfBlood);
	INJECT(0x0041C6C0, ControlBlood1);
	INJECT(0x0041C750, ControlExplosion1);
	INJECT(0x0041C7D0, Richochet);
	INJECT(0x0041C850, ControlRichochet1);
	INJECT(0x0041C880, CreateBubble);
	INJECT(0x0041C8F0, LaraBubbles);
	INJECT(0x0041C970, ControlBubble1);
	INJECT(0x0041CA70, Splash);

//	INJECT(----------, WadeSplash);

	INJECT(0x0041CB40, ControlSplash1);
	INJECT(0x0041CBC0, ControlWaterSprite);
	INJECT(0x0041CC70, ControlSnowSprite);
	INJECT(0x0041CD00, ControlHotLiquid);
	INJECT(0x0041CDE0, WaterFall);
	INJECT(0x0041CF20, finish_level_effect);
	INJECT(0x0041CF30, turn180_effect);
	INJECT(0x0041CF50, floor_shake_effect);
	INJECT(0x0041CFF0, lara_normal_effect);
	INJECT(0x0041D030, BoilerFX);
	INJECT(0x0041D050, FloodFX);
	INJECT(0x0041D0E0, RubbleFX);
	INJECT(0x0041D110, ChandelierFX);
	INJECT(0x0041D140, ExplosionFX);
	INJECT(0x0041D170, PistonFX);
	INJECT(0x0041D190, CurtainFX);
	INJECT(0x0041D1B0, StatueFX);
	INJECT(0x0041D1D0, SetChangeFX);
	INJECT(0x0041D1F0, ControlDingDong);
	INJECT(0x0041D230, ControlLaraAlarm);
	INJECT(0x0041D270, ControlAlarmSound);
	INJECT(0x0041D2E0, ControlBirdTweeter);
	INJECT(0x0041D340, DoChimeSound);
	INJECT(0x0041D3A0, ControlClockChimes);
	INJECT(0x0041D410, SphereOfDoomCollision);
	INJECT(0x0041D540, SphereOfDoom);
	INJECT(0x0041D630, DrawSphereOfDoom);
	INJECT(0x0041D760, lara_hands_free);
	INJECT(0x0041D770, flip_map_effect);
	INJECT(0x0041D780, draw_right_gun);
	INJECT(0x0041D7D0, draw_left_gun);

//	INJECT(----------, shoot_right_gun);
//	INJECT(----------, shoot_left_gun);

	INJECT(0x0041D820, swap_meshes_with_meshswap1);
	INJECT(0x0041D890, swap_meshes_with_meshswap2);
	INJECT(0x0041D900, swap_meshes_with_meshswap3);
	INJECT(0x0041D9A0, invisibility_on);
	INJECT(0x0041D9B0, invisibility_off);
	INJECT(0x0041D9D0, dynamic_light_on);
	INJECT(0x0041D9E0, dynamic_light_off);
	INJECT(0x0041D9F0, reset_hair);
	INJECT(0x0041DA00, AssaultStart);
	INJECT(0x0041DA30, AssaultStop);
	INJECT(0x0041DA50, AssaultReset);
	INJECT(0x0041DA70, AssaultFinished);
}
