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
#include <limits.h>
#include <signal.h>

int main(int argc, char* argv[]) {
    int opt;
    int error = 1;
    int daemonize;
    sigset_t sigset;
    int timeout = BTN_TIMEOUT;
    char *pidfile = PID_FILE;
    char *uinput_device = DEFAULT_UINPUT_DEV;
    
    daemonize = 0;
    while((opt = getopt(argc, argv, "hbt:p:u:")) != -1) {
        switch(opt) {
            case 'b': 
                daemonize = 1;
                break;
            case 't':
                timeout = strtol(optarg, NULL, 10);
                if (timeout == LONG_MAX) {
                    fprintf(stderr, "Incorrect timeout\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                pidfile = strdup(optarg);
                break;
            case 'u':
                uinput_device = strdup(optarg);
                break;
            case 'h': /* fall through */
            default:
                usage(stdout, argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /* Block all signals */
    sigfillset(&sigset);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    log_info("Init with timeout: %d", timeout);
    if (create_pid_file(pidfile) < 0) goto exit1;

    if (daemonize) {
        if (daemon_init() < 0) goto exit2;
    }

    if (virtual_mouse_create(uinput_device)< 0) goto exit2;
    error = event_loop(timeout); 
    virtual_mouse_destroy();
    
exit2:
    unlink(pidfile);
exit1:
    log_info("Exit");
    if (!error) exit(EXIT_SUCCESS);
    exit(EXIT_FAILURE);
}

