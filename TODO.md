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

Planned Features
================
- Implement Floating Layout for Containers
  * Mostly will be an update to placeNode in tree.c
  * Will need to modify events.c as well
- WM Hints/Atoms
- merge/absorb commands
- Socket responses for commands in command.c
- Title and Tab bar / Window Decorations 
