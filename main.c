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
#include <sys/time.h>
#include <signal.h>
#include <limits.h>
#include <math.h>

int timer_parse(const char *str, struct timeval *timeout);


int main(int argc, char* argv[]) {
    int opt;
    int error = 1;
    int daemonize;
    sigset_t sigset;
    struct timeval timeout = {.tv_sec = BTN_TIMEOUT_SEC, .tv_usec = BTN_TIMEOUT_USEC};
    char *pidfile = PID_FILE;
    char *uinput_device = DEFAULT_UINPUT_DEV;
    
    daemonize = 0;
    while((opt = getopt(argc, argv, "hbt:p:u:")) != -1) {
        switch(opt) {
            case 'b': 
                daemonize = 1;
                break;
            case 't':
                error = timer_parse(optarg, &timeout);
                if (error) goto exit2;
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

    log_info("Init with timeout: %lds %ldus", timeout.tv_sec, timeout.tv_usec);
    if (create_pid_file(pidfile) < 0) goto exit1;

    if (daemonize) {
        if (daemon_init() < 0) goto exit2;
    }

    if (virtual_mouse_create(uinput_device)< 0) goto exit2;
    error = event_loop(&timeout);
    virtual_mouse_destroy();
    
exit2:
    unlink(pidfile);
exit1:
    log_info("Exit");
    if (!error) exit(EXIT_SUCCESS);
    exit(EXIT_FAILURE);
}

int timer_parse(const char *str, struct timeval *timeout)
{
    errno = 0;
    char *str_last;
    double parameter_value = strtod(str, &str_last);

    if (str_last == str) {
        fprintf(stderr, "Invalid parameter: timeout has no digits\n");
        return EXIT_FAILURE;
    }

    if ((parameter_value == HUGE_VAL || parameter_value == -HUGE_VAL) && errno == ERANGE) {
        fprintf(stderr, "Invalid parameter: '%s' timeout out of  range\n", str);
        return EXIT_FAILURE;
    }

    if (parameter_value < 0) {
        fprintf(stderr, "Invalid parameter: negative timeout\n");
        return EXIT_FAILURE;
    }

    int factor = 0;
    /* Seconds is the default unit of time */
    if(*str_last == '\0' || strcmp(str_last, "s") == 0) {
        factor = 1000000;
    } else if (*str_last != '\0' && strcmp(str_last, "ms")==0) {
        factor = 1000;
    } else {
        fprintf(stderr, "Invalid parameter: unknown unit of time '%s', try -h\n", str_last);
        return EXIT_FAILURE;
    }

    /* It's bit conservative it will falsely reject some (big) valid values.
     * Since the timeout will be relatively small, this should be fine.
     */
    if (!(parameter_value < (LONG_MAX/factor))) {
            fprintf(stderr, "Invalid parameter: '%s' timeout out of  range\n", str);
            return EXIT_FAILURE;
    }

    long timeout_usec = parameter_value * factor;

    long seconds = timeout_usec / 1000000;
    long useconds = timeout_usec % 1000000;

    static_assert(((time_t) -1) < 0, "clicklockd assumes time_t is signed");
    static_assert(((time_t) .9) == 0, "clicklockd assumes time_t is integer type");
    static_assert(((suseconds_t) -1) < 0, "clicklockd assumes suseconds_t is signed");
    static_assert(((suseconds_t) .9) == 0, "clicklockd assumes suseconds_t is integer type");
    static_assert(sizeof(time_t) >= sizeof(long), "clicklockd assumes that time_t is at least a long int");
    static_assert(sizeof(suseconds_t) >= sizeof(long), "clicklockd assumes that suseconds_t is at least a long int");

    timeout->tv_sec = seconds;
    timeout->tv_usec = useconds;

    return EXIT_SUCCESS;
}
