/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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
#include "game/setup.h"
#include "game/bear.h"
#include "game/bird.h"
#include "game/boat.h"
#include "game/collide.h"
#include "game/cinema.h"
#include "game/diver.h"
#include "game/dog.h"
#include "game/dragon.h"
#include "game/draw.h"
#include "game/eel.h"
#include "game/effects.h"
#include "game/enemies.h"
#include "game/gameflow.h"
#include "game/hair.h"
#include "game/health.h"
#include "game/items.h"
#include "game/invfunc.h"
#include "game/lara1gun.h"
#include "game/laraflare.h"
#include "game/laramisc.h"
#include "game/lot.h"
#include "game/moveblock.h"
#include "game/missile.h"
#include "game/objects.h"
#include "game/people.h"
#include "game/pickup.h"
#include "game/rat.h"
#include "game/savegame.h"
#include "game/shark.h"
#include "game/skidoo.h"
#include "game/spider.h"
#include "game/sound.h"
#include "game/traps.h"
#include "game/text.h"
#include "game/wolf.h"
#include "game/yeti.h"
#include "specific/file.h"
#include "specific/init.h"
#include "specific/output.h"
#include "specific/sndpc.h"
#include "specific/winmain.h"
#include "global/vars.h"

#ifdef FEATURE_GOLD
extern bool IsGold();
#endif

// NOTE: Create simple pickup item (2D)
// Set isSecret if it's a secret pickup (Example: Dragon Item)
static void SetPickup(GAME_OBJECT_ID objectID, bool isSecret = false) {
	OBJECT_INFO* obj = &Objects[objectID];
	obj->drawRoutine = DrawSpriteItem;
	obj->collision = PickUpCollision;
	obj->save_position = TRUE;
	obj->save_flags = TRUE;
	if (isSecret) {
		obj->control = SecretControl;
	}
}

BOOL __cdecl InitialiseLevel(int levelID, GF_LEVEL_TYPE levelType) {
	BOOL isLoaded = FALSE;
	if (levelType != GFL_TITLE && levelType != GFL_CUTSCENE) {
		CurrentLevel = levelID;
	}
	IsDemoLevelType = levelType == GFL_DEMO;
	InitialiseGameFlags();
	Lara.item_number = -1;
	IsTitleLoaded = FALSE;
	if (levelType != GFL_TITLE) {
		if (levelType == GFL_SAVED || levelType != GFL_CUTSCENE) {
			isLoaded = S_LoadLevelFile(GF_LevelFilesStringTable[levelID], levelID, levelType);
		} else {
			isLoaded = S_LoadLevelFile(GF_CutsFilesStringTable[levelID], levelID, GFL_CUTSCENE);
		}
	} else {
		isLoaded = S_LoadLevelFile(GF_TitleFilesStringTable[0], levelID, GFL_TITLE);
	}

	if (isLoaded) {
		if (Lara.item_number != -1) {
			InitialiseLara(levelType);
		}
		if (levelType == GFL_NORMAL || levelType == GFL_SAVED || levelType == GFL_DEMO) {
			GetCarriedItems();
		}
		Effects = (FX_INFO*)game_malloc(sizeof(FX_INFO) * 100, GBUF_EffectsArray);
		InitialiseFXArray();
		InitialiseLOTarray();
		InitColours();
		T_InitPrint();
		InitialisePickUpDisplay();
		S_InitialiseScreen(levelType);
		HealthBarTimer = 100;
		SOUND_Stop();

		if (levelType == GFL_SAVED) {
			ExtractSaveGameInfo();
		} else if (levelType == GFL_NORMAL) {
			GF_ModifyInventory(CurrentLevel, FALSE);
		}

		if (Objects[ID_FINAL_LEVEL_COUNTER].loaded) {
			InitialiseFinalLevel();
		}

		if (levelType == GFL_NORMAL || levelType == GFL_SAVED || levelType == GFL_DEMO) {
			if (TrackIDs[0] != 0) {
				S_CDPlay(TrackIDs[0], TRUE);
			}
		}

		IsAssaultTimerActive = FALSE;
		IsAssaultTimerDisplay = FALSE;
		Camera.underwater = FALSE;
		isLoaded = TRUE;
	}

	return isLoaded;
}

