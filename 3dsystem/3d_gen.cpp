/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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
#include "3dsystem/3d_gen.h"
#include "3dsystem/3d_out.h"
#include "3dsystem/3dinsert.h"
#include "3dsystem/phd_math.h"
#include "3dsystem/scalespr.h"
#include "specific/hwr.h"
#include "global/vars.h"

// related to POLYTYPE enum
static void (__cdecl *PolyDrawRoutines[])(__int16 *) = {
	draw_poly_gtmap,		// gouraud shaded poly (texture)
	draw_poly_wgtmap,		// gouraud shaded poly (texture + colorkey)
	draw_poly_gtmap_persp,	// gouraud shaded poly (texture + perspective)
	draw_poly_wgtmap_persp,	// gouraud shaded poly (texture + colorkey + perspective)
	draw_poly_line,			// line (color)
	draw_poly_flat,			// flat shaded poly (color)
	draw_poly_gouraud,		// gouraud shaded poly (color)
	draw_poly_trans,		// shadow poly (color + semitransparent)
	draw_scaled_spriteC		// scaled sprite (texture + colorkey)
};

#if defined(FEATURE_EXTENDED_LIMITS) || defined(FEATURE_VIEW_IMPROVED)
SORT_ITEM SortBuffer[16000];
__int16 Info3dBuffer[480000];
#endif // defined(FEATURE_EXTENDED_LIMITS) || defined(FEATURE_VIEW_IMPROVED)

#ifdef FEATURE_EXTENDED_LIMITS
PHD_SPRITE PhdSpriteInfo[2048];
D3DTLVERTEX HWR_VertexBuffer[0x8000];
#endif // FEATURE_EXTENDED_LIMITS

#ifdef FEATURE_VIEW_IMPROVED
bool PsxFovEnabled;

// view distance
double ViewDistanceFactor = 6.0;

// regular fog
double FogBeginFactor = 1.0;
double FogEndFactor = 6.0;
int FogBeginDepth = DEPTHQ_START;
int FogEndDepth = DEPTHQ_END;

// underwater fog
double WaterFogBeginFactor = 0.6;
double WaterFogEndFactor = 1.0;
int WaterFogBeginDepth = DEPTHQ_START;
int WaterFogEndDepth = DEPTHQ_END;

// fog formula
int CalculateFogShade(int depth) {
	int fogBegin, fogEnd;

	if( IsWaterEffect ) {
		fogBegin = WaterFogBeginDepth;
		fogEnd = WaterFogEndDepth;
	} else {
		fogBegin = FogBeginDepth;
		fogEnd = FogEndDepth;
	}

	if( depth < fogBegin )
		return 0;
	if( depth >= fogEnd )
		return 0x1FFF;

	return (depth - fogBegin) * 0x1FFF / (fogEnd - fogBegin);
}
#endif // FEATURE_VIEW_IMPROVED

#ifdef FEATURE_VIDEOFX_IMPROVED
#include "modding/mod_utils.h"

extern DWORD ReflectionMode;
static POLYFILTER ReflectFilter;
static D3DCOLOR ReflectTint = 0;
static bool IsReflect = false;

void ClearMeshReflectState() {
	memset(&ReflectFilter, 0 , sizeof(ReflectFilter));
	ReflectTint = RGBA_MAKE(0xFF,0xFF,0xFF,0x80);
	IsReflect = false;
}

void SetMeshReflectState(int objID, int meshIdx) {
	// Clear poly filters and disable reflection by default
	ClearMeshReflectState();
	if( TextureFormat.bpp < 16 || !ReflectionMode ) return;

	if( objID == ID_NONE ) {
		// Reflect all meshes with custom tint instead of mesh index
		ReflectTint = meshIdx;
		IsReflect = true;
		return;
	}

#ifdef FEATURE_MOD_CONFIG
	// Check if config is presented
	if( IsModReflectConfigLoaded() ) {
		POLYFILTER_NODE *node = NULL;
		if( meshIdx < 0 ) {
			for( node = GetModReflectStaticsFilter(); node != NULL; node = node->next ) {
				if( node->id == objID ) {
					ReflectFilter = node->filter;
					IsReflect = true;
					break;
				}
			}
		} else if( objID >= 0 && objID < ID_NUMBER_OBJECTS ) {
			POLYFILTER_NODE **obj = GetModReflectObjectsFilter();
			for( node = obj[objID]; node != NULL; node = node->next ) {
				if( node->id == meshIdx ) {
					ReflectFilter = node->filter;
					IsReflect = true;
					break;
				}
			}
		}
		return;
	}
#endif // FEATURE_MOD_CONFIG

	// If config is absent or disabled, use hardcoded params
	if( objID >= 0 && meshIdx < 0 ) {
		// This is static object mesh
		return;
	}

	// This is animated object mesh
	switch( objID ) {
	case ID_SKIDOO_FAST :
		// This one is a fast showmobile from the Golden Mask
		// Reflect the windshield only (skidoo body is mesh #0)
		if( meshIdx == 0 ) {
			// Set filter conditions
			ReflectFilter.n_vtx = 59;
			ReflectFilter.n_gt4 = 14;
			ReflectFilter.n_gt3 = 73;
			ReflectFilter.n_g4 = 0;
			ReflectFilter.n_g3 = 17;
			// All colored triangles are reflective
			// The only reflective textured triangle is 48
			ReflectFilter.gt3[0].idx = 48;
			ReflectFilter.gt3[0].num = 1;
			// Quads are not reflective
			ReflectFilter.gt4[0].idx = ~0;
			ReflectFilter.g4[0].idx = ~0;
			IsReflect = true;
		}
		break;
	case ID_SKIDOO_ARMED :
		// This one is an armed showmobile
		// Reflect the windshield only (skidoo body is mesh #0)
		if( meshIdx == 0 ) {
			// Set filter conditions
			ReflectFilter.n_vtx = 88;
			ReflectFilter.n_gt4 = 45;
			ReflectFilter.n_gt3 = 60;
			ReflectFilter.n_g4 = 0;
			ReflectFilter.n_g3 = 0;
			// The reflective textured quads are 21..22, 34..47
			ReflectFilter.gt4[0].idx = 21;
			ReflectFilter.gt4[0].num = 2;
			ReflectFilter.gt3[0].idx = 34;
			ReflectFilter.gt3[0].num = 14;
			// Other polys are not reflective
			ReflectFilter.g4[0].idx = ~0;
			ReflectFilter.g3[0].idx = ~0;
			IsReflect = true;
		}
		break;
	case ID_WORKER5 :
		// Reflect the black glass mask of flamethrower buddy (his head is mesh #15)
		if( meshIdx == 15 ) {
			// Set filter conditions
			ReflectFilter.n_vtx = 38;
			ReflectFilter.n_gt4 = 30;
			ReflectFilter.n_gt3 = 12;
			ReflectFilter.n_g4 = 0;
			ReflectFilter.n_g3 = 0;
			// The reflective textured quads are 22..26
			ReflectFilter.gt4[0].idx = 22;
			ReflectFilter.gt4[0].num = 5;
			// Other polys are not reflective
			ReflectFilter.gt3[0].idx = ~0;
			ReflectFilter.g4[0].idx = ~0;
			ReflectFilter.g3[0].idx = ~0;
			IsReflect = true;
		}
		break;
	case ID_SPINNING_BLADE :
		if( meshIdx == 0 ) {
			// Reflect only quads, not triangles
			ReflectFilter.gt3[0].idx = ~0;
			ReflectFilter.g3[0].idx = ~0;
			IsReflect = true;
		}
		break;
	case ID_BLADE :
		// Reflect the blade only (mesh #1)
		if( meshIdx == 1 ) {
			IsReflect = true;
		}
		break;
	case ID_KILLER_STATUE :
		// Reflect the sword only (mesh #7)
		if( meshIdx == 7 ) {
			IsReflect = true;
		}
		break;
	}
}

