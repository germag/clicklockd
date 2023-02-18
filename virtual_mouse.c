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

#include "clicklockd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <strings.h>

/* Private state and functions */
static int uifd; /* Clicklock device file descriptor */

static void send_ev(int fd, int type, int code, int value);

/* Public functions */

int vmouse_create(const char *uinput_path) {
    struct uinput_user_dev uidev;

    assert(uinput_path != NULL);

    if ((uifd = open(uinput_path, O_WRONLY | O_NONBLOCK)) < 0) goto exit0;

    if (ioctl(uifd, UI_SET_EVBIT,  EV_SYN)     < 0) goto exit1;
    if (ioctl(uifd, UI_SET_EVBIT,  EV_KEY)     < 0) goto exit1;
    if (ioctl(uifd, UI_SET_KEYBIT, BTN_LEFT)   < 0) goto exit1;
    if (ioctl(uifd, UI_SET_KEYBIT, BTN_RIGHT)  < 0) goto exit1;
    if (ioctl(uifd, UI_SET_KEYBIT, BTN_MIDDLE) < 0) goto exit1;
    if (ioctl(uifd, UI_SET_EVBIT,  EV_REL)     < 0) goto exit1;
    if (ioctl(uifd, UI_SET_RELBIT, REL_X)      < 0) goto exit1;
    if (ioctl(uifd, UI_SET_RELBIT, REL_Y)      < 0) goto exit1;
    if (ioctl(uifd, UI_SET_RELBIT, REL_WHEEL)  < 0) goto exit1;

    bzero(&uidev,sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, CLICKLOCK_VMOUSE_NAME);
    uidev.id.bustype = BUS_VIRTUAL;
    uidev.id.vendor  = CLICKLOCK_VMOUSE_VENDOR;
    uidev.id.product = CLICKLOCK_VMOUSE_PRODUCT;
    uidev.id.version = CLICKLOCK_VMOUSE_VERSION;
    if(write(uifd, &uidev, sizeof(uidev)) <= 0) goto exit1;

    if(ioctl(uifd, UI_DEV_CREATE) < 0) goto exit1;

    return 0;
exit1:
    close(uifd);
exit0:
    log_errno(errno); 
    return -1;
}

void vmouse_destroy(void) {
    ioctl(uifd, UI_DEV_DESTROY);
    close(uifd);
}

void vmouse_send_btn_event(int down) {
    send_ev(uifd, EV_KEY, BTN_MOUSE, down);
    send_ev(uifd, EV_SYN, SYN_REPORT, 0);
}

static void send_ev(int fd, int type, int code, int value) {
    struct input_event ev;
    ev.type  = type;
    ev.code  = code;
    ev.value = value;
    gettimeofday(&ev.time, NULL);
    if ((write(fd, &ev, sizeof(ev))) <= 0) log_errno(errno);
}

