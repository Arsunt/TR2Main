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

#ifndef EFFECTS_H_INCLUDED
#define EFFECTS_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __cdecl ItemNearLara(PHD_3DPOS *pos, int distance); // 0x0041C4B0
void __cdecl SoundEffects(); // 0x0041C540
__int16 __cdecl DoBloodSplat(int x, int y, int z, __int16 speed, __int16 direction, __int16 roomID); // 0x0041C5B0
void __cdecl DoLotsOfBlood(int x, int y, int z, __int16 speed, __int16 direction, __int16 roomID, int number); // 0x0041C610
void __cdecl ControlBlood1(__int16 fxID); // 0x0041C6C0
void __cdecl ControlExplosion1(__int16 fxID); // 0x0041C750
void __cdecl Richochet(GAME_VECTOR *pos); // 0x0041C7D0
void __cdecl ControlRichochet1(__int16 fxID); // 0x0041C850
void __cdecl CreateBubble(PHD_3DPOS *pos, __int16 roomNumber); // 0x0041C880
void __cdecl LaraBubbles(ITEM_INFO *item); // 0x0041C8F0
void __cdecl ControlBubble1(__int16 fxID); // 0x0041C970
void __cdecl Splash(ITEM_INFO *item); // 0x0041CA70
void WadeSplash(ITEM_INFO *item, int height);
void __cdecl ControlSplash1(__int16 fxID); // 0x0041CB40
void __cdecl ControlWaterSprite(__int16 fxID); // 0x0041CBC0
void __cdecl ControlSnowSprite(__int16 fxID); // 0x0041CC70
void __cdecl ControlHotLiquid(__int16 fxID); // 0x0041CD00
void __cdecl WaterFall(__int16 itemID); // 0x0041CDE0
void __cdecl finish_level_effect(ITEM_INFO *item); // 0x0041CF20
void __cdecl turn180_effect(ITEM_INFO *item); // 0x0041CF30
void __cdecl floor_shake_effect(ITEM_INFO *item); // 0x0041CF50
void __cdecl lara_normal_effect(ITEM_INFO *item); // 0x0041CFF0
void __cdecl BoilerFX(ITEM_INFO *item); // 0x0041D030
void __cdecl FloodFX(ITEM_INFO *item); // 0x0041D050
void __cdecl RubbleFX(ITEM_INFO *item); // 0x0041D0E0
void __cdecl ChandelierFX(ITEM_INFO *item); // 0x0041D110
void __cdecl ExplosionFX(ITEM_INFO *item); // 0x0041D140
void __cdecl PistonFX(ITEM_INFO *item); // 0x0041D170
void __cdecl CurtainFX(ITEM_INFO *item); // 0x0041D190
void __cdecl StatueFX(ITEM_INFO *item); // 0x0041D1B0
void __cdecl SetChangeFX(ITEM_INFO *item); // 0x0041D1D0
void __cdecl ControlDingDong(__int16 itemID); // 0x0041D1F0
void __cdecl ControlLaraAlarm(__int16 itemID); // 0x0041D230
void __cdecl ControlAlarmSound(__int16 itemID); // 0x0041D270
void __cdecl ControlBirdTweeter(__int16 itemID); // 0x0041D2E0
void __cdecl DoChimeSound(ITEM_INFO *item); // 0x0041D340
void __cdecl ControlClockChimes(__int16 itemID); // 0x0041D3A0
void __cdecl SphereOfDoomCollision(__int16 itemID, ITEM_INFO *laraItem, COLL_INFO *coll); // 0x0041D410
void __cdecl SphereOfDoom(__int16 itemID); // 0x0041D540
void __cdecl DrawSphereOfDoom(ITEM_INFO *item); // 0x0041D630
void __cdecl lara_hands_free(ITEM_INFO *item); // 0x0041D760
void __cdecl flip_map_effect(ITEM_INFO *item); // 0x0041D770
void __cdecl draw_right_gun(ITEM_INFO *item); // 0x0041D780
void __cdecl draw_left_gun(ITEM_INFO *item); // 0x0041D7D0
//	----------:		shoot_right_gun
//	----------:		shoot_left_gun
void __cdecl swap_meshes_with_meshswap1(ITEM_INFO *item); // 0x0041D820
void __cdecl swap_meshes_with_meshswap2(ITEM_INFO *item); // 0x0041D890
void __cdecl swap_meshes_with_meshswap3(ITEM_INFO *item); // 0x0041D900
void __cdecl invisibility_on(ITEM_INFO *item); // 0x0041D9A0
void __cdecl invisibility_off(ITEM_INFO *item); // 0x0041D9B0
void __cdecl dynamic_light_on(ITEM_INFO *item); // 0x0041D9D0
void __cdecl dynamic_light_off(ITEM_INFO *item); // 0x0041D9E0
void __cdecl reset_hair(ITEM_INFO *item); // 0x0041D9F0
void __cdecl AssaultStart(ITEM_INFO *item); // 0x0041DA00
void __cdecl AssaultStop(ITEM_INFO *item); // 0x0041DA30
void __cdecl AssaultReset(ITEM_INFO *item); // 0x0041DA50
void __cdecl AssaultFinished(ITEM_INFO *item); // 0x0041DA70

#endif // EFFECTS_H_INCLUDED
