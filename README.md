# 2600 FINAL pt. 1

Your final is to modify the Tic-Tac-Toe game. [https://en.wikipedia.org/wiki/Tic-tac-toeLinks to an external site.](https://en.wikipedia.org/wiki/Tic-tac-toe)

PART1) The game should be programmed in the C programming language and be designed for two players to play. The game will run on your ESP32 but it will need to display the game as it is being played on your laptop as a TEXT USER INTERFACE, TUI.

After the game is created you should create a script, using BASH, to allow it to mimic a human player. The BASH script will run on the GCP server.

PART2) Modify the game creating a menu that allows the player to choose 1 player mode or 2 player mode.  The 1 player mode will execute part 1 above where a bash script needs to run to simulate a player for the human to play against.  2 player mode will need to be implemented to allow two human players to play on the same laptop with the game viewable on the laptop but being played on the ESP32.

PART3) Create another option to TIC-TAC-TOE that allows players to automate play. Additionally, create a C program that will mimic the player playing.  Note the game selection starts on your laptop but the game is running on the ESP32 and Player 1 is a C program and Player 2 is a bash script. Both Player 1 and Player 2 should be running on the GCP server but the game is viewable on the laptop.


## ESP32 (Our Game console)

**SUBSCRIBES TO:** 

`player1/move`: int 0-8 representing player moves

`player2/move`: int 0-8 representing player moves

`option`: int 0-2 menu options from our C program

0 = Human vs AI

1 = Human vs Human

2 = C AI vs Bash AI

**PUBLISHES TO:**

`board/board`: json array of all the board

`board/available1`: json array of available moves

`board/available2`: json array of available moves

`board/state`: int 1-4 representing game states

1 = X win

2 = O win

3 = Tie

4 = Undecided

## GCP (MQTT server host)

**SUBSCRIBES TO:**

`board/available1`: json array of available moves

`board/available2`: json array of available moves

**PUBLISHES TO:**

`player1/move`: int 0-8 representing player moves

`player2/move`: int 0-8 representing player moves

**SCRIPTS AND PROGRAMS:**

`getplayerB.sh`: reads `moves2.json` and returns and publishes  a random move to `player2/move`

`getplayerC.c`: reads `moves1.json` and returns and publishes a random move to `player1/move` 

`sub_moves.sh` : subscribes to `board/available1` and `board/available2` , it will run `getplayerB.sh`  and  `getplayerC.c` based on what is published

FILES:

`moves1.json` is a json array that holds all available moves

`moves2.json` is a json array that holds all available moves

## C Program on PC (Controller/Screen)

**SUBSCRIBES TO:**

`board/board`: json array of all the board

`board/state`: int 1-4 representing game states

1 = X win

2 = O win

3 = Tie

4 = Undecided

**PUBLISHES TO:**

`option`: int 0-2 menu options from our C program

0 = Human vs AI

1 = Human vs Human

2 = C AI vs Bash AI

`player1/move`: int 0-8 representing player moves

`player2/move`: int 0-8 representing player moves

## General Process outline

*** This was made during development of the code, not all the process actually follow these steps this was more of a guide during development ***

### Option [0] Human vs AI:

1. `C Program`: publishes 0 to `option` → `ESP32`: switches its mode to Human vs AI
2. `C Program` : polls for the board from `board/board`  to display
3. `ESP32`: publishes json array to `board/board`→ `C Program`: displays the board
    1. `C Program` will ask player for move
    2. `C Program`: publishes int 0-8 to `player1/move` →  `ESP32`: updates the game 
        1. ESP32 will place the move on the board
        2. Check for win: IF SO GO TO STEP 4
        3. ESP32 get all available positions
    3. `ESP32`: publishes json array to `board/available2`  → `GCP`: uses `getplayerB.sh` to get an available move 
    4. `GCP`: `getplayerB.sh` publishes int 0-8 to `player2/move` → `ESP32`: updates the game
        1. ESP32 will place the move on the board
        2. Check for win: IF SO GO TO STEP 4
        3. ESP32 get all available positions
    5. Loop to step 2
4. Displays who won on TUI
5. `ESP32`: publishes int 0-2 to `board/state`→ `C program`:displays winner and board

### Option [1] Human vs Human:

1. `C Program`: publishes 1 to `option` → `ESP32`: switches its mode to Human vs Human
2. `C Program` : polls for the board from `board/board`  to display
3. `ESP32`: publishes json array to `board/board` → `C Program`: displays the board
    1. `C Program` will ask player 1 for move
    2. `C Program`: publishes int 0-8 to `player1/move` →  `ESP32`: updates the game 
        1. ESP32 will place the move on the board
        2. Check for win: IF SO GO TO STEP 5
4. `ESP32`: publishes json array to `board/board` → `C Program`: displays the board
    1. `C Program` will ask player  for move
    2. `C Program`: publishes int 0-8 to `player2/move` →  `ESP32`: updates the game 
        1. ESP32 will place the move on the board
        2. Check for win: IF SO GO TO STEP 5
    3. loop to step 3
5. Display winner on TUI
6. `ESP32`: publishes int 0-2 to `board/state` → `C program`: displays winner and board

### Option [2] C vs BASH

1. `C Program`: publishes 2 to `option`→ `ESP32`: switches its mode to C vs BASH
2. `ESP32`: publishes json array to `board/board` → `C Program`: displays the board
    1. `ESP32`: publishes json array to `board/available1` → `GCP`: uses `getplayerC.c` to get an available move
    2. `GCP`: `getplayerC.c` publishes int 0-8 to `player1/move` →  `ESP32`: updates game 
        1. ESP32 will place the move on the board
        2. Check for win: IF SO GO TO STEP 3
        3. Get all available positions
    3. `ESP32`: publishes json array to `board/board` → `C Program`: displays the board
    4. `ESP32`: publishes json array to `board/available2`  → `GCP`: uses `getplayerB.sh` to get an available move
    5. `GCP`: `getplayerB.sh` publishes int 0-8 to `player2/move` → `ESP32`: updates game
        1. ESP32 will place the move on the board
        2. Check for win: IF SO GO TO STEP 3
        3. Get all available positions
    6. Loop to step 2
3. Display winner on TUI
4. `ESP32`: publishes int 0-2 to `board/state` → `C program` displays winner and board