static bool InsertEnvmap(__int16 *ptrObj, int vtxCount, bool colored, LPVOID param) {
	InsertObjectEM(ptrObj, vtxCount, ReflectTint, (PHD_UV *)param);
	return true;
}

static void phd_PutEnvmapPolygons(__int16 *ptrEnv) {
	if( ptrEnv == NULL || !IsReflect
		|| SavedAppSettings.RenderMode != RM_Hardware ) return;
	__int16 *ptrObj = ptrEnv;

	ptrObj += 5; // skip x, y, z, radius, flags
	__int16 num = *(ptrObj++); // get vertex counter
	ptrObj += num * 3; // skip vertices

	int vtxCount = *ptrObj++;
	if( vtxCount <= 0 ) return;

	PHD_UV *uv = new PHD_UV[vtxCount];

	for( int i = 0; i < vtxCount; ++i ) {
		// make sure that reflection will be drawn after normal poly
		PhdVBuf[i].zv -= (double)(W2V_SCALE/2);
		// set lighting that depends only from fog distance
		PhdVBuf[i].g = 0x1000;
		int depth = PhdMatrixPtr->_23 >> W2V_SHIFT;
#ifdef FEATURE_VIEW_IMPROVED
		PhdVBuf[i].g += CalculateFogShade(depth);
#else // !FEATURE_VIEW_IMPROVED
		if( depth > DEPTHQ_START ) // fog begin
			PhdVBuf[i].g += depth - DEPTHQ_START;
#endif // FEATURE_VIEW_IMPROVED
		CLAMP(PhdVBuf[i].g, 0x1000, 0x1FFF); // reflection can be darker but not brighter

		// rotate normal vectors for X/Y, no translation
		int x = (PhdMatrixPtr->_00 * ptrObj[0] +
				 PhdMatrixPtr->_01 * ptrObj[1] +
				 PhdMatrixPtr->_02 * ptrObj[2]) >> W2V_SHIFT;

		int y = (PhdMatrixPtr->_10 * ptrObj[0] +
				 PhdMatrixPtr->_11 * ptrObj[1] +
				 PhdMatrixPtr->_12 * ptrObj[2]) >> W2V_SHIFT;

		CLAMP(x, -PHD_IONE, PHD_IONE);
		CLAMP(y, -PHD_IONE, PHD_IONE);
		uv[i].u = PHD_ONE/PHD_IONE * (x + PHD_IONE)/2;
		uv[i].v = PHD_ONE/PHD_IONE * (y + PHD_IONE)/2;
		ptrObj += 3;
	}

	EnumeratePolys(ptrEnv, false, InsertEnvmap, &ReflectFilter, (LPVOID)uv);
	delete[] uv;
}
#endif // FEATURE_VIDEOFX_IMPROVED

void phd_GenerateW2V(PHD_3DPOS *viewPos) {
	int sx = phd_sin(viewPos->rotX);
	int cx = phd_cos(viewPos->rotX);
	int sy = phd_sin(viewPos->rotY);
	int cy = phd_cos(viewPos->rotY);
	int sz = phd_sin(viewPos->rotZ);
	int cz = phd_cos(viewPos->rotZ);

	PhdMatrixPtr = &MatrixStack[0]; // set matrix stack pointer to W2V

	MatrixW2V._00 = PhdMatrixPtr->_00 = TRIGMULT3(sx, sy, sz) + TRIGMULT2(cy, cz);
	MatrixW2V._01 = PhdMatrixPtr->_01 = TRIGMULT2(cx, sz);
	MatrixW2V._02 = PhdMatrixPtr->_02 = TRIGMULT3(sx, cy, sz) - TRIGMULT2(sy, cz);

	MatrixW2V._10 = PhdMatrixPtr->_10 = (int)(FltViewAspect * (double)(TRIGMULT3(sx, sy, cz) - TRIGMULT2(cy, sz)));
	MatrixW2V._11 = PhdMatrixPtr->_11 = (int)(FltViewAspect * (double)(TRIGMULT2(cx, cz)));
	MatrixW2V._12 = PhdMatrixPtr->_12 = (int)(FltViewAspect * (double)(TRIGMULT3(sx, cy, cz) + TRIGMULT2(sy, sz)));

	MatrixW2V._20 = PhdMatrixPtr->_20 = TRIGMULT2(cx, sy);
	MatrixW2V._21 = PhdMatrixPtr->_21 = -(sx);
	MatrixW2V._22 = PhdMatrixPtr->_22 = TRIGMULT2(cx, cy);

	MatrixW2V._03 = PhdMatrixPtr->_03 = viewPos->x;;
	MatrixW2V._13 = PhdMatrixPtr->_13 = viewPos->y;
	MatrixW2V._23 = PhdMatrixPtr->_23 = viewPos->z;
}