void __cdecl InitialiseGameFlags() {
	FlipStatus = 0;
	ZeroMemory(FlipMaps, sizeof(FlipMaps));
	ZeroMemory(CD_Flags, sizeof(CD_Flags));
	for (int i = 0; i < ID_NUMBER_OBJECTS; i++) {
		Objects[i].loaded = FALSE;
	}
	SunsetTimer = 0;
	AmmoTextInfo = NULL;
	IsLevelComplete = FALSE;
	FlipEffect = -1;
	MinesDetonated = FALSE;
	IsMonkAngry = FALSE;
}

void __cdecl InitialiseLevelFlags() {
	memset(&SaveGame.statistics, 0, sizeof(STATISTICS_INFO));
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
			S_ExitSystem("FATAL: CULT1A requires CULT1"); // NOTE: there was a typo: CULT1B instead of CULT1A
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
#ifdef FEATURE_GOLD
		obj->initialise = IsGold() ? InitialiseWolf : NULL;
		obj->control = IsGold() ? WolfControl : SpiderControl;
		obj->hitPoints = IsGold() ? 10 : 5;
		obj->radius = IsGold() ? 341 : 102;
#else
		obj->control = SpiderControl;
		obj->hitPoints = 5;
		obj->radius = 102;
#endif
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_hitpoints = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
	obj = &Objects[ID_BIG_SPIDER_or_BEAR];
	if( obj->loaded ) {
#ifdef FEATURE_GOLD
		obj->control = IsGold() ? BearControl : BigSpiderControl;
		obj->hitPoints = IsGold() ? 30 : 40;
#else
		obj->control = BigSpiderControl;
		obj->hitPoints = 40;
#endif
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
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
#ifdef FEATURE_GOLD
		obj->shadowSize = IsGold() ? 0 : 128;
#else
		obj->shadowSize = 128;
#endif
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
		obj->hitPoints = HP_DONT_TARGET;
		obj->shadowSize = 64;
		obj->radius = 102;
		obj->intelligent = 1;
		obj->save_position = 1;
		obj->save_flags = 1;
		obj->save_anim = 1;
	}
}

