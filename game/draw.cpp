/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
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
#include "game/draw.h"
#include "3dsystem/3d_gen.h"
#include "global/vars.h"

void __cdecl phd_RotYXZsuperpack(UINT16 **pptr, int index) {
	for( int i = 0; i < index; ++i ) {
		if( (**pptr >> 14) == 0 )
			*pptr += 2;
		else
			++*pptr;
	}

	switch( **pptr >> 14 ) {
		case 0 :
			phd_RotYXZpack(((UINT16)(*pptr)[0] << 16) + (UINT16)(*pptr)[1]);
			*pptr += 2;
			break;
		case 1 :
			phd_RotX(**pptr << 6);
			++*pptr;
			break;
		case 2 :
			phd_RotY(**pptr << 6);
			++*pptr;
			break;
		case 3 :
			phd_RotZ(**pptr << 6);
			++*pptr;
			break;
	}
}

/*
 * Inject function
 */
void Inject_Draw() {
	INJECT(0x0041B980, phd_RotYXZsuperpack);
}
