/*
   ClickLock: you can highlight or drag without holding the mouse button

   Copyright (C) 2014  German Maglione <germanm@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef CLICKLOCK_FSM_H
#define CLICKLOCK_FSM_H

#include "clicklockd.h"

#define IS_DOWN_EVENT(ev) ((ev)->value)
#define IS_RELEASE_EVENT(ev) (!(ev)->value)
#define EVENT_TIME(ev) ((ev)->time.tv_sec)
#define GET_EVENT(ev) ((ev)->value)

typedef void (*state_fn_ptr)(const struct input_event *, int);
extern  state_fn_ptr state;

#endif