void __cdecl phd_LookAt(int xsrc, int ysrc, int zsrc, int xtar, int ytar, int ztar, __int16 roll) {
	PHD_3DPOS viewPos;
	VECTOR_ANGLES angles;

	phd_GetVectorAngles(xtar - xsrc, ytar - ysrc, ztar - zsrc, &angles);
	viewPos.x = xsrc;
	viewPos.y = ysrc;
	viewPos.z = zsrc;
	viewPos.rotX = angles.pitch;
	viewPos.rotY = angles.yaw;
	viewPos.rotZ = roll;
	phd_GenerateW2V(&viewPos);
}

void __cdecl phd_GetVectorAngles(int x, int y, int z, VECTOR_ANGLES *angles) {
	__int16 pitch;

	angles->yaw = phd_atan(z, x);

	while( (__int16)x != x || (__int16)y != y || (__int16)z != z ) {
		x >>= 2;
		y >>= 2;
		z >>= 2;
	}
	pitch = phd_atan(phd_sqrt(SQR(x) + SQR(z)), y);

	if( (y > 0 && pitch > 0) || (y < 0 && pitch < 0) )
		pitch = -pitch;

	angles->pitch = pitch;
}

void __cdecl phd_RotX(__int16 angle) {
	if( angle != 0 ) {
		int m0, m1;
		int sx = phd_sin(angle);
		int cx = phd_cos(angle);

		m0 = PhdMatrixPtr->_01 * cx + PhdMatrixPtr->_02 * sx;
		m1 = PhdMatrixPtr->_02 * cx - PhdMatrixPtr->_01 * sx;
		PhdMatrixPtr->_01 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_02 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_11 * cx + PhdMatrixPtr->_12 * sx;
		m1 = PhdMatrixPtr->_12 * cx - PhdMatrixPtr->_11 * sx;
		PhdMatrixPtr->_11 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_12 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_21 * cx + PhdMatrixPtr->_22 * sx;
		m1 = PhdMatrixPtr->_22 * cx - PhdMatrixPtr->_21 * sx;
		PhdMatrixPtr->_21 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_22 = m1 >> W2V_SHIFT;
	}
}

void __cdecl phd_RotY(__int16 angle) {
	if( angle != 0 ) {
		int m0, m1;
		int sy = phd_sin(angle);
		int cy = phd_cos(angle);

		m0 = PhdMatrixPtr->_00 * cy - PhdMatrixPtr->_02 * sy;
		m1 = PhdMatrixPtr->_02 * cy + PhdMatrixPtr->_00 * sy;
		PhdMatrixPtr->_00 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_02 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_10 * cy - PhdMatrixPtr->_12 * sy;
		m1 = PhdMatrixPtr->_12 * cy + PhdMatrixPtr->_10 * sy;
		PhdMatrixPtr->_10 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_12 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_20 * cy - PhdMatrixPtr->_22 * sy;
		m1 = PhdMatrixPtr->_22 * cy + PhdMatrixPtr->_20 * sy;
		PhdMatrixPtr->_20 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_22 = m1 >> W2V_SHIFT;
	}
}

void __cdecl phd_RotZ(__int16 angle) {
	if( angle != 0 ) {
		int m0, m1;
		int sz = phd_sin(angle);
		int cz = phd_cos(angle);

		m0 = PhdMatrixPtr->_00 * cz + PhdMatrixPtr->_01 * sz;
		m1 = PhdMatrixPtr->_01 * cz - PhdMatrixPtr->_00 * sz;
		PhdMatrixPtr->_00 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_01 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_10 * cz + PhdMatrixPtr->_11 * sz;
		m1 = PhdMatrixPtr->_11 * cz - PhdMatrixPtr->_10 * sz;
		PhdMatrixPtr->_10 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_11 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_20 * cz + PhdMatrixPtr->_21 * sz;
		m1 = PhdMatrixPtr->_21 * cz - PhdMatrixPtr->_20 * sz;
		PhdMatrixPtr->_20 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_21 = m1 >> W2V_SHIFT;
	}
}

void __cdecl phd_RotYXZ(__int16 ry, __int16 rx, __int16 rz) {
	int sx, cx;
	int sy, cy;
	int sz, cz;
	int m0, m1;

	if( ry != 0 ) {
		sy = phd_sin(ry);
		cy = phd_cos(ry);
		m0 = PhdMatrixPtr->_00 * cy - PhdMatrixPtr->_02 * sy;
		m1 = PhdMatrixPtr->_02 * cy + PhdMatrixPtr->_00 * sy;
		PhdMatrixPtr->_00 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_02 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_10 * cy - PhdMatrixPtr->_12 * sy;
		m1 = PhdMatrixPtr->_12 * cy + PhdMatrixPtr->_10 * sy;
		PhdMatrixPtr->_10 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_12 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_20 * cy - PhdMatrixPtr->_22 * sy;
		m1 = PhdMatrixPtr->_22 * cy + PhdMatrixPtr->_20 * sy;
		PhdMatrixPtr->_20 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_22 = m1 >> W2V_SHIFT;
	}
	if( rx != 0 ) {
		sx = phd_sin(rx);
		cx = phd_cos(rx);
		m0 = PhdMatrixPtr->_01 * cx + PhdMatrixPtr->_02 * sx;
		m1 = PhdMatrixPtr->_02 * cx - PhdMatrixPtr->_01 * sx;
		PhdMatrixPtr->_01 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_02 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_11 * cx + PhdMatrixPtr->_12 * sx;
		m1 = PhdMatrixPtr->_12 * cx - PhdMatrixPtr->_11 * sx;
		PhdMatrixPtr->_11 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_12 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_21 * cx + PhdMatrixPtr->_22 * sx;
		m1 = PhdMatrixPtr->_22 * cx - PhdMatrixPtr->_21 * sx;
		PhdMatrixPtr->_21 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_22 = m1 >> W2V_SHIFT;
	}
	if( rz != 0 ) {
		sz = phd_sin(rz);
		cz = phd_cos(rz);
		m0 = PhdMatrixPtr->_00 * cz + PhdMatrixPtr->_01 * sz;
		m1 = PhdMatrixPtr->_01 * cz - PhdMatrixPtr->_00 * sz;
		PhdMatrixPtr->_00 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_01 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_10 * cz + PhdMatrixPtr->_11 * sz;
		m1 = PhdMatrixPtr->_11 * cz - PhdMatrixPtr->_10 * sz;
		PhdMatrixPtr->_10 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_11 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_20 * cz + PhdMatrixPtr->_21 * sz;
		m1 = PhdMatrixPtr->_21 * cz - PhdMatrixPtr->_20 * sz;
		PhdMatrixPtr->_20 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_21 = m1 >> W2V_SHIFT;
	}
}

