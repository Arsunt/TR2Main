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
#include "game/draw.h"
#include "3dsystem/3d_gen.h"
#include "3dsystem/scalespr.h"
#include "game/hair.h"
#include "specific/game.h"
#include "specific/output.h"
#include "global/vars.h"

#ifdef FEATURE_VIDEOFX_IMPROVED
extern DWORD AlphaBlendMode;
#endif // FEATURE_VIDEOFX_IMPROVED

void __cdecl DrawRooms(__int16 currentRoom) {
	ROOM_INFO *room = &RoomInfo[currentRoom];

	PhdWinLeft = room->left = 0;
	PhdWinTop = room->top = 0;
	PhdWinRight = room->right = PhdWinMaxX;
	PhdWinBottom = room->bottom = PhdWinMaxY;

	room->boundActive = 2;
	BoundRooms[0] = currentRoom;
	BoundStart = 0;
	BoundEnd = 1;
	DrawRoomsCount = 0;
	OutsideCamera = room->flags & ROOM_OUTSIDE;

	if( OutsideCamera ) {
		OutsideLeft = 0;
		OutsideTop = 0;
		OutsideRight = PhdWinMaxX;
		OutsideBottom = PhdWinMaxY;
	} else {
		OutsideLeft = PhdWinMaxX;
		OutsideTop = PhdWinMaxY;
		OutsideRight = 0;
		OutsideBottom = 0;
	}

	UnderwaterCamera = room->flags & ROOM_UNDERWATER;
	GetRoomBounds();
	MidSort = 0;

	if( OutsideCamera ) {
		PhdWinLeft = OutsideLeft;
		PhdWinRight = OutsideRight;
		PhdWinBottom = OutsideBottom;
		PhdWinTop = OutsideTop;
		if( Objects[ID_SKYBOX].loaded ) {
			// Draw skybox background
			S_SetupAboveWater(UnderwaterCamera);
			phd_PushMatrix();
			PhdMatrixPtr->_03 = PhdMatrixPtr->_13 = PhdMatrixPtr->_23 = 0;

			UINT16 *ptr = (UINT16 *)&Anims[Objects[ID_SKYBOX].animIndex].framePtr[9];
			phd_RotYXZsuperpack(&ptr, 0);
			S_InitialisePolyList(0);
			S_InsertBackground(MeshPtr[Objects[ID_SKYBOX].meshIndex]);
			--PhdMatrixPtr;
		} else {
			S_InitialisePolyList(1); // Fill backbuffer with black
			OutsideCamera = -1;
		}
	} else {
#ifdef FEATURE_VIEW_IMPROVED
		S_InitialisePolyList(1); // Fill backbuffer with black
#else // !FEATURE_VIEW_IMPROVED
		S_InitialisePolyList(0); // Leave backbuffer uncleaned
#endif // FEATURE_VIEW_IMPROVED
	}

	// Draw Lara
	if( Objects[ID_LARA].loaded && !(LaraItem->flags & IFL_INVISIBLE) ) {
		if( RoomInfo[LaraItem->roomNumber].flags & ROOM_UNDERWATER ) {
			S_SetupBelowWater(UnderwaterCamera);
		} else {
			S_SetupAboveWater(UnderwaterCamera);
		}
		MidSort = RoomInfo[LaraItem->roomNumber].boundActive >> 8;
		if( MidSort ) --MidSort;
		DrawLara(LaraItem);
	}

	// Draw rooms
	for( int i = 0; i < DrawRoomsCount; ++i ) {
		PrintRooms(DrawRoomsArray[i]);
	}

	// Draw movable and static objects
	for( int i = 0; i < DrawRoomsCount; ++i ) {
		PrintObjects(DrawRoomsArray[i]);
	}
}

