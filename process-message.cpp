#include "process-message.h"

std::vector<CanFrame> canFrames = {
    {0x100, {
        {10, 1, 1, "test-data1", "%", 8, 0},
        {1, 1, 1, "test-data2", "C", 16, 0}
    }}
};


void processCANMessage(int can_id, int dlc, unsigned char *frame) {

    // Loop through all CAN frames to find a matching ID
    for (auto &frameEntry : canFrames) {
        if (can_id == frameEntry.id) {
            int currentByteOffset = 0;

            // Iterate over each parameter in the frame
            for (auto &param : frameEntry.parameters) {
                double value = 0;

                // Determine how many bytes to read based on 'wide' field
                if (param.wide == 16) {
                    value = (frame[currentByteOffset +1] * 256 + frame[currentByteOffset]) / static_cast<double>(param.decimals) * param.multiplier / param.divider;
                    currentByteOffset += 2; // Move 2 bytes forward
                } else if (param.wide == 8) {
                    value = frame[currentByteOffset] / static_cast<double>(param.decimals) * param.multiplier / param.divider;
                    currentByteOffset += 1; // Move 1 byte forward
                }

                // Store the decoded value in the struct
                param.received_value = value;

                // Print the parsed value
                std::cout << param.name << " " << param.received_value << " " << param.symbol << std::endl;

                send_to_influx("data", param.name, param.received_value);
            }
            break;
        }
    }
}
