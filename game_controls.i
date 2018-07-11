%module game_controls
%{
  #include "game_controls.h"
%}

int moveLeftX(int x, int player);
int moveLeftY(int y, int player);
int moveRightX(int x, int player);
int moveRightY(int y, int player);
int moveUpX(int x, int player);
int moveUpY(int y, int player);
int moveDownX(int x, int player);
int moveDownY(int y, int player);
