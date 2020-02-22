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

#include "expander.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

using namespace wav12;

uint8_t ExpanderAD4::m_buffer[BUFFER_SIZE] = {0};

void ExpanderAD4::init(IStream* stream)
{
    m_stream = stream;
    rewind();
}

void ExpanderAD4::rewind()
{
    m_state = S4ADPCM::State();
    m_stream->rewind();
}

void ExpanderAD4::compress4(const int16_t* data, int32_t nSamples, 
    uint8_t** compressed, uint32_t* nCompressed)
{
    *nCompressed = (nSamples + 1) / 2;
    *compressed = new uint8_t[nSamples];

    S4ADPCM::State state;
    S4ADPCM::encode4(data, nSamples, *compressed, &state);
}


void ExpanderAD4::compress8(const int16_t* data, int32_t nSamples,
    uint8_t** compressed, uint32_t* nCompressed)
{
    *nCompressed = nSamples;
    *compressed = new uint8_t[nSamples];

    S4ADPCM::State state;
    S4ADPCM::encode8(data, nSamples, *compressed, &state);
}


int ExpanderAD4::expand(int32_t *target, uint32_t nSamples, int32_t volume, bool add, bool use8Bit)
{
    if (!m_stream)
        return 0;

    uint32_t n = 0;

    while(n < nSamples) {
        uint32_t samplesWanted = nSamples - n;
        uint32_t bytesWanted = use8Bit ? samplesWanted : (samplesWanted+1) / 2;
        uint32_t bytesFetched = m_stream->fetch(m_buffer, wav12Min(bytesWanted, uint32_t(BUFFER_SIZE)));
        uint32_t samplesFetched = use8Bit ? bytesFetched : bytesFetched * 2;
        if (samplesFetched > samplesWanted) {
            // edge case of odd number of samples.
            samplesFetched = samplesWanted;
        }

        if (bytesFetched) {
            if (use8Bit)
                S4ADPCM::decode8(m_buffer, samplesFetched, volume, add, target + n * 2, &m_state);
            else
                S4ADPCM::decode4(m_buffer, samplesFetched, volume, add, target + n * 2, &m_state);
        }
        else {
            break;
        }
        n += samplesFetched;
    }

    return n;
}

