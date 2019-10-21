#include "s4adpcm.h"

// 4 bit version:
// poweron: mse=4349732 -1 0 0 1 1 1 2 3
// hum:     mse=2437    -1 0 0 0 1 1 2 3
// fiati:   mse=20122   -1 0 0 0 0 0 1 2
const int S4ADPCM::DELTA_TABLE[TABLE_SIZE] = {
    -1, 0, 0, 1, 1, 1, 2, 3
};

void S4ADPCM::encode(const int16_t* data, int32_t nSamples, uint8_t* target, State* state)
{
    for (int i = 0; i < nSamples; ++i) {
        int value = data[i];
        int guess = state->guess();
        int delta = value - guess;

        uint8_t sign = 0;
        if (delta < 0) {
            sign = 8;
            delta = -delta;
        }

        delta >>= state->shift;
        if (delta > 7) delta = 7;

        if (state->high) {
            *target++ |= (delta | sign) << 4;
        }
        else {
            *target = (delta | sign);
        }
        state->high = !state->high;

        int p = guess + (delta << state->shift) * (sign ? -1 : 1);
        state->push(p);

        state->shift += DELTA_TABLE[delta];
        if (state->shift < 0) state->shift = 0;
        if (state->shift > 12) state->shift = 12;
    }
}


void S4ADPCM::decode(const uint8_t* p, int32_t nSamples,
    int volume, bool add, S4ADPCM_INT* out, State* state)
{
    for (int32_t i = 0; i < nSamples; ++i) {
        uint8_t delta = 0;
        if (state->high) {
            delta = *p >> 4;
            p++;
        }
        else {
            delta = *p & 0xf;
        }
        int guess = state->guess();

        uint8_t sign = delta & 0x8;
        delta &= 0x7;
        int value = 0;
        if (sign) {
            value = guess - (delta << state->shift);
        }
        else {
            value = guess + (delta << state->shift);
        }
        state->push(value);

#if S4ADPCM_OUT_BITS == 32
        int32_t s = value * (volume << 8);
        *out = add ? (*out + s) : s;
#    if S4ADPCM_CHANNELS == 2
        *(out + 1) = *out;
        out++;
#    endif
        out++;
#endif

        state->shift += DELTA_TABLE[delta];
        if (state->shift < 0) state->shift = 0;
        if (state->shift > 12) state->shift = 12;
        state->high = !state->high;
    }
}
