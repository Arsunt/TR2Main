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
#include "game/traps.h"
#include "3dsystem/phd_math.h"
#include "game/collide.h"
#include "game/control.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/missile.h"
#include "game/sound.h"
#include "game/sphere.h"
#include "specific/game.h"
#include "specific/init.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

void __cdecl MineControl(__int16 mineID) {
	ITEM_INFO *mine = &Items[mineID];
	if( CHK_ANY(mine->flags, IFL_INVISIBLE) ) {
		return;
	}

	if( !MinesDetonated ) {
		__int16 roomNumber = mine->roomNumber;
		GetFloor(mine->pos.x, mine->pos.y - 0x800, mine->pos.z, &roomNumber);

		ITEM_INFO *item = NULL;
		__int16 itemID = RoomInfo[roomNumber].itemNumber;
		for( ; itemID >= 0; itemID = item->nextItem ) {
			item = &Items[itemID];
			if( item->objectID == ID_BOAT ) {
				int x = item->pos.x - mine->pos.x;
				int y = item->pos.z - mine->pos.z;
				if( SQR(x) + SQR(y) < SQR(0x200) ) {
					break;
				}
			}
		}

		if( itemID < 0 ) {
			return;
		}

		if( Lara.skidoo == itemID ) {
			ExplodingDeath(Lara.item_number, ~0, 0);
			LaraItem->hitPoints = 0;
			LaraItem->flags |= IFL_INVISIBLE;
		}
		item->objectID = ID_BOAT_BITS;
		ExplodingDeath(itemID, ~0, 0);
		KillItem(itemID);
		item->objectID = ID_BOAT;

		FLOOR_INFO *floor = GetFloor(mine->pos.x, mine->pos.y, mine->pos.z, &roomNumber);
		GetHeight(floor, mine->pos.x, mine->pos.y, mine->pos.z);
		TestTriggers(TriggerPtr, 1);
		MinesDetonated = 1;
	} else if ( GetRandomControl() < 0x7800 ) {
		return;
	}

	__int16 fxID = CreateEffect(mine->roomNumber);
	if ( fxID != -1 )
	{
		FX_INFO *fx = &Effects[fxID];
		fx->pos.x = mine->pos.x;
		fx->pos.y = mine->pos.y - 0x400;
		fx->pos.z = mine->pos.z;
		fx->speed = 0;
		fx->frame_number = 0;
		fx->counter = 0;
		fx->object_number = ID_EXPLOSION;
	}

	Splash(mine);
	PlaySoundEffect(105, &mine->pos, 0);
	mine->flags |= IFL_INVISIBLE;
	mine->collidable = 0;
	mine->meshBits = 1;
#ifdef FEATURE_INPUT_IMPROVED
	JoyRumbleExplode(mine->pos.x, mine->pos.y, mine->pos.z, 0x2800, false);
#endif // FEATURE_INPUT_IMPROVED
}

void __cdecl ControlSpikeWall(__int16 itemID) {
	ITEM_INFO *item;
	int x, z;
	__int16 roomID;

	item = &Items[itemID];
	if (TriggerActive(item) && item->status != ITEM_DISABLED) {
		z = item->pos.z + (16 * phd_cos(item->pos.rotY) >> W2V_SHIFT);
		x = item->pos.x + (16 * phd_sin(item->pos.rotY) >> W2V_SHIFT);
		roomID = item->roomNumber;
		if (GetHeight(GetFloor(x, item->pos.y, z, &roomID), x, item->pos.y, z) != item->pos.y) {
			item->status = ITEM_DISABLED;
		} else {
			item->pos.z = z;
			item->pos.x = x;
			if (roomID != item->roomNumber)
				ItemNewRoom(itemID, roomID);
		}
		PlaySoundEffect(204, &item->pos, 0);
	}
	if (item->touchBits) {
		LaraItem->hitPoints -= 20;
		LaraItem->hit_status = 1;
		DoLotsOfBlood(LaraItem->pos.x, LaraItem->pos.y - 512, LaraItem->pos.z, 1, item->pos.rotY, LaraItem->roomNumber, 3);
		item->touchBits = 0;
		PlaySoundEffect(205, &item->pos, 0);
	}
}

