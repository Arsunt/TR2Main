/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Copyright (c) 2020 ChocolateFan <asasas9500@gmail.com>
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
#include "game/lara1gun.h"
#include "3dsystem/phd_math.h"
#include "game/box.h"
#include "game/control.h"
#include "game/draw.h"
#include "game/items.h"
#include "game/lara.h"
#include "game/larafire.h"
#include "game/objects.h"
#include "game/sound.h"
#include "specific/game.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

#ifdef FEATURE_GAMEPLAY_FIXES
bool IsRunningM16fix = true;
#endif // FEATURE_GAMEPLAY_FIXES

void __cdecl RifleHandler(int weaponType) {
	WEAPON_INFO *weapon = &Weapons[weaponType];

	if( CHK_ANY(InputStatus, IN_ACTION) )
		LaraTargetInfo(&Weapons[weaponType]);
	else {
		Lara.target = 0;
	}

	if( !Lara.target ) {
		LaraGetNewTarget(weapon);
	}

	AimWeapon(weapon, &Lara.left_arm);

	if( Lara.left_arm.lock ) {
		Lara.torso_x_rot = Lara.left_arm.x_rot;
		Lara.torso_y_rot = Lara.left_arm.y_rot;
		Lara.head_x_rot = Lara.head_y_rot = 0;
	}

	AnimateShotgun(weaponType);

	if( Lara.right_arm.flash_gun && (weaponType == LGT_Shotgun || weaponType == LGT_M16) ) {
		int x = LaraItem->pos.x + (phd_sin(LaraItem->pos.rotY) >> (W2V_SHIFT-10));
		int y = LaraItem->pos.y - 0x200;
		int z = LaraItem->pos.z + (phd_cos(LaraItem->pos.rotY) >> (W2V_SHIFT-10));
		AddDynamicLight(x, y, z, 12, 11);
	}
}

void __cdecl FireShotgun() {
	__int16 base[2], angles[2];
	BOOL isFired = FALSE;
	base[0] = Lara.left_arm.y_rot + LaraItem->pos.rotY;
	base[1] = Lara.left_arm.x_rot;

	for( int i=0; i<6; ++i ) {
		angles[0] = base[0] + 20*PHD_DEGREE * (GetRandomControl() - PHD_ONE/4) / PHD_ONE;
		angles[1] = base[1] + 20*PHD_DEGREE * (GetRandomControl() - PHD_ONE/4) / PHD_ONE;
		if( FireWeapon(LGT_Shotgun, Lara.target, LaraItem, angles) ) {
			isFired = TRUE;
		}
	}

	if( isFired ) {
		Lara.right_arm.flash_gun = Weapons[LGT_Shotgun].flashTime;
		PlaySoundEffect(Weapons[LGT_Shotgun].sampleNum, &LaraItem->pos, 0);
#ifdef FEATURE_INPUT_IMPROVED
		JoyVibrate(0x2000, 0x2000, 2, 0x800, 5, false);
#endif // FEATURE_INPUT_IMPROVED
	}
}

void __cdecl FireM16(BOOL isRunning) {
	__int16 angles[2];

	angles[0] = Lara.left_arm.y_rot + LaraItem->pos.rotY;
	angles[1] = Lara.left_arm.x_rot;

	// NOTE: Ther was a bug in the original game - ID_LARA_M16 instead of LGT_M16
#ifdef FEATURE_GAMEPLAY_FIXES
	if( IsRunningM16fix && isRunning ) {
		Weapons[LGT_M16].shotAccuracy = 12*PHD_DEGREE;
		Weapons[LGT_M16].damage = 1;
	} else {
		Weapons[LGT_M16].shotAccuracy = 4*PHD_DEGREE;
		Weapons[LGT_M16].damage = 3;
	}
#endif // FEATURE_GAMEPLAY_FIXES

	if( FireWeapon(LGT_M16, Lara.target, LaraItem, angles) ) {
		Lara.right_arm.flash_gun = Weapons[LGT_M16].flashTime;
#ifdef FEATURE_INPUT_IMPROVED
		JoyVibrate(0x400, 0x400, 2, 0x80, 4, false);
#endif // FEATURE_INPUT_IMPROVED
	}
}

