#include <Adafruit_SPIFlash.h>
#include <Adafruit_ZeroTimer.h>
#include <Adafruit_ZeroI2S.h>
#include <Adafruit_ZeroDMA.h>

#include <Arduino.h>    

#include "mcaudio.h"
#include "Grinliz_Util.h"
#include "compress.h"
#include "mcmemimage.h"

// Two stereo buffers are ping-ponged between
// filling and the DMA -> DAC.
int32_t I2SAudio::audioBuffer0[STEREO_BUFFER_SAMPLES];            // stereo buffers. throwing away memory. *sigh*
int32_t I2SAudio::audioBuffer1[STEREO_BUFFER_SAMPLES];

// When uncompressed, SPI is read in as 16 bit mono,
// and they are read to a buffer that is expanded to 32 bit stereo.
uint8_t subBuffer[AUDIO_SUB_BUFFER];
wav12::Expander expander(subBuffer, AUDIO_SUB_BUFFER);
DmacDescriptor* dmacDescriptor = 0;

I2STracker I2SAudio::tracker;
I2SAudio* I2SAudio::_instance = 0;

// Information about the state of audioBuffer0/1
AudioBufferData I2SAudio::audioBufferData[NUM_AUDIO_BUFFERS];

volatile uint8_t dmaPlaybackBuffer = 0;

bool I2SAudio::isStreamQueued = false;
uint32_t I2SAudio::queued_addr = 0;
uint32_t I2SAudio::queued_size = 0;
uint32_t I2SAudio::queued_nSamples = 0;
int      I2SAudio::queued_format = 0;

void TC4_Handler(){
    Adafruit_ZeroTimer::timerHandler(4);
}

void I2SAudio::timerCallback()
{
    I2SAudio::tracker.timerCalls++;

    if (I2SAudio::isStreamQueued) {
        I2SAudio::isStreamQueued = false;
        I2SAudio::tracker.timerQueued++;
        SPIStream& spiStream = I2SAudio::instance()->spiStream;
        spiStream.init(I2SAudio::queued_addr, I2SAudio::queued_size);
        expander.init(&spiStream, I2SAudio::queued_nSamples, I2SAudio::queued_format);
    }

    for(int i=0; i<NUM_AUDIO_BUFFERS; ++i) {
        if (audioBufferData[i].status == AUDBUF_EMPTY) {
            ASSERT(audioBuffer0 == audioBufferData[0].buffer);
            ASSERT(audioBuffer1 == audioBufferData[1].buffer);
            I2SAudio::tracker.timerFills++;
            int rc = audioBufferData[i].fillBuffer(expander, I2SAudio::instance()->expandVolume());
            if (rc != 0) {
                I2SAudio::tracker.timerErrors++;
            }
        }
    }
}

void I2SAudio::dmaCallback(Adafruit_ZeroDMA* dma)
{
    audioBufferData[dmaPlaybackBuffer].status = AUDBUF_EMPTY;
    dmaPlaybackBuffer = (dmaPlaybackBuffer + 1) % NUM_AUDIO_BUFFERS;
    
    I2SAudio::tracker.dmaCalls++;
    if (audioBufferData[dmaPlaybackBuffer].status != AUDBUF_READY) {
        I2SAudio::tracker.dmaErrors++;
    }
    audioBufferData[dmaPlaybackBuffer].status = AUDBUF_DRAINING;
    int32_t* src = audioBufferData[dmaPlaybackBuffer].buffer;

    dma->changeDescriptor(
        dmacDescriptor,
        src,                            // move data from here
        (void *)(&I2S->DATA[0].reg),    // to here (M0+)
        STEREO_BUFFER_SAMPLES);         // this many...
    dma->startJob();
}

I2SAudio::I2SAudio(Adafruit_ZeroI2S& _i2s, Adafruit_ZeroTimer& _timer, Adafruit_ZeroDMA& _dma, Adafruit_SPIFlash& _spiFlash, SPIStream& _stream) : 
    i2s(_i2s),
    timer(_timer),
    audioDMA(_dma),
    spiFlash(_spiFlash),
    spiStream(_stream)
{
    _instance = this;
}


