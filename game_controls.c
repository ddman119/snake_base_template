/*
THis will move the snake left once the left arrow key is pressed.
x = the horizontal velocity of the player
y = the vertical velocity of the player
Player = used to identify which player has made the move. (1 - 4)
Returns: the GameObject with the updated velocities.
*/
int moveLeftX(int x, int player)
{
  x = -10;
  switch(player)
  {
    case 1:
      // write to FIFO FIle for Player1
      break;
    case 2:
      // write to FIFO FIle for Player2
      break;
    case 3:
      // write to FIFO FIle for Player3
      break;
    case 4:
      // write to FIFO FIle for Player4
      break;
  }
  return x;
}

int moveLeftY(int y, int player)
{
  y = 0;
  switch(player)
  {
    case 1:
      // write to FIFO FIle for Player1
      break;
    case 2:
      // write to FIFO FIle for Player2
      break;
    case 3:
      // write to FIFO FIle for Player3
      break;
    case 4:
      // write to FIFO FIle for Player4
      break;
  }
  return y;
}
/*
This will move the snake right once the right arrow key is pressed.
x = the horizontal velocity of the player
y = the vertical velocity of the player
Player = used to identify which player has made the move. (1 - 4)
Returns: the GameObject with the updated velocities.
*/
int moveRightX(int x, int player)
{
  x = 10;
  switch(player)
  {
    case 1:
      // write to FIFO FIle for Player1
      break;
    case 2:
      // write to FIFO FIle for Player2
      break;
    case 3:
      // write to FIFO FIle for Player3
      break;
    case 4:
      // write to FIFO FIle for Player4
      break;
  }
  return x;
}

int moveRightY(int y, int player)
{
  y = 0;
  switch(player)
  {
    case 1:
      // write to FIFO FIle for Player1
      break;
    case 2:
      // write to FIFO FIle for Player2
      break;
    case 3:
      // write to FIFO FIle for Player3
      break;
    case 4:
      // write to FIFO FIle for Player4
      break;
  }
  return y;
}
/*
This will move the snake Up once the up arrow key is pressed.
x = the horizontal velocity of the player
y = the vertical velocity of the player
Player = used to identify which player has made the move. (1 - 4)
Returns: the GameObject with the updated velocities.
*/
int moveUpX(int x, int player)
{
  x = 0;
  switch(player)
  {
    case 1:
      // write to FIFO FIle for Player1
      break;
    case 2:
      // write to FIFO FIle for Player2
      break;
    case 3:
      // write to FIFO FIle for Player3
      break;
    case 4:
      // write to FIFO FIle for Player4
      break;
  }
  return x;
}

int moveUpY(int y, int player)
{
  y = -10;
  switch(player)
  {
    case 1:
      // write to FIFO FIle for Player1
      break;
    case 2:
      // write to FIFO FIle for Player2
      break;
    case 3:
      // write to FIFO FIle for Player3
      break;
    case 4:
      // write to FIFO FIle for Player4
      break;
  }
  return y;
}
/*
This will move the snake down once the down arrow key is pressed.
x = the horizontal velocity of the player
y = the vertical velocity of the player
Player = used to identify which player has made the move. (1 - 4)
Returns: the GameObject with the updated velocities.
*/
int moveDownX(int x, int player)
{
  x = 0;
  switch(player)
  {
    case 1:
      // write to FIFO FIle for Player1
      break;
    case 2:
      // write to FIFO FIle for Player2
      break;
    case 3:
      // write to FIFO FIle for Player3
      break;
    case 4:
      // write to FIFO FIle for Player4
      break;
  }
  return x;
}

int moveDownY(int y, int player)
{
  y = 10;
  switch(player)
  {
    case 1:
      // write to FIFO FIle for Player1
      break;
    case 2:
      // write to FIFO FIle for Player2
      break;
    case 3:
      // write to FIFO FIle for Player3
      break;
    case 4:
      // write to FIFO FIle for Player4
      break;
  }
  return y;
}
