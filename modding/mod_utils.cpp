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
	POLYINDEX *animtex;
	POLYFILTER_NODE *rooms;
	POLYFILTER_NODE *statics;
	POLYFILTER_NODE *objects[ID_NUMBER_OBJECTS];
} SEMITRANS_CONFIG;

typedef struct {
	bool isLoaded;
	POLYFILTER_NODE *statics;
	POLYFILTER_NODE *objects[ID_NUMBER_OBJECTS];
} REFLECT_CONFIG;

typedef struct {
	bool isLoaded;
	bool isBarefoot;
	char loadingPix[256];
	DWORD waterColor;
	SEMITRANS_CONFIG semitrans;
	REFLECT_CONFIG reflect;
} MOD_CONFIG;

static MOD_CONFIG ModConfig;

static POLYFILTER *CreatePolyfilterNode(POLYFILTER_NODE **root, int id) {
	if( root == NULL ) return NULL;
	POLYFILTER_NODE *node = (POLYFILTER_NODE *)malloc(sizeof(POLYFILTER_NODE));
	if( node == NULL ) return NULL;
	node->id = id;
	node->next = *root;
	memset(&node->filter, 0, sizeof(node->filter));
	*root = node;
	return &node->filter;
}

static void FreePolyfilterNodes(POLYFILTER_NODE **root) {
	if( root == NULL ) return;
	POLYFILTER_NODE *node = *root;
	while( node ) {
		POLYFILTER_NODE *next = node->next;
		free(node);
		node = next;
	}
	*root = NULL;
}
#endif // FEATURE_MOD_CONFIG

static bool IsCompatibleFilter(__int16 *ptrObj, bool isRoomMesh, POLYFILTER *filter) {
	if( !ptrObj || !filter || !filter->n_vtx ) return true;
	if( !isRoomMesh ) {
		ptrObj += 5; // skip x, y, z, radius, flags
	}
	__int16 num = *(ptrObj++); // get vertex counter
	if( num != filter->n_vtx ) return false;
	ptrObj += num * (isRoomMesh ? 6 : 3); // skip vertices
	if( !isRoomMesh ) {
		num = *(ptrObj++); // get normal counter
		ptrObj += (num > 0) ? num * 3 : ABS(num); // skip normals/shades
	}
	num = *(ptrObj++); // get gt4 number
	if( num != filter->n_gt4 ) return false;
	ptrObj += num * 5; // skip gt4 polys
	num = *(ptrObj++); // get gt3 number
	if( num != filter->n_gt3 ) return false;
	if( !isRoomMesh ) {
		ptrObj += num * 4; // skip gt3 polys
		num = *(ptrObj++); // get g4 number
		if( num != filter->n_g4 ) return false;
		ptrObj += num * 5; // skip g4 polys
		num = *(ptrObj++); // get g3 number
		if( num != filter->n_g3 ) return false;
	}
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

bool EnumeratePolys(__int16 *ptrObj, bool isRoomMesh, ENUM_POLYS_CB callback, POLYFILTER *filter, LPVOID param) {
	if( ptrObj == NULL || callback == NULL ) return false; // wrong parameters
	if( !IsCompatibleFilter(ptrObj, isRoomMesh, filter) ) return false; // filter is not compatible

	__int16 num;
	if( !isRoomMesh ) {
		ptrObj += 5; // skip x, y, z, radius, flags
	}
	num = *(ptrObj++); // get vertex counter
	ptrObj += num * (isRoomMesh ? 6 : 3); // skip vertices
	if( !isRoomMesh ) {
		num = *(ptrObj++); // get normal counter
		ptrObj += (num > 0) ? num * 3 : ABS(num); // skip normals/shades
	}
	ptrObj = EnumeratePolysSpecific(ptrObj, 4, false, callback, filter ? filter->gt4 : NULL, param); // enumerate textured quads
	if( ptrObj == NULL ) return true;
	ptrObj = EnumeratePolysSpecific(ptrObj, 3, false, callback, filter ? filter->gt3 : NULL, param); // enumerate textured triangles
	if( !isRoomMesh ) {
		if( ptrObj == NULL ) return true;
		ptrObj = EnumeratePolysSpecific(ptrObj, 4, true, callback, filter ? filter->g4 : NULL, param); // enumerate colored quads
		if( ptrObj == NULL ) return true;
		ptrObj = EnumeratePolysSpecific(ptrObj, 3, true, callback, filter ? filter->g3 : NULL, param); // enumerate colored triangles
	}
	return true;
}

#ifdef FEATURE_MOD_CONFIG
bool IsModConfigLoaded() {
	return ModConfig.isLoaded;
}

bool IsModBarefoot() {
	return ModConfig.isBarefoot;
}

const char *GetModLoadingPix() {
	return *ModConfig.loadingPix ? ModConfig.loadingPix : NULL;
}

DWORD GetModWaterColor() {
	return ModConfig.waterColor;
}

bool IsModSemitransConfigLoaded() {
	return ModConfig.semitrans.isLoaded;
}

POLYINDEX *GetModSemitransAnimtexFilter() {
	return ModConfig.semitrans.animtex;
}

POLYFILTER_NODE *GetModSemitransRoomsFilter() {
	return ModConfig.semitrans.rooms;
}

POLYFILTER_NODE *GetModSemitransStaticsFilter() {
	return ModConfig.semitrans.statics;
}

POLYFILTER_NODE **GetModSemitransObjectsFilter() {
	return ModConfig.semitrans.objects;
}

bool IsModReflectConfigLoaded() {
	return ModConfig.reflect.isLoaded;
}

POLYFILTER_NODE *GetModReflectStaticsFilter() {
	return ModConfig.reflect.statics;
}

POLYFILTER_NODE **GetModReflectObjectsFilter() {
	return ModConfig.reflect.objects;
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
		if( key && len == key->u.string.length &&
			(caseSensitive ? strncmp(key->u.string.ptr, str, len) : !strncasecmp(key->u.string.ptr, str, len)) )
		{
			result = root->u.array.values[i];
			break;
		}
	}
	if( pIndex ) *pIndex = i;
	return result;
}

