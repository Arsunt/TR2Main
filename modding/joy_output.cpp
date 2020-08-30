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
#include "modding/joy_output.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#define NUM_MOTORS (2)
#define NUM_VIBS (16)

extern void SetJoystickOutput(WORD leftMotor, WORD rightMotor, DWORD ledColor);
extern bool IsJoyVibrationEnabled();
extern bool IsJoyLedColorEnabled();

typedef struct {
	int inc, val, sus, dec, len, stage, value;
} VIBRATION;

static VIBRATION Vib[NUM_MOTORS][NUM_VIBS];
static DWORD LedColor = 0;

static int BlendLedColor(int c1, int c2) {
	if( c1 < c2 ) {
		c1 += 16;
		CLAMPG(c1 ,c2);
	} else if( c1 > c2 ) {
		c1 -= 16;
		CLAMPL(c1 ,c2);
	}
	return c1;
}

static void SetupVibration(int motor, int inc, int val, int sus, int dec, int len, bool isCamera) {
	int id = 0;
	if( !isCamera ) {
		int id = 1;
		for( int i=1; i<NUM_VIBS; ++i ) {
			if( Vib[motor][i].len <= 0 ) {
				id = i;
				break;
			}
			if( Vib[motor][i].len < Vib[motor][id].len ) {
				id = i;
			}
		}
	}
	CLAMP(inc, 0, 0xFFFF);
	CLAMP(val, 0, 0xFFFF);
	CLAMP(dec, 0, 0xFFFF);
	Vib[motor][id].inc = inc;
	Vib[motor][id].val = val;
	Vib[motor][id].sus = len-sus;
	Vib[motor][id].dec = dec;
	Vib[motor][id].len = len;
	Vib[motor][id].stage = 0;
	Vib[motor][id].value = 0;
}

void JoyOutputReset() {
	memset(Vib, 0, sizeof(Vib));
	JoyVibrationMute();
}

void JoyVibrationMute() {
	SetJoystickOutput(0, 0, LedColor);
}

void JoyVibrate(int inc, int val, int sus, int dec, int len, bool isCamera) {
	SetupVibration(1, inc, val, sus, dec, len, isCamera);
}

void JoyRumble(int inc, int val, int sus, int dec, int len, bool isCamera) {
	SetupVibration(0, inc, val, sus, dec, len, isCamera);
}

void JoyRumbleExplode(int x, int y, int z, DWORD range, bool fromLara) {
	x -= fromLara ? LaraItem->pos.x : Camera.pos.x;
	y -= fromLara ? LaraItem->pos.y : Camera.pos.y;
	z -= fromLara ? LaraItem->pos.z : Camera.pos.z;
	DWORD dist = SQR(x) + SQR(y) + SQR(z);
	if( dist < SQR(range) ) {
		// it is intended to be non-linear
		dist = 0xFFFF - (UINT64)dist*0xFFFF/SQR(range);
		JoyRumble(dist/8, dist, 20, dist/32, 40, false);
	}
}

void UpdateJoyOutput(bool isInGame) {
	WORD motor[2] = {0, 0};
	for( int i=0; i<NUM_MOTORS; ++i ) {
		for( int j=0; j<NUM_VIBS; ++j ) {
			if( !IsJoyVibrationEnabled() || !isInGame || Vib[i][j].len <= 0 ) {
				continue;
			}
			switch( Vib[i][j].stage ) {
			case 0:
				Vib[i][j].value += Vib[i][j].inc;
				if( Vib[i][j].value < Vib[i][j].val ) {
					break;
				}
				Vib[i][j].value = Vib[i][j].val;
				Vib[i][j].stage = 1;
				// fall through
			case 1:
				if( Vib[i][j].len > Vib[i][j].sus) {
					break;
				}
				Vib[i][j].stage = 2;
				// fall through
			case 2:
				Vib[i][j].value -= Vib[i][j].dec;
				if( Vib[i][j].value > 0 ) {
					break;
				}
				Vib[i][j].value = 0;
				Vib[i][j].stage = 3;
				// fall through
			default:
				break;
			}
			CLAMP(Vib[i][j].value, 0, 0xFFFF);
			CLAMPL(motor[i], Vib[i][j].value);
			--Vib[i][j].len;
		}
	}


	int r=0, g=0, b=0;
	bool isInjured = false;
	if( !IsJoyLedColorEnabled() || !isInGame || LaraItem == NULL ) {
		r = RGB_GETRED  (DEFAULT_JOYSTICK_LED_COLOR);
		g = RGB_GETGREEN(DEFAULT_JOYSTICK_LED_COLOR);
		b = RGB_GETBLUE (DEFAULT_JOYSTICK_LED_COLOR);
	} else {
		static __int16 hitPoints = 0;
		isInjured = (LaraItem->hitPoints < hitPoints && Lara.air > 0);
		hitPoints = LaraItem->hitPoints;
		if( isInjured ) {
			r = 255;
			if( hitPoints > 500 ) {
				g = 255 * (hitPoints - 500) / 500;
			}
		} else {
			if( Lara.water_status == LWS_Underwater ) {
				if( hitPoints > 0 && Lara.air > 0 ) {
					if( Lara.air > 1350 ) {
						g = 255;
						b = 128 + 127 * (1800 - Lara.air) / 450;
					} else if( Lara.air > 450 ) {
						g = 255 * (Lara.air - 450) / 900;
						b = 255;
					} else {
						r = 127 * (450 - Lara.air) / 450;
						b = 255;
					}
				} else {
					r = 128 + 127 * (1000 - hitPoints) / 1000;
					b = 255 * hitPoints / 1000;
				}
			} else {
				if( hitPoints > 500 ) {
					r = 255 * (1000 - hitPoints) / 500;
					g = 255;
				} else {
					r = 255;
					g = 255 * hitPoints / 500;
				}
			}
			if( hitPoints > 0 && Lara.air > 0 && Lara.gun_status != LGS_Ready ) {
				r /= 3;
				g /= 3;
				b /= 3;
			} else {
				r = r*2/3;
				g = g*2/3;
				b = b*2/3;
			}
		}
	}
	if( isInjured ) {
		LedColor = RGB_MAKE(r, g, b);
	} else if( LedColor != RGB_MAKE(r, g, b) ) {
		r = BlendLedColor(RGB_GETRED(LedColor), r);
		g = BlendLedColor(RGB_GETGREEN(LedColor), g);
		b = BlendLedColor(RGB_GETBLUE(LedColor), b);
		LedColor = RGB_MAKE(r, g, b);
	}

	SetJoystickOutput(motor[0], motor[1], LedColor);
}

#endif // FEATURE_INPUT_IMPROVED
