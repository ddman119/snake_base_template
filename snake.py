#! /usr/bin/env python
import pygame
import random
import sys
import os
os.system("./build.sh")
import game_controls as gs

######################
#   Game constants   #
######################

# Game Title
gameTitle = "Snake Multiplayer"

# Screen Size (The default is set to smallest screen resolution for PCs)
# TODO: Change this to detect the user's current screen resolution and use those dimensions for fullscreen.
width = 800
height = 600

# Score tracking
score = 0

# Number of players
NumPlayer = ""

# Flag that controls how many players are playing the game
# TODO: Remove these as the backend will tell the python's frontend how many players to start with.
onePlayer = False
twoPlayers = False
threePlayers = False
fourPlayer = False

# Array to keep track of how many players are currently in the game , "playing"
ingame = []

# Controls
# TODO: Remove these controls once we establish a shaRed network message handler.
player1_keys = [pygame.K_w, pygame.K_a, pygame.K_s, pygame.K_d]
player2_keys = [pygame.K_UP, pygame.K_DOWN, pygame.K_LEFT, pygame.K_RIGHT]
player3_keys = [pygame.K_t, pygame.K_g, pygame.K_f, pygame.K_h]
player4_keys = [pygame.K_i, pygame.K_k, pygame.K_j, pygame.K_l]

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
def startGame(width, height, gameTitle):
    pygame.init()
    pygame.font.init()
    random.seed()
    global screen
    screen = pygame.display.set_mode((width, height))
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

    def events(self, event):
        # Controls the left event
        if (
                event.key == pygame.K_LEFT or
                event.key == pygame.K_a or
                event.key == pygame.K_j or
                event.key == pygame.K_f
                ) and self.hdir != 10:
                    self.hdir = gs.moveLeftX(self.hdir, 1)
                    self.vdir = gs.moveLeftY(self.vdir, 1)
        # Controls the right event
        if (
                event.key == pygame.K_RIGHT or
                event.key == pygame.K_d or
                event.key == pygame.K_h or
                event.key == pygame.K_l
                ) and self.hdir != -10: # Check to make sure we aren't already going right.
                    self.hdir = gs.moveRightX(self.hdir, 1)
                    self.vdir = gs.moveRightY(self.vdir, 1)
        # Controls the up event
        if (
                event.key == pygame.K_UP or
                event.key == pygame.K_w or
                event.key == pygame.K_i or
                event.key == pygame.K_t
                ) and self.vdir != 10: # Check to make sure we aren't already going up.
                    self.hdir = gs.moveUpX(self.hdir, 1)
                    self.vdir = gs.moveUpY(self.vdir, 1)
        # Controls the down event
        if (
                event.key == pygame.K_DOWN or
                event.key == pygame.K_s or
                event.key == pygame.K_k or
                event.key == pygame.K_g
                ) and self.vdir != -10: # Check to make sure we aren't already going down.
                    self.hdir = gs.moveDownX(self.hdir, 1)
                    self.vdir = gs.moveDownY(self.vdir, 1)

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
        self.x = random.randrange(20, width, 10)
        self.y = random.randrange(20, height, 10)
    # Check if the snake hit himself.
    def hitCheck(self, snakePixels):
        if snakePixels[0][0] == self.x and snakePixels[0][1] == self.y:
            return True
    # After a snake eats, relocate the food to a random position.
    def relocate(self):
        self.x = random.randrange(20, width, 10)
        self.y = random.randrange(20, height, 10)
    # Draw the food onto the screen.
    def draw(self):
        pygame.draw.rect(screen, (255, 0, 0), (self.x, self.y + 10, 10, 10), 0)