static int GetJsonIntegerFieldValue(json_value *root, const char *name) {
	json_value *field = GetJsonField(root, json_integer, name, NULL);
	return field ? field->u.integer : 0;
}

static int ParsePolyString(const char *str, POLYINDEX *lst, DWORD lstLen){
	if( !lst || !lstLen ) {
		return -1;
	}

	lst[0].idx = ~0;
	lst[0].num = ~0;

	POLYINDEX *lstBuf = (POLYINDEX *)malloc(lstLen * sizeof(POLYINDEX));
	if( lstBuf == NULL ) {
		return -2;
	}

	char *strBuf = strdup(str);
	if( strBuf == NULL ) {
		free(lstBuf);
		return -2;
	}

	DWORD bufLen = 0;
	char *token = strtok(strBuf, ",");
	while( token != NULL ) {
		char *range = strchr(token, '-');
		if( range ) {
			int from = atoi(token);
			int to = atoi(range + 1);
			lstBuf[bufLen].idx = MIN(to, from);
			lstBuf[bufLen].num = ABS(to - from) + 1;
		} else {
			lstBuf[bufLen].idx = atoi(token);
			lstBuf[bufLen].num = 1;
		}
		if( ++bufLen >= lstLen ) {
			break;
		}
		token = strtok(NULL, ",");
	}

	free(strBuf);
	if( !bufLen ) {
		free(lstBuf);
		return 0;
	}

	for( DWORD i = 0; i < bufLen-1; ++i ) {
		for( DWORD j = i+1; j < bufLen; ++j ) {
			if( lstBuf[i].idx > lstBuf[j].idx ) {
				POLYINDEX t;
				SWAP(lstBuf[i], lstBuf[j], t);
			}
		}
	}

	lst[0] = lstBuf[0];
	DWORD resLen = 1;

	for( DWORD i = 1; i < bufLen; ++i ) {
		int bound = lst[resLen-1].idx + lst[resLen-1].num;
		if( lstBuf[i].idx > bound ) {
			lst[resLen] = lstBuf[i];
			++resLen;
		} else {
			int ext = lstBuf[i].idx + lstBuf[i].num;
			if( ext > bound ) {
				lst[resLen-1].num += ext - bound;
			}
		}
	}
	if( resLen < lstLen ) {
		lst[resLen].idx = 0;
		lst[resLen].num = 0;
	}

	free(lstBuf);
	return resLen;
}

static int ParsePolyValue(json_value *value, POLYINDEX *lst, DWORD lstLen) {
	if( !lst || !lstLen ) {
		return -1;
	}

	lst[0].idx = ~0;
	lst[0].num = ~0;
	if( value == NULL ) {
		return 0;
	}

	const char *str = value->u.string.ptr;
	if( !str || !*str || !strcasecmp(str, "none") ) {
		return 0;
	}
	if( !strcasecmp(str, "all") ) {
		lst[0].idx = 0;
		lst[0].num = 0;
		return 1;
	}
	return ParsePolyString(str, lst, lstLen);
}

