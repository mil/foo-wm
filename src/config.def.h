/* Path to store socket */
#define SOCKET_PATH "/tmp/"
/* If "NONE" is specified as the SOCKET_NAME,
 * one will automatically be generated based the sockets PID
 * In the form of: foo-wm-pid.socket */
#define SOCKET_NAME "foo-wm.socket" 

// Screen Padding
// I leave 18 px height space for LemonBoy's bar
// (could use dzen, conky, etc here)
#define SCREEN_PADDING_TOP    (18)
#define SCREEN_PADDING_LEFT   (0)
#define SCREEN_PADDING_BOTTOM (0)
#define SCREEN_PADDING_RIGHT  (0)

// Containers
#define CONTAINER_DEFAULT_LAYOUT (GRID)
#define CONTAINER_PADDING (2)

#define CLIENT_ACTIVE_FOCUSED_COLOR     "rgb:ab/db/f5"
#define CLIENT_ACTIVE_UNFOCUSED_COLOR   "rgb:ff/00/00"
#define CLIENT_INACTIVE_FOCUSED_COLOR   "rgb:a7/ff/87"
#define CLIENT_INACTIVE_UNFOCUSED_COLOR "rgb:54/54/54"

#define CLIENT_BORDER_WIDTH (1)
#define CLIENT_PADDING (0)