void __cdecl phd_RotYXZpack(DWORD rpack) {
	int sx, cx;
	int sy, cy;
	int sz, cz;
	int m0, m1;
	__int16 rx = ((rpack >> 20) & 0x3FF) << 6;
	__int16 ry = ((rpack >> 10) & 0x3FF) << 6;
	__int16 rz = ((rpack >> 00) & 0x3FF) << 6;

	if( ry != 0 ) {
		sy = phd_sin(ry);
		cy = phd_cos(ry);
		m0 = PhdMatrixPtr->_00 * cy - PhdMatrixPtr->_02 * sy;
		m1 = PhdMatrixPtr->_02 * cy + PhdMatrixPtr->_00 * sy;
		PhdMatrixPtr->_00 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_02 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_10 * cy - PhdMatrixPtr->_12 * sy;
		m1 = PhdMatrixPtr->_12 * cy + PhdMatrixPtr->_10 * sy;
		PhdMatrixPtr->_10 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_12 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_20 * cy - PhdMatrixPtr->_22 * sy;
		m1 = PhdMatrixPtr->_22 * cy + PhdMatrixPtr->_20 * sy;
		PhdMatrixPtr->_20 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_22 = m1 >> W2V_SHIFT;
	}
	if( rx != 0 ) {
		sx = phd_sin(rx);
		cx = phd_cos(rx);
		m0 = PhdMatrixPtr->_01 * cx + PhdMatrixPtr->_02 * sx;
		m1 = PhdMatrixPtr->_02 * cx - PhdMatrixPtr->_01 * sx;
		PhdMatrixPtr->_01 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_02 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_11 * cx + PhdMatrixPtr->_12 * sx;
		m1 = PhdMatrixPtr->_12 * cx - PhdMatrixPtr->_11 * sx;
		PhdMatrixPtr->_11 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_12 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_21 * cx + PhdMatrixPtr->_22 * sx;
		m1 = PhdMatrixPtr->_22 * cx - PhdMatrixPtr->_21 * sx;
		PhdMatrixPtr->_21 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_22 = m1 >> W2V_SHIFT;
	}
	if( rz != 0 ) {
		sz = phd_sin(rz);
		cz = phd_cos(rz);
		m0 = PhdMatrixPtr->_00 * cz + PhdMatrixPtr->_01 * sz;
		m1 = PhdMatrixPtr->_01 * cz - PhdMatrixPtr->_00 * sz;
		PhdMatrixPtr->_00 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_01 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_10 * cz + PhdMatrixPtr->_11 * sz;
		m1 = PhdMatrixPtr->_11 * cz - PhdMatrixPtr->_10 * sz;
		PhdMatrixPtr->_10 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_11 = m1 >> W2V_SHIFT;
		m0 = PhdMatrixPtr->_20 * cz + PhdMatrixPtr->_21 * sz;
		m1 = PhdMatrixPtr->_21 * cz - PhdMatrixPtr->_20 * sz;
		PhdMatrixPtr->_20 = m0 >> W2V_SHIFT;
		PhdMatrixPtr->_21 = m1 >> W2V_SHIFT;
	}
}

BOOL __cdecl phd_TranslateRel(int x, int y, int z) {
	PhdMatrixPtr->_03 += PhdMatrixPtr->_00 * x + PhdMatrixPtr->_01 * y + PhdMatrixPtr->_02 * z;
	PhdMatrixPtr->_13 += PhdMatrixPtr->_10 * x + PhdMatrixPtr->_11 * y + PhdMatrixPtr->_12 * z;
	PhdMatrixPtr->_23 += PhdMatrixPtr->_20 * x + PhdMatrixPtr->_21 * y + PhdMatrixPtr->_22 * z;

	if( ABS(PhdMatrixPtr->_03) > PhdFarZ ||
		ABS(PhdMatrixPtr->_13) > PhdFarZ ||
		ABS(PhdMatrixPtr->_23) > PhdFarZ )
	{
		return FALSE;
	}
	return TRUE;
}

void __cdecl phd_TranslateAbs(int x, int y, int z) {
	x -= MatrixW2V._03;
	y -= MatrixW2V._13;
	z -= MatrixW2V._23;

	PhdMatrixPtr->_03 = x * PhdMatrixPtr->_00 + y * PhdMatrixPtr->_01 + z * PhdMatrixPtr->_02;
	PhdMatrixPtr->_13 = x * PhdMatrixPtr->_10 + y * PhdMatrixPtr->_11 + z * PhdMatrixPtr->_12;
	PhdMatrixPtr->_23 = x * PhdMatrixPtr->_20 + y * PhdMatrixPtr->_21 + z * PhdMatrixPtr->_22;
}

void __cdecl phd_PutPolygons(__int16 *ptrObj, int clip) {
	FltWinLeft = (float)PhdWinMinX;
	FltWinTop = (float)PhdWinMinY;
	FltWinRight = (float)(PhdWinMinX + PhdWinMaxX + 1);
	FltWinBottom = (float)(PhdWinMinY + PhdWinMaxY + 1);
	FltWinCenterX = (float)(PhdWinMinX + PhdWinCenterX);
	FltWinCenterY = (float)(PhdWinMinY + PhdWinCenterY);
#ifdef FEATURE_VIDEOFX_IMPROVED
	__int16 *ptrEnv = ptrObj;
#endif // FEATURE_VIDEOFX_IMPROVED

	ptrObj += 4; // skip x, y, z, radius
	ptrObj = calc_object_vertices(ptrObj);
	if( ptrObj != NULL ) {
		ptrObj = calc_vertice_light(ptrObj);
		ptrObj = ins_objectGT4(ptrObj+1, *ptrObj, ST_AvgZ);
		ptrObj = ins_objectGT3(ptrObj+1, *ptrObj, ST_AvgZ);
		ptrObj = ins_objectG4(ptrObj+1, *ptrObj, ST_AvgZ);
		ptrObj = ins_objectG3(ptrObj+1, *ptrObj, ST_AvgZ);
#ifdef FEATURE_VIDEOFX_IMPROVED
		phd_PutEnvmapPolygons(ptrEnv);
#endif // FEATURE_VIDEOFX_IMPROVED
	}
}

