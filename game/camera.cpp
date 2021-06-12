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

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

#define CAM_DISTANCE (0x600)

void __cdecl InitialiseCamera() {
	Camera.shift = LaraItem->pos.y - 0x400;
	Camera.target.x = LaraItem->pos.x;
	Camera.target.y = Camera.shift;
	Camera.target.z = LaraItem->pos.z;
	Camera.target.roomNumber = LaraItem->roomNumber;

	Camera.pos.x = Camera.target.x;
	Camera.pos.y = Camera.target.y;
	Camera.pos.z = Camera.target.z - 100;
	Camera.pos.roomNumber = Camera.target.roomNumber;

	Camera.targetDistance = CAM_DISTANCE;
	Camera.item = NULL;
	Camera.numberFrames = 1;
	if ( !(Lara.extra_anim) ) {
		Camera.type = CAM_Chase;
	}
	Camera.speed = 1;
	Camera.flags = 0;
	Camera.bounce = 0;
	Camera.number = -1;
	Camera.fixedCamera = 0;
	AlterFOV(80 * PHD_DEGREE);
	CalculateCamera();
}

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
		int shake;
#ifdef FEATURE_INPUT_IMPROVED
		shake = 0xFFFF*(UINT64)(ABS(Camera.bounce)+50)/250;
		JoyRumble(shake, shake, 6, shake/6, 12, true);
