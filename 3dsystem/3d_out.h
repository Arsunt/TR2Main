/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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

#ifndef _3DOUT_H_INCLUDED
#define _3DOUT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl draw_poly_line(__int16 *bufPtr); // 0x00402960
void __cdecl draw_poly_flat(__int16 *bufPtr); // 0x00402B00
void __cdecl draw_poly_trans(__int16 *bufPtr); // 0x00402B40
void __cdecl draw_poly_gouraud(__int16 *bufPtr); // 0x00402B80
void __cdecl draw_poly_gtmap(__int16 *bufPtr); // 0x00402BC0
void __cdecl draw_poly_wgtmap(__int16 *bufPtr); // 0x00402C00
BOOL __cdecl xgen_x(__int16 *bufPtr); // 0x00402C40
BOOL __cdecl xgen_xg(__int16 *bufPtr); // 0x00402D20
BOOL __cdecl xgen_xguv(__int16 *bufPtr); // 0x00402E70
BOOL __cdecl xgen_xguvpersp_fp(__int16 *bufPtr); // 0x00403090
void __cdecl gtmap_persp32_fp(int y0, int y1, BYTE *texPage); // 0x00403320
void __cdecl wgtmap_persp32_fp(int y0, int y1, BYTE *texPage); // 0x004042F0
void __cdecl draw_poly_gtmap_persp(__int16 *bufPtr); // 0x004057C0
void __cdecl draw_poly_wgtmap_persp(__int16 *bufPtr); // 0x00405800
void __fastcall flatA(int y0, int y1, BYTE colorIdx); // 0x00457564
void __fastcall transA(int y0, int y1, BYTE depthQ); // 0x004575C5
void __fastcall gourA(int y0, int y1, BYTE colorIdx); // 0x004576FF
void __fastcall gtmapA(int y0, int y1, BYTE *texPage); // 0x0045785F
void __fastcall wgtmapA(int y0, int y1, BYTE *texPage); // 0x00457B5C

#endif // _3DOUT_H_INCLUDED
