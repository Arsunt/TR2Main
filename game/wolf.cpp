/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
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

#include "global/precompiled.h"
#include "game/wolf.h"
#include "game/box.h"
#include "game/effects.h"
#include "game/lot.h"
#include "specific/game.h"
#include "global/vars.h"

#define WOLF_TOUCH (0x774F)

typedef enum {
	WOLF_EMPTY,
	WOLF_STOP,
	WOLF_WALK,
	WOLF_RUN,
	WOLF_JUMP,
	WOLF_STALK,
	WOLF_ATTACK,
	WOLF_HOWL,
	WOLF_SLEEP,
	WOLF_CROUCH,
	WOLF_FASTTURN,
	WOLF_DEATH,
	WOLF_BITE
} WOLF_ANIMS;

static const BITE_INFO WolfBite = {
	0, -14, 174, 6
};

void __cdecl InitialiseWolf(__int16 itemID) {
	Items[itemID].frameNumber = 96; // lying wolf frame (hardcoded thing)
	InitialiseCreature(itemID);
}

void __cdecl WolfControl(__int16 itemID) {
	ITEM_INFO *item = &Items[itemID];

	if( item->status == ITEM_INVISIBLE ) {
		if( !EnableBaddieAI(itemID, FALSE) ) {
			return;
		}
		item->status = ITEM_ACTIVE;
	}

	CREATURE_INFO *wolf = (CREATURE_INFO *)item->data;
	if( wolf == NULL ) return; // NOTE: additional check not presented in the original game

	__int16 angle = 0;
	__int16 head = 0;
	__int16 tilt = 0;

	if( item->hitPoints <= 0 ) {
		if( item->currentAnimState != WOLF_DEATH ) {
			// some hardcoded death animation start
			item->animNumber = Objects[ID_SPIDER_or_WOLF].animIndex + GetRandomControl() / 11000 + 20;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = WOLF_DEATH;
		}
	} else {
		AI_INFO info;

		CreatureAIInfo(item, &info);
		if( info.ahead ) {
			head = info.angle;
		}
		CreatureMood(item, &info, FALSE);
		angle = CreatureTurn(item, wolf->maximum_turn);

		switch ( item->currentAnimState ) {
			case WOLF_SLEEP :
				head = 0;
				if( wolf->mood == MOOD_ESCAPE || info.zone_number == info.enemy_zone ) {
					item->requiredAnimState = WOLF_CROUCH;
					item->goalAnimState = WOLF_STOP;
				} else if( GetRandomControl() < 0x20 ) {
					item->requiredAnimState = WOLF_WALK;
					item->goalAnimState = WOLF_STOP;
				}
				break;

			case WOLF_STOP :
				if( item->requiredAnimState != WOLF_EMPTY ) {
					item->goalAnimState = item->requiredAnimState;
				} else {
					item->goalAnimState = WOLF_WALK;
				}
				break;

			case WOLF_WALK :
				wolf->maximum_turn = 2*PHD_DEGREE;
				if( wolf->mood != MOOD_BORED ) {
					item->goalAnimState = WOLF_STALK;
					item->requiredAnimState = WOLF_EMPTY;
				} else if( GetRandomControl() < 0x20 ) {
					item->requiredAnimState = WOLF_SLEEP;
					item->goalAnimState = WOLF_STOP;
				}
				break;

			case WOLF_CROUCH :
				if( item->requiredAnimState != WOLF_EMPTY ) {
					item->goalAnimState = item->requiredAnimState;
				} else if( wolf->mood == MOOD_ESCAPE ) {
					item->goalAnimState = WOLF_RUN;
				} else if( info.distance < 0x1D0F1 && info.bite ) {
					item->goalAnimState = WOLF_BITE;
				} else if( wolf->mood == MOOD_STALK ) {
					item->goalAnimState = WOLF_STALK;
				} else if( wolf->mood == MOOD_BORED ) {
					item->goalAnimState = WOLF_STOP;
				} else {
					item->goalAnimState = WOLF_RUN;
				}
				break;

			case WOLF_STALK :
				wolf->maximum_turn = 2*PHD_DEGREE;
				if( wolf->mood == MOOD_ESCAPE ) {
					item->goalAnimState = WOLF_RUN;
				} else if( info.distance < 0x1D0F1 && info.bite ) {
					item->goalAnimState = WOLF_BITE;
				} else if( info.distance > 0x900000 ) {
					item->goalAnimState = WOLF_RUN;
				} else if( wolf->mood == MOOD_ATTACK ) {
					if( !info.ahead || info.distance > 0x240000 ||
						(info.enemy_facing < 0x4000 && info.enemy_facing > -0x4000) )
					{
						item->goalAnimState = WOLF_RUN;
					}
				} else if( GetRandomControl() < 0x180 ) {
					item->requiredAnimState = WOLF_HOWL;
					item->goalAnimState = WOLF_CROUCH;
				}
				else if( wolf->mood == MOOD_BORED ) {
					item->goalAnimState = WOLF_CROUCH;
				}
				break;

			case WOLF_RUN :
				wolf->maximum_turn = 5*PHD_DEGREE;
				tilt = angle;
				if( info.ahead && info.distance < 0x240000 ) {
					if( info.distance > 0x120000 &&
						(info.enemy_facing > 0x4000 || info.enemy_facing < -0x4000) )
					{
						item->requiredAnimState = WOLF_STALK;
						item->goalAnimState = WOLF_CROUCH;
					} else {
						item->goalAnimState = WOLF_ATTACK;
						item->requiredAnimState = WOLF_EMPTY;
					}
				} else if( wolf->mood == MOOD_STALK && info.distance < 0x900000 ) {
					item->requiredAnimState = WOLF_STALK;
					item->goalAnimState = WOLF_CROUCH;
				} else if( wolf->mood == MOOD_BORED ) {
					item->goalAnimState = WOLF_CROUCH;
				}
				break;

			case WOLF_ATTACK :
				tilt = angle;
				if( item->requiredAnimState == WOLF_EMPTY &&
					CHK_ANY(item->touchBits, WOLF_TOUCH) )
				{
					CreatureEffect(item, &WolfBite, DoBloodSplat);
					LaraItem->hitPoints -= 50;
					LaraItem->hit_status = 1;
					item->requiredAnimState = WOLF_RUN;
				}
				item->goalAnimState = WOLF_RUN;
				break;

			case WOLF_BITE :
				if( info.ahead &&
					item->requiredAnimState == WOLF_EMPTY &&
					CHK_ANY(item->touchBits, WOLF_TOUCH) )
				{
					CreatureEffect(item, &WolfBite, DoBloodSplat);
					LaraItem->hitPoints -= 100;
					LaraItem->hit_status = 1;
					item->requiredAnimState = WOLF_CROUCH;
				}
				break;

			default :
				break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureHead(item, head);
	CreatureAnimation(itemID, angle, tilt);
}

/*
 * NOTE: there is no inject function for tomb2gold code
 */
