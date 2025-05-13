//Screen
#include <LiquidCrystal_I2C.h>
#include <Wire.h>


//MQTT (Mosquitto)
#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* ssid = ""; // Your Wi-Fi SSID
const char* password = ""; // Your Wi-Fi password


// IP of the machine running Mosquitto (GCP)
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

volatile bool moveReceived = false;

//Pins
#define SDA 14                    //Define SDA pins
#define SCL 13                    //Define SCL pins

//board states
#define X_WIN 1
#define O_WIN 2
#define TIE 3
#define UNDECIDED 4

// Temps
int results;
char msg[128];
volatile int option = -1;

volatile int player1Move = -1;
volatile bool move1Received = false;

volatile int player2Move = -1;
volatile bool move2Received = false;



void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "option" && message == "0" ) {
    option = 0;
    Serial.println("Option: 0");
  }

  if (String(topic) == "option" && message == "1" ) {
    option = 1;
    Serial.println("Option: 1");
  }

  if (String(topic) == "option" && message == "2" ) {
    option = 2;
    Serial.println("Option: 2");
  }

  if (String(topic) == "player2/move") {
    String message;
    for (unsigned int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    int move = message.toInt();
    if (move >= 0 && move <= 8) {
      player2Move = move;
      move2Received = true;
    }
  }
  if (String(topic) == "player1/move") {
    String message;
    for (unsigned int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    int move = message.toInt();
    if (move >= 0 && move <= 8) {
      player1Move = move;
      move1Received = true;
    }
  }
}


/*
 * note:If lcd1602 uses PCF8574T, IIC's address is 0x27,
 *      or lcd1602 uses PCF8574AT, IIC's address is 0x3F.
*/
LiquidCrystal_I2C lcd(0x27,16,2); 
void setup() {
  //Setting up Console
  Serial.begin(115200);
  
  //Setting up LCD screen
  Wire.begin(SDA, SCL);           // attach the IIC pin
  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  
  //Setting up  wins info
  lcd.setCursor(0,0);             // Move the cursor to row 1, column 0
  lcd.print("Tic Tac Toe"); 
  


  // Connect to Wi-Fi
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0,1);
    lcd.print("Waiting Wifi");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT Broker
  mqttClient.setServer(mqtt_server, 1883);  // Mosquitto default port
  mqttClient.setCallback(callback);

  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Connect to MQTT broker (you can use a unique client ID)
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribed Topics
      mqttClient.subscribe("start");
      mqttClient.subscribe("player1/move");
      mqttClient.subscribe("player2/move");
      mqttClient.subscribe("option");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}


void loop() {
  //Loops till max games are reached 
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();  // Keep the MQTT connection alive
  if (option == 0) {
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print("Option: 0");
    new_start_game(option);
    delay(1000);
    option = -1;
  } else if (option == 1) {
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print("Option: 1");
    new_start_game(option);
    delay(1000);
    option = -1;
  } else if (option == 2) {
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print("Option: 2");
    new_start_game(option);
    delay(1000);
    option = -1;
  } else {
    Serial.println("Waiting for Option");
    lcd.setCursor(0,1);
    lcd.print("Waiting Option");
    delay(1000);
  }
}

bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}

int new_get_move(int board[9], int player) {
  Serial.println("polling for AI move");
  move1Received = false;
  move2Received = false;

  int count = 0;
  int* available_moves = new_get_available_moves(board, count, player); // published moves to available moves

  unsigned long startTime = millis();
  while (!(move1Received || move2Received) && millis() - startTime < 100000) {  // wait max 5 seconds
    mqttClient.loop(); // keep MQTT alive
  }

  if (player == 1) {
    if (move1Received) {
      Serial.println(player1Move);
      return player1Move;
    } else {
      Serial.println("Timeout waiting for move. Using fallback.");
    }
  } else {
    if (move2Received) {
      Serial.println(player2Move);
      return player2Move;
    } else {
      Serial.println("Timeout waiting for move. Using fallback.");
    }
  }
  
  Serial.println("polling for AI move");
}
/*
int get_move(int board[9]) {
  int count = 0;
  int* available_moves = get_available_moves(board, count);

  if (count == 0) {
    return -1; // No available moves
  }

  int rand_index = random(count); // ESP32 Arduino uses random()
  return available_moves[rand_index];
}
*/