void __cdecl ControlCeilingSpikes(__int16 itemID) {
	ITEM_INFO *item;
	int y;
	__int16 roomID;

	item = &Items[itemID];
	if (TriggerActive(item) && item->status != ITEM_DISABLED) {
		y = item->pos.y + 5;
		roomID = item->roomNumber;
		if (GetHeight(GetFloor(item->pos.x, y, item->pos.z, &roomID), item->pos.x, y, item->pos.z) < y + 1024) {
			item->status = ITEM_DISABLED;
		} else {
			item->pos.y = y;
			if (roomID != item->roomNumber)
				ItemNewRoom(itemID, roomID);
		}
		PlaySoundEffect(204, &item->pos, 0);
	}
	if (item->touchBits) {
		LaraItem->hitPoints -= 20;
		LaraItem->hit_status = 1;
		DoLotsOfBlood(LaraItem->pos.x, item->pos.y + 768, LaraItem->pos.z, 1, item->pos.rotY, LaraItem->roomNumber, 3);
		item->touchBits = 0;
		PlaySoundEffect(205, &item->pos, 0);
	}
}

void __cdecl HookControl(__int16 itemID) {
	ITEM_INFO *item;
	static BOOL IsHookHit = FALSE;

	item = &Items[itemID];
	if (item->touchBits && !IsHookHit) {
		LaraItem->hitPoints -= 50;
		LaraItem->hit_status = 1;
		DoLotsOfBlood(LaraItem->pos.x, LaraItem->pos.y - 512, LaraItem->pos.z, LaraItem->speed, LaraItem->pos.rotY, LaraItem->roomNumber, 3);
	} else {
		IsHookHit = FALSE;
	}
	AnimateItem(item);
}

void __cdecl SpinningBlade(__int16 itemID) {
	ITEM_INFO *item;
	int x, z;
	__int16 roomID;
	BOOL reverse;

	item = &Items[itemID];
	if (item->currentAnimState == 2) {
		if (item->goalAnimState != 1) {
			z = item->pos.z + (1536 * phd_cos(item->pos.rotY) >> W2V_SHIFT);
			x = item->pos.x + (1536 * phd_sin(item->pos.rotY) >> W2V_SHIFT);
			roomID = item->roomNumber;
			if (GetHeight(GetFloor(x, item->pos.y, z, &roomID), x, item->pos.y, z) == NO_HEIGHT)
				item->goalAnimState = 1;
		}
		reverse = TRUE;
		if (item->touchBits) {
			LaraItem->hit_status = 1;
			LaraItem->hitPoints -= 100;
			DoLotsOfBlood(LaraItem->pos.x, LaraItem->pos.y - 512, LaraItem->pos.z, 2 * item->speed, LaraItem->pos.rotY, LaraItem->roomNumber, 2);
		}
		PlaySoundEffect(231, &item->pos, 0);
	} else {
		if (TriggerActive(item))
			item->goalAnimState = 2;
		reverse = FALSE;
	}
	AnimateItem(item);
	roomID = item->roomNumber;
	item->pos.y = GetHeight(GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomID), item->pos.x, item->pos.y, item->pos.z);
	item->floor = item->pos.y;
	if (roomID != item->roomNumber)
		ItemNewRoom(itemID, roomID);
	if (reverse && item->currentAnimState == 1)
		item->pos.rotY += PHD_180;
}