#endif // FEATURE_INPUT_IMPROVED
		if( Camera.bounce > 0 ) {
			Camera.pos.y += Camera.bounce;
			Camera.target.y += Camera.bounce;
			Camera.bounce = 0;
		} else {
			shake = Camera.bounce * (GetRandomControl() - 0x4000) / 0x7FFF;
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

void __cdecl ClipCamera(int *x, int *y, int *z, int tx, int ty, int tz, int left, int top, int right, int bottom) {
	if ((right > left) != (tx < left)) {
		*y = ty + (*y - ty) * (left - tx) / (*x - tx);
		*z = tz + (*z - tz) * (left - tx) / (*x - tx);
		*x = left;
	}
	if ((bottom > top && ty > top && *y < top) || (bottom < top && ty < top && *y > top)) {
		*x = tx + (*x - tx) * (top - ty) / (*y - ty);
		*z = tz + (*z - tz) * (top - ty) / (*y - ty);
		*y = top;
	}
}

void __cdecl ShiftCamera(int *x, int *y, int *z, int tx, int ty, int tz, int left, int top, int right, int bottom) {
	int squareL, squareT, squareB, squareR;

	squareL = SQR(tx - left);
	squareT = SQR(ty - top);
	if (Camera.targetSquare < squareL + squareT) {
		*x = left;
		if (Camera.targetSquare >= squareL)
			*y = ty + (top >= bottom ? phd_sqrt(Camera.targetSquare - squareL) : -phd_sqrt(Camera.targetSquare - squareL));
	} else {
		if (squareL + squareT > SQR(341)) {
			*x = left;
			*y = top;
		} else {
			squareB = SQR(ty - bottom) + squareL;
			if (Camera.targetSquare < squareB) {
				*x = left;
				if (Camera.targetSquare >= squareL)
					*y = ty + (top < bottom ? phd_sqrt(Camera.targetSquare - squareL) : -phd_sqrt(Camera.targetSquare - squareL));
			} else {
				squareR = SQR(tx - right) + squareT;
				if (2 * Camera.targetSquare < squareR) {
					if (2 * Camera.targetSquare >= squareT) {
						*x = tx + (left < right ? phd_sqrt(2 * Camera.targetSquare - squareT) : -phd_sqrt(2 * Camera.targetSquare - squareT));
						*y = top;
					}
				} else {
					if (squareB > squareR) {
						*x = left;
						*y = bottom;
					} else {
						*x = right;
						*y = top;
					}
				}
			}
		}
	}
}

FLOOR_INFO* __cdecl GoodPosition(int x, int y, int z, __int16 roomID) {
	FLOOR_INFO *floor;

	floor = GetFloor(x, y, z, &roomID);
	if (y <= GetHeight(floor, x, y, z) && y >= GetCeiling(floor, x, y, z))
		return floor;
	return NULL;
}

void __cdecl SmartShift(GAME_VECTOR *goal, CB_SMARTCAM shift) {
	ROOM_INFO *room;
	BOX_INFO *box;
	int left, right, top, bottom, side, x, z;
	__int16 boxID;
	FLOOR_INFO *floorL, *floorR, *floorT, *floorB;
	GAME_VECTOR secondary, primary;
	BOOL clear, first;

	LOS(&Camera.target, goal);
	room = &RoomInfo[Camera.target.roomNumber];
	box = &Boxes[room->floor[((Camera.target.x - room->x) >> WALL_SHIFT) * room->xSize + ((Camera.target.z - room->z) >> WALL_SHIFT)].box];
	left = box->left << WALL_SHIFT;
	right = (box->right << WALL_SHIFT) - 1;
	top = box->top << WALL_SHIFT;
	bottom = (box->bottom << WALL_SHIFT) - 1;
	room = &RoomInfo[goal->roomNumber];
	boxID = room->floor[((goal->x - room->x) >> WALL_SHIFT) * room->xSize + ((goal->z - room->z) >> WALL_SHIFT)].box;
	if (boxID != -1 && (goal->z < left || goal->z > right || goal->x < top || goal->x > bottom)) {
		box = &Boxes[boxID];
		left = box->left << WALL_SHIFT;
		right = (box->right << WALL_SHIFT) - 1;
		top = box->top << WALL_SHIFT;
		bottom = (box->bottom << WALL_SHIFT) - 1;
	}
	z = (goal->z - 1024) | 0x3FF;
	floorL = GoodPosition(goal->x, goal->y, z, goal->roomNumber);
	if (floorL) {
		side = Boxes[floorL->box].left << WALL_SHIFT;
		if (floorL->box != -1 && side < left)
			left = side;
	} else {
		left = z;
	}
	z = (goal->z + 1024) & -0x400;
	floorR = GoodPosition(goal->x, goal->y, z, goal->roomNumber);
	if (floorR) {
		side = (Boxes[floorR->box].right << WALL_SHIFT) - 1;
		if (floorR->box != -1 && side > right)
			right = side;
	} else {
		right = z;
	}
	x = (goal->x - 1024) | 0x3FF;
	floorT = GoodPosition(x, goal->y, goal->z, goal->roomNumber);
	if (floorT) {
		side = Boxes[floorT->box].top << WALL_SHIFT;
		if (floorT->box != -1 && side < top)
			top = side;
	} else {
		top = x;
	}
	x = (goal->x + 1024) & -0x400;
	floorB = GoodPosition(x, goal->y, goal->z, goal->roomNumber);
	if (floorB) {
		side = (Boxes[floorB->box].bottom << WALL_SHIFT) - 1;
		if (floorB->box != -1 && side > bottom)
			bottom = side;
	} else {
		bottom = x;
	}
	left += 256;
	right -= 256;
	top += 256;
	bottom -= 256;
	secondary.x = goal->x;
	primary.x = goal->x;
	secondary.y = goal->y;
	primary.y = goal->y;
	secondary.z = goal->z;
	secondary.roomNumber = goal->roomNumber;
	primary.roomNumber = goal->roomNumber;
	primary.z = goal->z;
	clear = TRUE;
	if (ABS(goal->z - Camera.target.z) > ABS(goal->x - Camera.target.x)) {
		if (goal->z < left && !floorL) {
			first = Camera.pos.x < Camera.target.x;
			clear = FALSE;
			shift(&primary.z, &primary.x, &primary.y, Camera.target.z, Camera.target.x, Camera.target.y, left, top, right, bottom);
			shift(&secondary.z, &secondary.x, &secondary.y, Camera.target.z, Camera.target.x, Camera.target.y, left, bottom, right, top);
		} else {
			if (goal->z > right && !floorR) {
				first = Camera.pos.x < Camera.target.x;
				clear = FALSE;
				shift(&primary.z, &primary.x, &primary.y, Camera.target.z, Camera.target.x, Camera.target.y, right, top, left, bottom);
				shift(&secondary.z, &secondary.x, &secondary.y, Camera.target.z, Camera.target.x, Camera.target.y, right, bottom, left, top);
			}
		}
		if (clear) {
			if (goal->x < top && !floorT) {
				first = goal->z < Camera.target.z;
				clear = FALSE;
				shift(&primary.x, &primary.z, &primary.y, Camera.target.x, Camera.target.z, Camera.target.y, top, left, bottom, right);
				shift(&secondary.x, &secondary.z, &secondary.y, Camera.target.x, Camera.target.z, Camera.target.y, top, right, bottom, left);
			} else {
				if (goal->x > bottom && !floorB) {
					first = goal->z < Camera.target.z;
					clear = FALSE;
					shift(&primary.x, &primary.z, &primary.y, Camera.target.x, Camera.target.z, Camera.target.y, bottom, left, top, right);
					shift(&secondary.x, &secondary.z, &secondary.y, Camera.target.x, Camera.target.z, Camera.target.y, bottom, right, top, left);
				}
			}
		}
	} else {
		if (goal->x < top && !floorT) {
			first = Camera.pos.z < Camera.target.z;
			clear = FALSE;
			shift(&primary.x, &primary.z, &primary.y, Camera.target.x, Camera.target.z, Camera.target.y, top, left, bottom, right);
			shift(&secondary.x, &secondary.z, &secondary.y, Camera.target.x, Camera.target.z, Camera.target.y, top, right, bottom, left);
		} else {
			if (goal->x > bottom && !floorB) {
				first = Camera.pos.z < Camera.target.z;
				clear = FALSE;
				shift(&primary.x, &primary.z, &primary.y, Camera.target.x, Camera.target.z, Camera.target.y, bottom, left, top, right);
				shift(&secondary.x, &secondary.z, &secondary.y, Camera.target.x, Camera.target.z, Camera.target.y, bottom, right, top, left);
			}
		}
		if (clear) {
			if (goal->z < left && !floorL) {
				first = goal->x < Camera.target.x;
				clear = FALSE;
				shift(&primary.z, &primary.x, &primary.y, Camera.target.z, Camera.target.x, Camera.target.y, left, top, right, bottom);
				shift(&secondary.z, &secondary.x, &secondary.y, Camera.target.z, Camera.target.x, Camera.target.y, left, bottom, right, top);
			} else {
				if (goal->z > right && !floorR) {
					first = goal->x < Camera.target.x;
					clear = FALSE;
					shift(&primary.z, &primary.x, &primary.y, Camera.target.z, Camera.target.x, Camera.target.y, right, top, left, bottom);
					shift(&secondary.z, &secondary.x, &secondary.y, Camera.target.z, Camera.target.x, Camera.target.y, right, bottom, left, top);
				}
			}
		}
	}
	if (!clear) {
		if (first && !LOS(&Camera.target, &primary)) {
			first = FALSE;
		} else {
			if (!first && !LOS(&Camera.target, &secondary))
				first = TRUE;
		}
		if (first) {
			goal->x = primary.x;
			goal->y = primary.y;
			goal->z = primary.z;
		} else {
			goal->x = secondary.x;
			goal->y = secondary.y;
			goal->z = secondary.z;
		}
		GetFloor(goal->x, goal->y, goal->z, &goal->roomNumber);
	}
}

void __cdecl ChaseCamera(ITEM_INFO *item) {
	int distance;
	GAME_VECTOR goal;

	Camera.targetElevation += item->pos.rotX;
	CLAMP(Camera.targetElevation, -85 * PHD_DEGREE, 85 * PHD_DEGREE);
	distance = Camera.targetDistance * phd_cos(Camera.targetElevation) >> W2V_SHIFT;
	goal.y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.targetElevation) >> W2V_SHIFT);
	Camera.targetSquare = SQR(distance);
	goal.x = Camera.target.x - (distance * phd_sin(Camera.targetAngle + item->pos.rotY) >> W2V_SHIFT);
	goal.z = Camera.target.z - (distance * phd_cos(Camera.targetAngle + item->pos.rotY) >> W2V_SHIFT);
	goal.roomNumber = Camera.pos.roomNumber;
	SmartShift(&goal, ShiftCamera);
	MoveCamera(&goal, Camera.speed);
}

