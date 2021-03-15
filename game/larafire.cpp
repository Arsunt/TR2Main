/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
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
#include "game/larafire.h"
#include "game/invfunc.h"
#include "game/lara1gun.h"
#include "game/lara2gun.h"
#include "game/laraflare.h"
#include "game/sound.h"
#include "global/vars.h"

void __cdecl LaraGun() {
	DWORD *ammo;

	if (Lara.left_arm.flash_gun > 0)
		--Lara.left_arm.flash_gun;
	if (Lara.right_arm.flash_gun > 0)
		--Lara.right_arm.flash_gun;
	if (LaraItem->hitPoints <= 0) {
		Lara.gun_status = LGS_Armless;
	} else {
		if (Lara.gun_status == LGS_Armless) {
			if (CHK_ANY(InputStatus, IN_DRAW)) {
				Lara.request_gun_type = Lara.last_gun_type;
			} else {
				if (CHK_ANY(InputStatus, IN_FLARE)) {
					if (Lara.gun_type == LGT_Flare) {
						Lara.gun_status = LGS_Undraw;
					} else {
						if (Inv_RequestItem(ID_FLARES_ITEM))
							Lara.request_gun_type = LGT_Flare;
					}
				}
			}
			if (Lara.request_gun_type != Lara.gun_type || CHK_ANY(InputStatus, IN_DRAW)) {
				if (Lara.request_gun_type != LGT_Flare &&
					(Lara.skidoo != -1 ||
					(Lara.request_gun_type != LGT_Harpoon &&
					Lara.water_status != LWS_AboveWater &&
					(Lara.water_status != LWS_Wade ||
					Lara.water_surface_dist <= -Weapons[Lara.gun_type].gunHeight))))
				{
					Lara.last_gun_type = Lara.request_gun_type;
					if (Lara.gun_type == LGT_Flare) {
						Lara.request_gun_type = LGT_Flare;
					} else {
						Lara.gun_type = Lara.request_gun_type;
					}
				} else {
					if (Lara.gun_type == LGT_Flare) {
						CreateFlare(FALSE);
						undraw_flare_meshes();
						Lara.flare_control_left = 0;
					}
					Lara.gun_type = Lara.request_gun_type;
					InitialiseNewWeapon();
					Lara.gun_status = LGS_Draw;
					Lara.right_arm.frame_number = 0;
					Lara.left_arm.frame_number = 0;
				}
			}
		} else {
			if (Lara.gun_status == LGS_Ready) {
				if (CHK_ANY(InputStatus, IN_FLARE) && Inv_RequestItem(ID_FLARES_ITEM))
					Lara.request_gun_type = LGT_Flare;
				if (CHK_ANY(InputStatus, IN_DRAW) ||
					Lara.request_gun_type != Lara.gun_type ||
					(Lara.gun_type != LGT_Harpoon &&
					Lara.water_status != LWS_AboveWater &&
					(Lara.water_status != LWS_Wade ||
					Lara.water_surface_dist < -Weapons[Lara.gun_type].gunHeight)))
				{
					Lara.gun_status = LGS_Undraw;
				}
			}
		}
	}
	switch (Lara.gun_status) {
		case LGS_Armless:
			if (Lara.gun_type == LGT_Flare) {
				if (Lara.skidoo == -1 && !CheckForHoldingState(LaraItem->currentAnimState)) {
					Lara.flare_control_left = 0;
				} else {
					if (!Lara.flare_control_left) {
						Lara.flare_control_left = 1;
						Lara.left_arm.frame_number = 95;
					} else {
						if (Lara.left_arm.frame_number) {
							++Lara.left_arm.frame_number;
							if (Lara.left_arm.frame_number == 110)
								Lara.left_arm.frame_number = 0;
						}
					}
				}
				DoFlareInHand(Lara.flare_age);
				set_flare_arm(Lara.left_arm.frame_number);
			}
			break;
		case LGS_HandBusy:
			if (Lara.gun_type == LGT_Flare) {
				Lara.flare_control_left = Lara.skidoo != -1 || CheckForHoldingState(LaraItem->currentAnimState);
				DoFlareInHand(Lara.flare_age);
				set_flare_arm(Lara.left_arm.frame_number);
			}
			break;
		case LGS_Draw:
			if (Lara.gun_type != LGT_Flare && Lara.gun_type != LGT_Unarmed)
				Lara.last_gun_type = Lara.gun_type;
			switch (Lara.gun_type) {
				case LGT_Pistols:
				case LGT_Magnums:
				case LGT_Uzis:
					if (Camera.type != CAM_Cinematic && Camera.type != CAM_Look)
						Camera.type = CAM_Combat;
					draw_pistols(Lara.gun_type);
					break;
				case LGT_Shotgun:
				case LGT_M16:
				case LGT_Grenade:
				case LGT_Harpoon:
					if (Camera.type != CAM_Cinematic && Camera.type != CAM_Look)
						Camera.type = CAM_Combat;
					draw_shotgun(Lara.gun_type);
					break;
				case LGT_Flare:
					draw_flare();
					break;
				default:
					Lara.gun_status = LGS_Armless;
					break;
			}
			break;
		case LGS_Undraw:
			Lara.mesh_ptrs[14] = MeshPtr[Objects[ID_LARA].meshIndex + 14];
			switch (Lara.gun_type) {
				case LGT_Pistols:
				case LGT_Magnums:
				case LGT_Uzis:
					undraw_pistols(Lara.gun_type);
					break;
				case LGT_Shotgun:
				case LGT_M16:
				case LGT_Grenade:
				case LGT_Harpoon:
					undraw_shotgun(Lara.gun_type);
					break;
				case LGT_Flare:
					undraw_flare();
					break;
			}
			break;
		case LGS_Ready:
			if (!Lara.pistol_ammo || !CHK_ANY(InputStatus, IN_ACTION)) {
				Lara.mesh_ptrs[14] = MeshPtr[Objects[ID_LARA].meshIndex + 14];
			} else {
				Lara.mesh_ptrs[14] = MeshPtr[Objects[ID_LARA_UZIS].meshIndex + 14];
			}
			if (Camera.type != CAM_Cinematic && Camera.type != CAM_Look)
				Camera.type = CAM_Combat;
			switch (Lara.gun_type) {
				case LGT_Magnums:
					ammo = &Lara.magnum_ammo;
					break;
				case LGT_Uzis:
					ammo = &Lara.uzi_ammo;
					break;
				case LGT_Shotgun:
					ammo = &Lara.shotgun_ammo;
					break;
				case LGT_M16:
					ammo = &Lara.m16_ammo;
					break;
				case LGT_Grenade:
					ammo = &Lara.grenade_ammo;
					break;
				case LGT_Harpoon:
					ammo = &Lara.harpoon_ammo;
					break;
				default:
					ammo = &Lara.pistol_ammo;
					break;
			}
			if (CHK_ANY(InputStatus, IN_ACTION) && *ammo <= 0) {
				*ammo = 0;
				PlaySoundEffect(48, &LaraItem->pos, 0);
				Lara.request_gun_type = Inv_RequestItem(ID_PISTOL_ITEM) ? LGT_Pistols : LGT_Unarmed;
			} else {
				switch (Lara.gun_type) {
					case LGT_Pistols:
					case LGT_Magnums:
					case LGT_Uzis:
						PistolHandler(Lara.gun_type);
						break;
					case LGT_Shotgun:
					case LGT_M16:
					case LGT_Grenade:
					case LGT_Harpoon:
						RifleHandler(Lara.gun_type);
						break;
				}
			}
			break;
		case LGS_Special:
			draw_flare();
			break;
	}
}

/*
 * Inject function
 */
void Inject_LaraFire() {
	INJECT(0x0042E740, LaraGun);

//	INJECT(0x0042ECB0, CheckForHoldingState);
//	INJECT(0x0042ECF0, InitialiseNewWeapon);
//	INJECT(0x0042EE30, LaraTargetInfo);
//	INJECT(0x0042EFD0, LaraGetNewTarget);
//	INJECT(0x0042F1F0, find_target_point);
//	INJECT(0x0042F2A0, AimWeapon);
//	INJECT(0x0042F370, FireWeapon);
//	INJECT(0x0042F6E0, HitTarget);
//	INJECT(0x0042F780, SmashItem);
//	INJECT(0x0042F7E0, WeaponObject);
}