void __cdecl FireHarpoon() {
	GAME_VECTOR pos;
	if( Lara.harpoon_ammo <= 0 ) return;
	__int16 itemID = CreateItem();
	if( itemID < 0 ) return;

	ITEM_INFO *item = &Items[itemID];
	item->objectID = ID_HARPOON_BOLT;
	item->roomNumber = LaraItem->roomNumber;
	pos.x = -2;
	pos.y = 373;
	pos.z = 77;
	GetLaraJointAbsPosition((PHD_VECTOR *)&pos, 10);
	item->pos.x = pos.x;
	item->pos.y = pos.y;
	item->pos.z = pos.z;
	InitialiseItem(itemID);
	if( Lara.target ) {
		find_target_point(Lara.target, &pos);
		item->pos.rotY = phd_atan(pos.z - item->pos.z, pos.x - item->pos.x);
		int distance = phd_sqrt(SQR(pos.x - item->pos.x) + SQR(pos.z - item->pos.z));
		item->pos.rotX = -phd_atan(distance, pos.y - item->pos.y);
	} else {
		item->pos.rotX = Lara.left_arm.x_rot + LaraItem->pos.rotX;
		item->pos.rotY = Lara.left_arm.y_rot + LaraItem->pos.rotY;
	}
	item->pos.rotZ = 0;
	item->fallSpeed = -150 * phd_sin(item->pos.rotX) >> W2V_SHIFT;
	item->speed = 150 * phd_cos(item->pos.rotX) >> W2V_SHIFT;
	AddActiveItem(itemID);
	if( !SaveGame.bonusFlag ) {
		--Lara.harpoon_ammo;
	}
	++SaveGame.statistics.shots;
#ifdef FEATURE_INPUT_IMPROVED
	JoyVibrate(0xC00, 0xC00, 2, 0x400, 4, false);
#endif // FEATURE_INPUT_IMPROVED
}

void __cdecl FireRocket() {
	__int16 itemID;
	ITEM_INFO *item;
	PHD_VECTOR pos;

	if (Lara.grenade_ammo > 0) {
		itemID = CreateItem();
		if (itemID != -1) {
			item = &Items[itemID];
			item->objectID = ID_ROCKET;
			item->roomNumber = LaraItem->roomNumber;
			pos.x = -2;
			pos.y = 373;
			pos.z = 77;
			GetLaraJointAbsPosition(&pos, 10);
			item->pos.x = pos.x;
			item->pos.y = pos.y;
			item->pos.z = pos.z;
			InitialiseItem(itemID);
			item->pos.rotX = LaraItem->pos.rotX + Lara.left_arm.x_rot;
			item->pos.rotZ = 0;
			item->speed = 200;
			item->fallSpeed = 0;
			item->pos.rotY = LaraItem->pos.rotY + Lara.left_arm.y_rot;
			AddActiveItem(itemID);
			if (!SaveGame.bonusFlag)
				--Lara.grenade_ammo;
			++SaveGame.statistics.shots;
#ifdef FEATURE_INPUT_IMPROVED
			JoyVibrate(0x1000, 0x1000, 2, 0x400, 4, false);
#endif // FEATURE_INPUT_IMPROVED
		}
	}
}

