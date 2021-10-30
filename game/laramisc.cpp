/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
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
#include "game/laramisc.h"
#include "3dsystem/phd_math.h"
#include "game/collide.h"
#include "game/control.h"
#include "game/effects.h"
#include "game/lara.h"
#include "game/larasurf.h"
#include "game/laraswim.h"
#include "game/invfunc.h"
#include "game/items.h"
#include "game/larafire.h"
#include "game/sound.h"
#include "specific/sndpc.h"
#include "global/vars.h"

void __cdecl LaraControl(__int16 itemID) {
	COLL_INFO coll;
	ITEM_INFO *item = LaraItem;

#ifdef FEATURE_CHEAT
	if( CHK_ANY(GF_GameFlow.flags, GFF_EnableCheatCode|GFF_DozyCheatEnabled) ) {
		// Recover health + get inventory stuff
		if( CHK_ANY(InputStatus, IN_STUFFCHEAT) ) {
			LaraCheatGetStuff();
			item->hitPoints = 1000;
		}
		// Enable dozy cheat (flying with full health). It is allowed for boat, but not for skidoo
		if( !Lara.extra_anim && (Lara.skidoo < 0 || Items[Lara.skidoo].objectID == ID_BOAT)
			&& Lara.water_status != LWS_Cheat && CHK_ANY(InputStatus, IN_DOZYCHEAT) )
		{
			if( Lara.water_status != LWS_Underwater || item->hitPoints <= 0 ) {
				item->pos.y -= 0x80;
				item->animNumber = 87;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = AS_SWIM;
				item->goalAnimState = AS_SWIM;
				item->gravity = 0;
				item->pos.rotX = 30*PHD_DEGREE;
				item->fallSpeed = 30;
				Lara.torso_x_rot = Lara.torso_y_rot = 0;
				Lara.head_x_rot = Lara.head_y_rot = 0;
				if( item->hitPoints <= 0 && Lara.gun_type != LGT_Flare ) {
					BYTE backupGunType = SaveGame.start[CurrentLevel].gunType;
					SaveGame.start[CurrentLevel].gunType = Lara.request_gun_type;
					LaraInitialiseMeshes(CurrentLevel);
					SaveGame.start[CurrentLevel].gunType = backupGunType;
					Lara.gun_status = LGS_Armless;
					Lara.target = NULL;
					memset(&Lara.left_arm, 0, sizeof(Lara.left_arm));
					memset(&Lara.right_arm, 0, sizeof(Lara.right_arm));
					if( Lara.weapon_item >= 0 ) {
						KillItem(Lara.weapon_item);
						Lara.weapon_item = -1;
					}
				}

			}
			Lara.water_status = LWS_Cheat;
			Lara.skidoo = -1; // get off from vehicle
			Lara.spaz_effect_count = 0;
			Lara.spaz_effect = NULL;
			Lara.hit_frame = 0;
			Lara.hit_direction = -1;
			Lara.air = 1800;
			Lara.death_count = 0;
			Lara.mesh_effects = 0x7FFF; // Lara has golden skin
		}
	}
#endif // FEATURE_CHEAT

	BOOL isRoomUnderwater = CHK_ANY(RoomInfo[item->roomNumber].flags, ROOM_UNDERWATER);
	int depth = GetWaterDepth(item->pos.x, item->pos.y, item->pos.z, item->roomNumber);
	int height = GetWaterHeight(item->pos.x, item->pos.y, item->pos.z, item->roomNumber);
	int water_surface_dist = ( height == NO_HEIGHT ) ? NO_HEIGHT : item->pos.y - height;

	Lara.water_surface_dist = -water_surface_dist;
	WadeSplash(item, height);

	if( Lara.skidoo == -1 && !Lara.extra_anim ) {
		switch( Lara.water_status ) {
			case LWS_AboveWater:
				if( water_surface_dist != NO_HEIGHT && water_surface_dist >= 0x180 ) {
					if( depth <= (0x2DA-0x100) ) {
						if( water_surface_dist > 0x180 ) {
							Lara.water_status = LWS_Wade;
							if( !item->gravity ) {
								item->goalAnimState = AS_STOP;
							}
						}
					} else if( isRoomUnderwater ) {
						Lara.air = 1800;
						Lara.water_status = LWS_Underwater;
						item->gravity = 0;
						item->pos.y += 100;
						UpdateLaraRoom(item, 0);
						StopSoundEffect(30);
						switch( item->currentAnimState ) {
							case AS_SWANDIVE:
								item->pos.rotX = -45*PHD_DEGREE;
								item->goalAnimState = AS_DIVE;
								AnimateLara(item);
								item->fallSpeed *= 2;
								break;
							case AS_FASTDIVE:
								item->pos.rotX = -85*PHD_DEGREE;
								item->goalAnimState = AS_DIVE;
								AnimateLara(item);
								item->fallSpeed *= 2;
								break;
							default:
								item->pos.rotX = -45*PHD_DEGREE;
								item->animNumber = 112;
								item->frameNumber = Anims[item->animNumber].frameBase;
								item->currentAnimState = AS_DIVE;
								item->goalAnimState = AS_SWIM;
								item->fallSpeed = item->fallSpeed * 3 / 2;
								break;
						}
						Lara.torso_x_rot = Lara.torso_y_rot = 0;
						Lara.head_x_rot = Lara.head_y_rot = 0;
						Splash(item);
					}
				}
				break;
			case LWS_Wade:
				Camera.targetElevation = -22*PHD_DEGREE;
				if( water_surface_dist >= 0x180 ) {
					if( water_surface_dist > 0x2DA ) {
						Lara.water_status = LWS_Surface;
						item->pos.y += 1 - water_surface_dist;
						switch( item->currentAnimState ) {
							case AS_BACK:
								item->currentAnimState = AS_SURFBACK;
								item->animNumber = 140;
								item->frameNumber = Anims[item->animNumber].frameBase;
								break;
							case AS_STEPRIGHT:
								item->currentAnimState = AS_SURFRIGHT;
								item->animNumber = 144;
								item->frameNumber = Anims[item->animNumber].frameBase;;
								break;
							case AS_STEPLEFT:
								item->currentAnimState = AS_SURFLEFT;
								item->animNumber = 143;
								item->frameNumber = Anims[item->animNumber].frameBase;;
								break;
							default:
								item->currentAnimState = AS_SURFSWIM;
								item->animNumber = 116;
								item->frameNumber = Anims[item->animNumber].frameBase;;
								break;
						}
						item->gravity = 0;
						item->goalAnimState = item->currentAnimState;
						item->fallSpeed = 0;
						Lara.dive_count = 0;
						item->pos.rotX = item->pos.rotZ = 0;
						Lara.torso_x_rot = Lara.torso_y_rot = 0;
						Lara.head_x_rot = Lara.head_y_rot = 0;
						UpdateLaraRoom(item, -0x17D);
					}
				} else {
					Lara.water_status = LWS_AboveWater;
					if( item->currentAnimState == AS_WADE )
						item->goalAnimState = AS_RUN;
				}
				break;
			case LWS_Surface:
				if( !isRoomUnderwater ) {
					if( water_surface_dist <= 0x180 ) {
						Lara.water_status = LWS_AboveWater;
						item->animNumber = 34;
						item->frameNumber = Anims[item->animNumber].frameBase;;
						item->goalAnimState = AS_FORWARDJUMP;
						item->currentAnimState = AS_FORWARDJUMP;
						item->gravity = 1;
						item->speed = item->fallSpeed / 4;
					} else {
						Lara.water_status = LWS_Wade;
						item->animNumber = 103;
						item->currentAnimState = AS_STOP;
						item->frameNumber = Anims[item->animNumber].frameBase;;
						item->goalAnimState = AS_WADE;
						AnimateItem(item);
					}
					item->fallSpeed = 0;
					item->pos.rotX = item->pos.rotZ = 0;
					Lara.torso_x_rot = Lara.torso_y_rot = 0;
					Lara.head_x_rot = Lara.head_y_rot = 0;
				}
				break;
			case LWS_Underwater:
				if( !isRoomUnderwater ) {
					if( depth == NO_HEIGHT || ABS(water_surface_dist) >= 0x100 ) {
						Lara.water_status = LWS_AboveWater;
						item->animNumber = 34;
						item->frameNumber = Anims[item->animNumber].frameBase;;
						item->goalAnimState = AS_FORWARDJUMP;
						item->currentAnimState = AS_FORWARDJUMP;
						item->speed = item->fallSpeed / 4;
						item->gravity = 1;
						item->fallSpeed = 0;
						item->pos.rotX = item->pos.rotZ = 0;
						Lara.torso_x_rot = Lara.torso_y_rot = 0;
						Lara.head_x_rot = Lara.head_y_rot = 0;
					} else {
						Lara.water_status = LWS_Surface;
						item->pos.y += 1 - water_surface_dist;
						item->animNumber = 114;
						item->frameNumber = Anims[item->animNumber].frameBase;;
						item->goalAnimState = AS_SURFTREAD;
						item->currentAnimState = AS_SURFTREAD;
						item->fallSpeed = 0;
						Lara.dive_count = 11;
						item->pos.rotX = item->pos.rotZ = 0;
						Lara.torso_x_rot = Lara.torso_y_rot = 0;
						Lara.head_x_rot = Lara.head_y_rot = 0;
						UpdateLaraRoom(item, -0x17D);
						PlaySoundEffect(36, &item->pos, SFX_ALWAYS);
					}
				}
				break;
			default:
				break;
		}
	}

	if( item->hitPoints > 0 ) {
		if( GF_NoFloor && item->pos.y >= GF_NoFloor ) {
			item->hitPoints = -1;
			Lara.death_count = 9*30; // let's skip 9 seconds to death
		}
	} else {
		item->hitPoints = -1;
		if( !Lara.death_count ) {
			S_CDStop();
		}
		++Lara.death_count;
		if( CHK_ANY(item->flags, IFL_INVISIBLE) ) { // Nothing left from Lara?
			++Lara.death_count; // In this case death timer ticks twice faster
			return;
		}
	}

	switch( Lara.water_status ) {
		case LWS_AboveWater:
		case LWS_Wade:
			Lara.air = 1800;
			LaraAboveWater(item, &coll);
			break;
		case LWS_Underwater:
			if( item->hitPoints >= 0 && --Lara.air < 0 ) {
				Lara.air = -1;
				item->hitPoints -= 5;
			}
			LaraUnderWater(item, &coll);
			break;
		case LWS_Surface:
			if( item->hitPoints >= 0 ) {
				Lara.air += 10;
				CLAMPG(Lara.air, 1800)
			}
			LaraSurface(item, &coll);
			break;
#ifdef FEATURE_CHEAT
		case LWS_Cheat:
			if( CHK_ANY(GF_GameFlow.flags, GFF_EnableCheatCode|GFF_DozyCheatEnabled) ) {
				// Update Dozy state just in case
				item->hitPoints = 1000;
				Lara.death_count = 0; // NOTE: if died already, forget about death
				LaraUnderWater(item, &coll);
				// Return Lara to normal state if Walk is pressed without Look
				if( !Lara.extra_anim && CHK_ANY(InputStatus, IN_SLOW) && !CHK_ANY(InputStatus, IN_LOOK) ) {
					if( isRoomUnderwater || (water_surface_dist != NO_HEIGHT && water_surface_dist > 0) ) {
						Lara.water_status = LWS_Underwater;
					} else {
						Lara.water_status = LWS_AboveWater;
						item->animNumber = 11;
						item->frameNumber = Anims[item->animNumber].frameBase;
						item->pos.rotX = item->pos.rotZ = 0;
						Lara.torso_x_rot = Lara.torso_y_rot = 0;
						Lara.head_x_rot = Lara.head_y_rot = 0;
					}
					Lara.mesh_effects = 0;
				}
			}
			break;
#endif // FEATURE_CHEAT
		default:
			break;
	}

	SaveGame.statistics.distance += phd_sqrt((item->pos.z - Lara.last_pos.z) * (item->pos.z - Lara.last_pos.z)
											+ (item->pos.y - Lara.last_pos.y) * (item->pos.y - Lara.last_pos.y)
											+ (item->pos.x - Lara.last_pos.x) * (item->pos.x - Lara.last_pos.x));
	Lara.last_pos.x = item->pos.x;
	Lara.last_pos.y = item->pos.y;
	Lara.last_pos.z = item->pos.z;
}

