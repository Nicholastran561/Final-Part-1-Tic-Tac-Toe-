#!/bin/bash

# Configuration
MQTT_HOST="localhost"
MQTT_PORT=1883
MQTT_TOPIC="player2/move"
MOVES_FILE="moves2.json"

# Check if moves.json exists
if [[ ! -f "$MOVES_FILE" ]]; then
  echo "Error: $MOVES_FILE not found."
  exit 1
fi

# Count the available moves
move_count=$(jq '.available_moves | length' "$MOVES_FILE")

if [[ "$move_count" -eq 0 ]]; then
  echo "No available moves to publish."
  exit 1
fi

# Select a random move index and get the move value
random_index=$((RANDOM % move_count))
selected_move=$(jq ".available_moves[$random_index]" "$MOVES_FILE")

# Publish the selected move
mosquitto_pub -h "$MQTT_HOST" -p "$MQTT_PORT" -t "$MQTT_TOPIC" -m "$selected_move"

echo "Bash AI Publishing $selected_move to topic: $MQTT_TOPIC"