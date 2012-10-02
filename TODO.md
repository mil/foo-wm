This is an ongoing list of bugs and planned features. If you are looking for something to contribute, this would be a good starting point. The codebase of foo-wm is small and hackable. Contributions are welcomed.

Bugs
====
- 'focus brother delta' updates view node, but does not set global viewNode
  * To Reproduce: 
    - Tree:
      *  Container [View Node]
        - Client [Focus Node]
        - Client
    - Commands:
      * zoom 1
      * focus brother 1
- 'set screen_padding_(top|left|right|bottom)' does not take immediate affect
- 1 Child containers within containers
- 'containerize' 'containerize' -- case of the missing container
- brotherNode() in tree.c, node->parent = brother->parent ?
    * This could potentially break the old node's brothers and node parent child ptr
- XMapWindow and XRaiseWindow always called for Windows in placeNode regardless if already mapped/raised

Planned Features
================
- Implement Floating Layout for Containers
  * Mostly will be an update to placeNode in tree.c
  * Will need to modify events.c as well
- WM Hints/Atoms
- reparent command (shifts a node into another container)
- transpose command (given two node ids - switch)
- Socket responses for commands in command.c
- Title and Tab bar / Window Decorations 
- Multi-Monitor
- Percent Fill for Nodes whose parent is Vertical/Horizontal mode (Resize)