# Allows user to select singleplayer or multiplayer modes
def playerSelect():
    loop = True
    global NumPlayer
    while loop:
        text("1-4 Player Snake, press (1,2,3,4)", 30, -1, height / 2, (255, 255, 255))
        pygame.display.flip()
        clock.tick(50)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                snake.crash = False
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_1:
                    NumPlayer = "1"
                    loop = False
                if event.key == pygame.K_2:
                    NumPlayer = "2"
                    loop = False
                if event.key == pygame.K_3:
                    NumPlayer = "3"
                    loop = False
                if event.key == pygame.K_4:
                    NumPlayer = "4"
                    loop = False
    del loop

startGame(width, height, gameTitle)

playerSelect()

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

while running:
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
    food.draw()

    if food.hitCheck(snake1.pixels):
        food.relocate()
        if onePlayer:
            score = score + 10
        snake1.length = snake1.length + 7
    if twoPlayers and food.hitCheck(snake2.pixels):
        food.relocate()
        snake2.length = snake2.length + 7
    if threePlayers:
        if food.hitCheck(snake3.pixels):
            food.relocate()
            snake3.length = snake3.length + 7
        if food.hitCheck(snake2.pixels):
            food.relocate()
            snake2.length = snake2.length + 7
    if fourPlayer:
        if food.hitCheck(snake4.pixels):
            food.relocate()
            snake4.length = snake4.length + 7
        if food.hitCheck(snake3.pixels):
            food.relocate()
            snake3.length = snake3.length + 7
        if food.hitCheck(snake2.pixels):
            food.relocate()
            snake2.length = snake2.length + 7

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
            if twoPlayers:
                if event.key in player2_keys:
                    snake2.events(event)
                if event.key in player1_keys:
                    snake1.events(event)
            if onePlayer and event.key in (player2_keys or player3_keys or player4_keys or player1_keys):
                snake1.events(event)
            if threePlayers:
                if event.key in player1_keys:
                    snake1.events(event)
                if event.key in player3_keys:
                    snake3.events(event)
                if event.key in player2_keys:
                    snake2.events(event)
            if fourPlayer:
                if event.key in player1_keys:
                    snake1.events(event)
                if event.key in player4_keys:
                    snake4.events(event)
                if event.key in player2_keys:
                    snake2.events(event)
                if event.key in player3_keys:
                    snake3.events(event)
            if event.key == pygame.K_ESCAPE:
                sys.exit()
            clock.tick(speed)

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
            fourPlayer and (snake4.crash or snake3.crash or snake2.crash))) and (len(inGame) == 1):
        if threePlayers or fourPlayer:
            if inGame == ["snake3"]:
                text("Winner: Player 3! ", 40, -1, -1, player3_color)
            elif inGame == ["snake4"]:
                text("Winner: Player 4! ", 40, -1, -1, player4_color)
            elif inGame == ["snake1"]:
                text("Winner: Player 1! ", 40, -1, -1, player1_color)
            elif inGame == ["snake2"]:
                text("Winner: Player 2! ", 40, -1, -1, player2_color)

        text("Press R to restart", 30, -1, height / 2 + 30, (255, 255, 255))
        text("ESC to return to main menu", 20, -1, height / 2 + 50, (255, 255, 255))
        pygame.display.flip()
        clock.tick(50)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                snake.crash = False
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r:
                    score = 0
                    newHighscore = False
                    food.__init__()
                    if onePlayer:
                        snake1.__init__(player1_x, player1_y, player1_color)
                    if twoPlayers:
                        snake1.__init__(player1_x, player1_y, player1_color)
                        snake2.__init__(player1_x, player2_y, player2_color)
                    if threePlayers:
                        snake1.__init__(player1_x, player1_y, player1_color)
                        snake2.__init__(player2_x, player2_y, player2_color)
                        snake3.__init__(player3_x, player3_y, player3_color)
                    if fourPlayer:
                        snake1.__init__(player1_x, player1_y, player1_color)
                        snake2.__init__(player2_x, player2_y, player2_color)
                        snake3.__init__(player3_x, player3_y, player3_color)
                        snake4.__init__(player4_x, player4_y, player4_color)
