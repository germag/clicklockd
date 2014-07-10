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
#include "fsm.h"
#include <poll.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <strings.h>
#include <signal.h>
#include <libudev.h>
#include <regex.h>

static int free_pos;
static struct pollfd idevs[POLLFD_MAX];

static inline unsigned int nbytes(unsigned int n) {
    return ((((n) - 1) / 8) + 1);
}
static inline unsigned int test_bit(const unsigned char* bitmask, int bit) {
    return bitmask[bit / 8] & (1 << (bit % 8));
}

static int valid_input_device(int devfd) {
    int ok = 0;
    struct stat devst;
    struct input_id iid;
    unsigned char ev_bits[nbytes(EV_MAX)];
    unsigned char code_bits[nbytes(KEY_MAX)];

    assert(devfd >= 0);

    /* Test char device */
    if (fstat(devfd, &devst) < 0) goto exit; 
    if (!S_ISCHR(devst.st_mode)) goto exit;                

    /* Test buttons and relative axes */
    bzero(&ev_bits, sizeof(ev_bits));
    ioctl(devfd, EVIOCGBIT(0, EV_MAX), ev_bits); 
    if (!(test_bit(ev_bits, EV_KEY) && test_bit(ev_bits, EV_REL))) goto exit;

    bzero(&code_bits, sizeof(code_bits));
    ioctl(devfd, EVIOCGBIT(EV_KEY, KEY_MAX), code_bits); 
    if (!test_bit(code_bits, BTN_MOUSE)) goto exit;

    bzero(&code_bits, sizeof(code_bits));
    ioctl(devfd, EVIOCGBIT(EV_REL, REL_MAX), code_bits); 
    if (!(test_bit(code_bits, REL_X) && test_bit(code_bits, REL_Y))) goto exit;
    
    /* Test clicklock virtual mouse */
    if (ioctl(devfd, EVIOCGID, &iid) < 0) goto exit;
    if (iid.vendor == CLICKLOCK_VMOUSE_VENDOR 
            && iid.product == CLICKLOCK_VMOUSE_PRODUCT
            && iid.version == CLICKLOCK_VMOUSE_VERSION) goto exit;
    ok = 1; 

exit:
    errno = 0;
    return ok;
}

static int open_device(const char *devnode) {
    int fd;
    regex_t regex;

    assert(devnode != NULL);

    if (regcomp(&regex, "event[0-9]*$", REG_NOSUB)) goto exit1;
    if (regexec(&regex, devnode, 0, NULL, 0) == REG_NOMATCH) goto exit1;

    if ((fd = open(devnode, O_RDONLY | O_NONBLOCK)) < 0) goto exit1;
    if (!valid_input_device(fd)) goto exit2;

    return fd;
exit2:
    close(fd);
exit1:
    return -1;
}

static int scan_devices(struct udev *udev) {
    struct udev_list_entry *devices;
    struct udev_list_entry *dev_entry;
    struct udev_enumerate *udev_enum;

    assert(udev != NULL);
    if ((udev_enum = udev_enumerate_new(udev)) == NULL) {
        log_error("udev enumerator == NULL");
        return -1;
    }

    udev_enumerate_add_match_subsystem(udev_enum, "input");
    udev_enumerate_add_match_sysname(udev_enum, "event[0-9]*");
    udev_enumerate_scan_devices(udev_enum);
    devices = udev_enumerate_get_list_entry(udev_enum);

    udev_list_entry_foreach(dev_entry, devices) {
        int fd;
        const char *syspath;
        const char *devnode;
        struct udev_device *idev;

        syspath = udev_list_entry_get_name(dev_entry);
        idev = udev_device_new_from_syspath(udev, syspath);
        devnode = udev_device_get_devnode(idev);
        if ((fd = open_device(devnode)) < 0) continue;
        log_info("Device found: %s", devnode);
        idevs[free_pos].fd = fd;
        free_pos++;
        if (free_pos >= POLLFD_MAX) break;
    }

    udev_enumerate_unref(udev_enum);
    return 0;
}

static void add_device(const char *devnode) {
    int fd; 

    assert(devnode != NULL);

    if (free_pos >= POLLFD_MAX) { 
        log_warning("Not space left for device: %s", devnode);
        return;
    } 

    if((fd = open_device(devnode)) != -1) {
        log_info("New device found: %s", devnode);
        idevs[free_pos].fd = fd;    
        idevs[free_pos].revents = 0;    
        free_pos++;
    }
}

static int do_event_loop(struct udev_monitor *udev_mon, int timeout) {
    int i;

    assert(udev_mon != NULL);

    while (1) {
        if (poll(idevs, free_pos, -1) < 0) goto exit;

        /* Signals */
        if ((idevs[0].revents & POLLIN)) { 
            struct signalfd_siginfo sfdi;
            if (read(idevs[0].fd, &sfdi, sizeof(sfdi)) == sizeof(sfdi)) {
               if (sfdi.ssi_signo == SIGTERM) break;  /* Got SIGTERM exiting */
               log_warning("Unexpected signal: %d", sfdi.ssi_signo);
            }
        }

        /* Devices events */
        for (i = 2; i < free_pos ; i++) {
            /* Device disconected or error */
            if (idevs[i].revents & (POLLHUP | POLLERR | POLLNVAL)){ 
                free_pos--;
                idevs[i].fd = idevs[free_pos].fd;
                idevs[i].revents = idevs[free_pos].revents;
            }

            /* Input event */
            if ((idevs[i].revents & POLLIN)) { 
                struct input_event ev;
                if ((read(idevs[i].fd, &ev, sizeof(ev)) == sizeof(ev))
                        && (ev.type == EV_KEY && ev.code == BTN_MOUSE)) {
                    state(&ev, timeout);
                }
            }
        }

        /* Udev events */
        if ((idevs[1].revents & POLLIN)) { 
            const char *devnode;
            struct udev_device *udev_dev;
            udev_dev = udev_monitor_receive_device(udev_mon);
            devnode = udev_device_get_devnode(udev_dev);
            if (devnode && (strncmp(udev_device_get_action(udev_dev), "add", 3) == 0))  {
                add_device(devnode);
            }
            udev_device_unref(udev_dev);
        }
    }

    return 0;
exit:
    log_errno(errno);
    return -1;
}

int event_loop(int timeout) {
    int i;
    int error;
    sigset_t sigset;
    struct udev *udev;
    struct udev_monitor *udev_mon = NULL;
    
    /* Init idevs */
    for (i = 0 ; i < POLLFD_MAX ; i++) {
        idevs[i].fd = -1;
        idevs[i].events = POLLIN;
    }

    /* First fd: signal handling */
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGTERM);
    idevs[0].fd = signalfd(-1, &sigset, SFD_NONBLOCK); 
    if (idevs[0].fd < 0) log_errno(errno); /* If error... go on! */

    if ((udev = udev_new()) == NULL) goto exit;

    /* Second fd: Udev monitor */
    udev_mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(udev_mon, "input", NULL);
    udev_monitor_enable_receiving(udev_mon);
    idevs[1].fd = udev_monitor_get_fd(udev_mon);

    /* Third onwards: Input devices */
    free_pos = 2;
    if (scan_devices(udev) < 0) goto exit;

    error = do_event_loop(udev_mon, timeout);
    
    /* Clean up */
    udev_monitor_unref(udev_mon);
    udev_unref(udev);
    for (i = 0 ; i < free_pos ; i++) {
        close(idevs[i].fd);
    }

    return error;
exit:
    log_errno(errno);
    return -1;
}
