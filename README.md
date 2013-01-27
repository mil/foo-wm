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
    - Issuing a zoom command with a negative delta approximates this node
2. **Focus** Node
    - The node which is currently manipulated on by using IPC commands
    - Issuing a zoom command with a positive delta approximates  this node
3. **View** Node
    - The node which the screen is currently viewing


The FOO-DSL and the Zooming Nature
---------------------------
Now that you understand that all foo-wm does is maintain a tree of windows, now you'll want to know how to mainuplate the windows in the tree. For this, you simply send commands to foo-wm in the specifications provided in `FOO-DSL.md`. 

For example, sending the zoom command through the provided command line client. 
`foo-wm-c '/tmp/foo-wm.socket' 'zoom 2'`


Inspiration
-----------
Inspirations include: 
DWM, monsterwm, catwm, dminiwm, i3, herbstluftwm, wmii, and tinywm

Contributing
------------
Foo Window Manager is very much open to contribution. Please get in touch via email if you are interested or just take a stab at something in `TODO.md` and send a pull request.
