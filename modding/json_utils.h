/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
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

#ifndef JSON_UTILS_H_INCLUDED
#define JSON_UTILS_H_INCLUDED

#include "global/types.h"
#include "json-parser/json.h"

json_value *GetJsonField(json_value *root, json_type fieldType, const char *name, DWORD *pIndex);
json_value *GetJsonObjectByStringField(json_value *root, const char *name, const char *str, bool caseSensitive, DWORD *pIndex);
int GetJsonIntegerFieldValue(json_value *root, const char *name, int defaultValue);
double GetJsonFloatFieldValue(json_value *root, const char *name, double defaultValue);

#endif // JSON_UTILS_H_INCLUDED
