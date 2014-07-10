# Clicklockd

ClickLock enables you to highlight or drag without holding down the mouse 
button. Allows you to hold the mouse button for a few seconds, move 
the mouse to the new location, and then click it again. The effect is the
same as a drag and drop but without having to hold the mouse button for a 
long time.

## Options

**-h**  
Print help and exit.

**-t timeout**  
Set how many seconds you need to hold down a mouse or trackball button before your click is locked. Default is 2 seconds.

**-b**  
Run in the background (as a Unix daemon).

**-p pid file**  
This option tells clicklockd to use the specified file as its pidfile.  If the file exists, it will be removed and over-written.  Default is /var/run/clicklockd.pid.

**-u uinput device**  
Set uinput device. Requires a 2.6 kernel with uinput support. Default is /dev/uinput.


