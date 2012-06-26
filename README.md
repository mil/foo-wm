fifo-wm: FIFO Window Manager
============================
The premise is quite simple, a window manager which is configured and used exclusivly through talking to a FIFO. 

This should be considered unusable pre-alpha software.

However, if you wish to use fifo-wm in its current state you will need to make a FIFO named wm-fifo like so: `mkfifo wm-fifo`. This name can be changed via the config.h.
