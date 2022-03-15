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

#ifndef COLLIDE_H_INCLUDED
#define COLLIDE_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
#define GetCollisionInfo ((void(__cdecl*)(COLL_INFO*, int, int, int, __int16, int)) 0x004128D0)
int __cdecl FindGridShift(int src, int dest); // 0x00412F90

int __cdecl CollideStaticObjects(COLL_INFO *coll, int x, int y, int z, __int16 roomID, int hite); // 0x00412FC0
void __cdecl GetNearByRooms(int x, int y, int z, int r, int h, __int16 roomID); // 0x004133B0
void __cdecl GetNewRoom(int x, int y, int z, __int16 roomID); // 0x00413480

void __cdecl ShiftItem(ITEM_INFO* item, COLL_INFO* coll); // 0x004134E0

void __cdecl UpdateLaraRoom(ITEM_INFO* item, int height); // 0x00413520

__int16 __cdecl GetTiltType(FLOOR_INFO* floor, int x, int y, int z); // 0x00413580
void __cdecl LaraBaddieCollision(ITEM_INFO* laraitem, COLL_INFO* coll); // 0x00413620
void __cdecl EffectSpaz(ITEM_INFO* laraitem, COLL_INFO* coll); // 0x004137C0

void __cdecl CreatureCollision(__int16 itemID, ITEM_INFO* laraitem, COLL_INFO* coll); // 0x00413840
void __cdecl ObjectCollision(__int16 itemID, ITEM_INFO* laraitem, COLL_INFO* coll); // 0x004138C0

void __cdecl DoorCollision(__int16 itemID, ITEM_INFO* laraitem, COLL_INFO* coll); // 0x00413920
void __cdecl TrapCollision(__int16 itemID, ITEM_INFO* laraitem, COLL_INFO* coll); // 0x004139A0
void __cdecl ItemPushLara(ITEM_INFO* item, ITEM_INFO* laraitem, COLL_INFO* coll, BOOL spazon, BOOL bigpush); // 0x00413A10
BOOL __cdecl TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* laraitem, int radius); // 0x00413D20
BOOL __cdecl TestLaraPosition(__int16* bounds, ITEM_INFO* item, ITEM_INFO* laraitem); // 0x00413DF0
#define AlignLaraPosition ((void(__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x00413F30)
#define MoveLaraPosition ((BOOL(__cdecl*)(PHD_VECTOR*, ITEM_INFO*, ITEM_INFO*)) 0x00414070)
#define Move3DPosTo3DPos ((BOOL(__cdecl*)(PHD_3DPOS*, PHD_3DPOS*, int, __int16)) 0x00414200)

#endif // COLLIDE_H_INCLUDED
