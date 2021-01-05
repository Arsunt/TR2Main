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

#ifndef LARA_H_INCLUDED
#define LARA_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
#define LaraAboveWater ((void(__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00427560)

// 0x00427700:		LookUpDown
// 0x00427770:		LookLeftRight
// 0x004277F0:		ResetLook
// 0x00427880:		lara_as_walk
// 0x00427910:		lara_as_run
// 0x00427A60:		lara_as_stop
// 0x00427BB0:		lara_as_forwardjump
// ----------:		lara_as_pose
// 0x00427C90:		lara_as_fastback
// 0x00427CF0:		lara_as_turn_r
// 0x00427D80:		lara_as_turn_l
// 0x00427E10:		lara_as_death
// 0x00427E30:		lara_as_fastfall
// 0x00427E70:		lara_as_hang
// 0x00427ED0:		lara_as_reach
// 0x00427EF0:		lara_as_splat
// ----------:		lara_as_land
// 0x00427F00:		lara_as_compress
// 0x00428010:		lara_as_back
// 0x004280A0:		lara_as_null
// 0x004280B0:		lara_as_fastturn
// 0x00428100:		lara_as_stepright
// 0x00428180:		lara_as_stepleft
// 0x00428200:		lara_as_slide
// 0x00428230:		lara_as_backjump
// 0x00428280:		lara_as_rightjump
// 0x004282C0:		lara_as_leftjump
// 0x00428300:		lara_as_upjump
// 0x00428320:		lara_as_fallback
// 0x00428350:		lara_as_hangleft
// 0x00428390:		lara_as_hangright
// 0x004283D0:		lara_as_slideback
// 0x004283F0:		lara_as_pushblock
// ----------:		lara_as_pullblock
// 0x00428420:		lara_as_ppready
// 0x00428450:		lara_as_pickup
// 0x00428480:		lara_as_pickupflare
// 0x004284E0:		lara_as_switchon
// ----------:		lara_as_switchoff
// 0x00428520:		lara_as_usekey
// ----------:		lara_as_usepuzzle
// ----------:		lara_as_roll
// ----------:		lara_as_roll2
// 0x00428550:		lara_as_special
// ----------:		lara_as_usemidas
// ----------:		lara_as_diemidas
// 0x00428570:		lara_as_swandive
// 0x004285A0:		lara_as_fastdive
// ----------:		lara_as_gymnast
// 0x00428600:		lara_as_waterout
// ----------:		lara_as_laratest1
// ----------:		lara_as_laratest2
// ----------:		lara_as_laratest3
// 0x00428620:		lara_as_wade
// ----------:		lara_as_twist
// ----------:		lara_as_kick
// 0x004286F0:		lara_as_deathslide
void __cdecl extra_as_breath (ITEM_INFO *item, COLL_INFO *coll); // 0x00428790
// ----------:		extra_as_plunger
void __cdecl extra_as_yetikill (ITEM_INFO *item, COLL_INFO *coll); // 0x004287E0
void __cdecl extra_as_sharkkill (ITEM_INFO *item, COLL_INFO *coll); // 0x00428830
void __cdecl extra_as_airlock (ITEM_INFO *item, COLL_INFO *coll); // 0x004288D0
void __cdecl extra_as_gongbong (ITEM_INFO *item, COLL_INFO *coll); // 0x004288F0
void __cdecl extra_as_dinokill (ITEM_INFO *item, COLL_INFO *coll); //0x00428910
void __cdecl extra_as_pulldagger (ITEM_INFO *item, COLL_INFO *coll); //0x00428970
void __cdecl extra_as_startanim (ITEM_INFO *item, COLL_INFO *coll); //0x00428A30
void __cdecl extra_as_starthouse(ITEM_INFO *item, COLL_INFO *coll); //0x00428A80
void __cdecl extra_as_finalanim(ITEM_INFO *item, COLL_INFO *coll); //0x00428B30
// 0x00428BE0:		LaraFallen
// 0x00428C40:		LaraCollideStop
// 0x00428D00:		lara_col_walk
// 0x00428EA0:		lara_col_run
// 0x00429020:		lara_col_stop
// 0x004290B0:		lara_col_forwardjump
// ----------:		lara_col_pose
// 0x00429190:		lara_col_fastback
// 0x00429250:		lara_col_turn_r
// 0x004292F0:		lara_col_turn_l
// 0x00429310:		lara_col_death
// 0x00429380:		lara_col_fastfall
// 0x00429420:		lara_col_hang
// 0x00429550:		lara_col_reach
// 0x004295E0:		lara_col_splat
// ----------:		lara_col_land
// 0x00429640:		lara_col_compress
// 0x004296E0:		lara_col_back
// ----------:		lara_col_null
// 0x004297E0:		lara_col_fastturn
// 0x00429800:		lara_col_stepright
// 0x004298C0:		lara_col_stepleft
// 0x004298E0:		lara_col_slide
// 0x00429900:		lara_col_backjump
// 0x00429930:		lara_col_rightjump
// 0x00429960:		lara_col_leftjump
// 0x00429990:		lara_col_upjump
// 0x00429AD0:		lara_col_fallback
// 0x00429B60:		lara_col_hangleft
// 0x00429BA0:		lara_col_hangright
// 0x00429BE0:		lara_col_slideback
// ----------:		lara_col_pushblock
// ----------:		lara_col_pullblock
// ----------:		lara_col_ppready
// ----------:		lara_col_pickup
// ----------:		lara_col_switchon
// ----------:		lara_col_switchoff
// ----------:		lara_col_usekey
// ----------:		lara_col_usepuzzle
// 0x00429C10:		lara_col_roll
// 0x00429CB0:		lara_col_roll2
// 0x00429D80:		lara_col_special
// ----------:		lara_col_usemidas
// ----------:		lara_col_diemidas
// 0x00429DA0:		lara_col_swandive
// 0x00429E10:		lara_col_fastdive
// ----------:		lara_col_gymnast
// ----------:		lara_col_waterout
// ----------:		lara_col_laratest1
// ----------:		lara_col_laratest2
// ----------:		lara_col_laratest3
// 0x00429E90:		lara_col_wade
// ----------:		lara_col_twist
// 0x0042A000:		lara_default_col