void I2SAudio::init()
{
    audioBufferData[0].buffer = audioBuffer0;
    audioBufferData[1].buffer = audioBuffer1;
    ASSERT(audioBuffer0 == audioBufferData[0].buffer);
    audioBufferData[0].reset();
    audioBufferData[1].reset();
    ASSERT(audioBuffer0 == audioBufferData[0].buffer);

    MemUnit file;
    readFile(spiFlash, 0, &file);
    wav12::Wav12Header header;
    uint32_t baseAddr = 0;
    readAudioInfo(spiFlash, file, &header, &baseAddr);

    spiStream.init(baseAddr, header.lenInBytes);
    expander.init(&spiStream, header.nSamples, header.format);
    ASSERT(audioBuffer0 == audioBufferData[0].buffer);
    audioBufferData[0].fillBuffer(expander, expandVolume());
    ASSERT(audioBuffer0 == audioBufferData[0].buffer);

    Log.p("-----------").eol();
    Log.p("Expander: samples=").p(expander.samples()).p(" pos=").p(expander.pos()).eol();
    Log.p("addr=").p(baseAddr).p(" nBytes=").p(header.lenInBytes).p(" nSamples=").p(header.nSamples).p(" format=").p(header.format).eol();
    uint32_t check = 0;
    for(int i=0; i<STEREO_BUFFER_SAMPLES; i+=2) {
        check += abs(audioBuffer0[i]);
    }
    ASSERT(audioBuffer0 == audioBufferData[0].buffer);
    Log.p("Buffer fill check=").p(check).eol();

    Log.p("Configuring DMA trigger").eol();
    audioDMA.setTrigger(I2S_DMAC_ID_TX_0);
    audioDMA.setAction(DMA_TRIGGER_ACTON_BEAT);

    ZeroDMAstatus stat = audioDMA.allocate();
    audioDMA.printStatus(stat);

    Log.p("Setting up transfer").eol();
    dmacDescriptor = audioDMA.addDescriptor(
        audioBuffer0,                   // move data from here
        (void *)(&I2S->DATA[0].reg),    // to here (M0+)
        STEREO_BUFFER_SAMPLES,          // this many...
        DMA_BEAT_SIZE_WORD,             // bytes/hword/words
        true,                           // increment source addr?
        false);

    Log.p("Adding callback").eol();
    audioDMA.setCallback(I2SAudio::dmaCallback);

    i2s.begin(I2S_32_BIT, AUDIO_FREQ);
    i2s.enableTx();

    // Clock: 48 000 000 / second
    // div 1024 = 46875 / second
    // 
    timer.configure( TC_CLOCK_PRESCALER_DIV1024,    // prescaler
                  TC_COUNTER_SIZE_16BIT,         // bit width of timer/counter
                  TC_WAVE_GENERATION_MATCH_PWM   // frequency or PWM mode
                 );

    static const uint32_t CLOCK = (48*1000*1000) / 1024;         // ticks / second
    static const uint32_t HZ = 1000 / MSEC_PER_AUDIO_BUFFER;
    static const uint32_t COUNTER = CLOCK / (HZ * 4);
    Log.p("MSec/Buffer=").p(MSEC_PER_AUDIO_BUFFER).p(" Counter=").p(COUNTER).eol();
    timer.setCompare(0, COUNTER);

    timer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, I2SAudio::timerCallback);  // this one sets pin low
    timer.enable(true);

    stat = audioDMA.startJob();
}


bool I2SAudio::play(int fileIndex)
{
    MemUnit file;
    readFile(spiFlash, fileIndex, &file);
    wav12::Wav12Header header;
    uint32_t baseAddr = 0;
    readAudioInfo(spiFlash, file, &header, &baseAddr);

    Log.p("Play [").p(fileIndex).p("]: lenInBytes=").p(header.lenInBytes).p(" nSamples=").p(header.nSamples).p(" format=").p(header.format).eol();

    noInterrupts();
    I2SAudio::queued_addr = baseAddr;
    I2SAudio::queued_size = header.lenInBytes;
    I2SAudio::queued_nSamples = header.nSamples;
    I2SAudio::queued_format = header.format;
    I2SAudio::isStreamQueued = true;
    interrupts();

    return header.nSamples > 0;
}


bool I2SAudio::play(const char* filename)
{
    Log.p("play").eol();
    return true;
}


void I2SAudio::stop()
{
    Log.p("stop").eol();
    noInterrupts();
    I2SAudio::queued_addr = 0;
    I2SAudio::queued_size = 0;
    I2SAudio::queued_nSamples = 0;
    I2SAudio::queued_format = 0;
    I2SAudio::isStreamQueued = true;
    interrupts();
}


bool I2SAudio::isPlaying()
{
    noInterrupts();
    bool isQueued = I2SAudio::isStreamQueued;
    bool hasSamples = expander.pos() < expander.samples();    
    interrupts();

    return isQueued || hasSamples;
}



int AudioBufferData::fillBuffer(wav12::Expander& expander, int32_t volume)
{
    if (status != AUDBUF_EMPTY) {
        I2SAudio::tracker.fillErrors++;
        return AUDERROR_BUFFER_NOT_EMPTY;
    }
    status = AUDBUF_FILLING;

    uint32_t MILLION2 = 2 * 1024 * 1024;
    if (expander.samples() < expander.pos()) {
        I2SAudio::tracker.fillErrors++;
        return AUDERROR_SAMPLES_POS_OUT_OF_RANGE;
    }
    if (expander.samples() > MILLION2 || expander.pos() > MILLION2) {
        I2SAudio::tracker.fillErrors++;
        return AUDERROR_SAMPLES_POS_OUT_OF_RANGE;
    }

    uint32_t toRead = glMin(expander.samples() - expander.pos(), (uint32_t)AUDIO_BUFFER_SAMPLES);
    if (toRead) {
        // The expand is buried here:
        expander.expand2(buffer, toRead, volume);
        I2SAudio::tracker.fillSome++;
    }
    else {
        I2SAudio::tracker.fillEmpty++;
    }

    for(int i=toRead*2; i<STEREO_BUFFER_SAMPLES; ++i) {
        buffer[i] = 0;
    }
    status = AUDBUF_READY;
    return AUDERROR_NONE;
}


uint32_t SPIStream::fetch(uint8_t* target, uint32_t nBytes)
{
    uint32_t r = m_flash.readBuffer(
        m_addr + m_pos, 
        target,
        nBytes);
    m_pos += nBytes;
    return r;
}
