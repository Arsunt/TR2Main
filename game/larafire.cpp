/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
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
#include "game/larafire.h"
#include "3dsystem/3d_gen.h"
#include "game/control.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/objects.h"
#include "game/sphere.h"
#include "specific/game.h"
#include "global/vars.h"

int __cdecl FireWeapon(int weaponType, ITEM_INFO *target, ITEM_INFO *item, __int16 *angles) {
	DWORD *ammo;
	WEAPON_INFO *weapon;
	PHD_3DPOS pos;
	SPHERE_INFO spheres[33];
	int count, best, distance, i, hit;
	GAME_VECTOR source, destination;
	__int16 itemID;
	BOOL ricochet;

	switch (weaponType) {
		case LGT_Magnums:
			ammo = &Lara.magnum_ammo;
			if (SaveGame.bonusFlag)
				*ammo = 1000;
			break;
		case LGT_Uzis:
			ammo = &Lara.uzi_ammo;
			if (SaveGame.bonusFlag)
				*ammo = 1000;
			break;
		case LGT_Shotgun:
			ammo = &Lara.shotgun_ammo;
			if (SaveGame.bonusFlag)
				*ammo = 1000;
			break;
		case LGT_M16:
			ammo = &Lara.m16_ammo;
			if (SaveGame.bonusFlag)
				*ammo = 1000;
			break;
		default:
			ammo = &Lara.pistol_ammo;
			*ammo = 1000;
			break;
	}
	if (*ammo > 0) {
		--*ammo;
		weapon = &Weapons[weaponType];
		pos.x = item->pos.x;
		pos.y = item->pos.y - weapon->gunHeight;
		pos.z = item->pos.z;
		pos.rotX = angles[1] + (GetRandomControl() - 16384) * weapon->shotAccuracy / 65536;
		pos.rotZ = 0;
		pos.rotY = angles[0] + (GetRandomControl() - 16384) * weapon->shotAccuracy / 65536;
		phd_GenerateW2V(&pos);
		count = GetSpheres(target, spheres, FALSE);
		best = -1;
		distance = 2147483647;
		for (i = 0; i < count; ++i) {
			if (ABS(spheres[i].x) < spheres[i].r &&
				ABS(spheres[i].y) < spheres[i].r &&
				spheres[i].z > spheres[i].r &&
				SQR(spheres[i].x) + SQR(spheres[i].y) <= SQR(spheres[i].r) &&
				spheres[i].z - spheres[i].r < distance)
			{
				distance = spheres[i].z - spheres[i].r;
				best = i;
			}
		}
		++SaveGame.statistics.shots;
		source.x = pos.x;
		source.y = pos.y;
		source.z = pos.z;
		source.roomNumber = item->roomNumber;
		if (best >= 0) {
			++SaveGame.statistics.hits;
			destination.x = pos.x + (PhdMatrixPtr->_20 * distance >> W2V_SHIFT);
			destination.y = pos.y + (PhdMatrixPtr->_21 * distance >> W2V_SHIFT);
			destination.z = pos.z + (PhdMatrixPtr->_22 * distance >> W2V_SHIFT);
			itemID = ObjectOnLOS(&source, &destination);
			if (itemID != -1)
				SmashItem(itemID, weaponType);
			HitTarget(target, &destination, weapon->damage);
			hit = 1;
		} else {
			destination.x = pos.x + (PhdMatrixPtr->_20 * weapon->targetDist >> W2V_SHIFT);
			destination.y = pos.y + (PhdMatrixPtr->_21 * weapon->targetDist >> W2V_SHIFT);
			destination.z = pos.z + (PhdMatrixPtr->_22 * weapon->targetDist >> W2V_SHIFT);
			ricochet = !LOS(&source, &destination);
			itemID = ObjectOnLOS(&source, &destination);
			if (itemID != -1) {
				SmashItem(itemID, weaponType);
			} else {
				if (ricochet)
					Richochet(&destination);
			}
			hit = -1;
		}
	} else {
		*ammo = 0;
		hit = 0;
	}
	return hit;
}

void __cdecl SmashItem(__int16 itemID, int weaponType) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (item->objectID == ID_WINDOW1) {
		SmashWindow(itemID);
	} else {
		if (item->objectID == ID_BELL && item->status != ITEM_ACTIVE) {
			item->status = ITEM_ACTIVE;
			AddActiveItem(itemID);
		}
	}
}

/*
 * Inject function
 */
void Inject_LaraFire() {
//	INJECT(0x0042E740, LaraGun);
//	INJECT(0x0042ECB0, CheckForHoldingState);
//	INJECT(0x0042ECF0, InitialiseNewWeapon);
//	INJECT(0x0042EE30, LaraTargetInfo);
//	INJECT(0x0042EFD0, LaraGetNewTarget);
//	INJECT(0x0042F1F0, find_target_point);
//	INJECT(0x0042F2A0, AimWeapon);

	INJECT(0x0042F370, FireWeapon);

//	INJECT(0x0042F6E0, HitTarget);

	INJECT(0x0042F780, SmashItem);

//	INJECT(0x0042F7E0, WeaponObject);
}