void __cdecl TrapObjects() {
	OBJECT_INFO *obj;
	int i;

	obj = &Objects[ID_GONDOLA];
	obj->collision = ObjectCollision;
	obj->control = GondolaControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_SPIKE_WALL];
	obj->collision = ObjectCollision;
	obj->control = ControlSpikeWall;
	obj->save_position = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_CEILING_SPIKES];
	obj->collision = TrapCollision;
	obj->control = ControlCeilingSpikes;
	obj->save_position = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_COPTER];
	obj->control = CopterControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_MINI_COPTER];
	obj->control = MiniCopterControl;
	obj->save_flags = TRUE;
	obj->save_position = TRUE;

	obj = &Objects[ID_HOOK];
	obj->collision = CreatureCollision;
	obj->control = HookControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_GENERAL];
	obj->collision = ObjectCollision;
	obj->control = GeneralControl;
	obj->water_creature = TRUE;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_DYING_MONK];
	obj->initialise = InitialiseDyingMonk;
	obj->collision = ObjectCollision;
	obj->control = DyingMonk;
	obj->save_flags = TRUE;

	obj = &Objects[ID_MINE];
	obj->collision = ObjectCollision;
	obj->control = MineControl;
	obj->save_flags = TRUE;

	obj = &Objects[ID_DEATH_SLIDE];
	obj->initialise = InitialiseRollingBall;
	obj->collision = DeathSlideCollision;
	obj->control = ControlDeathSlide;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;
	obj->save_position = TRUE;

	obj = &Objects[ID_PROPELLER1];
	obj->collision = TrapCollision;
	obj->control = PropellerControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_PROPELLER2];
	obj->collision = ObjectCollision;
	obj->control = PropellerControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

    obj = &Objects[ID_PROPELLER3];
	obj->collision = TrapCollision;
	obj->control = PropellerControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;
	obj->water_creature = TRUE;

	obj = &Objects[ID_PROPELLER4];
	obj->collision = ObjectCollision;
	obj->control = PropellerControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_SPINNING_BLADE];
	obj->initialise = InitialiseKillerStatue;
	obj->control = SpinningBlade;
	obj->collision = ObjectCollision;
	obj->save_position = TRUE;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_KILLER_STATUE];
	obj->initialise = InitialiseKillerStatue;
	obj->control = KillerStatueControl;
	obj->collision = TrapCollision;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	for (i = 0; i < 3; i++) {
		obj = &Objects[ID_FALLING_BLOCK1 + i];
		obj->control = FallingBlock;
		obj->ceiling = FallingBlockCeiling;
		obj->floor = FallingBlockFloor;
		obj->save_position = TRUE;
		obj->save_anim = TRUE;
		obj->save_flags = TRUE;
	}

	obj = &Objects[ID_ICICLE];
	obj->collision = TrapCollision;
	obj->control = IcicleControl;
	obj->save_position = TRUE;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_BLADE];
	obj->initialise = InitialiseBlade;
	obj->control = BladeControl;
	obj->collision = TrapCollision;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_SPRING_BOARD];
	obj->control = SpringBoardControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_PENDULUM1];
	obj->collision = ObjectCollision;
	obj->control = Pendulum;
	obj->shadowSize = 128;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_PENDULUM2];
	obj->collision = ObjectCollision;
	obj->control = Pendulum;
	obj->shadowSize = 128;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_TEETH_TRAP];
	obj->collision = TrapCollision;
	obj->control = TeethTrap;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_FALLING_CEILING];
	obj->collision = TrapCollision;
	obj->control = FallingCeiling;
	obj->save_position = TRUE;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	for (i = 0; i < 3; i++) {
		obj = &Objects[ID_ROLLING_BALL1 + i];
		obj->initialise = InitialiseRollingBall;
		obj->collision = RollingBallCollision;
		obj->control = RollingBallControl;
		obj->save_position = TRUE;
		obj->save_anim = TRUE;
		obj->save_flags = TRUE;
	}

	for (i = 0; i < 4; i++) {
		obj = &Objects[ID_MOVABLE_BLOCK1 + i];
		obj->initialise = InitialiseMovingBlock;
		obj->collision = MovableBlockCollision;
		obj->control = MovableBlock;
		obj->drawRoutine = DrawMovableBlock;
		obj->save_position = TRUE;
		obj->save_anim = TRUE;
		obj->save_flags = TRUE;
	}

	obj = &Objects[ID_SPIKES];
	obj->collision = SpikeCollision;

	obj = &Objects[ID_DARTS];
	obj->control = DartsControl;
	obj->collision = ObjectCollision;
	obj->shadowSize = 128;

	obj = &Objects[ID_DART_EMITTER];
	obj->control = DartEmitterControl;
	obj->save_flags = TRUE;

	obj = &Objects[ID_DART_EFFECT];
	obj->control = DartEffectControl;
	obj->drawRoutine = DrawSpriteItem;
	obj->semi_transparent = TRUE;

	obj = &Objects[ID_FLAME];
	obj->control = FlameControl;
	obj->semi_transparent = TRUE;

	obj = &Objects[ID_FLAME_EMITTER];
	obj->control = FlameEmitterControl;
	obj->drawRoutine = DrawDummyItem;
	obj->save_flags = TRUE;

	obj = &Objects[ID_LAVA];
	obj->control = ControlLavaBlob;
	obj->semi_transparent = TRUE;

	obj = &Objects[ID_LAVA_EMITTER];
	obj->control = LavaSpray;
	obj->collision = ObjectCollision;
	obj->drawRoutine = DrawDummyItem;
	obj->save_flags = TRUE;
}

