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

#ifndef CLICKLOCKD_H
#define CLICKLOCKD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

/* Defaults */
#define BTN_TIMEOUT          2
#define POLLFD_MAX          64
#define PID_FILE            "/var/run/clicklockd.pid"
#define DEFAULT_UINPUT_DEV  "/dev/uinput"

/* Event loop */
int  event_loop(const struct timeval *timeout);

/* Virtual Mouse */
#define CLICKLOCK_VMOUSE_NAME      "ClickLock Virtual Mouse"
#define CLICKLOCK_VMOUSE_VENDOR    0XC1
#define CLICKLOCK_VMOUSE_PRODUCT   0XC1
#define CLICKLOCK_VMOUSE_VERSION   0XC1

int  virtual_mouse_create(const char *uinput_path);
void virtual_mouse_destroy(void);
void send_btn_event(int down);

/* Utils */
int  daemon_init();
int  create_pid_file(const char *pidfile);
void usage(FILE *out, const char *pname);

/* Logger */
#define log_errno(err) syslog(LOG_ERR, "%s: %s",__func__, strerror(err))
#define log_error(errstr) syslog(LOG_ERR, "%s: %s",__func__, errstr)
#define log_warning(...) syslog(LOG_WARNING, __VA_ARGS__) 
#define log_info(...) syslog(LOG_INFO, __VA_ARGS__) 

#endif
