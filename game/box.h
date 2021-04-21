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

#ifndef BOX_H_INCLUDED
#define BOX_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
#define InitialiseCreature ((void(__cdecl*)(__int16)) 0x0040E190)
#define CreatureActive ((int(__cdecl*)(__int16)) 0x0040E1C0)
#define CreatureAIInfo ((void(__cdecl*)(ITEM_INFO *, AI_INFO *)) 0x0040E210)

//	0x0040E470:		SearchLOT
//	0x0040E670:		UpdateLOT
//	0x0040E6E0:		TargetBox
//	0x0040E780:		StalkBox
//	0x0040E880:		EscapeBox
//	0x0040E930:		ValidBox

#define CreatureMood ((void(__cdecl*)(ITEM_INFO *, AI_INFO *, BOOL)) 0x0040E9E0)

//	0x0040EE50:		CalculateTarget
//	0x0040F2B0:		CreatureCreature
//	0x0040F3B0:		BadFloor

void __cdecl CreatureDie(__int16 itemID, BOOL explode); // 0x0040F440

#define CreatureAnimation ((int(__cdecl*)(__int16, __int16, __int16)) 0x0040F500)
#define CreatureTurn ((__int16(__cdecl*)(ITEM_INFO *, __int16)) 0x0040FDD0)
#define CreatureTilt ((void(__cdecl*)(ITEM_INFO *, __int16)) 0x0040FEB0)
#define CreatureHead ((void(__cdecl*)(ITEM_INFO *, __int16)) 0x0040FEF0)

//	0x0040FF40:		CreatureNeck
//	0x0040FF90:		CreatureFloat
//	0x00410040:		CreatureUnderwater

#define CreatureEffect ((__int16(__cdecl*)(ITEM_INFO *, const BITE_INFO *, __int16(__cdecl*)(int, int, int, __int16, __int16, __int16))) 0x00410090)

//	0x004100F0:		CreatureVault

void __cdecl CreatureKill(ITEM_INFO *item, int killAnim, int killState, int laraKillState); // 0x00410230

//	0x004103A0:		GetBaddieTarget

#endif // BOX_H_INCLUDED
