/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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
#include "game/health.h"
#include "3dsystem/scalespr.h"
#include "game/text.h"
#include "specific/output.h"
#include "specific/sndpc.h"
#include "global/vars.h"

#define AMMO_XPOS_PC	(-10)
#define AMMO_YPOS_PC	(35)

#define AMMO_XPOS_PS	(-16)
#define AMMO_YPOS_PS	(64)

#ifdef FEATURE_HUD_IMPROVED
extern bool PsxBarPosEnabled;
#endif // FEATURE_HUD_IMPROVED

BOOL __cdecl FlashIt() {
	static int counter = 0;
	static BOOL state = FALSE;

	if( counter > 0 ) {
		--counter;
	} else {
		counter = 5;
		state = !state;
	}
	return state;
}

void __cdecl DrawAssaultTimer() {
	int xPos, yPos, d0, d1, d2;
	int minutes, seconds, deciseconds;
	DWORD scaleH, scaleV;
	char timeString[16]; // NOTE: the original buffer was 8

	// Exit if current level is not Assault or the timer is hidden
	if( CurrentLevel != 0 || !IsAssaultTimerDisplay )
		return;

	deciseconds = SaveGame.statistics.timer % 30 / 3;
	seconds = SaveGame.statistics.timer / 30 % 60;
	minutes = SaveGame.statistics.timer / 30 / 60;
	sprintf(timeString, "%d:%02d.%d", minutes, seconds, deciseconds);

#ifdef FEATURE_HUD_IMPROVED
	scaleH = GetRenderScale(PHD_ONE);
	scaleV = GetRenderScale(PHD_ONE);
	xPos = PhdWinMaxX / 2 - GetRenderScale(50);
	yPos = GetRenderScale(36);
	d0 = GetRenderScale(20);
	d1 = GetRenderScale(-6);
	d2 = GetRenderScale(14);
#else // !FEATURE_HUD_IMPROVED
	scaleH = PHD_ONE;
	scaleV = PHD_ONE;
	xPos = PhdWinMaxX / 2 - 50;
	yPos = 36;
	d0 = 20;
	d1 = -6;
	d2 = 14;
#endif // FEATURE_HUD_IMPROVED

	for( char *str = timeString; *str != 0; ++str ) {
		switch( *str ) {
			case ':' : // colon
				xPos += d1;
				S_DrawScreenSprite2d(xPos, yPos, 0, scaleH, scaleV, (Objects[ID_ASSAULT_DIGITS].meshIndex + 10), 0x1000, 0);
				xPos += d2;
				break;

			case '.' : // period
				xPos += d1;
				S_DrawScreenSprite2d(xPos, yPos, 0, scaleH, scaleV, (Objects[ID_ASSAULT_DIGITS].meshIndex + 11), 0x1000, 0);
				xPos += d2;
				break;

			default : // any digit
				S_DrawScreenSprite2d(xPos, yPos, 0, scaleH, scaleV, (Objects[ID_ASSAULT_DIGITS].meshIndex + (*str - '0')), 0x1000, 0);
				xPos += d0;
				break;
		}
	}
}

void __cdecl DrawGameInfo(BOOL pickupState) {
	BOOL flashState;

	DrawAmmoInfo();
	DrawModeInfo();

	if( OverlayStatus > 0 ) {
		flashState = FlashIt();
		DrawHealthBar(flashState);
		DrawAirBar(flashState);
		DrawPickups(pickupState);
		DrawAssaultTimer();
	}
	T_DrawText();
}

