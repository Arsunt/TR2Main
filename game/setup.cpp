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
#include "game/setup.h"
#include "game/bird.h"
#include "game/collide.h"
#include "game/diver.h"
#include "game/dog.h"
#include "game/dragon.h"
#include "game/draw.h"
#include "game/eel.h"
#include "game/enemies.h"
#include "game/hair.h"
#include "game/laramisc.h"
#include "game/moveblock.h"
#include "game/people.h"
#include "game/rat.h"
#include "game/shark.h"
#include "game/skidoo.h"
#include "game/spider.h"
#include "game/yeti.h"
#include "specific/winmain.h"
#include "global/vars.h"

void __cdecl InitialiseLevelFlags() {
	memset(&SaveGame.statistics, 0, sizeof(STATISTICS_INFO));
}

void __cdecl InitialiseObjects() {
	for( int i = 0; i < ID_NUMBER_OBJECTS; ++i ) {
		Objects[i].intelligent = 0;
		Objects[i].save_position = 0;
		Objects[i].save_hitpoints = 0;
		Objects[i].save_flags = 0;
		Objects[i].save_anim = 0;
		Objects[i].water_creature = 0;
		Objects[i].initialise = NULL;
		Objects[i].collision = NULL;
		Objects[i].control = NULL;
		Objects[i].drawRoutine = DrawAnimatingItem;
		Objects[i].ceiling = NULL;
		Objects[i].floor = NULL;
		Objects[i].pivotLength = 0;
		Objects[i].radius = 10;
		Objects[i].shadowSize = 0;
		Objects[i].hitPoints = 0xC000; // non targettable
	}
	BaddyObjects();
	TrapObjects();
	ObjectObjects();
	InitialiseHair();
}

