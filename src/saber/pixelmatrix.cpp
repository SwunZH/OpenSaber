/*
  Copyright (c) Lee Thomason, Grinning Lizard Software

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <Arduino.h>
#include "pins.h"
#include "pixelmatrix.h"
#include "assets.h"

#ifdef ANODE_COL_1

static const int NUM_ANODES = 5;
static const int NUM_CATHODES = 7;

static const uint8_t ANODES[NUM_ANODES] = {
    ANODE_COL_1,
    ANODE_COL_2,
    ANODE_COL_3,
    ANODE_COL_4,
    ANODE_COL_5
};

static const uint8_t CATHODES[NUM_CATHODES] = {
    CATHODE_ROW_1,
    CATHODE_ROW_2,
    CATHODE_ROW_3,
    CATHODE_ROW_4,
    CATHODE_ROW_5,
    CATHODE_ROW_6,
    CATHODE_ROW_7
};


PixelMatrix::PixelMatrix()
{
    for(int i=0; i<NUM_ANODES; ++i) {
        pinMode(ANODES[i], OUTPUT);
        digitalWrite(ANODES[i], LOW);
    }
    for(int i=0; i<NUM_CATHODES; ++i) {
        pinMode(CATHODES[i], OUTPUT);
        digitalWrite(CATHODES[i], HIGH);
    }
}


void PixelMatrix::loop(uint32_t t, const uint8_t* pixels)
{
    /* Remember the connection is from 
       anode (HIGH) to cathode (LOW)
    */

    uint32_t y = (t/1) % NUM_ANODES;
    for(uint8_t j=0; j<NUM_ANODES; ++j) {
        digitalWrite(ANODES[j], j == y ? HIGH : LOW);
    }
    for(uint8_t x=0; x<7; ++x) {
        uint8_t b = pixels[x] & (1 << (4-y));
        digitalWrite(CATHODES[x], b ? LOW : HIGH);
    }

#if 0
    uint32_t y = (t/1000) % NUM_ANODES;
    for(uint8_t j=0; j<NUM_ANODES; ++j) {
        digitalWrite(ANODES[j], j == y ? HIGH : LOW);
    }
    for(uint8_t i=0; i<7; ++i) {
        digitalWrite(CATHODES[i], LOW);
    }
#endif
#if 0
    uint32_t y = (t/2) % NUM_ANODES;
    for(uint8_t j=0; j<NUM_ANODES; ++j) {
        digitalWrite(ANODES[j], j == y ? HIGH : LOW);
    }

    uint8_t g0[4], g1[4];
    getGlypth_tomThumb5('P', g0);
    getGlypth_tomThumb5('4', g1);

    for(uint8_t x=0; x<3; ++x) {
        uint8_t b = g0[x] & (1 << (4-y));
        digitalWrite(CATHODES[x], b ? LOW : HIGH);      
    }
    for(uint8_t x=0; x<3; ++x) {
        uint8_t b = g1[x] & (1 << (4-y));
        digitalWrite(CATHODES[x+4], b ? LOW : HIGH);      
    }
#endif
}

#endif