void __cdecl S_InsertRoom(__int16 *ptrObj, BOOL isOutside) {
	FltWinLeft = (float)(PhdWinMinX + PhdWinLeft);
	FltWinTop = (float)(PhdWinMinY + PhdWinTop);
	FltWinRight = (float)(PhdWinMinX + PhdWinRight + 1);
	FltWinBottom = (float)(PhdWinMinY + PhdWinBottom + 1);
	FltWinCenterX = (float)(PhdWinMinX + PhdWinCenterX);
	FltWinCenterY = (float)(PhdWinMinY + PhdWinCenterY);

	ptrObj = calc_roomvert(ptrObj, isOutside?0x00:0x10);
	ptrObj = ins_objectGT4(ptrObj+1, *ptrObj, ST_MaxZ);
	ptrObj = ins_objectGT3(ptrObj+1, *ptrObj, ST_MaxZ);
	ptrObj = ins_room_sprite(ptrObj+1, *ptrObj);
}

__int16 *__cdecl calc_background_light(__int16 *ptrObj) {
	int vtxCount = *ptrObj++;

	if( vtxCount > 0 ) {
		ptrObj += 3 * vtxCount;
	}
	else if( vtxCount < 0 ) {
		vtxCount = -vtxCount;
		ptrObj += vtxCount;
	}

	// Skybox has normal brightness
	int shade = 0x0FFF;

	// NOTE: Sunset did not change the skybox brightness in the original game
	if( GF_SunsetEnabled )
		shade += 0x400 * SunsetTimer / SUNSET_TIMEOUT;

	for( int i = 0; i < vtxCount; ++i )
		PhdVBuf[i].g = shade;

	return ptrObj;
}

void __cdecl S_InsertBackground(__int16 *ptrObj) {
	FltWinLeft = (float)(PhdWinMinX + PhdWinLeft);
	FltWinTop = (float)(PhdWinMinY + PhdWinTop);
	FltWinRight = (float)(PhdWinMinX + PhdWinRight + 1);
	FltWinBottom = (float)(PhdWinMinY + PhdWinBottom + 1);
	FltWinCenterX = (float)(PhdWinMinX + PhdWinCenterX);
	FltWinCenterY = (float)(PhdWinMinY + PhdWinCenterY);

	ptrObj += 4; // skip x, y, z, radius
	ptrObj = calc_object_vertices(ptrObj);
	if( ptrObj == NULL ) {
		return;
	}
	ptrObj = calc_background_light(ptrObj);

#ifdef FEATURE_VIEW_IMPROVED
	MidSort = 0xFFFF;
#endif // FEATURE_VIEW_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Hardware ) {
		HWR_EnableZBuffer(false, false);
	}
	ptrObj = ins_objectGT4(ptrObj+1, *ptrObj, ST_FarZ);
	ptrObj = ins_objectGT3(ptrObj+1, *ptrObj, ST_FarZ);
	ptrObj = ins_objectG4(ptrObj+1, *ptrObj, ST_FarZ);
	ptrObj = ins_objectG3(ptrObj+1, *ptrObj, ST_FarZ);
	if( SavedAppSettings.RenderMode == RM_Hardware ) {
		HWR_EnableZBuffer(true, true);
	}
#ifdef FEATURE_VIEW_IMPROVED
	MidSort = 0;
#endif // FEATURE_VIEW_IMPROVED
}

void __cdecl S_InsertInvBgnd(__int16 *ptrObj) {
	// NOTE: Null function in the PC version.
	// But there is waving inventory function in the PlayStation version.
	// Main S_InsertInvBgnd() logic is similar to S_InsertBackground();
}

__int16 *__cdecl calc_object_vertices(__int16 *ptrObj) {
	double xv, yv, zv, persp, baseZ;
	int vtxCount;
	BYTE totalClip, clipFlags;

	baseZ = 0.0;
#ifndef FEATURE_VIEW_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Software || !SavedAppSettings.ZBuffer ) {
		baseZ = (double)(MidSort << (W2V_SHIFT + 8));
	}
#endif // !FEATURE_VIEW_IMPROVED

	totalClip = 0xFF;

	ptrObj++; // skip poly counter
	vtxCount = *(ptrObj++); // get vertex counter

	if( vtxCount < 0 ) {
		printf("vtxCount=%d", vtxCount);
	}

	for( int i = 0; i < vtxCount; ++i ) {
		xv = (double)(PhdMatrixPtr->_00 * ptrObj[0] +
					  PhdMatrixPtr->_01 * ptrObj[1] +
					  PhdMatrixPtr->_02 * ptrObj[2] +
					  PhdMatrixPtr->_03);

		yv = (double)(PhdMatrixPtr->_10 * ptrObj[0] +
					  PhdMatrixPtr->_11 * ptrObj[1] +
					  PhdMatrixPtr->_12 * ptrObj[2] +
					  PhdMatrixPtr->_13);

		zv = (double)(PhdMatrixPtr->_20 * ptrObj[0] +
					  PhdMatrixPtr->_21 * ptrObj[1] +
					  PhdMatrixPtr->_22 * ptrObj[2] +
					  PhdMatrixPtr->_23);

		PhdVBuf[i].xv = xv;
		PhdVBuf[i].yv = yv;

		if( zv < FltNearZ ) {
			clipFlags = 0x80;
			PhdVBuf[i].zv = zv;
		} else {
			clipFlags = 0;

			if( zv >= FltFarZ ) {
				zv = FltFarZ;
				PhdVBuf[i].zv = zv;
			} else {
				PhdVBuf[i].zv = zv + baseZ;
			}

			persp = FltPersp / zv;

			PhdVBuf[i].xs  = persp * xv + FltWinCenterX;
			PhdVBuf[i].ys  = persp * yv + FltWinCenterY;
			PhdVBuf[i].rhw = persp * FltRhwOPersp;

			if( PhdVBuf[i].xs < FltWinLeft )
				clipFlags |= 0x01;
			else if( PhdVBuf[i].xs > FltWinRight )
				clipFlags |= 0x02;

			if( PhdVBuf[i].ys < FltWinTop )
				clipFlags |= 0x04;
			else if( PhdVBuf[i].ys > FltWinBottom )
				clipFlags |= 0x08;
		}

		PhdVBuf[i].clip = clipFlags;
		totalClip &= clipFlags;
		ptrObj += 3;
	}
	return ( totalClip == 0 ) ? ptrObj : NULL;
}

