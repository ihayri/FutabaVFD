
/*
  FutabaVFD Library
  Copyright (c) 2024 <Your Name>

  Licensed under the Creative Commons Attribution-NonCommercial 4.0 International License.
  You may not use this file for commercial purposes.

  For details, see the LICENSE file in the project root or:
  https://creativecommons.org/licenses/by-nc/4.0/
*/


#ifndef FUTABAVFD_H
#define FUTABAVFD_H

#include <Arduino.h>

class FutabaVFD {
public:
    /**
     * Updated Constructor
     * Allows defining all control pins in the main sketch.
     * Order: ChipSelect, DataIn, Clock, Reset
     */
    FutabaVFD(uint8_t csPin, uint8_t dinPin, uint8_t clkPin, uint8_t rstPin);

    // Basic Setup
    void begin();
    void setDigit(uint8_t digit);
    void setBrightness(uint8_t brightness);
    
    // Display Control
    void displayOn();
    void displayOff();
    void clear();
    
    // Content Printing
    void showString(uint8_t pos, const char* str);
    void sendCommand(uint8_t cmd);
    
    /**
     * Advanced: Animation & Custom Icons
     * pos: Character position (0-5)
     * index: CG-RAM slot (0-7)
     * data: 5x7 matrix (7 bytes, only lower 5 bits used)
     */
    void writeCustomChar(uint8_t pos, uint8_t index, uint8_t data[7]);

private:
    void pulseByte(uint8_t data);
    
    uint8_t _csPin;
    uint8_t _dinPin;
    uint8_t _clkPin;
    uint8_t _rstPin;    // Added for universal reset support
    uint8_t _digitCount;
};

#endif