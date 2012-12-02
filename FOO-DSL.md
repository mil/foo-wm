Foo DSL
======
The Foo DSL is foo-wm's set of commands which are accepted from the socket.

## `get` `nodeIdentifier`
Gets a json representation of the tree with the parent being the `nodeIdentifier`. For example to get the JSON from what's visible do a: `get tree`. Or get the currently focused window do a `get focus`.

- *Node Identifiers*:
    * `tree`
    * `view`
    * `focus`

## `set` `propertyName propertyValue`
Sets an global `propertyName` to `propertyValue`.  For example to make clients border width 30 pixels simply issue a `set client_border_width 30`. The screen_padding_direction globals can be used to make room for a bar such as [dzen](), [conky](), [bar](), or similar minimal X bars.

- *Properties Names and Types*:
    * `client_border_width` : (integer)
    * `container_padding` : (integer)
    * `client_padding` : (integer)
    * `screen_padding_top` : (integer)
    * `screen_padding_left` : (integer)
    * `screen_padding_right` : (integer)
    * `screen_padding_bottom` : (integer)

## `zoom` `delta`
Given that the **View Node** tracks what is currently on the screen, you can zoom in and out to see the full tree or only a part of the tree by simply by manipulating the **View Node**. `zoom -1000` would zoom out or travel up a thousand levels in ancestory within the tree. `zoom 3` would zoom in 3 levels. 

In foo-wm there is no fullscreen command, rather issuing zoom until the **View Node** hits the **Focus Node** is equivilant going "fullscreen". Functionality of a fullscreen kind of action can be easily implemented to jump back and forth in maximizing 1 window and then traveling back to a "zoomed out" view via scripting and the get command.


## `layout` `type`
Manipulates the **Focus Node**. Sets the layout type of the focus node. This is useful for displaying certain windows within a container node. For example, `layout vertical` tiles nodes vertically.

* Layout Options
    - Currently Implemented
        * `vertical`: Children are arranged side by side vertically
        * `horizontal`: Children are arranged horizontally
        * `grid`: Children are arranged to be equal size in a grid	
        * `max`: One child takes up the full view of the container, other children hidden
    * Planned Implementation
        * `tabbed`: Exactly like Max, only there is a visual tab indication
        * `float`: Clients are floated, but bound by the container
        * `freefloat`: Clients are free to float, even on top of the current view

## `focus`  `brother|pc -+delta`
Sets the **Focus Node**, in the form `brother|pc -+delta` approximating the current node. Good for shifting focus around. For example, get the focus' brother, simply issue `focus brother 1` or to select the **Focus Node **'s parent, do a `focus pc -1`.

## `shift` `brother|pc -+delta`
Shift the current **Focus Node** in . This wraps around if you attempt to move to a node that doesn't exist. Note, shifting to a positive pc won't do anything, shift pc should only be used with a negative delta to shift a client up toward its parent.

## `containerize`
If the current client is in a container with 2 or more other clients, containerize creates a new container and parents the current client into this new container. Syntax is just `containerize` with no arguments.

## `kill`
Kills the **Focus Node** and any of its children. Simply issue `kill` to eliminate the **Focus Node** from the tree.

## `mark` `markname`
Marks the current **View Node**  so that it can later be `jump`'d back to. To mark the current **View Node** at any point: `mark nameOfMark`.

### `jump` `markname`
Jumps to the predefined marked node, syntax takes form of `jump markName`.
