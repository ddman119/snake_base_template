#! /usr/bin/env python

import errno
import os
import pygame
import random
import string
import sys
import threading

######################
#   Game constants   #
######################

# Screen Size (The default is set to smallest screen resolution for PCs)
# TODO: Change this to detect the user's current screen resolution and use those dimensions for fullscreen.
width = 800
height = 600

# Score tracking 
score = 0

# Number of players
NumPlayer = ""

# Self index
SelfIndex = ""

# Server or Client ?
IsServer = False

# Food position
Food_x = 0
Food_y = 0

# Fifo read/write path
FIFO_W_PATH = ""
FIFO_R_PATH = ""

# Sync to server time
IsSync = False;

# Flag that controls how many players are playing the game
# TODO: Remove these as the backend will tell the python's frontend how many players to start with.
onePlayer = False
twoPlayers = False
threePlayers = False
fourPlayer = False

# Array to keep track of how many players are currently in the game , "playing"
ingame = []

# Controls

# Each keys for control 4 snake instance
key1 = pygame.K_UP
key2 = pygame.K_UP
key3 = pygame.K_UP
key4 = pygame.K_UP

# Keys for control snake
control_keys = [pygame.K_UP, pygame.K_DOWN, pygame.K_LEFT, pygame.K_RIGHT]

# Colors
player1_color = (255, 0, 0)      # Red
player2_color = (0, 255, 0)      # Green
player3_color = (0, 0, 255)      # Blue
player4_color = (255, 255, 255)  # White

# Initial Position of the players
player1_x = width / 5
player1_y = height / 4
player2_x = width / 5
player2_y = (height * 3) / 4
player3_x = (width * 4) / 5
player3_y = height / 4
player4_x = (width * 4) / 5
player4_y = (height * 3) / 4

# Speed of the game
speed = 20

# Check if the game is running
running = True

# Function Name: startGame
def startGame(width, height):    
    # Anaylse argument
    args = sys.argv
    if len(args) != 4:
        print '[python] Argument is invalid. \n Argument type must be : python snake.py [player count] [self index] [IsServer]'
        sys.exit()
    
    global NumPlayer, SelfIndex, IsServer
    NumPlayer = args[1]
    SelfIndex = args[2]
    if args[3] == "Server":
        IsServer = True    

    global FIFO_W_PATH, FIFO_R_PATH
    FIFO_W_PATH = '/tmp/snakegame_fifo_w_{0}'.format(SelfIndex)
    FIFO_R_PATH = '/tmp/snakegame_fifo_r_{0}'.format(SelfIndex)

    # init pygame
    pygame.init()
    pygame.font.init()
    random.seed()
    global screen
    screen = pygame.display.set_mode((width, height))
    gameTitle = 'Network Snake Game : Player{0}'.format(SelfIndex)
    pygame.display.set_caption(gameTitle)
    global clock
    clock = pygame.time.Clock()
    
def text(intext, size, inx, iny, color):
    font = pygame.font.Font(None, size)
    text = font.render((intext), 0, color)
    if inx == -1:
        x = width / 2
    else:
        x = inx
    if iny == -1:
        y = height / 2
    else:
        y = iny
    textpos = text.get_rect(centerx=x, centery=y)
    screen.blit(text, textpos)