void __cdecl UseItem(__int16 itemID) {
	if( itemID <= ID_NONE || itemID >= ID_NUMBER_OBJECTS )
		return;

	switch( itemID ) {
		case ID_PISTOL_ITEM:
		case ID_PISTOL_OPTION:
			Lara.request_gun_type = LGT_Pistols;
			break;

		case ID_MAGNUM_ITEM:
		case ID_MAGNUM_OPTION:
			Lara.request_gun_type = LGT_Magnums;
			break;

		case ID_UZI_ITEM:
		case ID_UZI_OPTION:
			Lara.request_gun_type = LGT_Uzis;
			break;

		case ID_SHOTGUN_ITEM:
		case ID_SHOTGUN_OPTION:
			Lara.request_gun_type = LGT_Shotgun;
			break;

		case ID_HARPOON_ITEM:
		case ID_HARPOON_OPTION:
			Lara.request_gun_type = LGT_Harpoon;
			break;

		case ID_M16_ITEM:
		case ID_M16_OPTION:
			Lara.request_gun_type = LGT_M16;
			break;

		case ID_GRENADE_ITEM:
		case ID_GRENADE_OPTION:
			Lara.request_gun_type = LGT_Grenade;
			break;

		case ID_FLARES_ITEM:
		case ID_FLARES_OPTION:
			Lara.request_gun_type = LGT_Flare;
			break;

		case ID_SMALL_MEDIPACK_ITEM:
		case ID_SMALL_MEDIPACK_OPTION:
			if( LaraItem->hitPoints > 0 && LaraItem->hitPoints < 1000 ) {
				LaraItem->hitPoints += 500;
				if( LaraItem->hitPoints > 1000 )
					LaraItem->hitPoints = 1000;
				Inv_RemoveItem(ID_SMALL_MEDIPACK_ITEM);
				PlaySoundEffect(116, NULL, 2);
				++SaveGame.statistics.mediPacks;
			}
			break;

		case ID_LARGE_MEDIPACK_ITEM:
		case ID_LARGE_MEDIPACK_OPTION:
			if( LaraItem->hitPoints > 0 && LaraItem->hitPoints < 1000 ) {
				LaraItem->hitPoints += 1000;
				if( LaraItem->hitPoints > 1000 )
					LaraItem->hitPoints = 1000;
				Inv_RemoveItem(ID_LARGE_MEDIPACK_ITEM);
				PlaySoundEffect(116, NULL, 2);
				SaveGame.statistics.mediPacks += 2;
			}
			break;
	}
}

