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

#include "fsm.h"

/* Simple (and not quite right) FSM  */

static time_t t_down;

/* States  definitions */
static void state_init_unlock(const struct input_event *ev, int timeout);
static void state_up_unlock(const struct input_event *ev, int timeout);
static void state_down_unlock(const struct input_event *ev, int timeout);
static void state_up_lock(const struct input_event *ev, int timeout);

/* Initial state */
state_fn_ptr state = state_init_unlock;

/* States  implementations */
static void state_init_unlock(const struct input_event *ev, int timeout) {
    assert(ev != NULL);

    if (IS_DOWN_EVENT(ev)) {
        t_down = EVENT_TIME(ev);
        state = state_down_unlock;
        send_btn_event(GET_EVENT(ev));
    } else { 
        state = state_up_unlock; // Release event
    }
}

static void state_up_unlock(const struct input_event *ev, int timeout) {
    assert(ev != NULL);

    if (IS_DOWN_EVENT(ev)) {
        t_down = EVENT_TIME(ev);
        state = state_down_unlock;
        send_btn_event(GET_EVENT(ev));
    } 
}

static void state_down_unlock(const struct input_event *ev, int timeout) {
    assert(ev != NULL);

    if (IS_RELEASE_EVENT(ev)) {
        if ((EVENT_TIME(ev) - t_down) >= timeout) {
            state = state_up_lock; // Click locked
        } else {
            state = state_up_unlock;
            send_btn_event(GET_EVENT(ev));
        }
    }
}

static void state_up_lock(const struct input_event *ev, int timeout) {
    assert(ev != NULL);

    if (IS_RELEASE_EVENT(ev)) {
        state = state_up_unlock; // Click unlocked
        send_btn_event(GET_EVENT(ev));
    }
}

