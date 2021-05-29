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
#include "game/lara.h"
#include "global/vars.h"
#include "specific/sndpc.h"
#include "game/control.h"
#include "game/invfunc.h"
#include "3dsystem/3d_gen.h"

#ifdef FEATURE_GAMEPLAY_FIXES
bool IsLowCeilingJumpFix = true;
#endif // FEATURE_GAMEPLAY_FIXES

void __cdecl extra_as_breath (ITEM_INFO *item, COLL_INFO *coll)
{
  item->animNumber = 0x67;//103
  item->frameNumber = Anims[item->animNumber].frameBase;
  item->currentAnimState = 2;
  item->goalAnimState = 2;
  Lara.gun_status = LGS_Armless;
  Camera.type = CAM_Chase;
  AlterFOV(0x38e0);//14560 = 80 * 182
  Lara.extra_anim = 0;
  return;
}

void __cdecl extra_as_yetikill (ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->frameNumber < Anims[item->animNumber].frameEnd - 30)
        Lara.death_count = 1;//delay death

    Lara.hit_direction = -1;
    Camera.targetDistance = 0xc00;//3072 = 1024 * 3
    Camera.targetAngle = 0x71c0;//29120 = 160 * 182
    return;
}

void __cdecl extra_as_sharkkill (ITEM_INFO *item, COLL_INFO *coll)
{
    int water;

    Camera.targetAngle = 0x71c0;//29120 = 160 * 182
    Camera.targetDistance = 0xc00;//3027 = 1024 * 3
    Lara.hit_direction = -1;
    if (item->frameNumber == Anims[item->animNumber].frameEnd)
    {
        water = GetWaterHeight(item->pos.x, item->pos.y, item->pos.z, item->roomNumber);
        if ((water != -32750) && (water < item->pos.y - 100))
            item->pos.y = item->pos.y -5;//move lara up to the surface
    }

    if (item->frameNumber < Anims[item->animNumber].frameEnd - 30)
        Lara.death_count = 1;//delay death
  return;
}

void __cdecl extra_as_airlock (ITEM_INFO *item, COLL_INFO *coll)
{
  Camera.targetAngle = 0x38e0;//14560 = 80 * 182
  Camera.targetElevation = 0xee3a;//-4550 = -25 * 182
  return;
}

void __cdecl extra_as_gongbong (ITEM_INFO *item, COLL_INFO *coll)
{
  Camera.targetDistance = 0xc00;//3072 = 1024*3
  Camera.targetAngle = 0xee3a;//-4550 = -25 * 182
  Camera.targetElevation = 0xf1c8;//-3640 = -20 * 182
  return;
}

void __cdecl extra_as_dinokill (ITEM_INFO *item, COLL_INFO *coll)
{
    if (item ->frameNumber < Anims[item->animNumber].frameEnd - 30)
    Lara.death_count = 1;//delay death

    Lara.hit_direction = -1;
    Camera.flags = CFL_FollowCenter;
    Camera.targetAngle = 0x78dc;//30940 = 170 * 182
    Camera.targetElevation = 0xee3a;//-4550 = -25 * 182
    return;
}


void __cdecl extra_as_pulldagger (ITEM_INFO *item, COLL_INFO *coll)
{
    int currentCutFrame;
    int animStart;
    int animEnd;
    animStart = Anims[item->animNumber].frameBase;
    animEnd = Anims[item->animNumber].frameEnd;
    currentCutFrame = item->frameNumber - animStart;

    FLOOR_INFO *floor;
    short roomNum;

    if (currentCutFrame == 1)
        StartSyncedAudio(28);
    else
    if (currentCutFrame == 180)
    {
    Lara.mesh_ptrs[10] = MeshPtr[Objects[ID_LARA].meshIndex + 10];//lara's right hand, the dagger?
    Inv_AddItem(ID_PUZZLE_ITEM2);//0xAF but threw a build error
    }

    if (item->frameNumber == animEnd)
    {
    roomNum = item->roomNumber;
    item->pos.rotY += 0x4000;//= item->pos.rotY + 0x4000;
    floor = GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomNum);//for the GetHeight call
    GetHeight(floor, item->pos.x, item->pos.y, item->pos.z);//to set TriggerPtr
    TestTriggers(TriggerPtr, 1);//make lara trigger a heavy trigger??
    }
  return;
}