class snake:
    def __init__(self, x, y, color=(0, 255, 0), pixels=None):
        self.x = x
        self.y = y
        self.hdir = 0
        self.vdir = -10
        if pixels == None:
            self.pixels = [(x, y)]
        else:
            self.pixels = pixels
        self.color = color
        self.crash = False
        self.length = 7
        self.die = False

    def events(self, key):
        # Controls the left event
        if key == pygame.K_LEFT and self.hdir != 10: # Check to make sure we aren't already going left.
            self.hdir = -10
            self.vdir = 0
        # Controls the right event
        if key == pygame.K_RIGHT and self.hdir != -10: # Check to make sure we aren't already going right.
            self.hdir = 10
            self.vdir = 0
        # Controls the up event
        if key == pygame.K_UP and self.vdir != 10: # Check to make sure we aren't already going up.
            self.hdir = 0
            self.vdir = -10
        # Controls the down event
        if key == pygame.K_DOWN and self.vdir != -10: # Check to make sure we aren't already going down.
            self.hdir = 0
            self.vdir = 10

    def move(self, snk2=None, snk3=None, snk4=None):
        global NumPlayer
        if not self.die:
            self.x += self.hdir
            self.y += self.vdir

            if (self.x, self.y) in self.pixels:
                self.crash = True
            # Checks if another snake exists.
            if NumPlayer == "2":
                if snk2.die == False and (self.x, self.y) in snk2.pixels:
                    self.crash = True
            if NumPlayer == "3":
                if snk2.die == False and (self.x, self.y) in snk2.pixels:
                    self.crash = True
                if snk3.die == False and (self.x, self.y) in snk3.pixels:
                    self.crash = True
            if NumPlayer == "4":
                if snk2.die == False and (self.x, self.y) in snk2.pixels:
                    self.crash = True
                if snk3.die == False and (self.x, self.y) in snk3.pixels:
                    self.crash = True
                if snk4.die == False and (self.x, self.y) in snk4.pixels:
                    self.crash = True

            # Wraps the snake
            if self.x < 0:
                self.x = width - 10
            if self.x >= width:
                self.x = 0
            if self.y <= 0:
                self.y = height - 20
            if self.y >= height - 10:
                self.y = 10

            self.pixels.insert(0, (self.x, self.y))

            if len(self.pixels) > self.length:
                del self.pixels[self.length]

    def draw(self):
        if not self.die:
            for x, y in self.pixels:
                pygame.draw.rect(screen, self.color, (x, y + 10, 10, 10), 1)


class food():
    # Initialize the position for where food is placed.
    def __init__(self):
        if IsServer:
            self.x = random.randrange(20, width, 10)
            self.y = random.randrange(20, height, 10)
            self.sendToBackend()
        else:
            self.x = 0
            self.y = 0
    # Set position of food
    def setPosition(self, xpos, ypos):
        self.x = xpos
        self.y = ypos
    # Check if the snake hit himself.
    def hitCheck(self, snakePixels):
        if snakePixels[0][0] == self.x and snakePixels[0][1] == self.y:
            return True
    # After a snake eats, relocate the food to a random position.
    def relocate(self):
        if IsServer:            
            self.x = random.randrange(20, width, 10)
            self.y = random.randrange(20, height, 10)
            self.sendToBackend()
    # Send food position to backend
    def sendToBackend(self):
        try:
            with open(FIFO_W_PATH, 'w') as fifo:
                write_str = 'FOOD:' + str(self.x) + ':' + str(self.y) + '\n';
                print 'New Food position {0}'.format(write_str)
                fifo.write(write_str)
                fifo.flush()
                fifo.close()
        except Exception as e:
            print '[python] Open fifo failed in food.'
    # Draw the food onto the screen.
    def draw(self):
        pygame.draw.rect(screen, (255, 0, 0), (self.x, self.y + 10, 10, 10), 0)

# Observe status from backend via named pipe
class StatusThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)        
    def run(self):        
        # Checks for backend status
        try:
            with open(FIFO_R_PATH, 'r') as fifo:
                data_string = "";
                while running:                    
                    # Non-blocking read.
                    data = fifo.read(1)
                    if data != '\n' and data != '':
                        data_string += str(data)
                    else:                        
                        self.proc(data_string)
                        data_string = ""
        except Exception as e:
            print '[python] Open fifo failed in observe thread'            

    # Process packet function
    # packet structure : CODE:EXT_CODE:DATA
    def proc(self, packet = ''):
        if packet == '':
            return

        global IsSync, Food_x, Food_y, key1, key2, key3, key4

        str_list = packet.split(':')
        if len(str_list) == 1:
            code = str_list[0]
            if code == 'TIME':
                print 'Time Sync Req'
                IsSync = True
        elif len(str_list) == 3:
            code = str_list[0]
            ext = str_list[1]
            data = str_list[2]
            if data.endswith('TIME'):
                data = data.translate(None, 'TIME')            
            if code == 'KEY':
                # Input key and time not synchronize so can get like KEY:253TIME so get only 3 chars
                print '[python] key {0}'.format(data)
                if ext == '1':    # Snake1's keycode
                    key1 = int(data)
                elif ext == '2':    # Snake2's keycode
                    key2 = int(data)
                elif ext == '3':    # Snake3's keycode
                    key3 = int(data)                    
                elif ext == '4':    # Snake4's keycode
                    key4 = int(data)                    
            elif code == 'FOOD':    # Relocate food
                print '[python] food {0}:{1}'.format(ext, data)
                Food_x = int(ext)
                Food_y = int(data)