void __cdecl ControlRocket(__int16 itemID) {
	ITEM_INFO *item, *link;
	int oldX, oldY, oldZ, displacement, c, s, r, oldR;
	__int16 room, linkID, *frame, fxID;
	FLOOR_INFO *floor;
	BOOL collision;
	FX_INFO *fx;

	item = &Items[itemID];
	oldX = item->pos.x;
	oldY = item->pos.y;
	oldZ = item->pos.z;
	if (item->speed < 190)
		++item->fallSpeed;
	--item->speed;
	item->pos.y += item->fallSpeed - (item->speed * phd_sin(item->pos.rotX) >> W2V_SHIFT);
	item->pos.z += phd_cos(item->pos.rotY) * (item->speed * phd_cos(item->pos.rotX) >> W2V_SHIFT) >> W2V_SHIFT;
	item->pos.x += phd_sin(item->pos.rotY) * (item->speed * phd_cos(item->pos.rotX) >> W2V_SHIFT) >> W2V_SHIFT;
	room = item->roomNumber;
	floor = GetFloor(item->pos.x, item->pos.y, item->pos.z, &room);
	item->floor = GetHeight(floor, item->pos.x, item->pos.y, item->pos.z);
	if (item->roomNumber != room)
		ItemNewRoom(itemID, room);
	if (item->pos.y < item->floor && item->pos.y > GetCeiling(floor, item->pos.x, item->pos.y, item->pos.z)) {
		collision = FALSE;
		displacement = 0;
	} else {
		displacement = 512;
		collision = TRUE;
	}
	for (linkID = RoomInfo[item->roomNumber].itemNumber; linkID != -1; linkID = link->nextItem) {
		link = &Items[linkID];
		if (link != LaraItem &&
			link->collidable &&
			(link->objectID == ID_WINDOW1 ||
			(Objects[link->objectID].intelligent &&
			link->status != ITEM_INVISIBLE &&
			Objects[link->objectID].collision)))
		{
			frame = GetBestFrame(link);
			if (item->pos.y + displacement >= link->pos.y + frame[2] && item->pos.y - displacement <= link->pos.y + frame[3]) {
				c = phd_cos(link->pos.rotY);
				s = phd_sin(link->pos.rotY);
				r = (c * (item->pos.x - link->pos.x) - s * (item->pos.z - link->pos.z)) >> W2V_SHIFT;
				oldR = (c * (oldX - link->pos.x) - s * (oldZ - link->pos.z)) >> W2V_SHIFT;
				if ((r + displacement >= frame[0] ||
					oldR + displacement >= frame[0]) &&
					(r - displacement <= frame[1] ||
					oldR - displacement <= frame[1]))
				{
					r = (s * (item->pos.x - link->pos.x) + c * (item->pos.z - link->pos.z)) >> W2V_SHIFT;
					oldR = (s * (oldX - link->pos.x) + c * (oldZ - link->pos.z)) >> W2V_SHIFT;
					if ((r + displacement >= frame[4] ||
						oldR + displacement >= frame[4]) &&
						(r - displacement <= frame[5] ||
						oldR - displacement <= frame[5]))
					{
						if (link->objectID == ID_WINDOW1) {
							SmashWindow(linkID);
						} else {
							if (link->status == ITEM_ACTIVE) {
								HitTarget(link, NULL, 30);
								++SaveGame.statistics.hits;
								if (link->hitPoints <= 0) {
									++SaveGame.statistics.kills;
									if (link->objectID != ID_DRAGON_FRONT && link->objectID != ID_GIANT_YETI)
										CreatureDie(linkID, TRUE);
								}
							}
							collision = TRUE;
						}
					}
				}
			}
		}
	}
	if (collision) {
		fxID = CreateEffect(item->roomNumber);
		if (fxID != -1) {
			fx = &Effects[fxID];
			fx->pos.x = oldX;
			fx->pos.y = oldY;
			fx->pos.z = oldZ;
			fx->speed = 0;
			fx->frame_number = 0;
			fx->counter = 0;
			fx->object_number = ID_EXPLOSION;
		}
		PlaySoundEffect(105, NULL, 0);
		KillItem(itemID);
#ifdef FEATURE_INPUT_IMPROVED
		JoyRumbleExplode(oldX, oldY, oldZ, 0x1400, true);
#endif // FEATURE_INPUT_IMPROVED
	}
}