void __cdecl ObjectObjects() {
	OBJECT_INFO* obj;
	int i;

	obj = &Objects[ID_CAMERA_TARGET];
	obj->drawRoutine = DrawDummyItem;

	obj = &Objects[ID_ROCKET];
	obj->control = ControlRocket;
	obj->save_position = TRUE;

	obj = &Objects[ID_HARPOON_BOLT];
	obj->control = ControlHarpoonBolt;
	obj->save_position = TRUE;

	obj = &Objects[ID_MISSILE_KNIFE];
	obj->control = ControlMissile;
	obj->save_position = TRUE;

	obj = &Objects[ID_MISSILE_HARPOON];
	obj->control = ControlMissile;
	obj->save_position = TRUE;

	for (i = 0; i < 3; i++) {
		obj = &Objects[ID_SPHERE_OF_DOOM1 + i];
		obj->control = SphereOfDoom;
		obj->drawRoutine = DrawSphereOfDoom;
		obj->collision = SphereOfDoomCollision;
		if (i < 2) {
			obj->semi_transparent = TRUE;
		}
		obj->save_flags = TRUE;
		obj->save_position = TRUE;
	}

	obj = &Objects[ID_BIG_BOWL];
	obj->control = BigBowlControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_BELL];
	obj->control = BellControl;
	obj->collision = ObjectCollision;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_SKIDOO_FAST];
	obj->initialise = InitialiseSkidoo;
	obj->collision = SkidooCollision;
	obj->drawRoutine = DrawSkidoo;
	obj->save_position = TRUE;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_BOAT];
	obj->initialise = InitialiseBoat;
	obj->control = BoatControl;
	obj->collision = BoatCollision;
	obj->save_position = TRUE;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_FLARE_ITEM];
	obj->control = FlareControl;
	obj->collision = PickUpCollision;
	obj->drawRoutine = DrawFlareInAir;
	obj->save_position = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_WINDOW1];
	obj->initialise = InitialiseWindow;
	obj->control = WindowControl;
	obj->collision = ObjectCollision;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_WINDOW2];
	obj->initialise = InitialiseWindow;
	obj->control = SmashIceControl;
	obj->collision = ObjectCollision;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_LIFT];
	obj->initialise = InitialiseLift;
	obj->control = LiftControl;
	obj->floor = LiftFloor;
	obj->ceiling = LiftCeiling;
	obj->save_position = TRUE;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_BRIDGE_FLAT];
	obj->floor = BridgeFlatFloor;
	obj->ceiling = BridgeFlatCeiling;

	obj = &Objects[ID_BRIDGE_TILT1];
	obj->floor = BridgeTilt1Floor;
	obj->ceiling = BridgeTilt1Ceiling;

	obj = &Objects[ID_BRIDGE_TILT2];
	obj->floor = BridgeTilt2Floor;
	obj->ceiling = BridgeTilt2Ceiling;

	obj = &Objects[ID_DRAW_BRIDGE];
	if (obj->loaded) {
		obj->control = GeneralControl;
		obj->collision = DrawBridgeCollision;
		obj->floor = DrawBridgeFloor;
		obj->ceiling = DrawBridgeCeiling;
		obj->save_anim = TRUE;
		obj->save_flags = TRUE;
	}

	for (i = 0; i < 5; i++) {
		obj = &Objects[ID_SWITCH_TYPE1 + i];
		obj->control = SwitchControl;
		obj->collision = i < 4 ? SwitchCollision : SwitchCollision2;
		obj->save_anim = TRUE;
		obj->save_flags = TRUE;
	}

	for (i = 0; i < 8; i++) {
		obj = &Objects[ID_DOOR_TYPE1 + i];
		obj->initialise = InitialiseDoor;
		obj->control = DoorControl;
		obj->collision = DoorCollision;
		obj->drawRoutine = DrawUnclippedItem;
		obj->save_anim = TRUE;
		obj->save_flags = TRUE;
	}

	for (i = 0; i < 2; i++) {
		obj = &Objects[ID_TRAPDOOR_TYPE1 + i];
		obj->control = TrapDoorControl;
		obj->ceiling = TrapDoorCeiling;
		obj->floor = TrapDoorFloor;
		obj->save_anim = TRUE;
		obj->save_flags = TRUE;
	}

	SetPickup(ID_PICKUP_ITEM1);
	SetPickup(ID_PICKUP_ITEM2);
	SetPickup(ID_KEY_ITEM1);
	SetPickup(ID_KEY_ITEM2);
	SetPickup(ID_KEY_ITEM3);
	SetPickup(ID_KEY_ITEM4);
	SetPickup(ID_PUZZLE_ITEM1);
	SetPickup(ID_PUZZLE_ITEM2);
	SetPickup(ID_PUZZLE_ITEM3);
	SetPickup(ID_PUZZLE_ITEM4);
	SetPickup(ID_SECRET1, true);
	SetPickup(ID_SECRET2, true);
	SetPickup(ID_SECRET3, true);
	SetPickup(ID_PISTOL_ITEM);
	SetPickup(ID_SHOTGUN_ITEM);
	SetPickup(ID_MAGNUM_ITEM);
	SetPickup(ID_UZI_ITEM);
	SetPickup(ID_M16_ITEM);
	SetPickup(ID_HARPOON_ITEM);
	SetPickup(ID_GRENADE_ITEM);
	SetPickup(ID_PISTOL_AMMO_ITEM);
	SetPickup(ID_SHOTGUN_AMMO_ITEM);
	SetPickup(ID_MAGNUM_AMMO_ITEM);
	SetPickup(ID_UZI_AMMO_ITEM);
	SetPickup(ID_M16_AMMO_ITEM);
	SetPickup(ID_HARPOON_AMMO_ITEM);
	SetPickup(ID_GRENADE_AMMO_ITEM);
	SetPickup(ID_FLARES_ITEM);
	SetPickup(ID_SMALL_MEDIPACK_ITEM);
	SetPickup(ID_LARGE_MEDIPACK_ITEM);

	obj = &Objects[ID_GONG_BONGER];
	obj->control = ControlGongBonger;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	for (i = 0; i < 4; i++) {
		obj = &Objects[ID_KEY_HOLE1 + i];
		obj->collision = KeyHoleCollision;
		obj->save_flags = TRUE;
	}

	for (i = 0; i < 4; i++) {
		obj = &Objects[ID_PUZZLE_HOLE1 + i];
		obj->collision = PuzzleHoleCollision;
		obj->save_flags = TRUE;
	}

	for (i = 0; i < 4; i++) {
		obj = &Objects[ID_PUZZLE_DONE1 + i];
		obj->save_flags = TRUE;
	}

	obj = &Objects[ID_DETONATOR1];
	obj->collision = DetonatorCollision;

	obj = &Objects[ID_DETONATOR2];
	obj->collision = DetonatorCollision;
	obj->control = DetonatorControl;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_ALARM_SOUND];
	obj->control = ControlAlarmSound;
	obj->save_flags = TRUE;

	obj = &Objects[ID_LARA_ALARM];
	obj->control = ControlLaraAlarm;
	obj->drawRoutine = DrawDummyItem;
	obj->save_flags = TRUE;

	for (i = 0; i < 10; i++) {
		obj = &Objects[ID_PLAYER1 + i];
		obj->initialise = InitialiseGenPlayer;
		obj->control = ControlCinematicPlayer;
		obj->hitPoints = 1;
	}

	obj = &Objects[ID_BLOOD];
	obj->control = ControlBlood1;
	obj->semi_transparent = TRUE;

	obj = &Objects[ID_BUBBLES];
	obj->control = ControlBubble1;

	obj = &Objects[ID_EXPLOSION];
	obj->control = ControlExplosion1;
	obj->semi_transparent = TRUE;

	obj = &Objects[ID_MISSILE_FLAME];
	obj->control = ControlMissile;
	obj->semi_transparent = TRUE;

	obj = &Objects[ID_RICOCHET];
	obj->control = ControlRichochet1;

	obj = &Objects[ID_TWINKLE];
	obj->control = ControlTwinkle;

	obj = &Objects[ID_SPLASH];
	obj->control = ControlSplash1;
	obj->semi_transparent = TRUE;

	obj = &Objects[ID_SNOW_SPRITE];
	obj->control = ControlSnowSprite;

	obj = &Objects[ID_WATER_SPRITE];
	obj->control = ControlWaterSprite;
	obj->semi_transparent = TRUE;

	obj = &Objects[ID_WATERFALL];
	obj->control = WaterFall;
	obj->drawRoutine = DrawDummyItem;

	obj = &Objects[ID_BODY_PART];
	obj->nMeshes = 0;
	obj->control = ControlBodyPart;
	obj->loaded = TRUE;

	obj = &Objects[ID_GUN_FLASH];
	obj->control = ControlGunShot;

	obj = &Objects[ID_GLOW];
	obj->control = ControlGlow;

	obj = &Objects[ID_HOT_LIQUID];
	obj->control = ControlHotLiquid;
	obj->semi_transparent = TRUE;

	for (i = 0; i < 2; i++) {
		obj = &Objects[ID_BIRD_TWEETER1 + i];
		obj->control = ControlBirdTweeter;
		obj->drawRoutine = DrawDummyItem;
	}

	obj = &Objects[ID_DING_DONG];
	obj->control = ControlDingDong;
	obj->drawRoutine = DrawDummyItem;

	obj = &Objects[ID_CLOCK_CHIMES];
	obj->control = ControlClockChimes;
	obj->drawRoutine = DrawDummyItem;
	obj->save_flags = TRUE;

	obj = &Objects[ID_FINAL_LEVEL_COUNTER];
	obj->control = FinalLevelCounter;
	obj->drawRoutine = DrawDummyItem;
	obj->save_flags = TRUE;

	obj = &Objects[ID_CUT_SHOTGUN];
	obj->control = ControlCutShotgun;
	obj->save_anim = TRUE;
	obj->save_flags = TRUE;

	obj = &Objects[ID_EARTHQUAKE];
	obj->control = EarthQuake;
	obj->drawRoutine = DrawDummyItem;
	obj->save_flags = TRUE;
}

