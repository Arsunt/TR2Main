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
#include "game/camera.h"
#include "3dsystem/3d_gen.h"
#include "3dsystem/phd_math.h"
#include "game/cinema.h"
#include "game/control.h"
#include "game/draw.h"
#include "game/sound.h"
#include "specific/game.h"
#include "specific/sndpc.h"
#include "global/vars.h"

void __cdecl MoveCamera(GAME_VECTOR *destination, int speed) {
	Camera.pos.x += (destination->x - Camera.pos.x) / speed;
	Camera.pos.z += (destination->z - Camera.pos.z) / speed;
	Camera.pos.y += (destination->y - Camera.pos.y) / speed;
	Camera.pos.roomNumber = destination->roomNumber;

	IsChunkyCamera = FALSE;
	FLOOR_INFO *floor = GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);

	int height = GetHeight(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z) - 0x100;
	if( Camera.pos.y >= height && destination->y >= height ) {
		LOS(&Camera.target, &Camera.pos);
		floor = GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
		height = GetHeight(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z) - 0x100;
	}

	int ceiling = GetCeiling(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z) + 0x100;
	if( height < ceiling ) {
		height = ceiling = (height + ceiling) / 2;
	}

	if( Camera.bounce ) {
		if( Camera.bounce > 0 ) {
			Camera.pos.y += Camera.bounce;
			Camera.target.y += Camera.bounce;
			Camera.bounce = 0;
		} else {
			int shake = Camera.bounce * (GetRandomControl() - 0x4000) / 0x7FFF;
			Camera.pos.x += shake;
			Camera.target.y += shake;
			shake = Camera.bounce * (GetRandomControl() - 0x4000) / 0x7FFF;
			Camera.pos.y += shake;
			Camera.target.y += shake;
			shake = Camera.bounce * (GetRandomControl() - 0x4000);
			Camera.pos.z += shake / 0x7FFF;
			Camera.target.z += shake / 0x7FFF;
			Camera.bounce += 5;
		}
	}

	if( Camera.pos.y <= height ) {
		if ( Camera.pos.y >= ceiling ) {
			Camera.shift = 0;
		} else {
			Camera.shift = ceiling - Camera.pos.y;
		}
	} else {
		Camera.shift = height - Camera.pos.y;
	}

	GetFloor(Camera.pos.x, Camera.shift + Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
	phd_LookAt(Camera.pos.x, Camera.pos.y + Camera.shift, Camera.pos.z,
				Camera.target.x, Camera.target.y, Camera.target.z, 0);

	if( Camera.isLaraMic ) {
		Camera.actualAngle = Lara.torso_y_rot + Lara.head_y_rot + LaraItem->pos.rotY;
		Camera.micPos.x = LaraItem->pos.x;
		Camera.micPos.y = LaraItem->pos.y;
		Camera.micPos.z = LaraItem->pos.z;
	} else {
		Camera.actualAngle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.micPos.x = Camera.pos.x + ((phd_sin(Camera.actualAngle) * PhdPersp) >> W2V_SHIFT);
		Camera.micPos.z = Camera.pos.z + ((phd_cos(Camera.actualAngle) * PhdPersp) >> W2V_SHIFT);
		Camera.micPos.y = Camera.pos.y;
	}
}