void __cdecl AnimateShotgun(int gunType) {
	ITEM_INFO *item;
	BOOL change;
	static BOOL IsFireM16 = FALSE;
	static BOOL IsFireHarpoon = FALSE;

	item = &Items[Lara.weapon_item];
	if (gunType == LGT_M16 && LaraItem->speed) {
		change = TRUE;
	} else {
		change = FALSE;
	}
	switch (item->currentAnimState) {
		case 0:
			IsFireM16 = FALSE;
			if (IsFireHarpoon) {
				item->goalAnimState = 5;
				IsFireHarpoon = FALSE;
			} else {
				if (Lara.water_status != LWS_Underwater && !change) {
					if ((CHK_ANY(InputStatus, IN_ACTION) && !Lara.target) || Lara.left_arm.lock) {
						item->goalAnimState = 2;
					} else {
						item->goalAnimState = 4;
					}
				} else {
					item->goalAnimState = 6;
				}
			}
			break;
		case 2:
			if (item->frameNumber == Anims[item->animNumber].frameBase) {
				item->goalAnimState = 4;
				if (Lara.water_status != LWS_Underwater && !change && !IsFireHarpoon) {
					if (CHK_ANY(InputStatus, IN_ACTION)) {
						if (!Lara.target || Lara.left_arm.lock) {
							if (gunType == LGT_Harpoon) {
								FireHarpoon();
								if (!CHK_ANY(Lara.harpoon_ammo, 3))
									IsFireHarpoon = TRUE;
							} else {
								if (gunType == LGT_Grenade) {
									FireRocket();
								} else {
									if (gunType == LGT_M16) {
										FireM16(FALSE);
										PlaySoundEffect(78, &LaraItem->pos, 0);
										IsFireM16 = TRUE;
									} else {
										FireShotgun();
									}
								}
							}
							item->goalAnimState = 2;
						}
					} else {
						if (Lara.left_arm.lock)
							item->goalAnimState = 0;
					}
				}
				if (item->goalAnimState != 2 && IsFireM16) {
					PlaySoundEffect(104, &LaraItem->pos, 0);
					IsFireM16 = FALSE;
				}
			} else {
				if (IsFireM16) {
					PlaySoundEffect(78, &LaraItem->pos, 0);
				} else {
					if (gunType == LGT_Shotgun && !CHK_ANY(InputStatus, IN_ACTION) && !Lara.left_arm.lock)
						item->goalAnimState = 4;
				}
			}
			break;
		case 6:
			IsFireM16 = FALSE;
			if (IsFireHarpoon) {
				item->goalAnimState = 5;
				IsFireHarpoon = FALSE;
			} else {
				if (Lara.water_status != LWS_Underwater && !change) {
					item->goalAnimState = 0;
				} else {
					if ((CHK_ANY(InputStatus, IN_ACTION) && !Lara.target) || Lara.left_arm.lock) {
						item->goalAnimState = 8;
					} else {
						item->goalAnimState = 7;
					}
				}
			}
			break;
		case 8:
			if (item->frameNumber == Anims[item->animNumber].frameBase) {
				item->goalAnimState = 7;
				if ((Lara.water_status == LWS_Underwater || change) && !IsFireHarpoon) {
					if (CHK_ANY(InputStatus, IN_ACTION)) {
						if (!Lara.target || Lara.left_arm.lock) {
							if (gunType == LGT_Harpoon) {
								FireHarpoon();
								if (!CHK_ANY(Lara.harpoon_ammo, 3))
									IsFireHarpoon = TRUE;
							} else {
								FireM16(TRUE);
							}
							item->goalAnimState = 8;
						}
					} else {
						if (Lara.left_arm.lock)
							item->goalAnimState = 6;
					}
				}
			}
			if (gunType == LGT_M16 && item->goalAnimState == 8)
				PlaySoundEffect(78, &LaraItem->pos, 0);
			break;
	}
	AnimateItem(item);
	Lara.right_arm.frame_base = Anims[item->animNumber].framePtr;
	Lara.left_arm.frame_base = Lara.right_arm.frame_base;
	Lara.right_arm.frame_number = item->frameNumber - Anims[item->animNumber].frameBase;
	Lara.left_arm.frame_number = Lara.right_arm.frame_number;
	Lara.right_arm.anim_number = item->animNumber;
	Lara.left_arm.anim_number = Lara.right_arm.anim_number;
}

/*
 * Inject function
 */
void Inject_Lara1Gun() {
//	INJECT(0x0042BC90, draw_shotgun_meshes);
//	INJECT(0x0042BCD0, undraw_shotgun_meshes);
//	INJECT(0x0042BD00, ready_shotgun);

	INJECT(0x0042BD70, RifleHandler);
	INJECT(0x0042BE70, FireShotgun);
	INJECT(0x0042BF70, FireM16);
	INJECT(0x0042BFF0, FireHarpoon);

//	INJECT(0x0042C180, ControlHarpoonBolt);

	INJECT(0x0042C4D0, FireRocket);
	INJECT(0x0042C5C0, ControlRocket);

//	INJECT(0x0042C9D0, draw_shotgun);
//	INJECT(0x0042CB40, undraw_shotgun);

	INJECT(0x0042CC50, AnimateShotgun);
}
