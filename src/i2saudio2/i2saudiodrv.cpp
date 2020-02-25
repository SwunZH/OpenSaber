#include "i2saudiodrv.h"
#include "Grinliz_Util.h"
#include "manifest.h"

#include <Arduino.h>
#include <Adafruit_ZeroI2S.h>
#include <Adafruit_ZeroDMA.h>

#include <stdint.h>

#define DECODE_NONE 0
#define DECODE_SIN 1
#define DECODE_S4 2
#define DECODE DECODE_S4

//#define USE_16_BIT  // doesn't work - not sure if hardware or sw config

#ifdef USE_16_BIT
int16_t stereoBuffer0[AUDDRV_STEREO_SAMPLES];
int16_t stereoBuffer1[AUDDRV_STEREO_SAMPLES];
#else
int32_t stereoBuffer0[AUDDRV_STEREO_SAMPLES];
int32_t stereoBuffer1[AUDDRV_STEREO_SAMPLES];
#endif
DmacDescriptor* descriptor = 0;

int I2SAudioDriver::callbackCycle = 0;
I2SAudioDriver::Status I2SAudioDriver::status[AUDDRV_NUM_CHANNELS];
bool I2SAudioDriver::isQueued[AUDDRV_NUM_CHANNELS];
SPIStream I2SAudioDriver::spiStream[AUDDRV_NUM_CHANNELS];
wav12::ExpanderAD4 I2SAudioDriver::expander[AUDDRV_NUM_CHANNELS];
int I2SAudioDriver::volume[AUDDRV_NUM_CHANNELS];
uint32_t I2SAudioDriver::callbackMicros = 0;


void I2SAudioDriver::DMACallback(Adafruit_ZeroDMA* dma)
{
    int32_t startMicros = micros();
    callbackCycle++;
    #ifdef USE_16_BIT
    int16_t* src = (callbackCycle & 1) ? stereoBuffer1 : stereoBuffer0;
    int16_t* fill = (callbackCycle & 1) ? stereoBuffer0 : stereoBuffer1;
    #else
    int32_t* src  = (callbackCycle & 1) ? stereoBuffer1 : stereoBuffer0;
    int32_t* fill = (callbackCycle & 1) ? stereoBuffer0 : stereoBuffer1;
    #endif
    
    dma->changeDescriptor(descriptor, 
        src,                           // from here         
        (void *)(&I2S->DATA[0].reg),   // to here (M0+)
        AUDDRV_STEREO_SAMPLES);        // this many...

    dma->startJob();

    /* --- decode to fill --- */
#if DECODE == DECODE_SIN
    static uint32_t t = 0;
    static const int TEST_FREQ = 220;
    static const uint32_t MULT = 32768 * TEST_FREQ / AUDDRV_FREQ;

    for(int i=0; i<AUDDRV_BUFFER_SAMPLES; i++, t++) {
        #ifdef USE_16_BIT
        int16_t val = iSin_S3(t * MULT);
        #else
        int32_t val = iSin_S3(t * MULT) * 16000 * 4;
        #endif
        fill[i*2 + 0] = val;
        fill[i*2 + 1] = val;
        ++t;
    }
#elif DECODE == DECODE_S4

    for(int i=0; i<AUDDRV_NUM_CHANNELS; ++i) {
        if (isQueued[i]) {
            isQueued[i] = false;
            spiStream[i].set(status[i].addr, status[i].size);
            expander[i].rewind();
            //Serial.print("queue "); Serial.print(i); Serial.print(" "); Serial.print(status[i].addr); Serial.print(" "); Serial.println(status[i].size);
        }
    }

    for(int i=0; i<AUDDRV_NUM_CHANNELS; ++i) {
        int n = 0;
        if (status[i].addr) {
            n = expander[i].expand(fill, AUDDRV_BUFFER_SAMPLES, volume[i], i > 0, status[i].is8Bit, false);

            if (status[i].loop && n < AUDDRV_BUFFER_SAMPLES) {
                expander[i].rewind();
                expander[i].expand(fill + n*2, AUDDRV_BUFFER_SAMPLES - n, volume[i], i > 0, status[i].is8Bit, false);
                n = AUDDRV_BUFFER_SAMPLES;
            }
        }
        if (i == 0 && n < AUDDRV_BUFFER_SAMPLES) {
            for(int j=n; j<AUDDRV_BUFFER_SAMPLES; ++j) {
                fill[j*2 + 0] = 0;
                fill[j*2 + 1] = 0;
            }
        }
    }
#endif
    uint32_t endMicros = micros();
    callbackMicros += (endMicros - startMicros);
}