void __cdecl IcicleControl(__int16 itemID) {
	ITEM_INFO *item;
	__int16 roomID;
	FLOOR_INFO *floor;

	item = &Items[itemID];
	switch (item->currentAnimState) {
		case 1:
			item->goalAnimState = 2;
			break;
		case 2:
			if (!item->gravity) {
				item->fallSpeed = 50;
				item->gravity = 1;
			}
			if (item->touchBits) {
				LaraItem->hitPoints -= 200;
				LaraItem->hit_status = 1;
			}
			break;
		case 3:
			item->gravity = 0;
			break;
	}
	AnimateItem(item);
	if (item->status == ITEM_DISABLED) {
		RemoveActiveItem(itemID);
	} else {
		roomID = item->roomNumber;
		floor = GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomID);
		if (item->roomNumber != roomID)
			ItemNewRoom(itemID, roomID);
		item->floor = GetHeight(floor, item->pos.x, item->pos.y, item->pos.z);
		if (item->currentAnimState == 2 && item->pos.y >= item->floor) {
			item->gravity = 0;
			item->goalAnimState = 3;
			item->pos.y = item->floor;
			item->fallSpeed = 0;
			item->meshBits = 0x2B;
		}
	}
}

void __cdecl InitialiseBlade(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	item->animNumber = Objects[ID_BLADE].animIndex + 2;
	item->currentAnimState = 1;
	item->frameNumber = Anims[item->animNumber].frameBase;
}

void __cdecl BladeControl(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (TriggerActive(item) && item->currentAnimState == 1) {
		item->goalAnimState = 2;
	} else {
		item->goalAnimState = 1;
	}
	if (CHK_ANY(item->touchBits, 2) && item->currentAnimState == 2) {
		LaraItem->hit_status = 1;
		LaraItem->hitPoints -= 100;
		DoLotsOfBlood(LaraItem->pos.x, item->pos.y - 256, LaraItem->pos.z, LaraItem->speed, LaraItem->pos.rotY, LaraItem->roomNumber, 2);
	}
	AnimateItem(item);
}

void __cdecl InitialiseKillerStatue(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	item->animNumber = Objects[item->objectID].animIndex + 3;
	item->currentAnimState = 1;
	item->frameNumber = Anims[item->animNumber].frameBase;
}

void __cdecl KillerStatueControl(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (TriggerActive(item) && item->currentAnimState == 1) {
		item->goalAnimState = 2;
	} else {
		item->goalAnimState = 1;
	}
	if (CHK_ANY(item->touchBits, 0x80) && item->currentAnimState == 2) {
		LaraItem->hit_status = 1;
		LaraItem->hitPoints -= 20;
		DoBloodSplat(LaraItem->pos.x + (GetRandomControl() - 16384) / 256,
					LaraItem->pos.y - GetRandomControl() / 44,
					LaraItem->pos.z + (GetRandomControl() - 16384) / 256,
					LaraItem->speed,
					LaraItem->pos.rotY + (GetRandomControl() - 16384) / 8,
					LaraItem->roomNumber);
	}
	AnimateItem(item);
}

void __cdecl SpringBoardControl(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (!item->currentAnimState &&
		item->pos.y == LaraItem->pos.y &&
		!CHK_ANY(item->pos.x ^ LaraItem->pos.x, 0xFFFFFC00) &&
		!CHK_ANY(item->pos.z ^ LaraItem->pos.z, 0xFFFFFC00))
	{
		if (LaraItem->hitPoints > 0) {
			if (LaraItem->currentAnimState == AS_BACK || LaraItem->currentAnimState == AS_FASTBACK)
				LaraItem->speed = -LaraItem->speed;
			LaraItem->fallSpeed = -240;
			LaraItem->gravity = 1;
			LaraItem->animNumber = 34;
			LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = AS_FORWARDJUMP;
			LaraItem->goalAnimState = AS_FORWARDJUMP;
			item->goalAnimState = 1;
			AnimateItem(item);
		}
	} else {
		AnimateItem(item);
	}
}