void __cdecl DrawEffect(__int16 fx_id) {
	FX_INFO *fx = &Effects[fx_id];
	OBJECT_INFO *obj = &Objects[fx->object_number];
	if( !obj->loaded ) return;
	if( fx->object_number == ID_GLOW ) {
		// NOTE: Core's hacky way to store the sprite flags in the rotation fields
		S_DrawSprite((fx->pos.rotY << 16)|(fx->pos.rotX), // flags
					fx->pos.x, fx->pos.y, fx->pos.z, // coordinates
					Objects[ID_GLOW].meshIndex, // sprite id
					fx->shade, fx->frame_number); // shade, scale
	} else if( obj->nMeshes < 0 ) {
		S_DrawSprite(SPR_ABS | SPR_SHADE | (obj->semi_transparent ? SPR_SEMITRANS : 0), // flags
					fx->pos.x, fx->pos.y, fx->pos.z, // coordinates
					obj->meshIndex - fx->frame_number, // sprite id
					fx->shade, 0);  // shade, scale
	} else {
		phd_PushMatrix();
		phd_TranslateAbs(fx->pos.x, fx->pos.y, fx->pos.z);
		if( PhdMatrixPtr->_23 > PhdNearZ && PhdMatrixPtr->_23 < PhdFarZ ) {
			__int16 *meshPtr = NULL;
			phd_RotYXZ(fx->pos.rotY, fx->pos.rotX, fx->pos.rotZ);
			S_CalculateStaticLight(fx->shade);
			if( obj->nMeshes ) {
				meshPtr = MeshPtr[obj->meshIndex];
			} else {
				meshPtr = MeshPtr[fx->frame_number];
			}
			phd_PutPolygons(meshPtr, -1);
		}
		phd_PopMatrix();
	}
}

void __cdecl DrawSpriteItem(ITEM_INFO *item) {
	OBJECT_INFO *obj;

	phd_PushUnitMatrix(); // NOTE: this push is workaround for sprites with no matrix
	S_CalculateStaticMeshLight(item->pos.x, item->pos.y, item->pos.z,
		item->shade1, item->shade2, &RoomInfo[item->roomNumber]);
	phd_PopMatrix(); // NOTE: this pop is workaround for sprites with no matrix

	obj = &Objects[item->objectID];

	S_DrawSprite(SPR_ABS | SPR_SHADE | (obj->semi_transparent ? SPR_SEMITRANS : 0),
				 item->pos.x, item->pos.y, item->pos.z,
				 obj->meshIndex - item->frameNumber,
				 LsAdder + 0x1000, 0);
}

void __cdecl DrawDummyItem(ITEM_INFO *item) {
}