void __cdecl BaddyObjects() {
	OBJECT_INFO *obj;

	// Lara object is mandatory
	obj = &Objects[ID_LARA];
	obj->initialise = InitialiseLaraLoad;
	obj->shadowSize = 160;
	obj->hitPoints = 1000;
	obj->drawRoutine = DrawDummyItem;
	obj->save_position = 1;
	obj->save_hitpoints = 1;
	obj->save_flags = 1;
	obj->save_anim = 1;
	obj->control = ControlLaraExtra;

	// Other objects are optional
	obj = &Objects[ID_DOG];
	if( obj->loaded ) {
		obj->control = DogControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 10;
		obj->pivotLength = 300;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->radius = 341;
		AnimBones[obj->boneIndex + 19*4] |= 0x08;
	}
	obj = &Objects[ID_MOUSE];
	if( obj->loaded ) {
		obj->control = MouseControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 4;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 50;
		obj->radius = 102;
		AnimBones[obj->boneIndex + 3*4] |= 0x08;
	}
	obj = &Objects[ID_CULT1];
	if( obj->loaded ) {
		obj->initialise = InitialiseCult1;
		obj->control = Cult1Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->hitPoints = 25;
		obj->pivotLength = 50;
		obj->radius = 102;
		AnimBones[obj->boneIndex + 0*4] |= 0x08;
	}
	obj = &Objects[ID_CULT1A];
	if( obj->loaded ) {
		if( !Objects[ID_CULT1].loaded ) {
			S_ExitSystem("FATAL: CULT1B requires CULT1");
		}
		obj->frameBase = Objects[ID_CULT1].frameBase;
		obj->animIndex = Objects[ID_CULT1].animIndex;
		obj->initialise = InitialiseCult1;
		obj->control = Cult1Control;
		obj->collision = CreatureCollision;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->shadowSize = 128;
		obj->hitPoints = 25;
		obj->pivotLength = 50;
		obj->radius = 102;
		AnimBones[obj->boneIndex + 0*4] |= 0x08;
	}
	obj = &Objects[ID_CULT1B];
	if( obj->loaded ) {
		if( !Objects[ID_CULT1].loaded ) {
			S_ExitSystem("FATAL: CULT1B requires CULT1");
		}
		obj->frameBase = Objects[ID_CULT1].frameBase;
		obj->animIndex = Objects[ID_CULT1].animIndex;
		obj->initialise = InitialiseCult1;
		obj->control = Cult1Control;
		obj->collision = CreatureCollision;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->shadowSize = 128;
		obj->hitPoints = 25;
		obj->pivotLength = 50;
		obj->radius = 102;
		AnimBones[obj->boneIndex + 0*4] |= 0x08;
	}
	obj = &Objects[ID_CULT2];
	if( obj->loaded ) {
		obj->control = Cult2Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 60;
		obj->pivotLength = 50;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->radius = 102;
		AnimBones[obj->boneIndex + 0*4] |= 0x08;
		AnimBones[obj->boneIndex + 8*4] |= 0x08;
	}
	obj = &Objects[ID_SHARK];
	if( obj->loaded ) {
		obj->control = SharkControl;
		obj->drawRoutine = DrawUnclippedItem;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->water_creature = 1;
		obj->pivotLength = 200;
		obj->radius = 341;
		AnimBones[obj->boneIndex + 9*4] |= 0x08;
	}
	obj = &Objects[ID_TIGER];
	if( obj->loaded ) {
		obj->control = TigerControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 20;
		obj->pivotLength = 200;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->radius = 341;
		AnimBones[obj->boneIndex + 21*4] |= 0x08;
	}
	obj = &Objects[ID_BARRACUDA];
	if( obj->loaded ) {
		obj->control = BaracuddaControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 12;
		obj->pivotLength = 200;
		obj->radius = 204;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->water_creature = 1;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
	}
	obj = &Objects[ID_SPIDER_or_WOLF];
	if( obj->loaded ) {
		obj->control = SpiderControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 5;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
	obj = &Objects[ID_BIG_SPIDER_or_BEAR];
	if( obj->loaded ) {
		obj->control = BigSpiderControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 40;
		obj->radius = 341;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
	obj = &Objects[ID_YETI];
	if( obj->loaded ) {
		obj->control = YetiControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->radius = 128;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 100;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
		AnimBones[obj->boneIndex + 14*4] |= 0x08;
	}
	obj = &Objects[ID_JELLY];
	if( obj->loaded ) {
		obj->control = JellyControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 10;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->water_creature = 1;
	}
	obj = &Objects[ID_DIVER];
	if( obj->loaded ) {
		obj->control = DiverControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 20;
		obj->radius = 341;
		obj->pivotLength = 50;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->water_creature = 1;
		AnimBones[obj->boneIndex + 10*4] |= 0x08;
		AnimBones[obj->boneIndex + 14*4] |= 0x10;
	}
	obj = &Objects[ID_WORKER1];
	if( obj->loaded ) {
		obj->control = Worker1Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 25;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 4*4] |= 0x08;
		AnimBones[obj->boneIndex + 13*4] |= 0x08;
	}
	obj = &Objects[ID_WORKER2];
	if( obj->loaded ) {
		obj->control = Worker2Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 20;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 4*4] |= 0x08;
		AnimBones[obj->boneIndex + 13*4] |= 0x08;
	}
	obj = &Objects[ID_WORKER3];
	if( obj->loaded ) {
		obj->control = Worker3Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 27;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 0*4] |= 0x08;
		AnimBones[obj->boneIndex + 4*4] |= 0x08;
	}
	obj = &Objects[ID_WORKER4];
	if( obj->loaded ) {
		obj->hitPoints = 27;
		obj->control = Worker3Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->radius = 102;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 0*4] |= 0x08;
		AnimBones[obj->boneIndex + 4*4] |= 0x08;
	}
	obj = &Objects[ID_WORKER5];
	if( obj->loaded ) {
		obj->control = Worker2Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 20;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 4*4] |= 0x08;
		AnimBones[obj->boneIndex + 13*4] |= 0x08;
	}
	obj = &Objects[ID_CULT3];
	if( obj->loaded ) {
		obj->initialise = InitialiseCult3;
		obj->control = Cult3Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 150;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
	}
	obj = &Objects[ID_MONK1];
	if( obj->loaded ) {
		obj->control = MonkControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
	}
	obj = &Objects[ID_MONK2];
	if( obj->loaded ) {
		obj->control = MonkControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
	}
	obj = &Objects[ID_EAGLE];
	if( obj->loaded ) {
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 20;
		obj->radius = 204;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
	}
	obj = &Objects[ID_CROW];
	if( obj->loaded ) {
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 15;
		obj->radius = 204;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
	}
	obj = &Objects[ID_BIG_EEL];
	if( obj->loaded ) {
		obj->control = BigEelControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 20;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->water_creature = 1;
	}
	obj = &Objects[ID_EEL];
	if( obj->loaded ) {
		obj->control = EelControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 5;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->water_creature = 1;
	}
	obj = &Objects[ID_BANDIT1];
	if( obj->loaded ) {
		obj->control = BanditControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 45;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
		AnimBones[obj->boneIndex + 8*4] |= 0x08;
	}
	obj = &Objects[ID_BANDIT2];
	if( obj->loaded ) {
		obj->control = Bandit2Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 50;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
		AnimBones[obj->boneIndex + 8*4] |= 0x08;
	}
	obj = &Objects[ID_BANDIT2B];
	if( obj->loaded ) {
		if( !Objects[ID_BANDIT2].loaded ) {
			S_ExitSystem("FATAL: BANDIT2B requires BANDIT2");
		}
		obj->frameBase = Objects[ID_BANDIT2].frameBase;
		obj->animIndex = Objects[ID_BANDIT2].animIndex;
		obj->control = Bandit2Control;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->hitPoints = 50;
		obj->radius = 102;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
		AnimBones[obj->boneIndex + 8*4] |= 0x08;
	}
	obj = &Objects[ID_SKIDOO_ARMED];
	if( obj->loaded ) {
		obj->drawRoutine = DrawSkidoo;
		obj->collision = SkidmanCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 100;
		obj->radius = 341;
		obj->pivotLength = 0;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
	obj = &Objects[ID_SKIDMAN];
	if( obj->loaded ) {
		obj->initialise = InitialiseSkidman;
		obj->control = SkidManControl;
		obj->hitPoints = 1;
		obj->save_position = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
	obj = &Objects[ID_XIAN_LORD];
	if( obj->loaded ) {
		if( !Objects[ID_CHINESE2].loaded ) {
			S_ExitSystem("FATAL: XianLord requires CHINESE2 (statue)");
		}
		obj->initialise = InitialiseXianLord;
		obj->drawRoutine = DrawXianLord;
		obj->control = XianLordControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->hitPoints = 100;
		obj->radius = 204;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
		AnimBones[obj->boneIndex + 12*4] |= 0x08;
	}
	obj = &Objects[ID_WARRIOR];
	if( obj->loaded ) {
		if( !Objects[ID_CHINESE4].loaded ) {
			S_ExitSystem("FATAL: Warrior requires CHINESE4 (statue)");
		}
		obj->drawRoutine = DrawXianLord;
		obj->initialise = InitialiseXianLord;
		obj->control = WarriorControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->hitPoints = 80;
		obj->radius = 204;
		obj->pivotLength = 0;
		AnimBones[obj->boneIndex + 6*4] |= 0x08;
		AnimBones[obj->boneIndex + 16*4] |= 0x08;
	}
	obj = &Objects[ID_DRAGON_FRONT];
	if( obj->loaded ) {
		if( !Objects[ID_DRAGON_BACK].loaded ) {
			S_ExitSystem("FATAL: Dragon front needs back");
		}
		obj->hitPoints = 300;
		obj->pivotLength = 300;
		obj->control = DragonControl;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->collision = DragonCollision;
		obj->radius = 341;
		AnimBones[obj->boneIndex + 10*4] |= 0x10;
	}
	obj = &Objects[ID_DRAGON_BACK];
	if( obj->loaded ) {
		obj->control = DragonControl;
		obj->collision = DragonCollision;
		obj->radius = 341;
		obj->save_position = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
	obj = &Objects[ID_BARTOLI];
	if( obj->loaded ) {
		obj->initialise = InitialiseBartoli;
		obj->control = BartoliControl;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
	obj = &Objects[ID_GIANT_YETI];
	if( obj->loaded ) {
		obj->control = GiantYetiControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 200;
		obj->radius = 341;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		AnimBones[obj->boneIndex + 14*4] |= 0x08;
	}
	obj = &Objects[ID_DINO];
	if( obj->loaded ) {
		obj->control = DinoControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 100;
		obj->shadowSize = 64;
		obj->pivotLength = 1800;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
		obj->radius = 341;
		AnimBones[obj->boneIndex + 10*4] |= 0x08;
		AnimBones[obj->boneIndex + 11*4] |= 0x08;
	}
	obj = &Objects[ID_WINSTON];
	if( obj->loaded ) {
		obj->control = WinstonControl;
		obj->collision = ObjectCollision;
		obj->hitPoints = 0xC000;
		obj->shadowSize = 64;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
}

/*
 * Inject function
 */
void Inject_Setup() {
//	INJECT(0x0043A330, InitialiseLevel);
//	INJECT(0x0043A490, InitialiseGameFlags);

	INJECT(0x0043A500, InitialiseLevelFlags);
	INJECT(0x0043A530, BaddyObjects);

//	INJECT(0x0043B570, TrapObjects);
//	INJECT(0x0043BB70, ObjectObjects);

	INJECT(0x0043C7C0, InitialiseObjects);

//	INJECT(0x0043C830, GetCarriedItems);
}
