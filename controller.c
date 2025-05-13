#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <unistd.h>



#define BUFFER_SIZE 128

char* subscribe_once(const char* broker_ip, const char* topic);
void print_tictactoe_board(const char* buffer);
void poll_for_board(const char* server_ip_address);
void pub_input(int player, const char* server_ip_address );
int poll_for_board_state(const char* server_ip_address);
void sleep_ms(int milliseconds);


int main() {
    const char* mosquitto_path = "C:\\Program Files\\mosquitto"; // Set the path to the directory where mosquitto_pub and mosquitto_sub are located
    const char* server_ip_address = "0.0.0.0"; // Replace with your server IP address

    if (SetCurrentDirectory(mosquitto_path) == 0) {
        printf("Failed to change directory. Error: %d\n", GetLastError());
        return 1;
    }

    int input;
    while (1) {
        printf("\n[0]:Human vs AI\n[1]:Human vs Human\n[2]:AI(C) vs AI(BASH)\n[3]:Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &input);

        char pub_command[256];
        char temp[10];
        int player = 1;
        int game_state = 4;

        if (input == 0) { // Human vs AI
            snprintf(pub_command, sizeof(pub_command), 
                     "mosquitto_pub -h %s -t \"option\" -m \"0\"",
                     server_ip_address);
            system(pub_command);
            do {
                poll_for_board(server_ip_address);
                pub_input(1, server_ip_address);
                poll_for_board(server_ip_address);
                game_state = poll_for_board_state(server_ip_address);
            } while(game_state == 4);
            while(game_state == 4);
            poll_for_board(server_ip_address);
            if (game_state == 1) {
                printf("Player 1 wins!\n");
            } else if (game_state == 2) {
                printf("Player 2 wins!\n");
            } else if (game_state == 3) {
                printf("It's a draw!\n");
            } else {
                printf("Invalid game state!\n");
            }
        } else if (input == 1) { // Human vs Human
            snprintf(pub_command, sizeof(pub_command), 
                     "mosquitto_pub -h %s -t \"option\" -m \"1\"",
                     server_ip_address);
            system(pub_command);
            do{
                poll_for_board(server_ip_address);
                pub_input(player, server_ip_address);
                if (player == 1) {
                    player = 2;
                } else {
                    player = 1;
                }
                game_state = poll_for_board_state(server_ip_address);
            }while(game_state == 4);
            poll_for_board(server_ip_address);
            if (game_state == 1) {
                printf("Player 1 wins!\n");
            } else if (game_state == 2) {
                printf("Player 2 wins!\n");
            } else if (game_state == 3) {
                printf("It's a draw!\n");
            } else {
                printf("Invalid game state!\n");
            }
        } else if (input == 2) { // AI(C) vs AI(BASH)
            snprintf(pub_command, sizeof(pub_command),
                     "mosquitto_pub -h %s -t \"option\" -m \"2\"",
                     server_ip_address);
            system(pub_command);
            poll_for_board(server_ip_address);
            do {
                poll_for_board(server_ip_address);
                game_state = poll_for_board_state(server_ip_address);
            } while(game_state == 4);
            while(game_state == 4);
            poll_for_board(server_ip_address);
            if (game_state == 1) {
                printf("Player 1 wins!\n");
            } else if (game_state == 2) {
                printf("Player 2 wins!\n");
            } else if (game_state == 3) {
                printf("It's a draw!\n");
            } else {
                printf("Invalid game state!\n");
            }
        } else if (input == 3) { // Exit
            printf("Exiting program...\n");
            break;

        } else {
            printf("Invalid choice! Please try again.\n");
            continue;
        }

        
        
    }

    return 0;
}

void pub_input(int player, const char* server_ip_address) {
    char pub_command[256];
    int input;
    printf("Enter a an UNTAKEN space (1-9)\n");
    printf("          1 2 3\n");
    printf("          4 5 6\n");
    printf("          7 8 9\n");
    if (player == 1) {
        printf("Player 1, enter your move (1-9): ");
    } else if (player == 2) {
        printf("Player 2, enter your move (1-9): ");
    } else {
        printf("Invalid player number!\n");
        return;
    }
    scanf("%d", &input);

    input -= 1; // Adjust for 0-based index

    if (player == 1) {
        snprintf(pub_command, sizeof(pub_command),
                 "mosquitto_pub -h %s -t \"player1/move\" -m \"%d\"",
                 server_ip_address, input);
    } else if (player == 2) {
        snprintf(pub_command, sizeof(pub_command),
                 "mosquitto_pub -h %s -t \"player2/move\" -m \"%d\"",
                 server_ip_address, input);
    } else {
        printf("Invalid player number!\n");
        return;
    }

    sleep_ms(500);
    int result = system(pub_command);
    if (result != 0) {
        printf("Failed to publish message. Command: %s\n", pub_command);
    }
}

void poll_for_board(const char* server_ip_address) {
//    printf("Polling for board...\n");
    char* message = subscribe_once(server_ip_address, "board/board");
//    if (message) {
//        printf("Received message: %s\n", message);
//    }
    print_tictactoe_board(message);
    return;
}

int poll_for_board_state(const char* server_ip_address) {
//    printf("Polling for board state...\n");
    char* message = subscribe_once(server_ip_address, "board/state");
//    if (message) {
//        printf("Received message: %s\n", message);
//    }
    int game_state = atoi(message);
    return game_state;
}


char* subscribe_once(const char* broker_ip, const char* topic) {
    static char buffer[BUFFER_SIZE];  // Static so the pointer is still valid after returning
    buffer[0] = '\0';  // Clear buffer

    char command[256];
    snprintf(command, sizeof(command), 
             "mosquitto_sub -h %s -t \"%s\" -C 1", 
             broker_ip, topic);
    
    FILE* fp = _popen(command, "r");  // Use _popen on Windows
    if (fp == NULL) {
        fprintf(stderr, "Failed to run mosquitto_sub.\n");
        return NULL;
    }

    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fprintf(stderr, "No message received or error occurred.\n");
        _pclose(fp);
        return NULL;
    }

    // Remove trailing newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    _pclose(fp);
    return buffer;
}

void print_tictactoe_board(const char *buffer) {
    int board[9] = {0};
    const char *start = strchr(buffer, '[');
    const char *end = strchr(buffer, ']');

    if (!start || !end || end < start) {
        printf("Invalid input format.\n");
        return;
    }

    // Extract the content between [ and ]
    char numbers[100];
    size_t len = end - start - 1;
    strncpy(numbers, start + 1, len);
    numbers[len] = '\0';

    // Parse the numbers
    char *token = strtok(numbers, ",");
    int i = 0;
    while (token && i < 9) {
        board[i++] = atoi(token);
        token = strtok(NULL, ",");
    }

    if (i != 9) {
        printf("Invalid number of elements in the board.\n");
        return;
    }

    // Print the tic tac toe board
    printf("Tic Tac Toe Board:\n");
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            char symbol = ' ';
            switch (board[row * 3 + col]) {
                case 1: symbol = 'X'; break;
                case 2: symbol = 'O'; break;
            }
            printf(" %c ", symbol);
            if (col < 2) printf("|");
        }
        printf("\n");
        if (row < 2) printf("---+---+---\n");
    }
}

void sleep_ms(int milliseconds)
{
    // Convert milliseconds to microseconds
    usleep(milliseconds * 1000);
}