int __cdecl ShiftClamp(GAME_VECTOR *pos, int clamp) {
	FLOOR_INFO *floor;
	BOX_INFO *box;
	int x, z, left, right, top, bottom, height, ceiling, shift;

	x = pos->x;
	z = pos->z;
	floor = GetFloor(x, pos->y, z, &pos->roomNumber);
	box = &Boxes[floor->box];
	left = (box->left << WALL_SHIFT) + clamp;
	right = (box->right << WALL_SHIFT) - clamp - 1;
	if (z < left && !GoodPosition(x, pos->y, z - clamp, pos->roomNumber)) {
		pos->z = left;
	} else {
		if (z > right && !GoodPosition(x, pos->y, z + clamp, pos->roomNumber))
			pos->z = right;
	}
	top = (box->top << WALL_SHIFT) + clamp;
	bottom = (box->bottom << WALL_SHIFT) - clamp - 1;
	if (x < top && !GoodPosition(x - clamp, pos->y, z, pos->roomNumber)) {
		pos->x = top;
	} else {
		if (x > bottom && !GoodPosition(x + clamp, pos->y, z, pos->roomNumber))
			pos->x = bottom;
	}
	height = GetHeight(floor, x, pos->y, z) - clamp;
	ceiling = GetCeiling(floor, x, pos->y, z) + clamp;
	if (height < ceiling) {
		height = (height + ceiling) >> 1;
		ceiling = height;
	}
	if (pos->y > height) {
		shift = height - pos->y;
	} else {
		if (pos->y < ceiling) {
			shift = ceiling - pos->y;
		} else {
			shift = 0;
		}
	}
	return shift;
}

