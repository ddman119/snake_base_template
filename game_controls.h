/*
THis will move the snake left once the left arrow key is pressed.
x = the horizontal velocity of the player
y = the vertical velocity of the player
Player = used to identify which player has made the move. (1 - 4)
Returns: the GameObject with the updated velocities.
*/
int moveLeftX(int x, int player);
int moveLeftY(int y, int player);
/*
This will move the snake right once the right arrow key is pressed.
x = the horizontal velocity of the player
y = the vertical velocity of the player
Player = used to identify which player has made the move. (1 - 4)
Returns: the GameObject with the updated velocities.
*/
int moveRightX(int x, int player);
int moveRightY(int y, int player);
/*
This will move the snake Up once the up arrow key is pressed.
x = the horizontal velocity of the player
y = the vertical velocity of the player
Player = used to identify which player has made the move. (1 - 4)
Returns: the GameObject with the updated velocities.
*/
int moveUpX(int x, int player);
int moveUpY(int y, int player);
/*
This will move the snake down once the down arrow key is pressed.
x = the horizontal velocity of the player
y = the vertical velocity of the player
Player = used to identify which player has made the move. (1 - 4)
Returns: the GameObject with the updated velocities.
*/
int moveDownX(int x, int player);
int moveDownY(int y, int player);
