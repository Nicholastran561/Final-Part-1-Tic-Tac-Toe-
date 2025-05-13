#!/bin/bash

# Configuration
MQTT_HOST="localhost"
MQTT_PORT=1883

# Topics and output files
TOPIC1="board/available1"
TOPIC2="board/available2"
OUTPUT_FILE1="moves1.json"
OUTPUT_FILE2="moves2.json"

# Ensure output directories exist
mkdir -p "$(dirname "$OUTPUT_FILE1")"
mkdir -p "$(dirname "$OUTPUT_FILE2")"

# Subscriber for topic1: board/available1
mosquitto_sub -h "$MQTT_HOST" -p "$MQTT_PORT" -t "$TOPIC1" | while read -r message; do
    echo "[Message received on $TOPIC1]: $message"
    echo "$message" > "$OUTPUT_FILE1"
    echo "File written to $OUTPUT_FILE1"
    ./getplayerC
done &

# Subscriber for topic2: board/available2
mosquitto_sub -h "$MQTT_HOST" -p "$MQTT_PORT" -t "$TOPIC2" | while read -r message; do
    echo "[Message received on $TOPIC2]: $message"
    echo "$message" > "$OUTPUT_FILE2"
    echo "File written to $OUTPUT_FILE2"
    ./getplayerB.sh
done &

# Wait for background jobs
wait
