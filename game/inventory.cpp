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
#include "game/inventory.h"
#include "3dsystem/3d_gen.h"
#include "game/draw.h"
#include "specific/output.h"
#include "global/vars.h"

DWORD __cdecl GetDebouncedInput(DWORD input) {
	static DWORD oldInput = 0;
	DWORD result = input & ~oldInput;

	oldInput = input;
	return result;
}

void __cdecl DoInventoryBackground() {
	VECTOR_ANGLES angles;
	PHD_3DPOS viewPos;
	UINT16 *ptr;

	S_CopyBufferToScreen();

	if( (Objects[ID_INV_BACKGROUND].flags & 1) != 0 ) {
		// set view
		phd_GetVectorAngles(0, 0x1000, 0, &angles);
		viewPos.x = 0;
		viewPos.y = -0x200;
		viewPos.z = 0;
		viewPos.rotX = angles.pitch;
		viewPos.rotY = angles.yaw;
		viewPos.rotZ = 0;
		phd_GenerateW2V(&viewPos);

		// set lighting
		LsDivider = 0x6000;
		phd_GetVectorAngles(-0x600, 0x100, 0x400, &angles);
		phd_RotateLight(angles.pitch, angles.yaw);

		// transform and insert the mesh
		phd_PushMatrix();
		ptr = (UINT16 *)&Anims[Objects[ID_INV_BACKGROUND].animIndex].framePtr[9];
		phd_TranslateAbs(0, 0x3000, 0);
		phd_RotYXZ(0, PHD_90, PHD_180);
		phd_RotYXZsuperpack(&ptr, 0);
		phd_RotYXZ(PHD_180, 0, 0);
		S_InsertInvBgnd(MeshPtr[Objects[ID_INV_BACKGROUND].meshIndex]);
		phd_PopMatrix();
	}
}

/*
 * Inject function
 */
void Inject_Inventory() {
//	INJECT(0x00422060, Display_Inventory);
//	INJECT(0x004232F0, Construct_Inventory);
//	INJECT(0x00423450, SelectMeshes);
//	INJECT(0x004234E0, AnimateInventoryItem);
//	INJECT(0x00423570, DrawInventoryItem);

	INJECT(0x004239A0, GetDebouncedInput);

//	INJECT(0x004239C0, DoInventoryPicture);

	INJECT(0x004239D0, DoInventoryBackground);
}