void __cdecl InitialiseRollingBall(__int16 itemID) {
	ITEM_INFO *item;
	GAME_VECTOR *pos;

	item = &Items[itemID];
	item->data = game_malloc(sizeof(GAME_VECTOR), GBUF_RollingBallStuff);
	pos = (GAME_VECTOR *) item->data;
	pos->x = item->pos.x;
	pos->y = item->pos.y;
	pos->z = item->pos.z;
	pos->roomNumber = item->roomNumber;
}

void __cdecl RollingBallControl(__int16 itemID) {
	ITEM_INFO *item;
	int oldX, oldZ, distance, x, z;
	__int16 roomID;
	FLOOR_INFO *floor;
	GAME_VECTOR *pos;

	item = &Items[itemID];
	if (item->status == ITEM_ACTIVE) {
		if (item->goalAnimState == 2) {
			AnimateItem(item);
		} else {
			if (item->pos.y < item->floor) {
				if (!item->gravity) {
					item->fallSpeed = -10;
					item->gravity = 1;
				}
			} else {
				if (!item->currentAnimState)
					item->goalAnimState = 1;
			}
			oldX = item->pos.x;
			oldZ = item->pos.z;
			AnimateItem(item);
			roomID = item->roomNumber;
			floor = GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomID);
			if (item->roomNumber != roomID)
				ItemNewRoom(itemID, roomID);
			item->floor = GetHeight(floor, item->pos.x, item->pos.y, item->pos.z);
			TestTriggers(TriggerPtr, TRUE);
			if (item->pos.y >= item->floor - 256) {
				item->gravity = 0;
				item->pos.y = item->floor;
				item->fallSpeed = 0;
				if (item->objectID == ID_ROLLING_BALL2) {
					PlaySoundEffect(222, &item->pos, 0);
				} else {
					if (item->objectID == ID_ROLLING_BALL3) {
						PlaySoundEffect(227, &item->pos, 0);
					} else {
						PlaySoundEffect(147, &item->pos, 0);
					}
				}
				distance = phd_sqrt(SQR(Camera.micPos.x - item->pos.x) + SQR(Camera.micPos.z - item->pos.z));
				if (distance < 10240)
					Camera.bounce = 40 * (distance - 10240) / 10240;
			}
			distance = item->objectID == ID_ROLLING_BALL1 ? 384 : 1024;
			x = item->pos.x + (distance * phd_sin(item->pos.rotY) >> W2V_SHIFT);
			z = item->pos.z + (distance * phd_cos(item->pos.rotY) >> W2V_SHIFT);
			if (GetHeight(GetFloor(x, item->pos.y, z, &roomID), x, item->pos.y, z) < item->pos.y) {
				if (item->objectID == ID_ROLLING_BALL2) {
					PlaySoundEffect(223, &item->pos, 0);
					item->goalAnimState = 2;
				} else {
					if (item->objectID == ID_ROLLING_BALL3) {
						PlaySoundEffect(228, &item->pos, 0);
						item->goalAnimState = 2;
					} else {
						item->status = ITEM_DISABLED;
					}
				}
				item->pos.y = item->floor;
				item->pos.x = oldX;
				item->pos.z = oldZ;
				item->fallSpeed = 0;
				item->speed = 0;
				item->touchBits = 0;
			}
		}
	} else {
		if (item->status == ITEM_DISABLED && !TriggerActive(item)) {
			pos = (GAME_VECTOR *) item->data;
			item->status = ITEM_INACTIVE;
			item->pos.x = pos->x;
			item->pos.y = pos->y;
			item->pos.z = pos->z;
			if (item->roomNumber != pos->roomNumber) {
				RemoveDrawnItem(itemID);
				item->nextItem = RoomInfo[pos->roomNumber].itemNumber;
				RoomInfo[pos->roomNumber].itemNumber = itemID;
				item->roomNumber = pos->roomNumber;
			}
			item->animNumber = Objects[item->objectID].animIndex;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->requiredAnimState = 0;
			item->goalAnimState = Anims[item->animNumber].currentAnimState;
			item->currentAnimState = Anims[item->animNumber].currentAnimState;
			RemoveActiveItem(itemID);
		}
	}
}

