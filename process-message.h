#ifndef PROCESS_MESSAGE_H_INCLUDED
#define PROCESS_MESSAGE_H_INCLUDED

#include "influx-sender.h"

#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>

struct Parameters {
    int multiplier;
    int divider;
    int decimals;
    char name[22];
    char symbol[20];
    int wide;
    double received_value;
};

struct CanFrame {
    int id;
    std::vector<Parameters> parameters; // Use vector instead of raw pointer
};



void processCANMessage(int can_id, int dlc, unsigned char *frame);

#endif // PROCESS_MESSAGE_H_INCLUDED