__int16 *__cdecl calc_vertice_light(__int16 *ptrObj) {
	int i, xv, yv, zv;
	__int16 shade;
	int vtxCount = *ptrObj++;

	if( vtxCount > 0 ) {
		if( LsDivider != 0 ) {
			xv = (PhdMatrixPtr->_00 * LsVectorView.x +
				  PhdMatrixPtr->_10 * LsVectorView.y +
				  PhdMatrixPtr->_20 * LsVectorView.z) / LsDivider;
			yv = (PhdMatrixPtr->_01 * LsVectorView.x +
				  PhdMatrixPtr->_11 * LsVectorView.y +
				  PhdMatrixPtr->_21 * LsVectorView.z) / LsDivider;
			zv = (PhdMatrixPtr->_02 * LsVectorView.x +
				  PhdMatrixPtr->_12 * LsVectorView.y +
				  PhdMatrixPtr->_22 * LsVectorView.z) / LsDivider;

			for( i = 0; i < vtxCount; ++i ) {
				shade = LsAdder + ((ptrObj[0]*xv + ptrObj[1]*yv + ptrObj[2]*zv) >> 16);
				CLAMP(shade, 0, 0x1FFF);
				PhdVBuf[i].g = shade;
				ptrObj += 3;
			}
		} else {
			shade = LsAdder;
			CLAMP(shade, 0, 0x1FFF);
			for( i = 0; i < vtxCount; ++i ) {
				PhdVBuf[i].g = shade;
			}
			ptrObj += 3*vtxCount;
		}
	} else {
		for( i = 0; i < -vtxCount; ++i ) {
			shade = LsAdder + *ptrObj;
			CLAMP(shade, 0, 0x1FFF);
			PhdVBuf[i].g = shade;
			++ptrObj;
		}
	}
	return ptrObj;
}

__int16 *__cdecl calc_roomvert(__int16 *ptrObj, BYTE farClip) {
	double xv, yv, zv, persp, baseZ, depth;
	int vtxCount, zv_int;

	baseZ = 0.0;
#ifndef FEATURE_VIEW_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Software || !SavedAppSettings.ZBuffer ) {
		baseZ = (double)(MidSort << (W2V_SHIFT + 8));
	}
#endif // !FEATURE_VIEW_IMPROVED

	vtxCount = *(ptrObj++);

	for( int i = 0; i < vtxCount; ++i ) {
		xv = (double)(PhdMatrixPtr->_00 * ptrObj[0] +
					  PhdMatrixPtr->_01 * ptrObj[1] +
					  PhdMatrixPtr->_02 * ptrObj[2] +
					  PhdMatrixPtr->_03);

		yv = (double)(PhdMatrixPtr->_10 * ptrObj[0] +
					  PhdMatrixPtr->_11 * ptrObj[1] +
					  PhdMatrixPtr->_12 * ptrObj[2] +
					  PhdMatrixPtr->_13);

		zv_int =	 (PhdMatrixPtr->_20 * ptrObj[0] +
					  PhdMatrixPtr->_21 * ptrObj[1] +
					  PhdMatrixPtr->_22 * ptrObj[2] +
					  PhdMatrixPtr->_23);

		zv = (double)zv_int;
		PhdVBuf[i].xv = xv;
		PhdVBuf[i].yv = yv;

		PhdVBuf[i].g = ptrObj[5];
		if( IsWaterEffect != 0 )
			PhdVBuf[i].g += ShadesTable[(WibbleOffset + (BYTE)RandomTable[(vtxCount - i) % WIBBLE_SIZE]) % WIBBLE_SIZE];

		if( zv < FltNearZ ) {
			PhdVBuf[i].clip = 0xFF80;
			PhdVBuf[i].zv = zv;
		} else {
			persp = FltPersp / zv;
			depth = zv_int >> W2V_SHIFT;

#ifdef FEATURE_VIEW_IMPROVED
			if( depth >= PhdViewDistance ) {
				PhdVBuf[i].rhw = persp * FltRhwOPersp;
				PhdVBuf[i].zv = zv + baseZ;
#else // !FEATURE_VIEW_IMPROVED
			if( depth >= DEPTHQ_END ) { // fog end
				PhdVBuf[i].rhw = 0.0; // NOTE: zero RHW is an invalid value, but the original game sets it.
				PhdVBuf[i].zv = FltFarZ;
#endif // FEATURE_VIEW_IMPROVED
				PhdVBuf[i].g = 0x1FFF;
				PhdVBuf[i].clip = farClip;
			} else {
#ifdef FEATURE_VIEW_IMPROVED
				PhdVBuf[i].g += CalculateFogShade(depth);
#else // !FEATURE_VIEW_IMPROVED
				if( depth > DEPTHQ_START ) { // fog begin
					PhdVBuf[i].g += depth - DEPTHQ_START;
				}
#endif // FEATURE_VIEW_IMPROVED
				PhdVBuf[i].rhw = persp * FltRhwOPersp;
				PhdVBuf[i].clip = 0;
				PhdVBuf[i].zv = zv + baseZ;
			}

			PhdVBuf[i].xs = persp * xv + FltWinCenterX;
			PhdVBuf[i].ys = persp * yv + FltWinCenterY;

			if( IsWibbleEffect && ptrObj[4] >= 0 ) {
				PhdVBuf[i].xs += WibbleTable[(WibbleOffset + (BYTE)PhdVBuf[i].ys) % WIBBLE_SIZE];
				PhdVBuf[i].ys += WibbleTable[(WibbleOffset + (BYTE)PhdVBuf[i].xs) % WIBBLE_SIZE];
			}

			if( PhdVBuf[i].xs < FltWinLeft )
				PhdVBuf[i].clip |= 0x01;
			else if( PhdVBuf[i].xs > FltWinRight )
				PhdVBuf[i].clip |= 0x02;

			if( PhdVBuf[i].ys < FltWinTop )
				PhdVBuf[i].clip |= 0x04;
			else if( PhdVBuf[i].ys > FltWinBottom )
				PhdVBuf[i].clip |= 0x08;

			PhdVBuf[i].clip |= ~(BYTE)(PhdVBuf[i].zv / 0x155555.p0) << 8;
		}
		CLAMP(PhdVBuf[i].g, 0, 0x1FFF);
		ptrObj += 6;
	}
	return ptrObj;
}

