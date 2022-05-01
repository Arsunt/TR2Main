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
#include "game/sphere.h"
#include "3dsystem/3d_gen.h"
#include "game/draw.h"
#include "game/effects.h"
#include "global/vars.h"


void __cdecl GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *pos, int joint) {
	OBJECT_INFO *obj;
	int *bones, flags;
	UINT16 *rotation;
	__int16 *frame, *rot;

	obj = &Objects[item->objectID];
	bones = &AnimBones[obj->boneIndex];
	frame = GetBestFrame(item);
	rot = (__int16*)item->data;

	phd_PushUnitMatrix();
	PhdMatrixPtr->_03 = 0;
	PhdMatrixPtr->_13 = 0;
	PhdMatrixPtr->_23 = 0;
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
	phd_TranslateRel(frame[6], frame[7], frame[8]);
	rotation = (UINT16*)frame + 9;
	phd_RotYXZsuperpack(&rotation, 0);

	for (int i = 0; i < joint; i++, bones += 4) {
		flags = bones[0];
		if (flags & 1) {
			phd_PopMatrix();
		}
		if (flags & 2) {
			phd_PushMatrix();
		}

		phd_TranslateRel(bones[1], bones[2], bones[3]);
		phd_RotYXZsuperpack(&rotation, 0);

		if (flags & 0x1C) {
			if (flags & 8) {
				phd_RotY(*(rot++));
			}
			if (flags & 4) {
				phd_RotX(*(rot++));
			}
			if (flags & 0x10) {
				phd_RotZ(*(rot++));
			}
		}
	}

	phd_TranslateRel(pos->x, pos->y, pos->z);
	pos->x = (PhdMatrixPtr->_03 >> W2V_SHIFT) + item->pos.x;
	pos->y = (PhdMatrixPtr->_13 >> W2V_SHIFT) + item->pos.y;
	pos->z = (PhdMatrixPtr->_23 >> W2V_SHIFT) + item->pos.z;
	phd_PopMatrix();
}

void __cdecl BaddieBiteEffect(ITEM_INFO *item, BITE_INFO *bite) {
	PHD_VECTOR pos;
	pos.x = bite->x;
	pos.y = bite->y;
	pos.z = bite->z;
	GetJointAbsPosition(item, &pos, bite->meshIndex);
	DoBloodSplat(pos.x, pos.y, pos.z, item->speed, item->pos.rotY, item->roomNumber);
}

/*
 * Inject function
 */
void Inject_Sphere() {
//	INJECT(0x0043FA60, TestCollision);
//	INJECT(0x0043FB90, GetSpheres);
	INJECT(0x0043FE70, GetJointAbsPosition);
	INJECT(0x00440010, BaddieBiteEffect);
}