void __cdecl LaraCheatGetStuff() {
	// NOTE: there is no availability checks in the original code
	if( Objects[ID_PISTOL_OPTION].loaded ) {
		if( !Inv_RequestItem(ID_PISTOL_ITEM) ) {
			Inv_AddItem(ID_PISTOL_ITEM);
		}
	}
	if( Objects[ID_MAGNUM_OPTION].loaded ) {
		if( !Inv_RequestItem(ID_MAGNUM_ITEM) ) {
			Inv_AddItem(ID_MAGNUM_ITEM);
		}
		Lara.magnum_ammo = (SaveGame.bonusFlag && !IsDemoLevelType) ? 10001 : 1000;
	}
	if( Objects[ID_UZI_OPTION].loaded ) {
		if( !Inv_RequestItem(ID_UZI_ITEM) ) {
			Inv_AddItem(ID_UZI_ITEM);
		}
		Lara.uzi_ammo = (SaveGame.bonusFlag && !IsDemoLevelType) ? 10001 : 2000;
	}
	if( Objects[ID_SHOTGUN_OPTION].loaded ) {
		if( !Inv_RequestItem(ID_SHOTGUN_ITEM) ) {
			Inv_AddItem(ID_SHOTGUN_ITEM);
		}
		Lara.shotgun_ammo = (SaveGame.bonusFlag && !IsDemoLevelType) ? 10001 : 300;
	}
	if( Objects[ID_HARPOON_OPTION].loaded ) {
		if( !Inv_RequestItem(ID_HARPOON_ITEM) ) {
			Inv_AddItem(ID_HARPOON_ITEM);
		}
		Lara.harpoon_ammo = (SaveGame.bonusFlag && !IsDemoLevelType) ? 10001 : 300;
	}
	if( Objects[ID_M16_OPTION].loaded ) {
		if( !Inv_RequestItem(ID_M16_ITEM) ) {
			Inv_AddItem(ID_M16_ITEM);
		}
		Lara.m16_ammo = (SaveGame.bonusFlag && !IsDemoLevelType) ? 10001 : 300;
	}
	if( Objects[ID_GRENADE_OPTION].loaded ) {
		if( !Inv_RequestItem(ID_GRENADE_ITEM) ) {
			Inv_AddItem(ID_GRENADE_ITEM);
		}
		Lara.grenade_ammo = (SaveGame.bonusFlag && !IsDemoLevelType) ? 10001 : 300;
	}

	for( int i = 0; i < 10; ++i ) {
		// NOTE: there are no limits in the original code, but it works wrong without limits
		if( Objects[ID_FLARES_OPTION].loaded && Inv_RequestItem(ID_FLARE_ITEM) < 240 ) {
			Inv_AddItem(ID_FLARES_ITEM);
		}
		if( Objects[ID_SMALL_MEDIPACK_OPTION].loaded && Inv_RequestItem(ID_SMALL_MEDIPACK_ITEM) < 240 ) {
			Inv_AddItem(ID_SMALL_MEDIPACK_ITEM);
		}
		if( Objects[ID_LARGE_MEDIPACK_OPTION].loaded && Inv_RequestItem(ID_LARGE_MEDIPACK_ITEM) < 240 ) {
			Inv_AddItem(ID_LARGE_MEDIPACK_ITEM);
		}
	}

#ifdef FEATURE_CHEAT
	// NOTE: there is no cheat for keys/puzzles in the original code
	if( Objects[ID_KEY_OPTION1].loaded && !Inv_RequestItem(ID_KEY_ITEM1) ) {
		Inv_AddItem(ID_KEY_ITEM1);
	}
	if( Objects[ID_KEY_OPTION2].loaded && !Inv_RequestItem(ID_KEY_ITEM2) ) {
		Inv_AddItem(ID_KEY_ITEM2);
	}
	if( Objects[ID_KEY_OPTION3].loaded && !Inv_RequestItem(ID_KEY_ITEM3) ) {
		Inv_AddItem(ID_KEY_ITEM3);
	}
	if( Objects[ID_KEY_OPTION4].loaded && !Inv_RequestItem(ID_KEY_ITEM4) ) {
		Inv_AddItem(ID_KEY_ITEM4);
	}
	if( Objects[ID_PUZZLE_OPTION1].loaded && !Inv_RequestItem(ID_PUZZLE_ITEM1) ) {
		Inv_AddItem(ID_PUZZLE_ITEM1);
	}
	if( Objects[ID_PUZZLE_OPTION2].loaded && !Inv_RequestItem(ID_PUZZLE_ITEM2) ) {
		Inv_AddItem(ID_PUZZLE_ITEM2);
	}
	if( Objects[ID_PUZZLE_OPTION3].loaded && !Inv_RequestItem(ID_PUZZLE_ITEM3) ) {
		Inv_AddItem(ID_PUZZLE_ITEM3);
	}
	if( Objects[ID_PUZZLE_OPTION4].loaded && !Inv_RequestItem(ID_PUZZLE_ITEM4) ) {
		Inv_AddItem(ID_PUZZLE_ITEM4);
	}
	if( Objects[ID_PICKUP_OPTION1].loaded && !Inv_RequestItem(ID_PICKUP_ITEM1) ) {
		Inv_AddItem(ID_PICKUP_ITEM1);
	}
	if( Objects[ID_PICKUP_OPTION2].loaded && !Inv_RequestItem(ID_PICKUP_ITEM2) ) {
		Inv_AddItem(ID_PICKUP_ITEM2);
	}
#endif // FEATURE_CHEAT
}

