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

#ifndef EFFECTS_H_INCLUDED
#define EFFECTS_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
//	0x0041C4B0:		ItemNearLara
//	0x0041C540:		SoundEffects

#define DoBloodSplat ((__int16(__cdecl*)(int, int, int, __int16, __int16, __int16)) 0x0041C5B0)

//	0x0041C610:		DoLotsOfBlood
//	0x0041C6C0:		ControlBlood1
//	0x0041C750:		ControlExplosion1
//	0x0041C7D0:		Richochet
//	0x0041C850:		ControlRichochet1
//	0x0041C880:		CreateBubble
//	0x0041C8F0:		LaraBubbles
//	0x0041C970:		ControlBubble1

#define Splash ((void(__cdecl*)(ITEM_INFO*)) 0x0041CA70)

void WadeSplash(ITEM_INFO *item, int height);

//	----------:		WadeSplash
//	0x0041CB40:		ControlSplash1
//	0x0041CBC0:		ControlWaterSprite
//	0x0041CC70:		ControlSnowSprite
//	0x0041CD00:		ControlHotLiquid
//	0x0041CDE0:		WaterFall
//	0x0041CF20:		finish_level_effect
//	0x0041CF30:		turn180_effect
//	0x0041CF50:		floor_shake_effect
//	0x0041CFF0:		lara_normal_effect
//	0x0041D030:		BoilerFX
//	0x0041D050:		FloodFX
//	0x0041D0E0:		RubbleFX
//	0x0041D110:		ChandelierFX
//	0x0041D140:		ExplosionFX
//	0x0041D170:		PistonFX
//	0x0041D190:		CurtainFX
//	0x0041D1B0:		StatueFX
//	0x0041D1D0:		SetChangeFX
//	0x0041D1F0:		ControlDingDong
//	0x0041D230:		ControlLaraAlarm
//	0x0041D270:		ControlAlarmSound
//	0x0041D2E0:		ControlBirdTweeter
//	0x0041D340:		DoChimeSound
//	0x0041D3A0:		ControlClockChimes
//	0x0041D410:		SphereOfDoomCollision
//	0x0041D540:		SphereOfDoom
//	0x0041D630:		DrawSphereOfDoom
//	0x0041D760:		lara_hands_free
//	0x0041D770:		flip_map_effect
//	0x0041D780:		draw_right_gun
//	0x0041D7D0:		draw_left_gun
//	----------:		shoot_right_gun
//	----------:		shoot_left_gun
//	0x0041D820:		swap_meshes_with_meshswap1
//	0x0041D890:		swap_meshes_with_meshswap2
//	0x0041D900:		swap_meshes_with_meshswap3
//	0x0041D9A0:		invisibility_on
//	0x0041D9B0:		invisibility_off
//	0x0041D9D0:		dynamic_light_on
//	0x0041D9E0:		dynamic_light_off
//	0x0041D9F0:		reset_hair
//	0x0041DA00:		AssaultStart
//	0x0041DA30:		AssaultStop
//	0x0041DA50:		AssaultReset
//	0x0041DA70:		AssaultFinished

#endif // EFFECTS_H_INCLUDED
