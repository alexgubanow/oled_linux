#pragma once
#ifndef PROGMEM
#define PROGMEM
#endif

const unsigned char activeSymbol[] PROGMEM = {
    0b00000000,
    0b00000000,
    0b00011000,
    0b00100100,
    0b01000010,
    0b01000010,
    0b00100100,
    0b00011000
};

const unsigned char inactiveSymbol[] PROGMEM = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00011000,
    0b00011000,
    0b00000000,
    0b00000000
};
