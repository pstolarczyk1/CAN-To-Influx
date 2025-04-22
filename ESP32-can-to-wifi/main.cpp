#include <WiFi.h>
#include <WiFiClient.h>
#include "driver/twai.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "esp_wifi.h"

const char* ssid = "test";
const char* password = "testtest";
const char* serverIP = "10.0.0.20";
const int serverPort = 2137;

WiFiClient client;
unsigned long lastPing = 0;
const int pingInterval = 1000;

void setupCAN() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("TWAI driver installed");
    } else {
        Serial.println("TWAI install failed");
        return;
    }

    if (twai_start() == ESP_OK) {
        Serial.println("TWAI started");
    } else {
        Serial.println("TWAI start failed");
    }
}

void connectToServer() {
    client.stop();
    Serial.println("Connecting to server...");
    if (client.connect(serverIP, serverPort)) {
        Serial.println("Connected to server");
    } else {
        Serial.println("Server connection failed");
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

    setupCAN();
    connectToServer();
}

void loop() {
    if (!client.connected()) {
        Serial.println("TCP not connected, reconnecting...");
        connectToServer();
        delay(1000);
        return;
    }

    // Read CAN frame
    twai_message_t message;
    if (twai_receive(&message, pdMS_TO_TICKS(10)) == ESP_OK) {
        char buffer[64];
        int len = snprintf(buffer, sizeof(buffer), "0x%X,", message.identifier);
        for (int i = 0; i < message.data_length_code; i++) {
            len += snprintf(buffer + len, sizeof(buffer) - len, "%s%02X", (i == 0 ? "" : " "), message.data[i]);
        }
        for (int i = message.data_length_code; i < 8; i++) {
            len += snprintf(buffer + len, sizeof(buffer) - len, " 00");
        }

        // Final terminator: semicolon + newline
        len += snprintf(buffer + len, sizeof(buffer) - len, ";\n");

        // Send to TCP server
        size_t written = client.write((const uint8_t*)buffer, len);
        client.flush();
        delayMicroseconds(500);

        if (written != len) {
            Serial.printf("Write failed! Sent %d of %d bytes\n", written, len);
            client.stop();
        } else {
            //Serial.print("Sent: ");
            //Serial.print(buffer); 
        }
    }
}