void __cdecl RollingBallCollision(__int16 itemID, ITEM_INFO *laraItem, COLL_INFO *coll) {
	ITEM_INFO *item;
	int dx, dy, dz, distance, i;

	item = &Items[itemID];
	if (item->status == ITEM_ACTIVE) {
		if (TestBoundsCollide(item, laraItem, coll->radius) && TestCollision(item, laraItem)) {
			if (laraItem->gravity) {
				if (CHK_ANY(coll->flags, 8))
					ItemPushLara(item, laraItem, coll, CHK_ANY(coll->flags, 0x10), 1);
				laraItem->hitPoints -= 100;
				dx = laraItem->pos.x - item->pos.x;
				dy = laraItem->pos.y - item->pos.y + 162;
				dz = laraItem->pos.z - item->pos.z;
				distance = phd_sqrt(SQR(dx) + SQR(dy) + SQR(dz));
				if (distance < 512)
					distance = 512;
				DoBloodSplat(item->pos.x + (dx << WALL_SHIFT) / 2 / distance,
							item->pos.y + (dy << WALL_SHIFT) / 2 / distance - 512,
							item->pos.z + (dz << WALL_SHIFT) / 2 / distance,
							item->speed,
							item->pos.rotY,
							item->roomNumber);
			} else {
				laraItem->hit_status = 1;
				if (laraItem->hitPoints > 0) {
					laraItem->hitPoints = -1;
					laraItem->pos.rotY = item->pos.rotY;
					laraItem->pos.rotZ = 0;
					laraItem->pos.rotX = 0;
					laraItem->animNumber = 139;
					laraItem->frameNumber = Anims[laraItem->animNumber].frameBase;
					laraItem->currentAnimState = AS_SPECIAL;
					laraItem->goalAnimState = AS_SPECIAL;
					Camera.flags = CFL_FollowCenter;
					Camera.targetAngle = 170 * PHD_DEGREE;
					Camera.targetElevation = -25 * PHD_DEGREE;
					for (i = 0; i < 15; ++i)
						DoBloodSplat(laraItem->pos.x + (GetRandomControl() - 16384) / 256,
									laraItem->pos.y - GetRandomControl() / 64,
									laraItem->pos.z + (GetRandomControl() - 16384) / 256,
									2 * item->speed,
									item->pos.rotY + (GetRandomControl() - 16384) / 8,
									item->roomNumber);
				}
			}
		}
	} else {
		if (item->status != ITEM_INVISIBLE)
			ObjectCollision(itemID, laraItem, coll);
	}
}

void __cdecl Pendulum(__int16 itemID) {
	ITEM_INFO *item;

	item = &Items[itemID];
	if (item->touchBits) {
		LaraItem->hitPoints -= 50;
		LaraItem->hit_status = 1;
		DoBloodSplat(LaraItem->pos.x + (GetRandomControl() - 16384) / 256,
					LaraItem->pos.y - GetRandomControl() / 44,
					LaraItem->pos.z + (GetRandomControl() - 16384) / 256,
					LaraItem->speed,
					LaraItem->pos.rotY + (GetRandomControl() - 16384) / 8,
					LaraItem->roomNumber);
	}
	item->floor = GetHeight(GetFloor(item->pos.x, item->pos.y, item->pos.z, &item->roomNumber), item->pos.x, item->pos.y, item->pos.z);
	AnimateItem(item);
}

