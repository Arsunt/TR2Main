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
#include "modding/mod_utils.h"
#include "global/vars.h"

#ifdef FEATURE_MOD_CONFIG
#include "json-parser/json.h"

#define MOD_CONFIG_NAME "TR2Main.json"

typedef struct {
	bool isLoaded;
	char loadingPix[256];
} MOD_CONFIG;

static MOD_CONFIG ModConfig;
#endif // FEATURE_MOD_CONFIG

static bool IsCompatibleFilter(__int16 *ptrObj, POLYFILTER *filter) {
	if( !ptrObj || !filter || !filter->n_vtx ) return true;
	ptrObj += 5; // skip x, y, z, radius, flags
	__int16 num = *(ptrObj++); // get vertex counter
	if( num != filter->n_vtx ) return false;
	ptrObj += num * 3; // skip vertices
	num = *(ptrObj++); // get normal counter
	ptrObj += (num > 0) ? num * 3 : ABS(num); // skip normals/shades
	num = *(ptrObj++); // get gt4 number
	if( num != filter->n_gt4 ) return false;
	ptrObj += num * 5; // skip gt4 polys
	num = *(ptrObj++); // get gt3 number
	if( num != filter->n_gt3 ) return false;
	ptrObj += num * 4; // skip gt3 polys
	num = *(ptrObj++); // get g4 number
	if( num != filter->n_g4 ) return false;
	ptrObj += num * 5; // skip g4 polys
	num = *(ptrObj++); // get g3 number
	if( num != filter->n_g3 ) return false;
	return true;
}

static __int16 *EnumeratePolysSpecific(__int16 *ptrObj, int vtxCount, bool colored, ENUM_POLYS_CB callback, POLYINDEX *filter, LPVOID param) {
	int polyNumber = *ptrObj++;
	if( filter == NULL || (!filter[0].idx && !filter[0].num) ) {
		for( int i = 0; i < polyNumber; ++i ) {
			if( !callback(ptrObj, vtxCount, colored, param) ) return NULL;
			ptrObj += vtxCount + 1;
		}
	} else {
		int polyIndex = 0;
		for( int i=0; i<POLYFILTER_SIZE; i++ ) {
			if( filter[i].idx < polyIndex || filter[i].idx >= polyNumber ) {
				break;
			}
			int skip = filter[i].idx - polyIndex;
			if( skip > 0 ) {
				ptrObj += skip*(vtxCount+1);
				polyIndex += skip;
			}
			int number = MIN(filter[i].num, polyNumber - polyIndex);
			for( int j = 0; j < number; ++j ) {
				if( !callback(ptrObj, vtxCount, colored, param) ) return NULL;
				ptrObj += vtxCount + 1;
			}
			polyIndex += number;
		}
		ptrObj += (polyNumber-polyIndex)*(vtxCount+1);
	}
	return ptrObj;
}

bool EnumeratePolys(__int16 *ptrObj, ENUM_POLYS_CB callback, POLYFILTER *filter, LPVOID param) {
	if( ptrObj == NULL || callback == NULL ) return false; // wrong parameters
	if( !IsCompatibleFilter(ptrObj, filter) ) return false; // filter is not compatible

	__int16 num;
	ptrObj += 5; // skip x, y, z, radius, flags
	num = *(ptrObj++); // get vertex counter
	ptrObj += num * 3; // skip vertices
	num = *(ptrObj++); // get normal counter
	ptrObj += (num > 0) ? num * 3 : ABS(num); // skip normals/shades

	ptrObj = EnumeratePolysSpecific(ptrObj, 4, false, callback, filter ? filter->gt4 : NULL, param); // enumerate textured quads
	if( ptrObj == NULL ) return true;
	ptrObj = EnumeratePolysSpecific(ptrObj, 3, false, callback, filter ? filter->gt3 : NULL, param); // enumerate textured triangles
	if( ptrObj == NULL ) return true;
	ptrObj = EnumeratePolysSpecific(ptrObj, 4, true, callback, filter ? filter->g4 : NULL, param); // enumerate colored quads
	if( ptrObj == NULL ) return true;
	ptrObj = EnumeratePolysSpecific(ptrObj, 3, true, callback, filter ? filter->g3 : NULL, param); // enumerate colored triangles
	return true;
}