void __cdecl lara_col_jumper(ITEM_INFO *item, COLL_INFO *coll); // 0x0042A040

// 0x0042A120:		lara_col_kick
// ----------:		lara_col_deathslide

#define GetLaraCollisionInfo ((void (__cdecl*)(ITEM_INFO *item, COLL_INFO *coll)) 0x0042A130)

// 0x0042A170:		lara_slide_slope
// 0x0042A260:		LaraHitCeiling
// 0x0042A2D0:		LaraDeflectEdge

#define LaraDeflectEdgeJump ((void (__cdecl*)(ITEM_INFO *item, COLL_INFO *coll)) 0x0042A350)

// 0x0042A4D0:		LaraSlideEdgeJump
// 0x0042A5C0:		TestWall
// 0x0042A6D0:		LaraTestHangOnClimbWall
// 0x0042A7E0:		LaraTestClimbStance
// 0x0042A8A0:		LaraHangTest
// 0x0042AC00:		LaraTestEdgeCatch
// 0x0042ACB0:		LaraTestHangJumpUp
// 0x0042AE20:		LaraTestHangJump
// 0x0042AFC0:		TestHangSwingIn
// 0x0042B080:		TestLaraVault
// 0x0042B370:		TestLaraSlide
// 0x0042B4A0:		LaraFloorFront

#define LaraLandedBad ((bool (__cdecl*)(ITEM_INFO *item, COLL_INFO *coll)) 0x0042B520)
#define GetLaraJointAbsPosition ((void(__cdecl*)(PHD_VECTOR*,int)) 0x0042B5E0)

// 0x0042B970:		GetLJAInt

#endif // LARA_H_INCLUDED
