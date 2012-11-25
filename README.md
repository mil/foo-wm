foo-wm: Foo Window Manager
============================
Foo Window Manager is a minimalistic window manager that does two things and two things only:

1. Provides a tree data structure in which you can organize windows.
2. Provides a socket for IPC along with a basic DSL for manipulating and traversing the tree.

As a results of *only* doing these two things, `foo-wm` most likely works a bit differently than other window managers you may be acquainted with. Mainly, strangest of all to the newcomer, `foo-wm` **does not provide**: workspaces or keybindings. The former can be emulated through foo-wm's tree and the latter can be provided through any X keybinding program (such as `xbindkeys`).

> A foreword: `foo-wm` should be considered unusable alpha software. I use `foo-wm` on a daily basis, although that doesn't mean you should (unless you are interested in contributing). `foo-wm` doesn't have full support for many basic window managment functions (ICCCM/EWMH). Although, ICCCM/EWMH support is a top priority currently.

### The Makefile will generate:
`foo-wm`: The window manager itself which can be thrown in `.xinitrc`. It is reccomeneded that you start `foo-wm` with a keybindings manager, such as `.xbindkeys`, as it will be very diffucult to interact with `foo-wm`'s socket otherwise. The location and name of the socket, is to be specified in `src/config.h`. If no `SOCKET_NAME`, is specified then a name will be automatically generated based on the PID of the socket in the form of `/tmp/foo-wm-pid.socket`.

`foo-wm-c`: This is a simple and dumb command line client for interacting with `foo-wm`. `foo-wm-c` takes the syntax of `foo-wm-c '/tmp/your-foo-socket.socket' 'ipc command'`. Any language that provides a UNIX socket interface can be used to communicate with `foo-wm`, `foo-wm-c` is just a simple C client for doing so. Additionally a ruby client for interacting with `foo-wm`'s socket can be found at `scrips/client.rb`

The Tree
--------
All operations in `foo-wm` are related to manipulating and the traversal of the tree.  All nodes within the tree are one of two types.

1. **Client** Nodes
	- Just a single  X11 Window
2. **Container** Nodes
    - Holds one or more other **Client** or **Container** Nodes
    - Has a *Layout* property which may be changed with the *layout* IPC command

At any given time using `foo-wm` there are three essential nodes within the tree at play:

1. **Root** Node
    - The top of the tree
    - Issuing a zoom command with a negative &Delta; approximates this node
2. **Focus** Node
    - The node which is currently manipulated on by using IPC commands
    - Issuing a zoom command with a positive &Delta; approximates  this node
3. **View** Node
    - The node which the screen is currently viewing


The Foo-WM IPC DSL Commands
---------------------------
All user interaction with `foo-wm` takes place through its IPC via the socket `foo-wm` created on start.

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

- `vertical`: Children are arranged side by side vertically
- `horizontal`: Children are arranged horizontally
- `grid`: Children are arranged to be equal size in a grid	
- `max`: One child takes up the full view of the container 
-   * The other children are hidden
- `tabbed`: Exactly like Max, only there is a visual tab indication
- `float`: Clients are floated, but bound by the container
- `freefloat`: Clients are free to float, even on top of the current view

Updates the layout of the current container. Currenly only vertical and horizontal layouts are supported, although future layouts planned include: floating, max, and tabbed.

### zoom 
**Usage:**
`zoom -+&Delta;`

Controls the view of the screen. Using zoom with a negative &Delta; will zoom out based on the current view node. Using zoom with a positive number will zoom the screen in one level closer to the focused node.

### focus 
**Usage:**
`focus brother|pc -+&Delta;`
`focus direction left|up|right|down`

The focus command switches the node which is currently focused.

### shift 
**Usage:**
`shift brother|pc -+ &Delta;`

Shift the currently focused node. This wraps around if you attempt to move to a node that doesn't exist. Note, shifting to a positive pc won't do anything, shift pc should only be used with a negative &Delta; to shift a client up toward its parent.

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