void __cdecl DrawHealthBar(BOOL flashState) {
	static int oldHitPoints;
	int hitPoints;

	// NOTE: fixes original game bug when health bar is visible while final cut scene
	if( Lara.extra_anim && LaraItem->currentAnimState == EXTRA_FINALANIM ) {
		return;
	}

	hitPoints = LaraItem->hitPoints;
	CLAMP(hitPoints, 0, 1000);

	if( oldHitPoints != hitPoints ){
		oldHitPoints = hitPoints;
		HealthBarTimer = 40; // 1.33 seconds
	}
	CLAMPL(HealthBarTimer, 0);

	if( hitPoints <= 250 ) {
		if( flashState == 0 ) {
			S_DrawHealthBar(0);
		} else {
#ifdef FEATURE_HUD_IMPROVED
			S_DrawHealthBar(PHD_ONE * hitPoints / 1000);
#else // !FEATURE_HUD_IMPROVED
			S_DrawHealthBar(hitPoints / 10);
#endif // FEATURE_HUD_IMPROVED
		}
	}
	else if( HealthBarTimer > 0 || hitPoints <= 0 || Lara.gun_status == LGS_Ready ) {
#ifdef FEATURE_HUD_IMPROVED
		S_DrawHealthBar(PHD_ONE * hitPoints / 1000);
#else // !FEATURE_HUD_IMPROVED
		S_DrawHealthBar(hitPoints / 10);
#endif // FEATURE_HUD_IMPROVED
	}
}

void __cdecl DrawAirBar(BOOL flashState) {
	int air;

	if( Lara.water_status != LWS_Underwater && Lara.water_status != LWS_Surface )
		return;

	air = Lara.air;
	CLAMP(air, 0, 1800);

	if( air <= 450 && flashState == 0 ) {
		S_DrawAirBar(0);
	} else {
#ifdef FEATURE_HUD_IMPROVED
		S_DrawAirBar(PHD_ONE * air / 1800);
#else // !FEATURE_HUD_IMPROVED
		S_DrawAirBar(100 * air / 1800);
#endif // FEATURE_HUD_IMPROVED
	}
}

void __cdecl MakeAmmoString(char *str) {
	for( ; *str != 0; ++str ) {
		if( *str == 0x20 ) {
			continue; // space chars
		}
		if( (BYTE)*str < 'A' ) {
			*str += 1 - '0'; // ammo digit chars
		} else {
			*str += 0xC - 'A'; // ammo special chars
		}
	}
}

void __cdecl DrawAmmoInfo() {
	char ammoString[80] = "";

	if( Lara.gun_status != LGS_Ready || OverlayStatus <= 0 || SaveGame.bonusFlag ) {
		if( AmmoTextInfo != NULL ) {
			T_RemovePrint(AmmoTextInfo);
			AmmoTextInfo = NULL;
		}
		return;
	}

	switch( Lara.gun_type ) {
		case LGT_Magnums :
			sprintf(ammoString, "%5d", (int)Lara.magnum_ammo);
			break;

		case LGT_Uzis :
			sprintf(ammoString, "%5d", (int)Lara.uzi_ammo);
			break;

		case LGT_Shotgun :
			sprintf(ammoString, "%5d", (int)(Lara.shotgun_ammo / 6));
			break;

		case LGT_Harpoon :
			sprintf(ammoString, "%5d", (int)Lara.harpoon_ammo);
			break;

		case LGT_M16 :
			sprintf(ammoString, "%5d", (int)Lara.m16_ammo);
			break;

		case LGT_Grenade :
			sprintf(ammoString, "%5d", (int)Lara.grenade_ammo);
			break;

		default :
			return;
	}

	MakeAmmoString(ammoString);

	if( AmmoTextInfo == NULL ) {
#ifdef FEATURE_HUD_IMPROVED
		if( PsxBarPosEnabled ) {
			AmmoTextInfo = T_Print(AMMO_XPOS_PS, AMMO_YPOS_PS, 0, ammoString);
		} else {
			AmmoTextInfo = T_Print(AMMO_XPOS_PC, AMMO_YPOS_PC, 0, ammoString);
		}
#else // !FEATURE_HUD_IMPROVED
		AmmoTextInfo = T_Print(AMMO_XPOS_PC, AMMO_YPOS_PC, 0, ammoString);
#endif // FEATURE_HUD_IMPROVED
		T_RightAlign(AmmoTextInfo, 1);
	} else {
		T_ChangeText(AmmoTextInfo, ammoString);
	}
}