void __cdecl extra_as_startanim (ITEM_INFO *item, COLL_INFO *coll)
{
    short roomNum;
    FLOOR_INFO *floor;

    roomNum = item->roomNumber;
    floor = GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomNum);//for the GetHeight call
    GetHeight(floor, item->pos.x, item->pos.y, item->pos.z);//to set TriggerPtr
    TestTriggers(TriggerPtr, 0);//do trigger
    return;
}

void __cdecl extra_as_starthouse(ITEM_INFO *item, COLL_INFO *coll)
{
    int currentCutFrame;
    int animStart;
    int animEnd;
    animStart = Anims[item->animNumber].frameBase;
    animEnd = Anims[item->animNumber].frameEnd;
    currentCutFrame = item->frameNumber - animStart;

    if (currentCutFrame == 1)
    {
    StartSyncedAudio(63);
    Lara.mesh_ptrs[10] = MeshPtr[Objects[ID_LARA_EXTRA].meshIndex + 10];//right hand
    Lara.mesh_ptrs[0] = MeshPtr[Objects[ID_LARA_EXTRA].meshIndex];//hip
    return;
    }

  if (currentCutFrame == 401)
    {
    Lara.mesh_ptrs[10] = MeshPtr[Objects[ID_LARA].meshIndex + 10];//right hand
    Lara.mesh_ptrs[0] = MeshPtr[Objects[ID_LARA].meshIndex];//hip
    Inv_AddItem(ID_PUZZLE_ITEM1); //0xAE but it was throwing a build error!
    return;
    }

  if (item->frameNumber == animEnd)
    {
    Camera.type = CAM_Chase;
    AlterFOV(0x38E0);//14560 = 80 * 182
    }
  return;
}

void __cdecl extra_as_finalanim(ITEM_INFO *item, COLL_INFO *coll)
{
    int currentCutFrame;
    int animStart;
    int animEnd;

    item->hitPoints = 1000;

    animStart = Anims[item->animNumber].frameBase;
    animEnd = Anims[item->animNumber].frameEnd;
    currentCutFrame = item->frameNumber - animStart;

    if (currentCutFrame == 1)
        {
        Lara.back_gun = 0; //remove the shotgun off her back
        Lara.mesh_ptrs[10] = MeshPtr[Objects[ID_LARA].meshIndex + 10];//make sure the right hand doesn't have the shotgun
        Lara.mesh_ptrs[14] = MeshPtr[Objects[ID_LARA].meshIndex + 14];//make sure her head is good, to prevent the screaming head from appearing if the cutscene started with the player shooting
        Lara.mesh_ptrs[0] = MeshPtr[Objects[ID_LARA_EXTRA].meshIndex];//make sure the dagger is on her
        StartSyncedAudio(0x1b);
        return;
        }

    if (currentCutFrame == 316)
        {
        Lara.mesh_ptrs[10] = MeshPtr[Objects[ID_LARA_SHOTGUN].meshIndex + 10]; //equip shotgun
        return;
        }

    if (item->frameNumber == animEnd -1) //1 frame before the animation ends
        IsLevelComplete = 1; //end the level
    return;
}

void __cdecl lara_col_jumper(ITEM_INFO *item, COLL_INFO *coll) {
	coll->badPos = 0x7F00;
	coll->badNeg = -0x0180;
	coll->badCeiling = 0x00C0;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if( item->fallSpeed > 0 && coll->sideMid.floor <= 0 ) {
		item->goalAnimState = LaraLandedBad(item, coll) ? AS_DEATH : AS_STOP;
		item->fallSpeed = 0;
		item->gravity = 0;
		item->pos.y += coll->sideMid.floor;
	}

	// NOTE: Low ceiling check must be skipped because it produces the bug
	// Core Design removed this check in later game releases
#ifdef FEATURE_GAMEPLAY_FIXES
	if( IsLowCeilingJumpFix ) return;
#endif // FEATURE_GAMEPLAY_FIXES
	if( ABS(coll->sideMid.ceiling - coll->sideMid.floor) < 0x02FA ) {
		item->currentAnimState = AS_FASTFALL;
		item->goalAnimState = AS_FASTFALL;
		item->animNumber = 32;
		item->frameNumber = Anims[item->animNumber].frameBase + 1;
		item->speed /= 4;
		Lara.move_angle += PHD_180;
		if( item->fallSpeed <= 0 )
			item->fallSpeed = 1;
	}
}