void __cdecl CalculateCamera() {
	ITEM_INFO *item;
	__int16 *bounds;
	FLOOR_INFO *floor;
	int y, shift, fixedCamera;
	__int16 angle, tilt, rot;

	if( CHK_ANY(RoomInfo[Camera.pos.roomNumber].flags, ROOM_UNDERWATER) ) {
		PlaySoundEffect(60, 0, SFX_ALWAYS);
		if( !Camera.underwater ) {
#ifdef FEATURE_AUDIO_IMPROVED
			extern double UnderwaterMusicMute;
			double volume = (1.0 - UnderwaterMusicMute) * (double)(MusicVolume * 25 + 5);
			if( MusicVolume > 0 && volume >= 1.0 ) {
				S_CDVolume((DWORD)volume);
			} else {
				S_CDVolume(0);
			}
#else // FEATURE_AUDIO_IMPROVED
			S_CDVolume(0); // NOTE: Core supposed to pause CD Audio this way
#endif // FEATURE_AUDIO_IMPROVED
			Camera.underwater = 1;
		}
	} else if ( Camera.underwater ) {
		if( MusicVolume ) {
			S_CDVolume(MusicVolume * 25 + 5);
		}
		Camera.underwater = 0;
	}
	if( Camera.type == CAM_Cinematic ) {
		InGameCinematicCamera();
		return;
	}

	if( Camera.flags != CFL_NoChunky ) {
		IsChunkyCamera = 1;
	}

	fixedCamera = (Camera.item && (Camera.type == CAM_Fixed || Camera.type == CAM_Heavy)) ? 1 : 0;
	item = fixedCamera ? Camera.item : LaraItem;
	bounds = GetBoundsAccurate(item);
	y = item->pos.y;
	if( fixedCamera ) {
		y += (bounds[2] + bounds[3]) / 2;
	} else {
		y += bounds[3] + (bounds[2] - bounds[3]) * 3 / 4;
	}

	if( Camera.item && !fixedCamera ) {
		shift = phd_sqrt(SQR(Camera.item->pos.x - item->pos.x) + SQR(Camera.item->pos.z - item->pos.z));
		angle = phd_atan(Camera.item->pos.z - item->pos.z, Camera.item->pos.x - item->pos.x) - item->pos.rotY;
		bounds = GetBoundsAccurate(Camera.item);
		tilt = phd_atan(shift, y - Camera.item->pos.y - (bounds[2] + bounds[3]) / 2);
		angle /= 2;
		tilt  /= 2;
		if( angle > -50*PHD_DEGREE && angle < 50*PHD_DEGREE && tilt > -85*PHD_DEGREE && tilt < 85*PHD_DEGREE ) {
			rot = angle - Lara.head_y_rot;
			CLAMP(rot, -4*PHD_DEGREE, 4*PHD_DEGREE);
			Lara.head_y_rot += rot;
			Lara.torso_y_rot += rot;
			rot = tilt - Lara.head_x_rot;
			CLAMP(rot, -4*PHD_DEGREE, 4*PHD_DEGREE);
			Lara.head_x_rot += rot;
			Lara.torso_x_rot += rot;
			Camera.type = CAM_Look;
			Camera.item->looked_at = 1;
		}
	}

	if( Camera.type == CAM_Look || Camera.type == CAM_Combat ) {
		y -= 0x100;
		Camera.target.roomNumber = item->roomNumber;
		if( Camera.fixedCamera ) {
			Camera.target.y = y;
			Camera.speed = 1;
		} else {
			Camera.target.y += (y - Camera.target.y) / 4;
			Camera.speed = (Camera.type != CAM_Look) ? 8 : 4;
		}
		Camera.fixedCamera = 0;
		if ( Camera.type == CAM_Look ) {
			LookCamera(item);
		} else {
			CombatCamera(item);
		}
	} else {
		Camera.target.x = item->pos.x;
		Camera.target.z = item->pos.z;
		if( Camera.flags == CFL_FollowCenter ) {
			shift = (bounds[4] + bounds[5]) / 2;
			Camera.target.z += shift * phd_cos(item->pos.rotY) >> W2V_SHIFT;
			Camera.target.x += shift * phd_sin(item->pos.rotY) >> W2V_SHIFT;
		}
		Camera.target.roomNumber = item->roomNumber;
		if ( Camera.fixedCamera == fixedCamera ) {
			Camera.fixedCamera = 0;
			Camera.target.y += (y - Camera.target.y) / 4;
		} else {
			Camera.target.y = y;
			Camera.fixedCamera = 1;
			Camera.speed = 1;
		}
		floor = GetFloor(Camera.target.x, Camera.target.y, Camera.target.z, &Camera.target.roomNumber);
		if( Camera.target.y > GetHeight(floor, Camera.target.x, Camera.target.y, Camera.target.z) ) {
			IsChunkyCamera = 0;
		}
		if( Camera.type == CAM_Chase || Camera.flags == CFL_ChaseObject ) {
			ChaseCamera(item);
		} else {
			FixedCamera();
		}
	}

	Camera.last = Camera.number;
	Camera.fixedCamera = fixedCamera;
	if( Camera.type != CAM_Heavy || Camera.timer == -1 ) {
		Camera.type = CAM_Chase;
		Camera.speed = 10;
		Camera.number = -1;
		Camera.last_item = Camera.item;
		Camera.item = NULL;
		Camera.targetElevation = 0;
		Camera.targetAngle = 0;
		Camera.targetDistance = 0x600;
		Camera.flags = CFL_None;
	}
	IsChunkyCamera = 0;
}

/*
 * Inject function
 */
void Inject_Camera() {
//	INJECT(0x00410580, InitialiseCamera);

	INJECT(0x00410630, MoveCamera);

//	INJECT(0x004109B0, ClipCamera);
//	INJECT(0x00410A90, ShiftCamera);
//	INJECT(0x00410BF0, BadPosition);
//	INJECT(0x00410C40, SmartShift);
//	INJECT(0x004113D0, ChaseCamera);
//	INJECT(0x004114C0, ShiftClamp);
//	INJECT(0x00411660, CombatCamera);
//	INJECT(0x004117F0, LookCamera);
//	INJECT(0x004119E0, FixedCamera);

	INJECT(0x00411A80, CalculateCamera);
}
