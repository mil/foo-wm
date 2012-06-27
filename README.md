fifo-wm: FIFO Window Manager
============================
The premise is quite simple, a window manager which is configured and used exclusivly through talking to a FIFO. 

This should be considered unusable pre-alpha software.

FIFO Commands
-------------
The FIFO specified in src/config.h must be made with `mkfifo wm-fifo`. Once the FIFO is created you may use several commands to interact with the window manager by simply echoing to the FIFO like so:

`echo "dump tree" > wm-fifo`

Commands that may be sent to the FIFO are listed below:

### dump
Dumps a tree view of the root container to STDOUT.
```
[0]=> Container (Vertical)
	Client
	[1]=> Container (Horizontal)
		Client
		Client
		Client
```

### layout [vertical/horizontal]
Updates the layout of the current container. Currenly only vertical and horizontal layouts are supported, although future layouts planned include: floating, max, and tabbed.

### containerize
If the current client is in a container with 2 or more other clients, containerize creates  new container and parents the current client into this new container.