// Plays a game of tic tac toe returns:
// 1 : X wins
// 2 : O wins
// 3 : Tie
int new_start_game(int option) {

  int board_state = UNDECIDED;
  int turn = 0; // Even turns is X odd turn is O
  int player; // Player 1 is X player 2 is )
  int move;
  int winner = TIE; //Winner defult to tie value
  char temp[10];

  int board[9] =
  {
    0, 0, 0,
    0, 0, 0,
    0, 0, 0
  };
  // INDEXES
  // 0, 1, 2,
  // 3, 4, 5,
  // 6, 7, 8,

  while(board_state == UNDECIDED) {
    //Deciding who's turn
    if (turn % 2 == 0){
      player = 1; //x
    } else {
      player = 2; //o
    }
    delay(500);
    send_full_board(board); // Sending board to TUI

    if (option == 0) { // (Human vs AI)
      if (player == 1) {
        move = get_human_move(player);
      } else {
        move = new_get_move(board, player);
      }
    } else if (option == 1) {
      move = get_human_move(player);
    } else if (option == 2) {
      move = new_get_move(board, player);
    } else {
      Serial.println("ERR no option");
    }

    //Placing move
    Serial.print("Setting board[");
    Serial.print(move);
    Serial.print("] (was ");
    Serial.print(board[move]);
    Serial.print(") to ");
    Serial.println(player);
    board[move] = player;
    turn++;


    // INDEXES
    // 0, 1, 2,
    // 3, 4, 5,
    // 6, 7, 8,
    //Check if there is a winner
    if ((board[0] == player && board[1] == player && board[2] == player) || // Top Row
      (board[3] == player && board[4] == player && board[5] == player) || // Mid Row
      (board[6] == player && board[7] == player && board[8] == player) || // Bot Row

      (board[0] == player && board[3] == player && board[6] == player) || // Left col
      (board[1] == player && board[4] == player && board[7] == player) || // Mid col
      (board[2] == player && board[5] == player && board[8] == player) || // Right col

      (board[0] == player && board[4] == player && board[8] == player) || // \ Diagnal
      (board[2] == player && board[4] == player && board[6] == player)) { // / Diagnal
  
      winner = player;
      board_state = winner;
      delay(500);
      sprintf(temp, "%d", board_state);
      mqttClient.publish("board/state", temp);
      delay(500);
      send_full_board(board);
    } else if (turn == 9) {
      board_state = TIE;
      delay(500);
      sprintf(temp, "%d", board_state);
      mqttClient.publish("board/state", temp);
      delay(500);
      send_full_board(board);
    }
    delay(500);
    sprintf(temp, "%d", board_state);
    mqttClient.publish("board/state", temp);
    delay(500);
    send_full_board(board);
  } // End of the game while loop (while(board_state != UNDECIDED))

  print_board(board);
  if (winner == X_WIN)
    Serial.println("X win");
  if (winner == O_WIN)
    Serial.println("O win");
  if (winner == TIE)
    Serial.println("Tie");
  Serial.println("-------");

  delay(500);
  sprintf(temp, "%d", board_state);
  mqttClient.publish("board/state", temp);
  return winner;
}

void print_board(int board[9]) {
  Serial.printf("%d %d %d\n", board[0], board[1], board[2]);
  Serial.printf("%d %d %d\n", board[3], board[4], board[5]);
  Serial.printf("%d %d %d\n", board[6], board[7], board[8]);
}



int* new_get_available_moves(int board[9], int &count, int player) {
  static int available[9];
  count = 0;

  char msg[128]; // Buffer for the JSON message
  char json_array[64] = "["; // To build JSON array manually

  for (int i = 0; i < 9; i++) {
    if (board[i] == 0) {
      available[count] = i;

      // Append to json_array
      char buf[4];
      sprintf(buf, "%d", i);
      strcat(json_array, buf);

      if (count < 8) strcat(json_array, ","); // Comma unless last
      count++;
    }
  }

  if (json_array[strlen(json_array) - 1] == ',') {
    json_array[strlen(json_array) - 1] = ']'; // Replace last comma with closing bracket
  } else {
    strcat(json_array, "]"); // Close array if no comma at end
  }

  if (player == 1) {
    sprintf(msg, "{\"available_moves\":%s}", json_array);
    mqttClient.publish("board/available1", msg);
    delay(3); // Tweak this around Too Slow: It will take a long time Too fast: GCP can't keep up and there will be a timming bug
  } else if (player == 2) {
    sprintf(msg, "{\"available_moves\":%s}", json_array);
    mqttClient.publish("board/available2", msg);
    delay(3); // Tweak this around Too Slow: It will take a long time Too fast: GCP can't keep up and there will be a timming bug
  } else {
    Serial.println("Err player not 1 or 2");
  }

  return available;
}

int get_human_move(int player) {
  Serial.println("polling for human move");
  if (player == 1) {
    move1Received = false;
    player1Move = -1;
  } else{
    move2Received = false;
    player2Move = -1;
  }
  
  if (player == 1) {
    while (!move1Received) {
    if (!mqttClient.connected()) {
      reconnect();
    }
    mqttClient.loop();
    }
  } else{
    while (!move2Received) {
    if (!mqttClient.connected()) {
      reconnect();
    }
    mqttClient.loop();
    }
  }
  Serial.println("received human move");
  if (player == 1) {
    return player1Move;
  } else{
    return player2Move;
  }
}

void send_full_board(int board[9]) {
  Serial.println("Sending Full board");
  char msg[128]; // Final JSON message buffer
  char json_array[64] = "["; // To build JSON array manually

  for (int i = 0; i < 9; i++) {
    char buf[4];
    sprintf(buf, "%d", board[i]);
    strcat(json_array, buf);

    if (i < 8) strcat(json_array, ",");
  }

  strcat(json_array, "]"); // Close the JSON array

  sprintf(msg, "{\"tictactoe\":%s}", json_array);
  mqttClient.publish("board/board", msg);
  delay(3); // Adjust timing if needed
  Serial.println("Sent Full board");
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connected");
      mqttClient.subscribe("start");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

