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
#include "modding/json_utils.h"
#include "global/vars.h"

json_value *GetJsonField(json_value *root, json_type fieldType, const char *name, DWORD *pIndex) {
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

json_value *GetJsonObjectByStringField(json_value *root, const char *name, const char *str, bool caseSensitive, DWORD *pIndex) {
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

int GetJsonIntegerFieldValue(json_value *root, const char *name) {
	json_value *field = GetJsonField(root, json_integer, name, NULL);
	return field ? field->u.integer : 0;
}