void __cdecl CombatCamera(ITEM_INFO *item) {
	int distance, y, dy;
	GAME_VECTOR goal;

	Camera.target.z = item->pos.z;
	Camera.target.x = item->pos.x;
	if (Lara.target) {
		Camera.targetAngle = item->pos.rotY + Lara.target_angles[0];
		Camera.targetElevation = item->pos.rotX + Lara.target_angles[1];
	} else {
		Camera.targetAngle = item->pos.rotY + Lara.head_y_rot + Lara.torso_y_rot;
		Camera.targetElevation = item->pos.rotX + Lara.head_x_rot + Lara.torso_x_rot;
	}
	Camera.targetDistance = 2560;
	distance = Camera.targetDistance * phd_cos(Camera.targetElevation) >> W2V_SHIFT;
	goal.x = Camera.target.x - (distance * phd_sin(Camera.targetAngle) >> W2V_SHIFT);
	goal.y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.targetElevation) >> W2V_SHIFT);
	goal.z = Camera.target.z - (distance * phd_cos(Camera.targetAngle) >> W2V_SHIFT);
	goal.roomNumber = Camera.pos.roomNumber;
	y = LaraItem->pos.y + Lara.water_surface_dist;
	if (Lara.water_status == LWS_Underwater && Camera.target.y > y && y > goal.y) {
		dy = goal.y - Camera.target.y;
		goal.y = y;
		goal.z = Camera.target.z + (goal.z - Camera.target.z) * (y - Camera.target.y) / dy;
		goal.x = Camera.target.x + (goal.x - Camera.target.x) * (y - Camera.target.y) / dy;
	}
	SmartShift(&goal, ShiftCamera);
	MoveCamera(&goal, Camera.speed);
}

void __cdecl LookCamera(ITEM_INFO *item) {
	int zOld = Camera.target.z;
	int xOld = Camera.target.x;
	Camera.target.z = item->pos.z;
	Camera.target.x = item->pos.x;
	Camera.targetAngle = item->pos.rotY + Lara.torso_y_rot + Lara.head_y_rot;
	Camera.targetDistance = CAM_DISTANCE;
	Camera.targetElevation = item->pos.rotX + Lara.torso_x_rot + Lara.head_x_rot;
	Camera.shift = (-0x200 * phd_sin(Camera.targetElevation)) >> W2V_SHIFT;
	Camera.target.z += Camera.shift * phd_cos(item->pos.rotY) >> W2V_SHIFT;
	Camera.target.x += Camera.shift * phd_sin(item->pos.rotY) >> W2V_SHIFT;

	if( !GoodPosition(Camera.target.x, Camera.target.y, Camera.target.z, Camera.target.roomNumber) ) {
		Camera.target.x = item->pos.x;
		Camera.target.z = item->pos.z;
	}
	Camera.target.y += ShiftClamp(&Camera.target, 306);

	GAME_VECTOR goal;
	int dist = Camera.targetDistance * phd_cos(Camera.targetElevation) >> W2V_SHIFT;
	goal.x = Camera.target.x - (dist * phd_sin(Camera.targetAngle) >> W2V_SHIFT);
	goal.y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.targetElevation) >> W2V_SHIFT);
	goal.z = Camera.target.z - (dist * phd_cos(Camera.targetAngle) >> W2V_SHIFT);
	goal.roomNumber = Camera.pos.roomNumber;

	SmartShift(&goal, ClipCamera);
	Camera.target.z = zOld + (Camera.target.z - zOld) / Camera.speed;
	Camera.target.x = xOld + (Camera.target.x - xOld) / Camera.speed;
	MoveCamera(&goal, Camera.speed);
}

void __cdecl FixedCamera() {
	GAME_VECTOR goal;
	OBJECT_VECTOR *fixed = &Camera.fixed[Camera.number];
	goal.x = fixed->x;
	goal.y = fixed->y;
	goal.z = fixed->z;
	goal.roomNumber = fixed->data;
	if( !LOS(&Camera.target, &goal) ) {
		ShiftClamp(&goal, 0x100);
	}
	Camera.fixedCamera = 1;
	MoveCamera(&goal, Camera.speed);
	if( Camera.timer && !--Camera.timer ) {
		Camera.timer = -1;
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
		Camera.targetDistance = CAM_DISTANCE;
		Camera.flags = CFL_None;
	}
	IsChunkyCamera = 0;
}

/*
 * Inject function
 */
void Inject_Camera() {
	INJECT(0x00410580, InitialiseCamera);
	INJECT(0x00410630, MoveCamera);
	INJECT(0x004109B0, ClipCamera);
	INJECT(0x00410A90, ShiftCamera);
	INJECT(0x00410BF0, GoodPosition);
	INJECT(0x00410C40, SmartShift);
	INJECT(0x004113D0, ChaseCamera);
	INJECT(0x004114C0, ShiftClamp);
	INJECT(0x00411660, CombatCamera);
	INJECT(0x004117F0, LookCamera);
	INJECT(0x004119E0, FixedCamera);
	INJECT(0x00411A80, CalculateCamera);
}
