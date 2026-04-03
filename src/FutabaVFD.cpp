
/*
  FutabaVFD Library
  Copyright (c) 2024 <Your Name>

  Licensed under the Creative Commons Attribution-NonCommercial 4.0 International License.
  You may not use this file for commercial purposes.

  For details, see the LICENSE file in the project root or:
  https://creativecommons.org/licenses/by-nc/4.0/
*/


#include "FutabaVFD.h"
#include <Arduino.h>

/**
 * Universal Constructor
 * Accepts custom pins for CS, DIN (Data), CLK (Clock), and RST (Reset).
 * This allows the same library to work on UNO (default pins) or 
 * XIAO ESP32-S3 (Piggyback pins).
 */
FutabaVFD::FutabaVFD(uint8_t csPin, uint8_t dinPin, uint8_t clkPin, uint8_t rstPin) {
    _csPin  = csPin;
    _dinPin = dinPin;
    _clkPin = clkPin;
    _rstPin = rstPin;
    _digitCount = 6; // Default for your display
}

/**
 * Initialization
 * Sets up the GPIOs and performs the hardware wake-up sequence.
 */
void FutabaVFD::begin() {
    pinMode(_dinPin, OUTPUT);
    pinMode(_clkPin, OUTPUT);
    pinMode(_csPin,  OUTPUT);
    pinMode(_rstPin, OUTPUT);

    // Initial Idle State
    digitalWrite(_csPin,  HIGH);
    digitalWrite(_clkPin, HIGH);
    digitalWrite(_dinPin, LOW);

    // --- HARDWARE RESET SEQUENCE ---
    // Required to clear the internal CIG (Chip-In-Glass) registers
    digitalWrite(_rstPin, LOW);
    delay(100);
    digitalWrite(_rstPin, HIGH);
    delay(300); // Give the VFD oscillator time to stabilize

    // Default VFD Startup Commands
    setDigit(_digitCount); 
    setBrightness(200);    
    clear();
}

/**
 * Manual Bit-Bang Engine (LSB First)
 * Optimized for high-speed MCUs like ESP32-S3.
 */
void FutabaVFD::pulseByte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(_clkPin, LOW);
        
        // VFDs require Least Significant Bit first
        digitalWrite(_dinPin, (data & 0x01) ? HIGH : LOW);
        data >>= 1;
        
        // Critical Delays: ESP32-S3 is too fast for the 0.5MHz VFD bus.
        // These 5us delays ensure a stable ~100kHz clock.
        delayMicroseconds(5); 
        digitalWrite(_clkPin, HIGH);
        delayMicroseconds(5);
    }
}

void FutabaVFD::sendCommand(uint8_t cmd) {
    digitalWrite(_csPin, LOW);
    pulseByte(cmd);
    digitalWrite(_csPin, HIGH);
    delayMicroseconds(10);
}

void FutabaVFD::setDigit(uint8_t digit) {
    _digitCount = digit;
    digitalWrite(_csPin, LOW);
    pulseByte(0xE0);      // Command: Set Digit Amount
    pulseByte(digit - 1); // 0x05 for 6 digits
    digitalWrite(_csPin, HIGH);
    delayMicroseconds(10);
}

void FutabaVFD::setBrightness(uint8_t brightness) {
    digitalWrite(_csPin, LOW);
    pulseByte(0xE4);      // Command: Set Brightness
    pulseByte(brightness); // 0x00 to 0xFF
    digitalWrite(_csPin, HIGH);
    delayMicroseconds(10);
}

void FutabaVFD::displayOn() {
    sendCommand(0xE8); // Refresh/Display On
}

void FutabaVFD::displayOff() {
    sendCommand(0xEA); // Standby Mode
}

void FutabaVFD::clear() {
    digitalWrite(_csPin, LOW);
    pulseByte(0x20); // Start at Address 0
    for(int i = 0; i < _digitCount; i++) {
        pulseByte(0x20); // Write ASCII 'Space' to all positions
    }
    digitalWrite(_csPin, HIGH);
    displayOn();
}

void FutabaVFD::showString(uint8_t pos, const char* str) {
    digitalWrite(_csPin, LOW);
    pulseByte(0x20 + pos); // Set RAM Address (Position)
    
    uint8_t count = 0;
    while (str[count] != '\0' && (pos + count) < _digitCount) {
        pulseByte((uint8_t)str[count]);
        count++;
    }
    digitalWrite(_csPin, HIGH);
    displayOn(); // Commit to display
}

/**
 * Custom Character Support
 * Allows creating special symbols (like your clock colon or custom icons).
 */
void FutabaVFD::writeCustomChar(uint8_t pos, uint8_t index, uint8_t data[7]) {
    // 1. Write pixel data to CG-RAM
    digitalWrite(_csPin, LOW);
    pulseByte(0x40 + (index * 8)); 
    for(int i = 0; i < 7; i++) {
        pulseByte(data[i]);
    }
    digitalWrite(_csPin, HIGH);
    
    delayMicroseconds(200); 

    // 2. Map the custom character index to a physical position
    digitalWrite(_csPin, LOW);
    pulseByte(0x20 + pos);    // Target position
    pulseByte(0x00 + index);  // Character index (0-7)
    digitalWrite(_csPin, HIGH);

    displayOn(); 
}