void __cdecl InitialiseObjects() {
	for( int i = 0; i < ID_NUMBER_OBJECTS; ++i ) {
		Objects[i].intelligent = FALSE;
		Objects[i].save_position = FALSE;
		Objects[i].save_hitpoints = FALSE;
		Objects[i].save_flags = FALSE;
		Objects[i].save_anim = FALSE;
		Objects[i].water_creature = FALSE;
		Objects[i].initialise = NULL;
		Objects[i].collision = NULL;
		Objects[i].control = NULL;
		Objects[i].drawRoutine = DrawAnimatingItem;
		Objects[i].ceiling = NULL;
		Objects[i].floor = NULL;
		Objects[i].pivotLength = 0;
		Objects[i].radius = 10;
		Objects[i].shadowSize = 0;
		Objects[i].hitPoints = HP_DONT_TARGET;
	}
	BaddyObjects();
	TrapObjects();
	ObjectObjects();
	InitialiseHair();
}

void __cdecl GetCarriedItems() {
	ITEM_INFO *item, *pickup;
	int i;

	for (i = 0, item = Items; i < LevelItemCount; i++, item++) {
		if (Objects[item->objectID].intelligent) {
			item->carriedItem = -1;
			__int16 pickupID = RoomInfo[item->roomNumber].itemNumber;
			while (pickupID != -1)
			{
				pickup = &Items[pickupID];
				if (pickup->pos.x == item->pos.x
				&&  pickup->pos.y == item->pos.y
				&&  pickup->pos.z == item->pos.z
				&&  Objects[pickup->objectID].collision == PickUpCollision) {
					pickup->carriedItem = item->carriedItem;
					item->carriedItem = pickupID;
					RemoveDrawnItem(pickupID);
					pickup->roomNumber = 255;
				}
				pickupID = pickup->nextItem;
			}
		}
	}
}

/*
 * Inject function
 */
void Inject_Setup() {
	INJECT(0x0043A330, InitialiseLevel);
	INJECT(0x0043A490, InitialiseGameFlags);
	INJECT(0x0043A500, InitialiseLevelFlags);
	INJECT(0x0043A530, BaddyObjects);
	INJECT(0x0043B570, TrapObjects);
	INJECT(0x0043BB70, ObjectObjects);
	INJECT(0x0043C7C0, InitialiseObjects);
	INJECT(0x0043C830, GetCarriedItems);
}
