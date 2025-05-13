#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <mosquitto.h>

#define MQTT_HOST "localhost"
#define MQTT_PORT 1883
#define MQTT_TOPIC "player1/move"
#include <time.h>  // Add to the top of your file

char *get_random_move(const char *filename) {
    json_t *root;
    json_error_t error;

    root = json_load_file(filename, 0, &error);
    if (!root) {
        fprintf(stderr, "JSON error on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    json_t *moves = json_object_get(root, "available_moves");
    if (!json_is_array(moves)) {
        fprintf(stderr, "\"available_moves\" is not an array\n");
        json_decref(root);
        return NULL;
    }

    size_t array_size = json_array_size(moves);
    if (array_size == 0) {
        fprintf(stderr, "No available moves in array\n");
        json_decref(root);
        return NULL;
    }

    srand(time(NULL)); // Seed the RNG
    size_t random_index = rand() % array_size;

    json_t *move = json_array_get(moves, random_index);
    if (!json_is_integer(move)) {
        fprintf(stderr, "Selected move is not an integer\n");
        json_decref(root);
        return NULL;
    }

    int move_val = (int)json_integer_value(move);
    char *result = malloc(12);
    snprintf(result, 12, "%d", move_val);

    json_decref(root);
    return result;
}
void publish_move(const char *move) {
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Failed to create Mosquitto client\n");
        return;
    }

    if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Failed to connect to MQTT broker\n");
        mosquitto_destroy(mosq);
        return;
    }

    mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(move), move, 0, false);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
}
char *get_first_move(const char *filename) {
    json_t *root;
    json_error_t error;

    root = json_load_file(filename, 0, &error);
    if (!root) {
        fprintf(stderr, "JSON error on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    json_t *moves = json_object_get(root, "available_moves");
    if (!json_is_array(moves)) {
        fprintf(stderr, "\"available_moves\" is not an array\n");
        json_decref(root);
        return NULL;
    }

    json_t *move = json_array_get(moves, 0); // Get the first move
    if (!json_is_integer(move)) {
        fprintf(stderr, "First move is not an integer\n");
        json_decref(root);
        return NULL;
    }

    int move_val = (int)json_integer_value(move);
    char *result = malloc(12); // Enough space for int string
    snprintf(result, 12, "%d", move_val); // Convert to string

    json_decref(root);
    return result;
}
int main() {
    mosquitto_lib_init();

    char *move = get_random_move("moves1.json");
    if (move) {
        printf("C AI Publishing move: %s to topic: player1/moves\n", move);
        publish_move(move);
        free(move);
    }

    mosquitto_lib_cleanup();
    return 0;
}