void __cdecl TeethTrap(__int16 itemID) {
	ITEM_INFO *item;
	static BITE_INFO Teeth[3][2] = {
		{{-23, 0, -1718, 0}, {71, 0, -1718, 1}},
		{{-23, 10, -1718, 0}, {71, 10, -1718, 1}},
		{{-23, -10, -1718, 0}, {71, -10, -1718, 1}}
	};

	item = &Items[itemID];
	if (TriggerActive(item)) {
		item->goalAnimState = 1;
		if (item->touchBits && item->currentAnimState == 1) {
			LaraItem->hitPoints -= 400;
			LaraItem->hit_status = 1;
			BaddieBiteEffect(item, &Teeth[0][0]);
			BaddieBiteEffect(item, &Teeth[0][1]);
			BaddieBiteEffect(item, &Teeth[1][0]);
			BaddieBiteEffect(item, &Teeth[1][1]);
			BaddieBiteEffect(item, &Teeth[2][0]);
			BaddieBiteEffect(item, &Teeth[2][1]);
		}
	} else {
		item->goalAnimState = 0;
	}
	AnimateItem(item);
}

void __cdecl FallingCeiling(__int16 itemID) {
	ITEM_INFO *item;
	__int16 roomID;

	item = &Items[itemID];
	if (!item->currentAnimState) {
		item->gravity = 1;
		item->goalAnimState = 1;
	} else {
		if (item->currentAnimState == 1 && item->touchBits) {
			LaraItem->hitPoints -= 300;
			LaraItem->hit_status = 1;
		}
	}
	AnimateItem(item);
	if (item->status == ITEM_DISABLED) {
		RemoveActiveItem(itemID);
	} else {
		roomID = item->roomNumber;
		item->floor = GetHeight(GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomID), item->pos.x, item->pos.y, item->pos.z);
		if (roomID != item->roomNumber)
			ItemNewRoom(itemID, roomID);
		if (item->currentAnimState == 1 && item->pos.y >= item->floor) {
			item->gravity = 0;
			item->goalAnimState = 2;
			item->pos.y = item->floor;
			item->fallSpeed = 0;
		}
	}
}

void __cdecl DartEmitterControl(__int16 itemID) {
	ITEM_INFO *item, *dynamic;
	__int16 dynamicID;
	int dx, dz;

	item = &Items[itemID];
	if (TriggerActive(item)) {
		if (!item->currentAnimState)
			item->goalAnimState = 1;
	} else {
		if (item->currentAnimState == 1)
			item->goalAnimState = 0;
	}
	if (item->currentAnimState == 1 && item->frameNumber == Anims[item->animNumber].frameBase) {
		dynamicID = CreateItem();
		if (dynamicID != -1) {
			dynamic = &Items[dynamicID];
			dynamic->objectID = ID_DARTS;
			dynamic->roomNumber = item->roomNumber;
			dynamic->shade1 = -1;
			dynamic->pos.rotY = item->pos.rotY;
			dynamic->pos.y = item->pos.y - 512;
			dz = 0;
			dx = 0;
			if (dynamic->pos.rotY <= -PHD_90) {
				if (dynamic->pos.rotY == -PHD_90) {
					dx = 412;
				} else {
					if (dynamic->pos.rotY == -PHD_180)
						dz = 412;
				}
			} else {
				if (!dynamic->pos.rotY) {
					dz = -412;
				} else {
					if (dynamic->pos.rotY == PHD_90)
						dx = -412;
				}
			}
			dynamic->pos.x = item->pos.x + dx;
			dynamic->pos.z = item->pos.z + dz;
			InitialiseItem(dynamicID);
			AddActiveItem(dynamicID);
			dynamic->status = ITEM_ACTIVE;
			PlaySoundEffect(254, &dynamic->pos, 0);
		}
	}
	AnimateItem(item);
}

