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

#ifndef TRAPS_H_INCLUDED
#define TRAPS_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */

void __cdecl MineControl(__int16 mineID); // 0x00440FC0
void __cdecl ControlSpikeWall(__int16 itemID); // 0x004411C0
void __cdecl ControlCeilingSpikes(__int16 itemID); // 0x00441300
void __cdecl HookControl(__int16 itemID); // 0x00441420

//	0x004414B0:		PropellerControl

void __cdecl SpinningBlade(__int16 itemID); // 0x00441640
void __cdecl IcicleControl(__int16 itemID); // 0x004417C0
void __cdecl InitialiseBlade(__int16 itemID); // 0x004418C0
void __cdecl BladeControl(__int16 itemID); // 0x00441900
void __cdecl InitialiseKillerStatue(__int16 itemID); // 0x004419A0
void __cdecl KillerStatueControl(__int16 itemID); // 0x004419F0
void __cdecl SpringBoardControl(__int16 itemID); // 0x00441B00
void __cdecl InitialiseRollingBall(__int16 itemID); // 0x00441BE0
void __cdecl RollingBallControl(__int16 itemID); // 0x00441C20
void __cdecl RollingBallCollision(__int16 itemID, ITEM_INFO *laraItem, COLL_INFO *coll); // 0x00441F70

//	0x004421C0:		SpikeCollision
//	0x00442320:		TrapDoorControl
//	0x00442370:		TrapDoorFloor
//	0x004423B0:		TrapDoorCeiling
//	0x004423F0:		OnTrapDoor

void __cdecl Pendulum(__int16 itemID); // 0x004424A0

//	0x004425B0:		FallingBlock
//	0x004426C0:		FallingBlockFloor
//	0x00442700:		FallingBlockCeiling

void __cdecl TeethTrap(__int16 itemID); // 0x00442750
void __cdecl FallingCeiling(__int16 itemID); // 0x00442810
void __cdecl DartEmitterControl(__int16 itemID); // 0x004428F0
void __cdecl DartsControl(__int16 itemID); // 0x00442A30
void __cdecl DartEffectControl(__int16 fxID); // 0x00442B90
void __cdecl FlameEmitterControl(__int16 item_id); // 0x00442BE0
void __cdecl FlameControl(__int16 fx_id); // 0x00442C70
void __cdecl LaraBurn(); // 0x00442DE0
void __cdecl LavaBurn(ITEM_INFO *item); // 0x00442E30

//	0x00442F20:		LavaSpray
//	0x00442FF0:		ControlLavaBlob

#endif // TRAPS_H_INCLUDED