void __cdecl DrawPickups(BOOL pickupState) {
	static int oldGameTimer = 0;
	int time;
	int x, y;
	int cellH, cellV;
	int column = 0;

	time = SaveGame.statistics.timer - oldGameTimer;
	oldGameTimer = SaveGame.statistics.timer;
	if( time <= 0 || time >= 60 ) // 0..2 seconds
		return;

#ifdef FEATURE_HUD_IMPROVED
	cellH = MIN(PhdWinWidth, PhdWinHeight*320/200) / 10;
#else // !FEATURE_HUD_IMPROVED
	cellH = PhdWinWidth / 10;
#endif // FEATURE_HUD_IMPROVED
	cellV = cellH * 2 / 3;

	x = PhdWinWidth - cellH;
	y = PhdWinHeight - cellH;

	for( int i = 0; i < 12; ++i ) {
		if( pickupState ) {
			PickupInfos[i].timer -= time;
			CLAMPL(PickupInfos[i].timer, 0);
		}

		if( PickupInfos[i].timer > 0 ) {
			if( ++column > 4 ) {
				column = 1;
				x = PhdWinWidth - cellH;
				y -= cellV;
			}
			S_DrawPickup(x, y, 0x3000, PickupInfos[i].sprite, 0x1000);
			x -= cellH;
		}
	}
}

void __cdecl AddDisplayPickup(__int16 itemID) {
	if( itemID == ID_SECRET1 || itemID == ID_SECRET2 || itemID == ID_SECRET3 ) {
		S_CDPlay(GF_GameFlow.secretTrack, FALSE);
	}

	for( int i = 0; i < 12; ++i ) {
		if( PickupInfos[i].timer <= 0 ) {
			PickupInfos[i].timer = 75; // 2.5 seconds
			PickupInfos[i].sprite = Objects[itemID].meshIndex;
			break;
		}
	}
}

void __cdecl DisplayModeInfo(char *modeString) {
	if( modeString == NULL ) {
		T_RemovePrint(DisplayModeTextInfo);
		DisplayModeTextInfo = NULL;
		return;
	}

	if( DisplayModeTextInfo != NULL ) {
		T_ChangeText(DisplayModeTextInfo, modeString);
	} else {
#ifdef FEATURE_HUD_IMPROVED
		DisplayModeTextInfo = T_Print(-8, -8, 0, modeString);
#else // FEATURE_HUD_IMPROVED
		DisplayModeTextInfo = T_Print(-16, -16, 0, modeString);
#endif // FEATURE_HUD_IMPROVED
		T_RightAlign(DisplayModeTextInfo, 1);
		T_BottomAlign(DisplayModeTextInfo, 1);
	}
	DisplayModeInfoTimer = 75; // 2.5 seconds
#ifdef FEATURE_HUD_IMPROVED
	T_HideText(DisplayModeTextInfo, (OverlayStatus <= 0));
#endif // FEATURE_HUD_IMPROVED
}

void __cdecl DrawModeInfo() {
	if( DisplayModeTextInfo != NULL && --DisplayModeInfoTimer == 0 ) {
		T_RemovePrint(DisplayModeTextInfo);
		DisplayModeTextInfo = NULL;
	}
#ifdef FEATURE_HUD_IMPROVED
	T_HideText(DisplayModeTextInfo, (OverlayStatus <= 0));
#endif // FEATURE_HUD_IMPROVED
}

void __cdecl InitialisePickUpDisplay() {
	for( int i = 0; i < 12; ++i ) {
		PickupInfos[i].timer = 0;
	}
}

/*
 * Inject function
 */
void Inject_Health() {
	INJECT(0x00421980, FlashIt);
	INJECT(0x004219B0, DrawAssaultTimer);
	INJECT(0x00421B00, DrawGameInfo);
	INJECT(0x00421B50, DrawHealthBar);
	INJECT(0x00421C00, DrawAirBar);
	INJECT(0x00421CA0, MakeAmmoString);
	INJECT(0x00421CD0, DrawAmmoInfo);
	INJECT(0x00421E20, InitialisePickUpDisplay);
	INJECT(0x00421E40, DrawPickups);
	INJECT(0x00421F40, AddDisplayPickup);
	INJECT(0x00421FB0, DisplayModeInfo);
	INJECT(0x00422030, DrawModeInfo);
}