void I2SAudioDriver::begin()
{
    Log.p("I2SAudioDriver::begin starting.").eol();

    for(int i=0; i<AUDDRV_NUM_CHANNELS; ++i) {
        spiStream[i].init(spiFlash);
        expander[i].init(&spiStream[i]);   
        volume[i] = 64;
    }

    Log.p("DMA Configure.").eol();
    dma->setTrigger(I2S_DMAC_ID_TX_0);
    dma->setAction(DMA_TRIGGER_ACTON_BEAT);

    Log.p("Allocating DMA channel:").eol();
    ZeroDMAstatus stat = dma->allocate();
    dma->printStatus(stat);

    Log.p("Setting up transfer").eol();
    descriptor = dma->addDescriptor(
        stereoBuffer0,                // move data from here
        (void *)(&I2S->DATA[0].reg),  // to here (M0+)
        AUDDRV_STEREO_SAMPLES,        // this many...
        #ifdef USE_16_BIT
        DMA_BEAT_SIZE_HWORD,           // bytes/hword/words
        #else
        DMA_BEAT_SIZE_WORD,           // bytes/hword/words
        #endif
        true,                         // increment source addr?
        false);

    dma->loop(false);
    Log.p("Adding callback").eol();
    dma->setCallback(DMACallback);
    Log.p("DMA configuration done. Starting I2S").eol();

#ifdef USE_16_BIT
    i2s->begin(I2S_16_BIT, AUDDRV_FREQ);
#else
    i2s->begin(I2S_32_BIT, AUDDRV_FREQ);
#endif
    i2s->enableTx();

    Log.p("I2S started. Starting DMA job.").eol();
    stat = dma->startJob();
    Log.p("I2SAudioDriver begin() complete.").eol();
}


void I2SAudioDriver::play(int index, bool loop, int channel)
{
    // Log.p("I2SAudioDriver::play index=").p(index).p(" loop=").p(loop).p(" ch=").p(channel).eol();
    channel = glClamp(channel, 0, AUDDRV_NUM_CHANNELS - 1);
    const MemUnit& memUnit = manifest.getUnit(index);

    noInterrupts();
    status[channel].addr = memUnit.offset;
    status[channel].size = memUnit.size;
    status[channel].is8Bit = memUnit.is8Bit == true;
    status[channel].loop = loop;
    isQueued[channel] = true;
    interrupts();
}


bool I2SAudioDriver::isPlaying(int channel)
{
    channel = glClamp(channel, 0, AUDDRV_NUM_CHANNELS-1);
    noInterrupts();
    bool playing = !expander[channel].done();
    playing |= isQueued[channel];
    interrupts();
    return playing;    
}


void I2SAudioDriver::stop(int channel)
{
    channel = glClamp(channel, 0, AUDDRV_NUM_CHANNELS - 1);

    noInterrupts();
    status[channel].clear();
    isQueued[channel] = true;
    interrupts();
}


void I2SAudioDriver::setVolume(int v, int channel)
{
    channel = glClamp(channel, 0, AUDDRV_NUM_CHANNELS - 1);
    volume[channel] = v;
}


int I2SAudioDriver::getVolume(int channel)
{
    channel = glClamp(channel, 0, AUDDRV_NUM_CHANNELS - 1);
    return volume[channel];
}