void __cdecl DartsControl(__int16 itemID) {
	ITEM_INFO *item;
	__int16 roomID, fxID;
	FLOOR_INFO *floor;
	FX_INFO *fx;

	item = &Items[itemID];
	if (item->touchBits) {
		LaraItem->hitPoints -= 50;
		LaraItem->hit_status = 1;
		DoBloodSplat(item->pos.x, item->pos.y, item->pos.z, LaraItem->speed, LaraItem->pos.rotY, LaraItem->roomNumber);
	}
	AnimateItem(item);
	roomID = item->roomNumber;
	floor = GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomID);
	if (item->roomNumber != roomID)
		ItemNewRoom(itemID, roomID);
	item->floor = GetHeight(floor, item->pos.x, item->pos.y, item->pos.z);
	item->pos.rotX += PHD_45 / 2;
	if (item->pos.y >= item->floor) {
		KillItem(itemID);
		fxID = CreateEffect(item->roomNumber);
		if (fxID != -1) {
			fx = &Effects[fxID];
			fx->pos = item->pos;
			fx->speed = 0;
			fx->counter = 6;
			fx->object_number = ID_RICOCHET;
			fx->frame_number = -3 * GetRandomControl() / 32768;
		}
		PlaySoundEffect(258, &item->pos, 0);
	}
}

void __cdecl DartEffectControl(__int16 fxID) {
	FX_INFO *fx;

	fx = &Effects[fxID];
	++fx->counter;
	if (fx->counter >= 3) {
		--fx->frame_number;
		fx->counter = 0;
		if (fx->frame_number <= Objects[fx->object_number].nMeshes)
			KillEffect(fxID);
	}
}

void __cdecl FlameEmitterControl(__int16 item_id) {
	ITEM_INFO *item;
	__int16 fxID;
	FX_INFO *fx;

	item = &Items[item_id];
	if (TriggerActive(item)) {
		if (!item->data) {
			fxID = CreateEffect(item->roomNumber);
			if (fxID != -1) {
				fx = &Effects[fxID];
				fx->pos.x = item->pos.x;
				fx->pos.y = item->pos.y;
				fx->pos.z = item->pos.z;
				fx->frame_number = 0;
				fx->object_number = ID_FLAME;
				fx->counter = 0;
			}
			item->data = (LPVOID) (fxID + 1);
		}
	} else {
		if (item->data) {
			KillEffect((int) item->data - 1);
			item->data = (LPVOID) 0;
		}
	}
}

void __cdecl FlameControl(__int16 fx_id) {
	FX_INFO *fx = &Effects[fx_id];
	if( --fx->frame_number <= Objects[ID_FLAME].nMeshes ) {
		fx->frame_number = 0;
	}
	if( fx->counter < 0 ) {
#ifdef FEATURE_CHEAT
		if( Lara.water_status == LWS_Cheat ) {
			fx->counter = 0;
			KillEffect(fx_id);
			Lara.burn = 0;
			return;
		}
#endif // FEATURE_CHEAT
		fx->pos.x = 0;
		fx->pos.y = 0;
		fx->pos.z = ( fx->counter == -1 ) ? -100 : 0;
		GetJointAbsPosition(LaraItem, (PHD_VECTOR *)&fx->pos, -1 - fx->counter);
		if( LaraItem->roomNumber != fx->room_number ) {
			EffectNewRoom(fx_id, LaraItem->roomNumber);
		}
		int height = GetWaterHeight(fx->pos.x, fx->pos.y, fx->pos.z, fx->room_number);
		if( height != NO_HEIGHT && fx->pos.y > height ) {
			fx->counter = 0;
			KillEffect(fx_id);
			Lara.burn = 0;
		} else {
			PlaySoundEffect(150, &fx->pos, 0);
			LaraItem->hitPoints -= 7;
			LaraItem->hit_status = 1;
		}
	} else {
		PlaySoundEffect(150, &fx->pos, 0);
		if( fx->counter ) {
			--fx->counter;
		} else if( ItemNearLara(&fx->pos, 600) ) {
			LaraItem->hitPoints -= 5;
			LaraItem->hit_status = 1;
			int dx = LaraItem->pos.x - fx->pos.x;
			int dz = LaraItem->pos.z - fx->pos.z;
			if( SQR(dx) + SQR(dz) < SQR(450) ) {
				fx->counter = 100;
				LaraBurn();
			}
		}
	}
}

