/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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

#ifndef OPTION_H_INCLUDED
#define OPTION_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl do_inventory_options(INVENTORY_ITEM *item); // 0x0044EE80
void __cdecl do_passport_option(INVENTORY_ITEM *item); // 0x0044EF90
void __cdecl do_gamma_option(INVENTORY_ITEM *item); // ----------
void __cdecl do_detail_option(INVENTORY_ITEM *item); // 0x0044F5E0
void __cdecl do_sound_option(INVENTORY_ITEM *item); // 0x0044F8C0
void __cdecl do_compass_option(INVENTORY_ITEM *item); // 0x0044FD60
void __cdecl FlashConflicts(); // 0x0044FE20
void __cdecl DefaultConflict(); // 0x0044FEA0
void __cdecl do_control_option(INVENTORY_ITEM *item); // 0x0044FEE0
void __cdecl S_ShowControls(); // 0x004505F0
void __cdecl S_ChangeCtrlText(); // 0x00450AC0
void __cdecl S_RemoveCtrlText(); // 0x00450B60

#endif // OPTION_H_INCLUDED