void __cdecl DrawAnimatingItem(ITEM_INFO *item) {
	static __int16 no_rotation[12] = {0};
	__int16 *frames[2] = {0};
	int rate = 0;
	DWORD bit = 1;
	int frac = GetFrames(item, frames, &rate);
	OBJECT_INFO *obj = &Objects[item->objectID];

	if( obj->shadowSize ) {
		S_PrintShadow(obj->shadowSize, frames[0], item);
	}

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
	int clip = S_GetObjectBounds(frames[0]);

	if( clip ) {
		CalculateObjectLighting(item, frames[0]);

		__int16 *rots = item->data ? (__int16 *)item->data : no_rotation;
		__int16 **meshPtr = &MeshPtr[obj->meshIndex];
		int *bonePtr = &AnimBones[obj->boneIndex];
		if( frac ) {
			InitInterpolate(frac, rate);
			phd_TranslateRel_ID(frames[0][6], frames[0][7], frames[0][8], frames[1][6], frames[1][7], frames[1][8]);
			UINT16 *rot1 = (UINT16 *)&frames[0][9];
			UINT16 *rot2 = (UINT16 *)&frames[1][9];
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);

			if( CHK_ANY(item->meshBits, 1) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
				SetMeshReflectState(item->objectID, 0);
#endif // FEATURE_VIDEOFX_IMPROVED
				phd_PutPolygons_I(meshPtr[0], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
				ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
			}

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
				if( CHK_ANY(state, 0x1C) ) {
					if( CHK_ANY(state, 0x08) ) {
						phd_RotY_I(*(rots++));
					}
					if( CHK_ANY(state, 0x04) ) {
						phd_RotX_I(*(rots++));
					}
					if( CHK_ANY(state, 0x10) ) {
						phd_RotZ_I(*(rots++));
					}
				}
				bonePtr+=4;
				bit <<= 1;
				if( CHK_ANY(item->meshBits, bit) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
					SetMeshReflectState(item->objectID, i);
#endif // FEATURE_VIDEOFX_IMPROVED
					phd_PutPolygons_I(meshPtr[i], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
				}
			}
		} else {
			phd_TranslateRel(frames[0][6], frames[0][7], frames[0][8]);
			UINT16 *rot = (UINT16 *)&frames[0][9];
			phd_RotYXZsuperpack(&rot, 0);

			if( CHK_ANY(item->meshBits, 1) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
				SetMeshReflectState(item->objectID, 0);
#endif // FEATURE_VIDEOFX_IMPROVED
				phd_PutPolygons(meshPtr[0], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
				ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
			}

			for( int i = 1; i < obj->nMeshes; ++i) {
				DWORD state = *bonePtr;
				if( CHK_ANY(state, 1) ) {
					phd_PopMatrix();
				}
				if( CHK_ANY(state, 2) ) {
					phd_PushMatrix();
				}
				phd_TranslateRel(bonePtr[1], bonePtr[2], bonePtr[3]);
				phd_RotYXZsuperpack(&rot, 0);
				if( CHK_ANY(state, 0x1C) ) {
					if( CHK_ANY(state, 0x08) ) {
						phd_RotY(*(rots++));
					}
					if( CHK_ANY(state, 0x04) ) {
						phd_RotX(*(rots++));
					}
					if( CHK_ANY(state, 0x10) ) {
						phd_RotZ(*(rots++));
					}
				}
				bonePtr += 4;
				bit <<= 1;
				if( CHK_ANY(item->meshBits, bit) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
					SetMeshReflectState(item->objectID, i);
#endif // FEATURE_VIDEOFX_IMPROVED
					phd_PutPolygons(meshPtr[i], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
				}
			}
		}
	}
	phd_PopMatrix();
}

void __cdecl DrawLaraInt(ITEM_INFO *item, __int16 *frame1, __int16 *frame2, int frac, int rate) {
	PHD_MATRIX matrix;
	UINT16 *rot1, *rot2, *rot1copy, *rot2copy;
	int frame, *bones;

	OBJECT_INFO *obj = &Objects[item->objectID];
	__int16 *bounds = GetBoundsAccurate(item);
	if( Lara.skidoo == -1 ) {
		S_PrintShadow(obj->shadowSize, bounds, item);
	}
	matrix = *PhdMatrixPtr;
	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);

	int clip = S_GetObjectBounds(frame1);
	if( !clip ) {
		phd_PopMatrix();
		return;
	}

	phd_PushMatrix();
	CalculateObjectLighting(item, frame1);
	bones = &AnimBones[obj->meshIndex]; // NOTE: seems like a bug. Should be &AnimBones[obj->boneIndex]

	rot1 = (UINT16 *)frame1 + 9;
	rot2 = (UINT16 *)frame2 + 9;

	InitInterpolate(frac, rate);
	phd_TranslateRel_ID(frame1[6], frame1[7], frame1[8], frame2[6], frame2[7], frame2[8]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[0], clip);

	phd_PushMatrix_I();
	phd_TranslateRel_I(bones[1], bones[2], bones[3]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[1], clip);
	phd_TranslateRel_I(bones[5], bones[6], bones[7]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[2], clip);
	phd_TranslateRel_I(bones[9], bones[10], bones[11]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[3], clip);
	phd_PopMatrix_I();

	phd_PushMatrix_I();
	phd_TranslateRel_I(bones[13], bones[14], bones[15]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[4], clip);
	phd_TranslateRel_I(bones[17], bones[18], bones[19]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[5], clip);
	phd_TranslateRel_I(bones[21], bones[22], bones[23]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[6], clip);
	phd_PopMatrix_I();

	phd_TranslateRel_I(bones[25], bones[26], bones[27]);
	if ( Lara.weapon_item != -1 && Lara.gun_type == LGT_M16
		&& (Items[Lara.weapon_item].currentAnimState == 0
		|| Items[Lara.weapon_item].currentAnimState == 2
		|| Items[Lara.weapon_item].currentAnimState == 4) )
	{
		frame = Lara.right_arm.frame_number * (Anims[Lara.right_arm.anim_number].interpolation >> 8) + 9;
		rot1 = rot2 = (UINT16 *)&Lara.right_arm.frame_base[frame];
		phd_RotYXZsuperpack_I(&rot1, &rot2, 7);
	} else {
		phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	}
	phd_RotYXZ_I(Lara.torso_y_rot, Lara.torso_x_rot, Lara.torso_z_rot);
	phd_PutPolygons_I(Lara.mesh_ptrs[7], clip);

	phd_PushMatrix_I();
	phd_TranslateRel_I(bones[53], bones[54], bones[55]);
	rot1copy = rot1;
	rot2copy = rot2;
	phd_RotYXZsuperpack_I(&rot1, &rot2, 6);
	rot1 = rot1copy;
	rot2 = rot2copy;
	phd_RotYXZ_I(Lara.head_y_rot, Lara.head_x_rot, Lara.head_z_rot);
	phd_PutPolygons_I(Lara.mesh_ptrs[14], clip);
	*PhdMatrixPtr = matrix;
	DrawHair();
	phd_PopMatrix_I();

	if( Lara.back_gun ) {
		phd_PushMatrix_I();
		int *bone = &AnimBones[Objects[Lara.back_gun].boneIndex];
		phd_TranslateRel_I(bone[53], bone[54], bone[55]);
		rot2copy = (UINT16 *)Objects[Lara.back_gun].frameBase + 9;
		rot1copy = (UINT16 *)Objects[Lara.back_gun].frameBase + 9;
		phd_RotYXZsuperpack_I(&rot1copy, &rot2copy, 14);
		phd_PutPolygons_I(MeshPtr[Objects[Lara.back_gun].meshIndex + 14], clip);
		phd_PopMatrix_I();
	}

	int gunType = LGT_Unarmed;
	if( Lara.gun_status == LGS_Ready
		|| Lara.gun_status == LGS_Special
		|| Lara.gun_status == LGS_Draw
		|| Lara.gun_status == LGS_Undraw )
	{
		gunType = Lara.gun_type;
	}

	switch( gunType ) {
		case LGT_Unarmed:
		case LGT_Flare:
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[29], bones[30], bones[31]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[8], clip);
			phd_TranslateRel_I(bones[33], bones[34], bones[35]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[9], clip);
			phd_TranslateRel_I(bones[37], bones[38], bones[39]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[10], clip);
			phd_PopMatrix_I();
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[41], bones[42], bones[43]);
			if( Lara.flare_control_left ) {
				frame = (Anims[Lara.left_arm.anim_number].interpolation >> 8) * (Lara.left_arm.frame_number - Anims[Lara.left_arm.anim_number].frameBase) + 9;
				rot1 = rot2 = (UINT16 *)&Lara.left_arm.frame_base[frame];
				phd_RotYXZsuperpack_I(&rot1, &rot2, 11);
			} else {
				phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			}
			phd_PutPolygons_I(Lara.mesh_ptrs[11], clip);
			phd_TranslateRel_I(bones[45], bones[46], bones[47]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[12], clip);
			phd_TranslateRel_I(bones[49], bones[50], bones[51]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[13], clip);
			if( Lara.gun_type == LGT_Flare ) {
				if( Lara.left_arm.flash_gun ) {
					phd_TranslateRel_I(11, 32, 80);
					phd_RotX_I(-90 * PHD_DEGREE);
					phd_RotY_I(2 * GetRandomDraw());
					S_CalculateStaticLight(0x800);
					phd_PutPolygons_I(MeshPtr[Objects[ID_FLARE_FIRE].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					if( AlphaBlendMode ) {
						int shade = (GetRandomDraw() & 0xFFF) + 0x1000;
						DWORD flags = RGB_MAKE(0xFF,0x80,0x80);
						flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SHADE|SPR_SEMITRANS;
						S_DrawSprite(flags, 0, 0, 0, Objects[ID_GLOW].meshIndex, shade, 0);
					}
#endif // FEATURE_VIDEOFX_IMPROVED
				}
			}
			break;
		case LGT_Pistols:
		case LGT_Magnums:
		case LGT_Uzis:
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[29], bones[30], bones[31]);
			InterpolateArmMatrix();
			phd_RotYXZ(Lara.right_arm.y_rot, Lara.right_arm.x_rot, Lara.right_arm.z_rot);
			frame = (Anims[Lara.right_arm.anim_number].interpolation >> 8) * (Lara.right_arm.frame_number - Anims[Lara.right_arm.anim_number].frameBase) + 9;
			rot1 = (UINT16 *)&Lara.right_arm.frame_base[frame];
			phd_RotYXZsuperpack(&rot1, 8);
			phd_PutPolygons(Lara.mesh_ptrs[8], clip);
			phd_TranslateRel(bones[33], bones[34], bones[35]);
			phd_RotYXZsuperpack(&rot1, 0);
			phd_PutPolygons(Lara.mesh_ptrs[9], clip);
			phd_TranslateRel(bones[37], bones[38], bones[39]);
			phd_RotYXZsuperpack(&rot1, 0);
			phd_PutPolygons(Lara.mesh_ptrs[10], clip);
			if( Lara.right_arm.flash_gun ) {
				matrix = *PhdMatrixPtr;
			}
			phd_PopMatrix_I();
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[41], bones[42], bones[43]);
			InterpolateArmMatrix();
			phd_RotYXZ(Lara.left_arm.y_rot, Lara.left_arm.x_rot, Lara.left_arm.z_rot);
			frame = (Anims[Lara.left_arm.anim_number].interpolation >> 8) * (Lara.left_arm.frame_number - Anims[Lara.left_arm.anim_number].frameBase) + 9;
			rot1 = (UINT16 *)&Lara.left_arm.frame_base[frame];
			phd_RotYXZsuperpack(&rot1, 11);
			phd_PutPolygons(Lara.mesh_ptrs[11], clip);
			phd_TranslateRel(bones[45], bones[46], bones[47]);
			phd_RotYXZsuperpack(&rot1, 0);
			phd_PutPolygons(Lara.mesh_ptrs[12], clip);
			phd_TranslateRel(bones[49], bones[50], bones[51]);
			phd_RotYXZsuperpack(&rot1, 0);
			phd_PutPolygons(Lara.mesh_ptrs[13], clip);
			if( Lara.left_arm.flash_gun ) {
				DrawGunFlash(gunType, clip);
			}
			if( Lara.right_arm.flash_gun ) {
				*PhdMatrixPtr = matrix;
				DrawGunFlash(gunType, clip);
			}
			break;
		case LGT_Shotgun:
		case LGT_M16:
		case LGT_Grenade:
		case LGT_Harpoon:
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[29], bones[30], bones[31]);
			frame = Lara.right_arm.frame_number * (Anims[Lara.right_arm.anim_number].interpolation >> 8) + 9;
			rot1 = rot2 = (UINT16 *)&Lara.right_arm.frame_base[frame];
			phd_RotYXZsuperpack_I(&rot1, &rot2, 8);
			phd_PutPolygons_I(Lara.mesh_ptrs[8], clip);
			phd_TranslateRel_I(bones[33], bones[34], bones[35]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[9], clip);
			phd_TranslateRel_I(bones[37], bones[38], bones[39]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[10], clip);
			if( Lara.right_arm.flash_gun ) {
				matrix = *PhdMatrixPtr;
			}
			phd_PopMatrix_I();
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[41], bones[42], bones[43]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[11], clip);
			phd_TranslateRel_I(bones[45], bones[46], bones[47]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[12], clip);
			phd_TranslateRel_I(bones[49], bones[50], bones[51]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[13], clip);
			if( Lara.right_arm.flash_gun ) {
				*PhdMatrixPtr = matrix;
				DrawGunFlash(gunType, clip);
			}
			break;
		default:
			break;
	}
	phd_PopMatrix();
	phd_PopMatrix();
	phd_PopMatrix();
}

