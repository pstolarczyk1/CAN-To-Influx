#include <Arduino.h>
#include "driver/twai.h"

#define TX_GPIO_NUM 5  // Set your CAN TX pin
#define RX_GPIO_NUM 4  // Set your CAN RX pin

void setup() {
    Serial.begin(115200);
    
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_GPIO_NUM, (gpio_num_t)RX_GPIO_NUM, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("CAN driver installed");
    } else {
        Serial.println("Failed to install CAN driver");
    }
    
    if (twai_start() == ESP_OK) {
        Serial.println("CAN driver started");
    } else {
        Serial.println("Failed to start CAN driver");
    }
}

void loop() {
    static uint8_t value1 = 0;
    static uint16_t value2 = 0;

    // Prepare the CAN message
    twai_message_t message;
    message.identifier = 0x100; // CAN ID
    message.extd = 0;           // Standard ID
    message.data_length_code = 3; // Sending 3 bytes

    // Pack values into the CAN frame
    message.data[0] = value1;            // First byte (0-100)
    message.data[1] = value2 & 0xFF;     // Low byte of value2
    message.data[2] = (value2 >> 8) & 0xFF; // High byte of value2

    // Send the CAN frame
    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
        Serial.printf("Sent: %d, %d\n", value1, value2);
    } else {
        Serial.println("CAN Transmission failed");
    }

    //update the values
    value1++;
    value2++;
    if (value1 >= 100)
        value1 = 0;
    if (value2 >= 800) 
        value2 = 0;

    delay(100); 
}