#ifdef FEATURE_MOD_CONFIG
bool IsModConfigLoaded() {
	return ModConfig.isLoaded;
}

const char *GetModLoadingPix() {
	return *ModConfig.loadingPix ? ModConfig.loadingPix : NULL;
}

static json_value *GetJsonField(json_value *root, json_type fieldType, const char *name, DWORD *pIndex) {
	if( root == NULL || root->type != json_object ) {
		return NULL;
	}
	json_value *result = NULL;
	DWORD len = name ? strlen(name) : 0;
	DWORD i = pIndex ? *pIndex : 0;
	for( ; i < root->u.object.length; ++i ) {
		if( root->u.object.values[i].value->type == fieldType ) {
			if( !name || (len == root->u.object.values[i].name_length
				&& !strncmp(root->u.object.values[i].name, name, len)) )
			{
				result = root->u.object.values[i].value;
				break;
			}
		}
	}
	if( pIndex ) *pIndex = i;
	return result;
}

static json_value *GetJsonObjectByStringField(json_value *root, const char *name, const char *str, bool caseSensitive, DWORD *pIndex) {
	if( root == NULL || root->type != json_array || !name || !*name || !str ) {
		return NULL;
	}
	json_value *result = NULL;
	DWORD len = strlen(str);
	DWORD i = pIndex ? *pIndex : 0;
	for( ; i < root->u.array.length; ++i ) {
		json_value *key = GetJsonField(root->u.array.values[i], json_string, name, NULL);
		if( len == key->u.string.length &&
			(caseSensitive ? strncmp(key->u.string.ptr, str, len) : !strncasecmp(key->u.string.ptr, str, len)) )
		{
			result = root->u.array.values[i];
			break;
		}
	}
	if( pIndex ) *pIndex = i;
	return result;
}

static bool ParseLevelConfiguration(json_value *root) {
	if( root == NULL || root->type != json_object ) {
		return false;
	}
	json_value* field = NULL;

	field = GetJsonField(root, json_string, "picture", NULL);
	if( field ) {
		snprintf(ModConfig.loadingPix, sizeof(ModConfig.loadingPix), "data\\%.*s.pcx", field->u.string.length, field->u.string.ptr);
	}
	return true;
}

static bool ParseModConfiguration(char *levelName, json_value *root) {
	if( root == NULL || root->type != json_object ) {
		return false;
	}
	// parsing default configs
	ParseLevelConfiguration(GetJsonField(root, json_object, "default", NULL));
	// parsing level specific configs
	json_value* levels = GetJsonField(root, json_array, "levels", NULL);
	if( levels ) ParseLevelConfiguration(GetJsonObjectByStringField(levels, "filename", levelName, false, NULL));
	return true;
}

void UnloadModConfiguration() {
	memset(&ModConfig, 0, sizeof(ModConfig));
}

bool LoadModConfiguration(LPCTSTR levelFilePath) {
	UnloadModConfiguration();
	if( !PathFileExists(MOD_CONFIG_NAME) ) {
		return false;
	}
	char levelName[256] = {0};
	strncpy(levelName, PathFindFileName(levelFilePath), sizeof(levelName));
	char *ext = PathFindExtension(levelName);
	if( ext != NULL ) *ext = 0;

	DWORD bytesRead = 0;
	HANDLE hFile = CreateFile(MOD_CONFIG_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN|FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE ) {
		return false;
	}
	DWORD cfgSize = GetFileSize(hFile, NULL);
	void *cfgData = malloc(cfgSize);
	ReadFile(hFile, cfgData, cfgSize, &bytesRead, NULL);
	CloseHandle(hFile);

	json_char* json = (json_char*)cfgData;
	json_value* value = json_parse(json, cfgSize);
	if( value != NULL ) {
		ModConfig.isLoaded = ParseModConfiguration(levelName, value);
	}
	json_value_free(value);
	free(cfgData);
	return ModConfig.isLoaded;
}
#endif // FEATURE_MOD_CONFIG
