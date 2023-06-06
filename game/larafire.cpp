/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Embracer Group AB.
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
#include "game/lot.h"
#include "global/vars.h"

void __cdecl LaraGetNewTarget(WEAPON_INFO* weapon) {
	GAME_VECTOR start, target;
	CREATURE_INFO *creature = NULL;
	ITEM_INFO *bestItem = NULL, *item = NULL;
	int distance;
	int bestDistance = 0x7FFFFFFF;
	int maxDistance = weapon->targetDist;
	int sqrMaxDistance = SQR(maxDistance);
	int x, y, z;
	VECTOR_ANGLES angles;
	__int16 bestYRot = 0x7FFF, yRot;

	start.x = LaraItem->pos.x;
	start.y = LaraItem->pos.y - 650;
	start.z = LaraItem->pos.z;
	start.roomNumber = LaraItem->roomNumber;

	for (int i = 0; i < MAXIMUM_CREATURE_SLOTS; i++) {
		creature = &ActiveCreatures[i];
		if (creature->item_num != -1 && creature->item_num != Lara.item_number) {
			item = &Items[creature->item_num];
			if (item->hitPoints <= 0)
				continue;
			x = item->pos.x - start.x;
			y = item->pos.y - start.y;
			z = item->pos.z - start.z;
			if ((ABS(x) > maxDistance) ||
				(ABS(y) > maxDistance) ||
				(ABS(z) > maxDistance))
				continue;
			distance = SQR(z) + SQR(y) + SQR(x);
			if (distance < sqrMaxDistance) {
				find_target_point(item, &target);

				if (LOS(&start, &target)) {
					phd_GetVectorAngles(target.x - start.x, target.y - start.y, target.z - start.z, &angles);
					angles.yaw -= LaraItem->pos.rotY + Lara.torso_y_rot;
					angles.pitch -= LaraItem->pos.rotX + Lara.torso_x_rot;
					if (angles.yaw >= weapon->lockAngles[0] &&
						angles.yaw <= weapon->lockAngles[1] &&
						angles.pitch >= weapon->lockAngles[2] &&
						angles.pitch <= weapon->lockAngles[3]) {
						yRot = ABS(angles.yaw);
						if (yRot < (bestYRot + 2730) && distance < bestDistance) {
							bestDistance = distance;
							bestYRot = yRot;
							bestItem = item;
						}
					}
				}
			}
		}
	}

	Lara.target = bestItem;
	LaraTargetInfo(weapon);
}

/*
 * Inject function
 */
void Inject_LaraFire() {
//	INJECT(0x0042E740, LaraGun);
//	INJECT(0x0042ECB0, CheckForHoldingState);
//	INJECT(0x0042ECF0, InitialiseNewWeapon);
//	INJECT(0x0042EE30, LaraTargetInfo);
	INJECT(0x0042EFD0, LaraGetNewTarget);
//	INJECT(0x0042F1F0, find_target_point);
//	INJECT(0x0042F2A0, AimWeapon);
//	INJECT(0x0042F370, FireWeapon);
//	INJECT(0x0042F6E0, HitTarget);
//	INJECT(0x0042F780, SmashItem);
//	INJECT(0x0042F7E0, WeaponObject);
}