/*
 * Inject function
 */
void Inject_Lara() {
//	INJECT(0x00427560, LaraAboveWater);
//	INJECT(0x00427700, LookUpDown);
//	INJECT(0x00427770, LookLeftRight);
//	INJECT(0x004277F0, ResetLook);
//	INJECT(0x00427880, lara_as_walk);
//	INJECT(0x00427910, lara_as_run);
//	INJECT(0x00427A60, lara_as_stop);
//	INJECT(0x00427BB0, lara_as_forwardjump);
//	INJECT(----------, lara_as_pose);
//	INJECT(0x00427C90, lara_as_fastback);
//	INJECT(0x00427CF0, lara_as_turn_r);
//	INJECT(0x00427D80, lara_as_turn_l);
//	INJECT(0x00427E10, lara_as_death);
//	INJECT(0x00427E30, lara_as_fastfall);
//	INJECT(0x00427E70, lara_as_hang);
//	INJECT(0x00427ED0, lara_as_reach);
//	INJECT(0x00427EF0, lara_as_splat);
//	INJECT(----------, lara_as_land);
//	INJECT(0x00427F00, lara_as_compress);
//	INJECT(0x00428010, lara_as_back);
//	INJECT(0x004280A0, lara_as_null);
//	INJECT(0x004280B0, lara_as_fastturn);
//	INJECT(0x00428100, lara_as_stepright);
//	INJECT(0x00428180, lara_as_stepleft);
//	INJECT(0x00428200, lara_as_slide);
//	INJECT(0x00428230, lara_as_backjump);
//	INJECT(0x00428280, lara_as_rightjump);
//	INJECT(0x004282C0, lara_as_leftjump);
//	INJECT(0x00428300, lara_as_upjump);
//	INJECT(0x00428320, lara_as_fallback);
//	INJECT(0x00428350, lara_as_hangleft);
//	INJECT(0x00428390, lara_as_hangright);
//	INJECT(0x004283D0, lara_as_slideback);
//	INJECT(0x004283F0, lara_as_pushblock);
//	INJECT(----------, lara_as_pullblock);
//	INJECT(0x00428420, lara_as_ppready);
//	INJECT(0x00428450, lara_as_pickup);
//	INJECT(0x00428480, lara_as_pickupflare);
//	INJECT(0x004284E0, lara_as_switchon);
//	INJECT(----------, lara_as_switchoff);
//	INJECT(0x00428520, lara_as_usekey);
//	INJECT(----------, lara_as_usepuzzle);
//	INJECT(----------, lara_as_roll);
//	INJECT(----------, lara_as_roll2);
//	INJECT(0x00428550, lara_as_special);
//	INJECT(----------, lara_as_usemidas);
//	INJECT(----------, lara_as_diemidas);
//	INJECT(0x00428570, lara_as_swandive);
//	INJECT(0x004285A0, lara_as_fastdive);
//	INJECT(----------, lara_as_gymnast);
//	INJECT(0x00428600, lara_as_waterout);
//	INJECT(----------, lara_as_laratest1);
//	INJECT(----------, lara_as_laratest2);
//	INJECT(----------, lara_as_laratest3);
//	INJECT(0x00428620, lara_as_wade);
//	INJECT(----------, lara_as_twist);
//	INJECT(----------, lara_as_kick);
//	INJECT(0x004286F0, lara_as_deathslide);
	INJECT(0x00428790, extra_as_breath);
//	INJECT(----------, extra_as_plunger);
	INJECT(0x004287E0, extra_as_yetikill);
	INJECT(0x00428830, extra_as_sharkkill);
	INJECT(0x004288D0, extra_as_airlock);
	INJECT(0x004288F0, extra_as_gongbong);
	INJECT(0x00428910, extra_as_dinokill);
	INJECT(0x00428970, extra_as_pulldagger);
	INJECT(0x00428A30, extra_as_startanim);
	INJECT(0x00428A80, extra_as_starthouse);
	INJECT(0x00428B30, extra_as_finalanim);
//	INJECT(0x00428BE0, LaraFallen);
//	INJECT(0x00428C40, LaraCollideStop);
//	INJECT(0x00428D00, lara_col_walk);
//	INJECT(0x00428EA0, lara_col_run);
//	INJECT(0x00429020, lara_col_stop);
//	INJECT(0x004290B0, lara_col_forwardjump);
//	INJECT(----------, lara_col_pose);
//	INJECT(0x00429190, lara_col_fastback);
//	INJECT(0x00429250, lara_col_turn_r);
//	INJECT(0x004292F0, lara_col_turn_l);
//	INJECT(0x00429310, lara_col_death);
//	INJECT(0x00429380, lara_col_fastfall);
//	INJECT(0x00429420, lara_col_hang);
//	INJECT(0x00429550, lara_col_reach);
//	INJECT(0x004295E0, lara_col_splat);
//	INJECT(----------, lara_col_land);
//	INJECT(0x00429640, lara_col_compress);
//	INJECT(0x004296E0, lara_col_back);
//	INJECT(----------, lara_col_null);
//	INJECT(0x004297E0, lara_col_fastturn);
//	INJECT(0x00429800, lara_col_stepright);
//	INJECT(0x004298C0, lara_col_stepleft);
//	INJECT(0x004298E0, lara_col_slide);
//	INJECT(0x00429900, lara_col_backjump);
//	INJECT(0x00429930, lara_col_rightjump);
//	INJECT(0x00429960, lara_col_leftjump);
//	INJECT(0x00429990, lara_col_upjump);
//	INJECT(0x00429AD0, lara_col_fallback);
//	INJECT(0x00429B60, lara_col_hangleft);
//	INJECT(0x00429BA0, lara_col_hangright);
//	INJECT(0x00429BE0, lara_col_slideback);
//	INJECT(----------, lara_col_pushblock);
//	INJECT(----------, lara_col_pullblock);
//	INJECT(----------, lara_col_ppready);
//	INJECT(----------, lara_col_pickup);
//	INJECT(----------, lara_col_switchon);
//	INJECT(----------, lara_col_switchoff);
//	INJECT(----------, lara_col_usekey);
//	INJECT(----------, lara_col_usepuzzle);
//	INJECT(0x00429C10, lara_col_roll);
//	INJECT(0x00429CB0, lara_col_roll2);
//	INJECT(0x00429D80, lara_col_special);
//	INJECT(----------, lara_col_usemidas);
//	INJECT(----------, lara_col_diemidas);
//	INJECT(0x00429DA0, lara_col_swandive);
//	INJECT(0x00429E10, lara_col_fastdive);
//	INJECT(----------, lara_col_gymnast);
//	INJECT(----------, lara_col_waterout);
//	INJECT(----------, lara_col_laratest1);
//	INJECT(----------, lara_col_laratest2);
//	INJECT(----------, lara_col_laratest3);
//	INJECT(0x00429E90, lara_col_wade);
//	INJECT(----------, lara_col_twist);
//	INJECT(0x0042A000, lara_default_col);

	INJECT(0x0042A040, lara_col_jumper);

//	INJECT(0x0042A120, lara_col_kick);
//	INJECT(----------, lara_col_deathslide);
//	INJECT(0x0042A130, GetLaraCollisionInfo);
//	INJECT(0x0042A170, lara_slide_slope);
//	INJECT(0x0042A260, LaraHitCeiling);
//	INJECT(0x0042A2D0, LaraDeflectEdge);
//	INJECT(0x0042A350, LaraDeflectEdgeJump);
//	INJECT(0x0042A4D0, LaraSlideEdgeJump);
//	INJECT(0x0042A5C0, TestWall);
//	INJECT(0x0042A6D0, LaraTestHangOnClimbWall);
//	INJECT(0x0042A7E0, LaraTestClimbStance);
//	INJECT(0x0042A8A0, LaraHangTest);
//	INJECT(0x0042AC00, LaraTestEdgeCatch);
//	INJECT(0x0042ACB0, LaraTestHangJumpUp);
//	INJECT(0x0042AE20, LaraTestHangJump);
//	INJECT(0x0042AFC0, TestHangSwingIn);
//	INJECT(0x0042B080, TestLaraVault);
//	INJECT(0x0042B370, TestLaraSlide);
//	INJECT(0x0042B4A0, LaraFloorFront);
//	INJECT(0x0042B520, LaraLandedBad);
//	INJECT(0x0042B5E0, GetLaraJointAbsPosition);
//	INJECT(0x0042B970, GetLJAInt);
}