void __cdecl ControlLaraExtra(__int16 itemID) {
	AnimateItem(&Items[itemID]);
}

void __cdecl InitialiseLaraLoad(__int16 itemID) {
	Lara.item_number = itemID;
	LaraItem = &Items[itemID];
}

void __cdecl InitialiseLaraInventory(int levelID) {
	int i;
	START_INFO *start = &SaveGame.start[levelID];

	Inv_RemoveAllItems();

	if( GF_RemoveWeapons ) {
		start->has_pistols = 0;
		start->has_magnums = 0;
		start->has_uzis = 0;
		start->has_shotgun = 0;
		start->has_m16 = 0;
		start->has_grenade = 0;
		start->has_harpoon = 0;
		start->gunType = LGT_Unarmed;
		start->gunStatus = LGS_Armless;
		GF_RemoveWeapons = 0;
	}

	if( GF_RemoveAmmo ) {
		start->m16Ammo = 0;
		start->grenadeAmmo = 0;
		start->harpoonAmmo = 0;
		start->shotgunAmmo = 0;
		start->uziAmmo = 0;
		start->magnumAmmo = 0;
		start->pistolAmmo = 0;
		start->flares = 0;
		start->largeMedipacks = 0;
		start->smallMedipacks = 0;
		GF_RemoveAmmo = 0;
	}

	// NOTE: additional weapon availability checks not presented in the original game
	if( !Objects[ID_PISTOL_OPTION].loaded ) {
		start->has_pistols = 0;
	}
	if( !Objects[ID_UZI_OPTION].loaded ) {
		start->has_uzis = 0;
	}
	if( !Objects[ID_MAGNUM_OPTION].loaded ) {
		start->has_magnums = 0;
	}
	if( !Objects[ID_SHOTGUN_OPTION].loaded ) {
		start->has_shotgun = 0;
	}
	if( !Objects[ID_GRENADE_OPTION].loaded ) {
		start->has_grenade = 0;
	}
	if( !Objects[ID_M16_OPTION].loaded ) {
		start->has_m16 = 0;
	}
	if( !Objects[ID_HARPOON_OPTION].loaded ) {
		start->has_harpoon = 0;
	}

	// NOTE: additional ammo availability checks not presented in the original game
	if( !start->has_pistols && !Objects[ID_PISTOL_AMMO_OPTION].loaded ) {
		start->pistolAmmo = 0;
	}
	if( !start->has_uzis && !Objects[ID_UZI_AMMO_OPTION].loaded ) {
		start->uziAmmo = 0;
	}
	if( !start->has_magnums && !Objects[ID_MAGNUM_AMMO_OPTION].loaded ) {
		start->magnumAmmo = 0;
	}
	if( !start->has_shotgun && !Objects[ID_SHOTGUN_AMMO_OPTION].loaded ) {
		start->shotgunAmmo = 0;
	}
	if( !start->has_grenade && !Objects[ID_GRENADE_AMMO_OPTION].loaded ) {
		start->grenadeAmmo = 0;
	}
	if( !start->has_m16 && !Objects[ID_M16_AMMO_OPTION].loaded ) {
		start->m16Ammo = 0;
	}
	if( !start->has_harpoon && !Objects[ID_HARPOON_AMMO_OPTION].loaded ) {
		start->harpoonAmmo = 0;
	}

	// NOTE: additional start gun type availability checks not presented in the original game
	switch( start->gunType ) {
		// if the current weapon is absent then fall through
		case LGT_Pistols	: if( start->has_pistols ) break; // fall through
		case LGT_Magnums	: if( start->has_magnums ) break; // fall through
		case LGT_Uzis		: if( start->has_uzis	 ) break; // fall through
		case LGT_Shotgun	: if( start->has_shotgun ) break; // fall through
		case LGT_M16		: if( start->has_m16	 ) break; // fall through
		case LGT_Grenade	: if( start->has_grenade ) break; // fall through
		case LGT_Harpoon	: if( start->has_harpoon ) break; // fall through
			start->gunType = start->has_pistols ? LGT_Pistols : LGT_Unarmed;
			break;
		default :
			break;
	}

	// Statistics
	Inv_AddItem(ID_COMPASS_ITEM);

	// Pistols
	Lara.pistol_ammo = 1000;
	if( start->has_pistols ) {
		Inv_AddItem(ID_PISTOL_ITEM);
	}

	// Magnums
	if( start->has_magnums ) {
		Inv_AddItem(ID_MAGNUM_ITEM);
		Lara.magnum_ammo = start->magnumAmmo;
		GlobalItemReplace(ID_MAGNUM_ITEM, ID_MAGNUM_AMMO_ITEM);
	} else {
		for( i = 0; i < start->magnumAmmo / 40; ++i ) {
			Inv_AddItem(ID_MAGNUM_AMMO_ITEM);
		}
		Lara.magnum_ammo = 0;
	}

	// Uzis
	if( start->has_uzis ) {
		Inv_AddItem(ID_UZI_ITEM);
		Lara.uzi_ammo = start->uziAmmo;
		GlobalItemReplace(ID_UZI_ITEM, ID_UZI_AMMO_ITEM);
	} else {
		for( i = 0; i < start->uziAmmo / 80; ++i ) {
			Inv_AddItem(ID_UZI_AMMO_ITEM);
		}
		Lara.uzi_ammo = 0;
	}

	// Shotgun
	if( start->has_shotgun ) {
		Inv_AddItem(ID_SHOTGUN_ITEM);
		Lara.shotgun_ammo = start->shotgunAmmo;
		GlobalItemReplace(ID_SHOTGUN_ITEM, ID_SHOTGUN_AMMO_ITEM);
	} else {
		for( i = 0; i < start->shotgunAmmo / 12; ++i ) {
			Inv_AddItem(ID_SHOTGUN_AMMO_ITEM);
		}
		Lara.shotgun_ammo = 0;
	}

	// Grenade
	if( start->has_grenade ) {
		Inv_AddItem(ID_GRENADE_ITEM);
		Lara.grenade_ammo = start->grenadeAmmo;
		GlobalItemReplace(ID_GRENADE_ITEM, ID_GRENADE_AMMO_ITEM);
	} else {
		for( i = 0; i < start->grenadeAmmo / 2; ++i ) {
			Inv_AddItem(ID_GRENADE_AMMO_ITEM);
		}
		Lara.grenade_ammo = 0;
	}

	// M16
	if( start->has_m16 ) {
		Inv_AddItem(ID_M16_ITEM);
		Lara.m16_ammo = start->m16Ammo;
		GlobalItemReplace(ID_M16_ITEM, ID_M16_AMMO_ITEM);
	} else {
		for( i = 0; i < start->m16Ammo / 40; ++i ) {
			Inv_AddItem(ID_M16_AMMO_ITEM);
		}
		Lara.m16_ammo = 0;
	}

	// Harpoon
	if( start->has_harpoon ) {
		Inv_AddItem(ID_HARPOON_ITEM);
		Lara.harpoon_ammo = start->harpoonAmmo;
		GlobalItemReplace(ID_HARPOON_ITEM, ID_HARPOON_AMMO_ITEM);
	} else {
		for( i = 0; i < start->harpoonAmmo / 3; ++i ) {
			Inv_AddItem(ID_HARPOON_AMMO_ITEM);
		}
		Lara.harpoon_ammo = 0;
	}

	// Flares
	for( i = 0; i < start->flares; ++i ) {
		Inv_AddItem(ID_FLARE_ITEM);
	}

	// Small medipacks
	for( i = 0; i < start->smallMedipacks; ++i ) {
		Inv_AddItem(ID_SMALL_MEDIPACK_ITEM);
	}

	// Large medipacks
	for( i = 0; i < start->largeMedipacks; ++i ) {
		Inv_AddItem(ID_LARGE_MEDIPACK_ITEM);
	}

	Lara.gun_status = LGS_Armless;
	Lara.last_gun_type = start->gunType;
	Lara.gun_type = start->gunType;
	Lara.request_gun_type = start->gunType;
	LaraInitialiseMeshes(levelID);
	InitialiseNewWeapon();
}

/*
 * Inject function
 */
void Inject_LaraMisc() {
	INJECT(0x00430380, LaraControl);

//	INJECT(0x00430A10, AnimateLara);

	INJECT(0x00430D10, UseItem);
	INJECT(0x00430ED0, LaraCheatGetStuff);
	INJECT(0x00430F90, ControlLaraExtra);
	INJECT(0x00430FB0, InitialiseLaraLoad);

//	INJECT(0x00430FE0, InitialiseLara);

	INJECT(0x004312A0, InitialiseLaraInventory);

//	INJECT(0x00431610, LaraInitialiseMeshes);
}