static bool ParsePolyfilterConfiguration(json_value *root, const char *name, POLYFILTER_NODE **pNodes) {
	FreePolyfilterNodes(pNodes);
	if( root == NULL || root->type != json_array || !name || !*name ) {
		return false;
	}
	for( DWORD i = 0; i < root->u.array.length; ++i ) {
		json_value *item = root->u.array.values[i];
		json_value *field = GetJsonField(item, json_integer, name, NULL);
		if( !field || field->u.integer < 0 ) continue;
		POLYFILTER *filter = CreatePolyfilterNode(pNodes, field->u.integer);
		if( !filter ) continue;
		field = GetJsonField(item, json_object, "filter", NULL);
		if( field ) {
			filter->n_vtx = GetJsonIntegerFieldValue(field, "v");
			filter->n_gt4 = GetJsonIntegerFieldValue(field, "t4");
			filter->n_gt3 = GetJsonIntegerFieldValue(field, "t3");
			filter->n_g4  = GetJsonIntegerFieldValue(field, "c4");
			filter->n_g3  = GetJsonIntegerFieldValue(field, "c3");
		}
		json_value *t4list = GetJsonField(item, json_string, "t4list", NULL);
		json_value *t3list = GetJsonField(item, json_string, "t3list", NULL);
		json_value *c4list = GetJsonField(item, json_string, "c4list", NULL);
		json_value *c3list = GetJsonField(item, json_string, "c3list", NULL);
		// If no lists presented, consider that lists set to "all"
		if( t4list || t3list || c4list || c3list ) {
			ParsePolyValue(t4list, filter->gt4, ARRAY_SIZE(filter->gt4));
			ParsePolyValue(t3list, filter->gt3, ARRAY_SIZE(filter->gt3));
			ParsePolyValue(c4list, filter->g4,  ARRAY_SIZE(filter->g4));
			ParsePolyValue(c3list, filter->g3,  ARRAY_SIZE(filter->g3));
		}
	}
	return true;
}

static bool ParseSemitransConfiguration(json_value *root) {
	if( root == NULL || root->type != json_object ) {
		return false;
	}
	json_value* field = NULL;

	field = GetJsonField(root, json_string, "animtex", NULL);
	if( field ) {
		if( ModConfig.semitrans.animtex ) {
			free(ModConfig.semitrans.animtex);
			ModConfig.semitrans.animtex = NULL;
		}
		if( strcasecmp(field->u.string.ptr, "auto") ) {
			ModConfig.semitrans.animtex = (POLYINDEX *)malloc(sizeof(POLYINDEX) * POLYFILTER_SIZE);
			if( ModConfig.semitrans.animtex ) {
				ParsePolyValue(field, ModConfig.semitrans.animtex, POLYFILTER_SIZE);
			}
		}
	}
	json_value* objects = GetJsonField(root, json_array, "objects", NULL);
	if( objects ) {
		for( DWORD i = 0; i < objects->u.array.length; ++i ) {
			json_value *object = objects->u.array.values[i];
			field = GetJsonField(object, json_integer, "object", NULL);
			if( !field || field->u.integer < 0 || field->u.integer >= ARRAY_SIZE(ModConfig.semitrans.objects) ) continue;
			ParsePolyfilterConfiguration(GetJsonField(object, json_array, "meshes", NULL), "mesh", &ModConfig.semitrans.objects[field->u.integer]);
		}
	}
	ParsePolyfilterConfiguration(GetJsonField(root, json_array, "statics", NULL), "static", &ModConfig.semitrans.statics);
	ParsePolyfilterConfiguration(GetJsonField(root, json_array, "rooms", NULL), "room", &ModConfig.semitrans.rooms);
	ModConfig.semitrans.isLoaded = true;
	return true;
}

static bool ParseReflectConfiguration(json_value *root) {
	if( root == NULL || root->type != json_object ) {
		return false;
	}
	json_value* field = NULL;

	json_value* objects = GetJsonField(root, json_array, "objects", NULL);
	if( objects ) {
		for( DWORD i = 0; i < objects->u.array.length; ++i ) {
			json_value *object = objects->u.array.values[i];
			field = GetJsonField(object, json_integer, "object", NULL);
			if( !field || field->u.integer < 0 || field->u.integer >= ARRAY_SIZE(ModConfig.reflect.objects) ) continue;
			ParsePolyfilterConfiguration(GetJsonField(object, json_array, "meshes", NULL), "mesh", &ModConfig.reflect.objects[field->u.integer]);
		}
	}
	ParsePolyfilterConfiguration(GetJsonField(root, json_array, "statics", NULL), "static", &ModConfig.reflect.statics);
	ModConfig.reflect.isLoaded = true;
	return true;
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
	field = GetJsonField(root, json_string, "watercolor", NULL);
	if( field && field->u.string.length == 6 ) {
		ModConfig.waterColor = strtol(field->u.string.ptr, NULL, 16);
	}
	field = GetJsonField(root, json_boolean, "barefoot", NULL);
	if( field ) {
		ModConfig.isBarefoot = field->u.boolean;
	}
	ParseSemitransConfiguration(GetJsonField(root, json_object, "semitransparent", NULL));
	ParseReflectConfiguration(GetJsonField(root, json_object, "reflective", NULL));
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
	if( ModConfig.semitrans.animtex ) {
		free(ModConfig.semitrans.animtex);
		ModConfig.semitrans.animtex = NULL;
	}
	FreePolyfilterNodes(&ModConfig.semitrans.rooms);
	FreePolyfilterNodes(&ModConfig.semitrans.statics);
	FreePolyfilterNodes(&ModConfig.reflect.statics);
	for( DWORD i=0; i<ARRAY_SIZE(ModConfig.semitrans.objects); ++i ) {
		FreePolyfilterNodes(&ModConfig.semitrans.objects[i]);
		FreePolyfilterNodes(&ModConfig.reflect.objects[i]);
	}
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
