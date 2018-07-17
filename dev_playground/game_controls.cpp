#include <iostream>
#include <cstdlib>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
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
  int fd;
  // FIFO file path
  char const *myfifo = "/tmp/snakegame_fifo";
  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_WRONLY);
  std::string data = std::to_string(player) + " : " + std::to_string(x) + "\n";
  switch(player)
  {
    case 1:
    {
      write(fd, data.c_str(), strlen(data.c_str()));
    }
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
    close(fd);
  }
  return x;
}

int moveLeftY(int y, int player)
{
  y = 0;
  int fd;
  // FIFO file path
  char const *myfifo = "/tmp/snakegame_fifo";
  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_WRONLY);
  std::string data = std::to_string(player) + " : " + std::to_string(y) + "\n";
  switch(player)
  {
    case 1:
    {
      write(fd, data.c_str(), strlen(data.c_str()));
    }
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
    close(fd);
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
  int fd;
  // FIFO file path
  char const *myfifo = "/tmp/snakegame_fifo";
  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_WRONLY);
  std::string data = std::to_string(player) + " : " + std::to_string(x) + "\n";
  switch(player)
  {
    case 1:
      write(fd, data.c_str(), strlen(data.c_str()));
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
  int fd;
  // FIFO file path
  char const *myfifo = "/tmp/snakegame_fifo";
  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_WRONLY);
  std::string data = std::to_string(player) + " : " + std::to_string(y) + "\n";
  switch(player)
  {
    case 1:
      write(fd, data.c_str(), strlen(data.c_str()));
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
  int fd;
  // FIFO file path
  char const *myfifo = "/tmp/snakegame_fifo";
  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_WRONLY);
  std::string data = std::to_string(player) + " : " + std::to_string(x) + "\n";
  switch(player)
  {
    case 1:
      write(fd, data.c_str(), strlen(data.c_str()));
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
  int fd;
  // FIFO file path
  char const *myfifo = "/tmp/snakegame_fifo";
  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_WRONLY);
  std::string data = std::to_string(player) + " : " + std::to_string(y) + "\n";
  switch(player)
  {
    case 1:
      write(fd, data.c_str(), strlen(data.c_str()));
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
  int fd;
  // FIFO file path
  char const *myfifo = "/tmp/snakegame_fifo";
  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_WRONLY);
  std::string data = std::to_string(player) + " : " + std::to_string(x) + "\n";
  switch(player)
  {
    case 1:
      write(fd, data.c_str(), strlen(data.c_str()));
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
  int fd;
  // FIFO file path
  char const *myfifo = "/tmp/snakegame_fifo";
  // Creating the named file(FIFO)
  // mkfifo(<pathname>, <permission>)
  mkfifo(myfifo, 0666);
  fd = open(myfifo, O_WRONLY);
  std::string data = std::to_string(player) + " : " + std::to_string(y) + "\n";
  switch(player)
  {
    case 1:
      write(fd, data.c_str(), strlen(data.c_str()));
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
