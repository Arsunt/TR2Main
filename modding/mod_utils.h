/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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

#ifndef MOD_UTILS_H_INCLUDED
#define MOD_UTILS_H_INCLUDED

#include "global/types.h"

// Filter is presented by an array of poly index and polys number (starting from the index).
// The filter must be always terminated by an index 0.
// If the first item has index=~0 then there are no polys of such type to process.
// If the first item has index=0 and number=0 then all polys of such type must be processed.
#define POLYFILTER_SIZE 256

typedef struct {__int16 idx; __int16 num;} POLYINDEX;

typedef struct {
	__int16 n_vtx, n_gt4, n_gt3, n_g4, n_g3;
	POLYINDEX gt4[POLYFILTER_SIZE];
	POLYINDEX gt3[POLYFILTER_SIZE];
	POLYINDEX g4[POLYFILTER_SIZE];
	POLYINDEX g3[POLYFILTER_SIZE];
} POLYFILTER;

typedef struct PolyfilterNode_t {
	int id;
	POLYFILTER filter;
	struct PolyfilterNode_t *next;
} POLYFILTER_NODE;

typedef bool (*ENUM_POLYS_CB) (__int16 *ptrObj, int vtxCount, bool colored, LPVOID param);

/*
 * Function list
 */
bool EnumeratePolys(__int16 *ptrObj, bool isRoomMesh, ENUM_POLYS_CB callback, POLYFILTER *filter, LPVOID param);

#ifdef FEATURE_MOD_CONFIG
bool IsModConfigLoaded();
bool IsModBarefoot();
const char *GetModLoadingPix();
DWORD GetModWaterColor();
bool IsModSemitransConfigLoaded();
POLYINDEX *GetModSemitransAnimtexFilter();
POLYFILTER_NODE *GetModSemitransRoomsFilter();
POLYFILTER_NODE *GetModSemitransStaticsFilter();
POLYFILTER_NODE **GetModSemitransObjectsFilter();
bool IsModReflectConfigLoaded();
POLYFILTER_NODE *GetModReflectStaticsFilter();
POLYFILTER_NODE **GetModReflectObjectsFilter();

void UnloadModConfiguration();
bool LoadModConfiguration(LPCTSTR levelFilePath);
#endif // FEATURE_MOD_CONFIG

#endif // MOD_UTILS_H_INCLUDED