startGame(width, height)

if NumPlayer == "1":
    onePlayer = True
    snake1 = snake(player1_x, player1_y, player1_color)
    inGame = ["snake1"]
elif NumPlayer == "2":
    twoPlayers = True
    snake1 = snake(player1_x, player1_y, player1_color)
    snake2 = snake(player2_x, player2_y, player2_color)
    inGame = ["snake1", "snake2"]
elif NumPlayer == "3":
    threePlayers = True
    snake1 = snake(player1_x, player1_y, player1_color)
    snake2 = snake(player2_x, player2_y, player2_color)
    snake3 = snake(player3_x, player3_y, player3_color)
    inGame = ["snake1", "snake2", "snake3"]
elif NumPlayer == "4":
    fourPlayer = True
    snake1 = snake(player1_x, player1_y, player1_color)
    snake2 = snake(player2_x, player2_y, player2_color)
    snake3 = snake(player3_x, player3_y, player3_color)
    snake4 = snake(player4_x, player4_y, player4_color)
    inGame = ["snake1", "snake2", "snake3", "snake4"]

food = food()

# Thread for watch status from backend
statThread = StatusThread()
statThread.start()

while running:
    while not IsSync:
        clock.tick(10)
    IsSync = False
    screen.fill((0, 0, 0))
    if onePlayer:
        snake1.move()
    if twoPlayers:
        snake1.move(snake2)
        snake2.move(snake1)
        snake2.draw()
    if threePlayers:
        snake1.move(snake2, snake3)
        snake2.move(snake1, snake3)
        snake3.move(snake1, snake2)
        snake2.draw()
        snake3.draw()
    if fourPlayer:
        snake1.move(snake2, snake3, snake4)
        snake2.move(snake1, snake3, snake4)
        snake3.move(snake1, snake2, snake4)
        snake4.move(snake1, snake2, snake3)
        snake2.draw()
        snake3.draw()
        snake4.draw()
    snake1.draw()
    if not IsServer:
        food.setPosition(Food_x, Food_y)
    food.draw()

    isHit = False
    if food.hitCheck(snake1.pixels):
        isHit = True
        if onePlayer:
            score = score + 10
        snake1.length = snake1.length + 7
    if twoPlayers and food.hitCheck(snake2.pixels):
        isHit = True
        snake2.length = snake2.length + 7
    if threePlayers:
        if food.hitCheck(snake3.pixels):
            isHit = True
            snake3.length = snake3.length + 7
        if food.hitCheck(snake2.pixels):
            isHit = True
            snake2.length = snake2.length + 7
    if fourPlayer:
        if food.hitCheck(snake4.pixels):
            isHit = True
            snake4.length = snake4.length + 7
        if food.hitCheck(snake3.pixels):
            isHit = True
            snake3.length = snake3.length + 7
        if food.hitCheck(snake2.pixels):
            isHit = True
            snake2.length = snake2.length + 7

    if isHit and IsServer:
        food.relocate()

    if onePlayer:
        text("Player 1: Red", 16, width / 5, 10, player1_color)
    elif twoPlayers:
        text("Player 1: Red", 16, width / 5, 10, player1_color)
        text("Player 2: Green", 16, (width * 2) / 5, 10, player2_color)
    elif threePlayers:
        text("Player 1: Red", 16, width / 5, 10, player1_color)
        text("Player 2: Green", 16, (width * 2) / 5, 10, player2_color)
        text("Player 3: Blue", 16, (width * 3) / 5, 10, player3_color)
    elif fourPlayer:
        text("Player 1: Red", 16, width / 5, 10, player1_color)
        text("Player 2: Green", 16, (width * 2) / 5, 10, player2_color)
        text("Player 3: Blue", 16, (width * 3) / 5, 10, player3_color)
        text("Player 4: White", 16, (width * 4) / 5, 10, player4_color)

    # Checks for user input and perform the relevant actions.
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.KEYDOWN:
            if event.key in control_keys:
                try:
                    with open(FIFO_W_PATH, 'w') as fifo:
                        write_str = str(event.key) + '\n';
                        fifo.write(write_str)
                        fifo.flush()
                        fifo.close()
                except Exception as e:
                    print '[python] Open fifo failed in key event.'

                if SelfIndex == "1":
                    key1 = event.key
                elif SelfIndex == "2":
                    key2 = event.key
                elif SelfIndex == "3":
                    key3 = event.key
                elif SelfIndex == "4":
                    key4 = event.key
            
            if event.key == pygame.K_ESCAPE:
                running = False
                # sys.exit()
            clock.tick(speed)

    if onePlayer:
        snake1.events(key1)
    if twoPlayers:        
        snake1.events(key1)
        snake2.events(key2)    
    if threePlayers:
        snake1.events(key1)
        snake2.events(key2)
        snake3.events(key3)
    if fourPlayer:
        snake1.events(key1)
        snake2.events(key2)
        snake3.events(key3)
        snake4.events(key4)

    # Updates the display at the end.
    pygame.display.flip()
    clock.tick(speed)

    if onePlayer:
        inGame = ["snake1"]
        if snake1.crash:
            text("Game Over, Score: " + str(score), 40, -1, -1, (255, 255, 255))
    if twoPlayers:
        inGame = ["snake1", "snake2"]
        if snake1.crash:
            text("Winner: Player 2! ", 40, -1, -1, player2_color)
            inGame.remove("snake1")
        if snake2.crash:
            text("Winner: Player 1! ", 40, -1, -1, player1_color)
            inGame.remove("snake2")
    if threePlayers:
        inGame = ["snake1", "snake2", "snake3"]
        if snake1.crash:
            snake1.die = True
            inGame.remove("snake1")
        if snake2.crash:
            snake2.die = True
            inGame.remove("snake2")
        if snake3.crash:
            snake3.die = True
            inGame.remove("snake3")
    if fourPlayer:
        inGame = ["snake1", "snake2", "snake3", "snake4"]
        if snake1.crash:
            snake1.die = True
            inGame.remove("snake1")
        if snake2.crash:
            snake2.die = True
            inGame.remove("snake2")
        if snake3.crash:
            snake3.die = True
            inGame.remove("snake3")
        if snake4.crash:
            snake4.die = True
            inGame.remove("snake4")

    # Logic to check who wins in a multiplayer game
    while (snake1.crash or (twoPlayers and snake2.crash) or (threePlayers and (snake3.crash or snake2.crash)) or (
            fourPlayer and (snake4.crash or snake3.crash or snake2.crash))) and (len(inGame) == 1) and running:
        if threePlayers or fourPlayer:
            if inGame == ["snake3"]:
                text("Winner: Player 3! ", 40, -1, -1, player3_color)
            elif inGame == ["snake4"]:
                text("Winner: Player 4! ", 40, -1, -1, player4_color)
            elif inGame == ["snake1"]:
                text("Winner: Player 1! ", 40, -1, -1, player1_color)
            elif inGame == ["snake2"]:
                text("Winner: Player 2! ", 40, -1, -1, player2_color)

        text("Press X to exit", 30, -1, height / 2 + 30, (255, 255, 255))
        # text("ESC to return to main menu", 20, -1, height / 2 + 50, (255, 255, 255))
        
        pygame.display.flip()
        clock.tick(50)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                snake.crash = False
                # sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_x:
                    running = False
                    # score = 0
                    # newHighscore = False
                    # food.__init__()
                    # if onePlayer:
                    #     snake1.__init__(player1_x, player1_y, player1_color)
                    # if twoPlayers:
                    #     snake1.__init__(player1_x, player1_y, player1_color)
                    #     snake2.__init__(player1_x, player2_y, player2_color)
                    # if threePlayers:
                    #     snake1.__init__(player1_x, player1_y, player1_color)
                    #     snake2.__init__(player2_x, player2_y, player2_color)
                    #     snake3.__init__(player3_x, player3_y, player3_color)
                    # if fourPlayer:
                    #     snake1.__init__(player1_x, player1_y, player1_color)
                    #     snake2.__init__(player2_x, player2_y, player2_color)
                    #     snake3.__init__(player3_x, player3_y, player3_color)
                    #     snake4.__init__(player4_x, player4_y, player4_color)

# Wait for status thread exit
statThread.join()
sys.exit()