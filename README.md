# Clicklockd

Clicklockd enables you to highlight or drag without holding down the mouse button.
This feature allows you to hold the mouse button for a few seconds, move the mouse
to the new location, and then click it again. The effect is the same as a drag and
drop but without having to hold the mouse button for a long time


## Requirements
  - Linux with gcc
  - libudev-dev (libgudev-devel in CentOS/Fedora)

For Ubuntu:
```sh
sudo apt install build-essential libudev-dev

```

## Options

**-h**  
Print help and exit.

**-t timeout[s|ms]**  
Set how many seconds (s) or milliseconds (ms) you need to hold down a mouse or
trackball button before your click is locked. You can use real numbers or integers.
If the unit of time is not specified, clicklockd assumes seconds (s).
Default is 2 seconds.

Ex: -t 1.5s (or -t 1.5) is the same as -t 1500ms

**-b**  
Run in the background (as a Unix daemon).

**-p pid file**  
This option tells clicklockd to use the specified file as its pidfile.  If the file exists, it will be removed and over-written.  Default is /var/run/clicklockd.pid.

**-u uinput device**  
Set uinput device. Requires a 2.6 kernel with uinput support. Default is /dev/uinput.


