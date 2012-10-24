foo-wm: Foo Window Manager
============================
The basic premises of Foo Window Manager are:

1. Provide a tree data structure in which you can organize windows.
2. Provide a socket for IPC along with a basic DSL for manipulating and traversing the tree.

foo-wm was previously named fifo-wm and used a FIFO for IPC. The switch over to a socket was in need for provide responses to IPC commands. Foo-wm may not be this window manager's final name.

foo-wm should be considered unusable pre-alpha software. I use foo-wm on a daily basis though that doesn't mean you should. Foo-wm doesn't have support for many basic window managment functions (ICCCM/EWMH). If you're interested in contributing, contact me. Below is an explanation of the basic structure of foo-wm if you want to hack on it.

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

Foo-WM's Socket
---------------
The location and name of the socket, is to be specified in `src/config.h`. If no SOCKET_NAME, is specified then a name will be automatically generated based on the PID of the socket in the form of `foo-wm-pid.socket`.

foo-wm-c
--------
`foo-wm-c` is a command line client to interface with foo-wm's socket, although similar interfaces could be made in any language with a socket class.

*Usage:* `foo-wm-c socketpath "ipc command"`

Properties
----------
IPC Commands
------------
All user interaction with foo-wm takes place through its IPC via the socket. The one exception to this is mouse actions which are bound by the IPC.

### get 
**Usage:**
`get tree|view|focus`

Gets data from the WM.
*Gettable Properties*:
tree (returns json tree starting from root node)
view (returns json tree starting from view node)
focus (returns json tree starting from focus node)

### set 
**Usage:**
`set client_padding 10`

Sets WM data.
*Settable Properties*:
client_border_width (integer)
container_padding (integer)
client_padding (integer)
screen_padding_top (integer)
screen_padding_left (integer)
screen_padding_right (integer)
screen_padding_bottom (integer)


### layout 
**Usage:**
`layout vertical|horizontal|grid|max|tabbed|float|freefloat`

Updates the layout of the current container. Currenly only vertical and horizontal layouts are supported, although future layouts planned include: floating, max, and tabbed.

### zoom 
**Usage:**
`zoom -+delta`

Controls the view of the screen. Using zoom with a negative delta will zoom out based on the current view node. Using zoom with a positive number will zoom the screen in one level closer to the focused node.

### focus 
**Usage:**
`focus brother|pc -+delta`
`focus direction left|up|right|down`

The focus command switches the node which is currently focused.

### shift 
**Usage:**
`shift brother|pc -+ delta`

Shift the currently focused node. This wraps around if you attempt to move to a node that doesn't exist. Note, shifting to a positive pc won't do anything, shift pc should only be used with a negative delta to shift a client up toward its parent.

### containerize
**Usage:**
`containerize`

If the current client is in a container with 2 or more other clients, containerize creates  new container and parents the current client into this new container.

### mark
**Usage:**
`mark markName`

Marks the current View Node. The viewnode may be restored with the `jump` command.

### jump
**Usage:**
`jump markName`

Jumps to the predefined marked node.

### kill
**Usage:**
`kill`

Kills the currently focused node (and any nodes that are the focused node's children).

Inspiration
-----------
Inspirations include: 
DWM, monsterwm, catwm, dminiwm, i3, herbstluftwm, wmii, and tinywm

Contributing
------------
Foo Window Manager is very much open to contribution. Please get in touch via email if you are interested or just take a stab at something in `TODO.md` and send a pull request.