void __cdecl phd_RotateLight(__int16 pitch, __int16 yaw) {
	int xcos, ysin, wcos, wsin;
	int ls_x, ls_y, ls_z;

	PhdLsYaw = yaw;
	PhdLsPitch = pitch;

	xcos = phd_cos(pitch);
	ysin = phd_sin(pitch);

	wcos = phd_cos(yaw);
	wsin = phd_sin(yaw);

	ls_x = TRIGMULT2(xcos, wsin);
	ls_y = -ysin;
	ls_z = TRIGMULT2(xcos, wcos);

	LsVectorView.x = (MatrixW2V._00 * ls_x + MatrixW2V._01 * ls_y + MatrixW2V._02 * ls_z) >> W2V_SHIFT;
	LsVectorView.y = (MatrixW2V._10 * ls_x + MatrixW2V._11 * ls_y + MatrixW2V._12 * ls_z) >> W2V_SHIFT;
	LsVectorView.z = (MatrixW2V._20 * ls_x + MatrixW2V._21 * ls_y + MatrixW2V._22 * ls_z) >> W2V_SHIFT;
}

void __cdecl phd_InitPolyList() {
	SurfaceCount = 0;
	Sort3dPtr = SortBuffer;
	Info3dPtr = Info3dBuffer;
	if( SavedAppSettings.RenderMode == RM_Hardware )
		HWR_VertexPtr = HWR_VertexBuffer;
}

void __cdecl phd_SortPolyList() {
	if( SurfaceCount ) {
		for( DWORD i=0; i<SurfaceCount; ++i ) {
#ifdef FEATURE_VIEW_IMPROVED
			SortBuffer[i]._1 <<= 16;
#endif // FEATURE_VIEW_IMPROVED
			SortBuffer[i]._1 += i;
		}
		do_quickysorty(0, SurfaceCount-1);
	}
}

void __cdecl do_quickysorty(int left, int right) {
#ifdef FEATURE_VIEW_IMPROVED
	UINT64 swapBuf;
	UINT64 compare = SortBuffer[(left + right) / 2]._1;
#else // FEATURE_VIEW_IMPROVED
	DWORD swapBuf;
	DWORD compare = SortBuffer[(left + right) / 2]._1;
#endif // FEATURE_VIEW_IMPROVED
	int i = left;
	int j = right;

	do {
		while( (i < right) && (SortBuffer[i]._1 > compare) ) ++i;
		while( (left <  j) && (compare > SortBuffer[j]._1) ) --j;
		if( i > j ) break;

		SWAP(SortBuffer[i]._0, SortBuffer[j]._0, swapBuf);
		SWAP(SortBuffer[i]._1, SortBuffer[j]._1, swapBuf);
	} while( ++i <= --j );

	if( left < j )
		do_quickysorty(left, j);
	if( i < right )
		do_quickysorty(i, right);
}

void __cdecl phd_PrintPolyList(BYTE *surfacePtr) {
	__int16 polyType, *bufPtr;
	PrintSurfacePtr = surfacePtr;

	for( DWORD i=0; i<SurfaceCount; ++i ) {
		bufPtr = (__int16 *)SortBuffer[i]._0;
		polyType = *(bufPtr++); // poly has type as routine index in first word
		PolyDrawRoutines[polyType](bufPtr); // send poly data as parameter to routine
	}
}

void __cdecl AlterFOV(__int16 fov) {
	fov /= 2; // half fov angle

#ifdef FEATURE_VIEW_IMPROVED
	int fovWidth = PhdWinHeight*320/(PsxFovEnabled ? 200 : 240);
	FltViewAspect = 1.0; // must always be 1.0 for unstretched view
	PhdPersp = (fovWidth / 2) * phd_cos(fov) / phd_sin(fov);
#else // !FEATURE_VIEW_IMPROVED
	PhdPersp = (PhdWinWidth / 2) * phd_cos(fov) / phd_sin(fov);
#endif // FEATURE_VIEW_IMPROVED

	FltPersp = (float)PhdPersp;
	FltRhwOPersp = RhwFactor / FltPersp;
	FltPerspONearZ = FltPersp / FltNearZ;

#ifndef FEATURE_VIEW_IMPROVED // if feature is not defined!!!
	double windowAspect = 4.0 / 3.0;
	if( !SavedAppSettings.FullScreen && SavedAppSettings.AspectMode == AM_16_9 ) {
		windowAspect = 16.0 / 9.0;
	}
	FltViewAspect = windowAspect / ((double)PhdWinWidth / (double)PhdWinHeight);
#endif // !FEATURE_VIEW_IMPROVED
}

void __cdecl phd_SetNearZ(int nearZ) {
	PhdNearZ = nearZ;
	FltNearZ = (float)nearZ;
	FltRhwONearZ = RhwFactor / FltNearZ;
	FltPerspONearZ = FltPersp / FltNearZ;

	double resZ = 0.99 * FltFarZ * FltNearZ / (FltFarZ - FltNearZ);

	FltResZ = resZ;
	FltResZORhw = resZ / RhwFactor;
	FltResZBuf = 0.005 + resZ / FltNearZ;
}

void __cdecl phd_SetFarZ(int farZ) {
	PhdFarZ = farZ;
	FltFarZ = (float)farZ;

	double resZ = 0.99 * FltFarZ * FltNearZ / (FltFarZ - FltNearZ);

	FltResZ = resZ;
	FltResZORhw = resZ / RhwFactor;
	FltResZBuf = 0.005 + resZ / FltNearZ;

}