void __cdecl LaraBurn() {
#ifdef FEATURE_CHEAT
	if( Lara.water_status == LWS_Cheat ) {
		return;
	}
#endif // FEATURE_CHEAT
	if( Lara.burn ) {
		return;
	}

	__int16 fx_id = CreateEffect(LaraItem->roomNumber);
	if( fx_id < 0 ) {
		return;
	}
	FX_INFO *fx = &Effects[fx_id];
	fx->object_number = ID_FLAME;
	fx->frame_number = 0;
	fx->counter = -1;
	Lara.burn = 1;
}

void __cdecl LavaBurn(ITEM_INFO *item) {
#ifdef FEATURE_CHEAT
	if( Lara.water_status == LWS_Cheat ) {
		return;
	}
#endif // FEATURE_CHEAT
	if( item->hitPoints < 0 ) {
		return;
	}

	__int16 room_number = item->roomNumber;
	FLOOR_INFO *floor = GetFloor(item->pos.x, 32000, item->pos.z, &room_number);
	if( item->floor != GetHeight(floor, item->pos.x, 32000, item->pos.z) ) {
		return;
	}

	item->hit_status = 1;
	item->hitPoints = -1;
	for( int i = 0; i < 10; ++i ) {
		__int16 fx_id = CreateEffect(item->roomNumber);
		if( fx_id < 0 ) continue;
		FX_INFO *fx = &Effects[fx_id];
		fx->object_number = ID_FLAME;
		fx->frame_number = Objects[ID_FLAME].nMeshes * GetRandomControl() / 0x7FFF;
		fx->counter = -1 - 24 * GetRandomControl() / 0x7FFF;
	}
}

/*
 * Inject function
 */
void Inject_Traps() {
	INJECT(0x00440FC0, MineControl);
	INJECT(0x004411C0, ControlSpikeWall);
	INJECT(0x00441300, ControlCeilingSpikes);
	INJECT(0x00441420, HookControl);

//	INJECT(0x004414B0, PropellerControl);

	INJECT(0x00441640, SpinningBlade);
	INJECT(0x004417C0, IcicleControl);
	INJECT(0x004418C0, InitialiseBlade);
	INJECT(0x00441900, BladeControl);
	INJECT(0x004419A0, InitialiseKillerStatue);
	INJECT(0x004419F0, KillerStatueControl);
	INJECT(0x00441B00, SpringBoardControl);
	INJECT(0x00441BE0, InitialiseRollingBall);
	INJECT(0x00441C20, RollingBallControl);
	INJECT(0x00441F70, RollingBallCollision);

//	INJECT(0x004421C0, SpikeCollision);
//	INJECT(0x00442320, TrapDoorControl);
//	INJECT(0x00442370, TrapDoorFloor);
//	INJECT(0x004423B0, TrapDoorCeiling);
//	INJECT(0x004423F0, OnTrapDoor);

	INJECT(0x004424A0, Pendulum);

//	INJECT(0x004425B0, FallingBlock);
//	INJECT(0x004426C0, FallingBlockFloor);
//	INJECT(0x00442700, FallingBlockCeiling);

	INJECT(0x00442750, TeethTrap);
	INJECT(0x00442810, FallingCeiling);
	INJECT(0x004428F0, DartEmitterControl);
	INJECT(0x00442A30, DartsControl);
	INJECT(0x00442B90, DartEffectControl);
	INJECT(0x00442BE0, FlameEmitterControl);
	INJECT(0x00442C70, FlameControl);
	INJECT(0x00442DE0, LaraBurn);
	INJECT(0x00442E30, LavaBurn);

//	INJECT(0x00442F20, LavaSpray);
//	INJECT(0x00442FF0, ControlLavaBlob);
}
