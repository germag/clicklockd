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
#include <fcntl.h>

void usage(FILE *out, const char *pname) {
    assert(pname != NULL);
    fprintf(out, "Usage: %s [OPTIONS]\n"
                 "Clicklockd enables you to highlight or drag without holding down the mouse button.\n"
                 "This feature allows you to hold the mouse button for a few seconds, move the mouse\n"
                 "to the new location, and then click it again. The effect is the same as a drag and\n"
                 "drop but without having to hold the mouse button for a long time\n\n"
                 "Options:\n"
                 "\t-h\t\t\t\tShow this message\n"
                 "\t-b\t\t\t\tSend to background (daemonize)\n"
                 "\t-t timeout[s|ms]\t\tSet timeout in seconds (s) or milliseconds (ms), accepts both real numbers and integers (default: %ds)\n"
                 "\t\t\t\t\tEx: -t 2.5s is the same as -t 2500ms\n"
                 "\t\t\t\t\tNote: if the unit of time is not specified, clicklockd assumes seconds (s).\n"
                 "\t-p pidfile\t\t\tSet pid file (default: %s)\n"
                 "\t-u uinput device\t\tSet uinput device (default: %s)\n", 
                 pname, BTN_TIMEOUT_SEC, PID_FILE, DEFAULT_UINPUT_DEV);
}

int daemon_init() {
    pid_t pid ;
    
    if ((pid = fork()) < 0) goto exit;
    if (pid != 0) exit(EXIT_SUCCESS); /* Parent die */ 
    
    /* child */
    if (setsid() < 0) goto exit;
    if (chdir("/") < 0) goto exit;
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return 0;
exit:
    log_errno(errno);
    return -1;
}

int create_pid_file(const char *pidfile) {
    FILE *pidf;
    pid_t pid;

    assert(pidfile != NULL);

    if ((pidf = fopen(pidfile, "w")) == NULL) goto exit;
    pid = getpid();
    fprintf(pidf,"%d\n", pid);
    fclose(pidf);
exit:
    if (errno) {
        log_errno(errno);
        return -1;
    }
    return 0;
}