void __cdecl phd_InitWindow(__int16 x, __int16 y, int width, int height, int nearZ, int farZ, __int16 viewAngle, int screenWidth, int screenHeight) {
	PhdWinMinX = x;
	PhdWinMinY = y;
	PhdWinMaxX = width - 1;
	PhdWinMaxY = height - 1;

	PhdWinWidth = width;
	PhdWinHeight = height;

	PhdWinCenterX = width / 2;
	PhdWinCenterY = height / 2;
	FltWinCenterX = (float)PhdWinCenterX;
	FltWinCenterY = (float)PhdWinCenterY;

	PhdWinLeft = 0;
	PhdWinTop = 0;
	PhdWinRight = PhdWinMaxX;
	PhdWinBottom = PhdWinMaxY;

	PhdWinRect.left = PhdWinMinX;
	PhdWinRect.bottom = PhdWinMinY + PhdWinHeight;
	PhdWinRect.top = PhdWinMinY;
	PhdWinRect.right = PhdWinMinX + PhdWinWidth;

	PhdScreenWidth = screenWidth;
	PhdScreenHeight = screenHeight;

#ifdef FEATURE_VIEW_IMPROVED
	double baseDistance = (double)farZ;
	farZ				= (int)(baseDistance * ViewDistanceFactor);
	FogBeginDepth		= (int)(baseDistance * FogBeginFactor);
	FogEndDepth			= (int)(baseDistance * FogEndFactor);
	WaterFogBeginDepth	= (int)(baseDistance * WaterFogBeginFactor);
	WaterFogEndDepth	= (int)(baseDistance * WaterFogEndFactor);
#endif // FEATURE_VIEW_IMPROVED

	PhdNearZ = nearZ << W2V_SHIFT;
	PhdFarZ = farZ << W2V_SHIFT;
	PhdViewDistance = farZ;

	AlterFOV(PHD_DEGREE * viewAngle); // convert degrees to PHD angle
	phd_SetNearZ(PhdNearZ);
	phd_SetFarZ(PhdFarZ);

	PhdMatrixPtr = MatrixStack; // reset matrix stack pointer

	if( SavedAppSettings.RenderMode == RM_Software ) {
		PerspectiveDistance = SavedAppSettings.PerspectiveCorrect ? SW_DETAIL_HIGH : SW_DETAIL_MEDIUM;

		ins_objectGT3	= InsertObjectGT3;
		ins_objectGT4	= InsertObjectGT4;
		ins_objectG3	= InsertObjectG3;
		ins_objectG4	= InsertObjectG4;
		ins_flat_rect	= InsertFlatRect;
		ins_line		= InsertLine;

		ins_sprite		= InsertSprite;
		ins_poly_trans8	= InsertTrans8;
		ins_trans_quad	= InsertTransQuad;
	}
	else if( SavedAppSettings.RenderMode == RM_Hardware ) {
		if( SavedAppSettings.ZBuffer ) {
			ins_objectGT3	= InsertObjectGT3_ZBuffered;
			ins_objectGT4	= InsertObjectGT4_ZBuffered;
			ins_objectG3	= InsertObjectG3_ZBuffered;
			ins_objectG4	= InsertObjectG4_ZBuffered;
			ins_flat_rect	= InsertFlatRect_ZBuffered;
			ins_line		= InsertLine_ZBuffered;
		} else {
			ins_objectGT3	= InsertObjectGT3_Sorted;
			ins_objectGT4	= InsertObjectGT4_Sorted;
			ins_objectG3	= InsertObjectG3_Sorted;
			ins_objectG4	= InsertObjectG4_Sorted;
			ins_flat_rect	= InsertFlatRect_Sorted;
			ins_line		= InsertLine_Sorted;
		}
		ins_sprite		= InsertSprite_Sorted;
		ins_poly_trans8	= InsertTrans8_Sorted;
		ins_trans_quad	= InsertTransQuad_Sorted;
	}
}

void __cdecl phd_PopMatrix() {
	--PhdMatrixPtr;
}

void __cdecl phd_PushMatrix() {
	PhdMatrixPtr[1] = PhdMatrixPtr[0]; // copy the last matrix
	++PhdMatrixPtr;
}

void __cdecl phd_PushUnitMatrix() {
	++PhdMatrixPtr;
	memset(PhdMatrixPtr, 0, sizeof(PHD_MATRIX));
	PhdMatrixPtr->_00 = W2V_SCALE;
	PhdMatrixPtr->_11 = W2V_SCALE;
	PhdMatrixPtr->_22 = W2V_SCALE;
}

/*
 * Inject function
 */
void Inject_3Dgen() {
	INJECT(0x00401000, phd_GenerateW2V);
	INJECT(0x004011D0, phd_LookAt);
	INJECT(0x00401250, phd_GetVectorAngles);
	INJECT(0x004012D0, phd_RotX);
	INJECT(0x00401380, phd_RotY);
	INJECT(0x00401430, phd_RotZ);
	INJECT(0x004014E0, phd_RotYXZ);
	INJECT(0x004016C0, phd_RotYXZpack);
	INJECT(0x004018B0, phd_TranslateRel);
	INJECT(0x00401960, phd_TranslateAbs);
	INJECT(0x004019E0, phd_PutPolygons);
	INJECT(0x00401AE0, S_InsertRoom);
	INJECT(0x00401BD0, calc_background_light);
	INJECT(0x00401C10, S_InsertBackground);
//	INJECT(----------, S_InsertInvBgnd); // NOTE: this is null in the original code
	INJECT(0x00401D50, calc_object_vertices);
	INJECT(0x00401F30, calc_vertice_light);
	INJECT(0x004020A0, calc_roomvert);
	INJECT(0x00402320, phd_RotateLight);
	INJECT(0x004023F0, phd_InitPolyList);
	INJECT(0x00402420, phd_SortPolyList);
	INJECT(0x00402460, do_quickysorty);
	INJECT(0x00402530, phd_PrintPolyList);
	INJECT(0x00402570, AlterFOV);
	INJECT(0x00402680, phd_SetNearZ);
	INJECT(0x004026D0, phd_SetFarZ);
	INJECT(0x004026F0, phd_InitWindow);
//	INJECT(----------, phd_PopMatrix); // NOTE: this is inline or macro in the original code
	INJECT(0x00457510, phd_PushMatrix);
	INJECT(0x0045752E, phd_PushUnitMatrix);
}
