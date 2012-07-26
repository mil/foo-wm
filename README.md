fifo-wm: FIFO Window Manager
============================
The premise is quite simple, a window manager which is configured and used exclusivly through talking to a FIFO. 

This should be considered unusable pre-alpha software. I'm using fifo-wm on a daily basis though it doesn't have support for many basic window managment functions (ICCCM/EWMH). Regardless, here is some explanations of how things are currently set up.

The Tree
--------
All windows are stored as nodes within a tree data structure. The tree is made up of two fundamental types of nodes. 

1. **Client** Nodes
	- Holds: 1 X11 Window

2. **Container** Nodes
	- Holds: 1 or more **Client** Nodes
	- Holds: 1 or more **Container** Nodes**
	- Has a Property called *Layout* Which May Be Set To:
		* *Vertical*: Children are arranged side by side vertically
		* *Horizontal*: Children are arranged horizontally
		* *Grid*: Children are arranged to be equal size in a grid	
		* *Max*: One child takes up the full view of the container 
			- The other children are hidden
		* *Tabbed*: Exactly like Max, only there is a visual tab indication
		* *Float*: Clients are floated, but bound by the container
		* *Freefloat*: Clients are free to float, even on top of the current view

FIFO Commands
-------------
The FIFO specified in src/config.h must be made with `mkfifo wm-fifo`. Once the FIFO is created you may use several commands to interact with the window manager by simply echoing to the FIFO like so:

`echo "dump" > wm-fifo`

Commands that may be sent to the FIFO are listed below:

### dump
Dumps a tree view of the root container to STDERR.
```
[0]=> Container (Vertical)
	Client
	[1]=> Container (Horizontal)
		Client
		Client
		Client
```

### layout *[vertical/horizontal/grid/max/tabbed/float/freefloat]*
Updates the layout of the current container. Currenly only vertical and horizontal layouts are supported, although future layouts planned include: floating, max, and tabbed.

### zoom *[(-+)delta]*
Controls the view of the screen. Using zoom with a negative delta will zoom out based on the current view node. Using zoom with a positive number will zoom the screen in one level closer to the focused node.


### focus cycle *[(-+)delta]*
The focus command switches the client which is currently focused.
Using focus cycle allows you to cycle through client in your current container.

### move cycle *[(+-)delta]*
Moves the current node (focused or selected) within its container delta positions.

### select *[parent/child]*
Selects the parent container


### containerize
If the current client is in a container with 2 or more other clients, containerize creates  new container and parents the current client into this new container.


Inspiration
------
Inspirations include: 
DWM, monsterwm, catwm, dminiwm, i3, herbstluftwm, wmii, and tinywm