void __cdecl phd_RotYXZsuperpack(UINT16 **pptr, int index) {
	for( int i = 0; i < index; ++i ) {
		if( (**pptr >> 14) == 0 )
			*pptr += 2;
		else
			++*pptr;
	}

	switch( **pptr >> 14 ) {
		case 0 :
			phd_RotYXZpack(((UINT16)(*pptr)[0] << 16) + (UINT16)(*pptr)[1]);
			*pptr += 2;
			break;
		case 1 :
			phd_RotX(**pptr << 6);
			++*pptr;
			break;
		case 2 :
			phd_RotY(**pptr << 6);
			++*pptr;
			break;
		case 3 :
			phd_RotZ(**pptr << 6);
			++*pptr;
			break;
	}
}

void __cdecl phd_PutPolygons_I(__int16 *ptrObj, int clip) {
	phd_PushMatrix();
	InterpolateMatrix();
	phd_PutPolygons(ptrObj, clip);
	phd_PopMatrix();
}

void __cdecl DrawGunFlash(int weapon, int clip) {
	__int16 light;
	int len;
	int off;

	switch( weapon ) {
		case LGT_Shotgun:
			return;
		case LGT_Flare:
			phd_TranslateRel(11, 32, 80);
			phd_RotX(-90 * PHD_DEGREE);
			phd_RotY(2 * GetRandomDraw());
			S_CalculateStaticLight(0x800);
			phd_PutPolygons(MeshPtr[Objects[ID_FLARE_FIRE].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			if( AlphaBlendMode ) {
				int shade = (GetRandomDraw() & 0xFFF) + 0x1000;
				DWORD flags = RGB_MAKE(0xFF,0x80,0x80);
				flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SHADE|SPR_SEMITRANS;
				S_DrawSprite(flags, 0, 0, 0, Objects[ID_GLOW].meshIndex, shade, 0);
			}
#endif // FEATURE_VIDEOFX_IMPROVED
			return;
		case LGT_M16:
			phd_TranslateRel(0, 400, 99);
			phd_RotYXZ(0, -85 * PHD_DEGREE, (2 * GetRandomDraw() & 0x4000) + 0x2000);
			S_CalculateStaticLight(0xA00);
			phd_PutPolygons(MeshPtr[Objects[ID_M16_FLASH].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			if( AlphaBlendMode ) {
				DWORD flags = RGB_MAKE(0x7F,0x70,0x1F);
				flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SCALE|SPR_SEMITRANS;
				S_DrawSprite(flags, 0, 0, -65, Objects[ID_GLOW].meshIndex, 0, 0x200);
			}
#endif // FEATURE_VIDEOFX_IMPROVED
			return;
		case LGT_Magnums:
			light = 0x1000;
			len = 215;
			off = 65;
			break;
		case LGT_Uzis:
			light = 0xA00;
			len = 200;
			off = 50;
			break;
		default:
			light = 0x1400;
			len = 185;
			off = 40;
			break;
	}
	phd_TranslateRel(0, len, off);
	phd_RotYXZ(0, -90 * PHD_DEGREE, 2 * GetRandomDraw());
	S_CalculateStaticLight(light);
	phd_PutPolygons(MeshPtr[Objects[ID_GUN_FLASH].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
	if( AlphaBlendMode ) {
		DWORD flags = RGB_MAKE(0x3F,0x38,0x0F);
		flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SCALE|SPR_SEMITRANS;
		S_DrawSprite(flags, 0, 0, 0, Objects[ID_GLOW].meshIndex, 0, 0x200);
	}
#endif // FEATURE_VIDEOFX_IMPROVED
}

/*
 * Inject function
 */
void Inject_Draw() {
//	INJECT(0x00418920, DrawPhaseCinematic);
//	INJECT(0x00418960, DrawPhaseGame);

	INJECT(0x004189A0, DrawRooms);

//	INJECT(0x00418C50, GetRoomBounds);
//	INJECT(0x00418E20, SetRoomBounds);
//	INJECT(0x004191A0, ClipRoom);
//	INJECT(0x00419580, PrintRooms);
//	INJECT(0x00419640, PrintObjects);

//	INJECT(0x00419870, DrawEffect);
	INJECT(0x004199C0, DrawSpriteItem);
//	INJECT(----------, DrawDummyItem);
	INJECT(0x00419A50, DrawAnimatingItem);

//	INJECT(0x00419DD0, DrawLara);

	INJECT(0x0041AB00, DrawLaraInt);

//	INJECT(0x0041B6F0, InitInterpolate);
//	INJECT(0x0041B730, phd_PopMatrix_I);
//	INJECT(0x0041B760, phd_PushMatrix_I);
//	INJECT(0x0041B790, phd_RotY_I);
//	INJECT(0x0041B7D0, phd_RotX_I);
//	INJECT(0x0041B810, phd_RotZ_I);
//	INJECT(0x0041B850, phd_TranslateRel_I);
//	INJECT(0x0041B8A0, phd_TranslateRel_ID);
//	INJECT(0x0041B8F0, phd_RotYXZ_I);
//	INJECT(0x0041B940, phd_RotYXZsuperpack_I);

	INJECT(0x0041B980, phd_RotYXZsuperpack);
	INJECT(0x0041BA30, phd_PutPolygons_I);

//	INJECT(0x0041BA60, InterpolateMatrix);
//	INJECT(0x0041BC10, InterpolateArmMatrix);

	INJECT(0x0041BD10, DrawGunFlash);

//	INJECT(0x0041BE80, CalculateObjectLighting);
//	INJECT(0x0041BF70, GetFrames);
//	INJECT(0x0041C010, GetBoundsAccurate);
//	INJECT(0x0041C090, GetBestFrame);
//	INJECT(0x0041C0D0, AddDynamicLight);
}
