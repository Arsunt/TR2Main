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

#ifndef OBJECTS_H_INCLUDED
#define OBJECTS_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
//	0x004342C0:		EarthQuake
//	0x004343A0:		ControlCutShotgun
//	0x004343E0:		InitialiseFinalLevel
//	0x004344B0:		FinalLevelCounter
//	0x004346C0:		MiniCopterControl
//	0x004347A0:		InitialiseDyingMonk
//	0x00434820:		DyingMonk
//	0x004348B0:		ControlGongBonger
//	0x00434970:		DeathSlideCollision
//	0x00434A30:		ControlDeathSlide
//	0x00434CC0:		BigBowlControl
//	0x00434DB0:		BellControl
//	0x00434E30:		InitialiseWindow

#define SmashWindow ((void(__cdecl*)(__int16)) 0x00434EB0)

//	0x00434F80:		WindowControl
//	0x00435020:		SmashIceControl
void __cdecl ShutThatDoor(DOORPOS_DATA* door); // 0x00435100
void __cdecl OpenThatDoor(DOORPOS_DATA* door); // 0x00435150
#define InitialiseDoor ((void(__cdecl*)(__int16 itemID)) 0x00435190)
void __cdecl DoorControl(__int16 itemID); // 0x00435570
//	0x00435640:		OnDrawBridge
//	0x00435700:		DrawBridgeFloor
//	0x00435740:		DrawBridgeCeiling
//	0x00435780:		DrawBridgeCollision
//	0x004357B0:		InitialiseLift
//	0x004357F0:		LiftControl
//	0x004358D0:		LiftFloorCeiling
//	0x00435A50:		LiftFloor
//	0x00435A90:		LiftCeiling
//	0x00435AD0:		BridgeFlatFloor
//	0x00435AF0:		BridgeFlatCeiling
//	0x00435B10:		GetOffset
//	0x00435B50:		BridgeTilt1Floor
//	0x00435B80:		BridgeTilt1Ceiling
//	0x00435BC0:		BridgeTilt2Floor
//	0x00435BF0:		BridgeTilt2Ceiling
//	0x00435C30:		CopterControl
//	0x00435D40:		GeneralControl
//	0x00435E20:		DetonatorControl

#endif // OBJECTS_H_INCLUDED
