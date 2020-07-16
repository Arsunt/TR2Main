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
#include "game/skidoo.h"
#include "3dsystem/3d_gen.h"
#include "game/draw.h"
#include "game/items.h"
#include "game/missile.h"
#include "game/sound.h"
#include "specific/output.h"
#include "global/vars.h"

void __cdecl SkidooExplode(ITEM_INFO *item) {
	__int16 fxID;
	FX_INFO *fx;

	fxID = CreateEffect(item->roomNumber);
	if (fxID != -1) {
		fx = &Effects[fxID];
		fx->pos.x = item->pos.x;
		fx->pos.y = item->pos.y;
		fx->pos.z = item->pos.z;
		fx->speed = 0;
		fx->frame_number = 0;
		fx->counter = 0;
		fx->object_number = ID_EXPLOSION;
	}
	ExplodingDeath(Lara.skidoo, ~3, 0);
	PlaySoundEffect(105, NULL, 0);
	Lara.skidoo = -1;
}

void __cdecl DrawSkidoo(ITEM_INFO *item) {
	__int16 *frames[2];
	int rate = 0;
	UINT16 flags = 0;
	OBJECT_INFO *obj;

	int frac = GetFrames(item, frames, &rate);
	if( item->data ) {
		flags = *(UINT16 *)item->data;
	}
	if( CHK_ANY(flags, 4) ) {
		obj = &Objects[ID_SKIDOO_ARMED];
	} else {
		obj = &Objects[item->objectID];
	}

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
	int clip = S_GetObjectBounds(frames[0]);

	if( clip ) {
		CalculateObjectLighting(item, frames[0]);

		__int16 **track = 0;
		__int16 **meshPtr = &MeshPtr[obj->meshIndex];
		int *bonePtr = &AnimBones[obj->boneIndex];
		if( (flags & 3) == 1 ) {
			track = &MeshPtr[Objects[ID_SKIDOO_LARA].meshIndex + 1];
		} else if( (flags & 3) == 2 ) {
			track = &MeshPtr[Objects[ID_SKIDOO_LARA].meshIndex + 7];
		}

		if( frac ) {
			UINT16 *rot1 = (UINT16 *)&frames[0][9];
			UINT16 *rot2 = (UINT16 *)&frames[1][9];
			InitInterpolate(frac, rate);
			phd_TranslateRel_ID(frames[0][6], frames[0][7], frames[0][8], frames[1][6], frames[1][7], frames[1][8]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
#ifdef FEATURE_VIDEOFX_IMPROVED
			SetMeshReflectState(CHK_ANY(flags, 4) ? (int)ID_SKIDOO_ARMED : item->objectID, 0);
#endif // FEATURE_VIDEOFX_IMPROVED
			phd_PutPolygons_I(meshPtr[0], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED

			for( int i = 1; i < obj->nMeshes; ++i ) {
				DWORD state = *bonePtr;
				if( CHK_ANY(state, 1) ) {
					phd_PopMatrix_I();
				}
				if( CHK_ANY(state, 2) ) {
					phd_PushMatrix_I();
				}
				phd_TranslateRel_I(bonePtr[1], bonePtr[2], bonePtr[3]);
				phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
				if( track ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
					SetMeshReflectState(ID_SKIDOO_LARA, (flags & 3) == 1 ? 1 : 7);
#endif // FEATURE_VIDEOFX_IMPROVED
					phd_PutPolygons_I(*track, clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
					track = NULL;
				} else {
#ifdef FEATURE_VIDEOFX_IMPROVED
					SetMeshReflectState(CHK_ANY(flags, 4) ? (int)ID_SKIDOO_ARMED : item->objectID, i);
#endif // FEATURE_VIDEOFX_IMPROVED
					phd_PutPolygons_I(meshPtr[i], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
				}
				bonePtr += 4;
			}
		} else {
			UINT16 *rot = (UINT16 *)&frames[0][9];
			phd_TranslateRel(frames[0][6], frames[0][7], frames[0][8]);
			phd_RotYXZsuperpack(&rot, 0);
#ifdef FEATURE_VIDEOFX_IMPROVED
			SetMeshReflectState(CHK_ANY(flags, 4) ? (int)ID_SKIDOO_ARMED : item->objectID, 0);
#endif // FEATURE_VIDEOFX_IMPROVED
			phd_PutPolygons(meshPtr[0], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED

			for( int i = 1; i < obj->nMeshes; ++i ) {
				DWORD state = *bonePtr;
				if( CHK_ANY(state, 1) ) {
					phd_PopMatrix();
				}
				if( CHK_ANY(state, 2) ) {
					phd_PushMatrix();
				}
				phd_TranslateRel(bonePtr[1], bonePtr[2], bonePtr[3]);
				phd_RotYXZsuperpack(&rot, 0);
				if( track ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
					SetMeshReflectState(ID_SKIDOO_LARA, (flags & 3) == 1 ? 1 : 7);
#endif // FEATURE_VIDEOFX_IMPROVED
					phd_PutPolygons(*track, clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
					track = NULL;
				} else {
#ifdef FEATURE_VIDEOFX_IMPROVED
					SetMeshReflectState(CHK_ANY(flags, 4) ? (int)ID_SKIDOO_ARMED : item->objectID, i);
#endif // FEATURE_VIDEOFX_IMPROVED
					phd_PutPolygons(meshPtr[i], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
				}
				bonePtr += 4;
			}
		}
	}
	phd_PopMatrix();
}

/*
 * Inject function
 */
void Inject_Skidoo() {
//	INJECT(0x0043CEE0, InitialiseSkidoo);
//	INJECT(0x0043CF20, SkidooCheckGeton);
//	INJECT(0x0043D010, SkidooCollision);
//	INJECT(0x0043D110, SkidooBaddieCollision);
//	INJECT(0x0043D310, TestHeight);
//	INJECT(0x0043D3D0, DoShift);
//	INJECT(0x0043D650, DoDynamics);
//	INJECT(0x0043D6B0, GetCollisionAnim);
//	INJECT(0x0043D740, DoSnowEffect);
//	INJECT(0x0043D880, SkidooDynamics);
//	INJECT(0x0043DD20, SkidooUserControl);
//	INJECT(0x0043DEE0, SkidooCheckGetOffOK);
//	INJECT(0x0043DFF0, SkidooAnimation);

	INJECT(0x0043E2D0, SkidooExplode);

//	INJECT(0x0043E350, SkidooCheckGetOff);
//	INJECT(0x0043E590, SkidooGuns);
//	INJECT(0x0043E6B0, SkidooControl);

	INJECT(0x0043EB10, DrawSkidoo);

//	INJECT(0x0043EDF0, InitialiseSkidman);
//	INJECT(0x0043EE80, SkidManControl);
//	INJECT(0x0043F280, SkidmanPush);
//	INJECT(0x0043F3A0, SkidmanCollision);
}
