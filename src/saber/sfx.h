/*
Copyright (c) 2016 Lee Thomason, Grinning Lizard Software

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

#ifndef SFX_HEADER
#define SFX_HEADER

#include <Arduino.h>
#include <Grinliz_Arduino_Util.h>

// SFX in priority order!
enum {            //  Max
  SFX_IDLE,       //  1
  SFX_MOTION,     //  16
  SFX_IMPACT,     //  16
  SFX_USER_TAP,   //  4
  SFX_USER_HOLD,  //  1
  SFX_POWER_ON,   //  4
  SFX_POWER_OFF,  //  4

  NUM_SFX_TYPES,
  MAX_SFX_FILES = 48,
  SFX_NONE = 255
};

enum {
  SFX_GREATER,
  SFX_GREATER_OR_EQUAL,
  SFX_OVERRIDE
};

class AudioPlayer;

class SFX
{
public:
  SFX(AudioPlayer* audioPlayer);

  bool init();
  bool playSound(int sfx, int mode);
  bool bladeOn(bool on);
  void process();

  const uint32_t getIgniteTime() const { return 1000; }
  const uint32_t getRetractTime() const { return 1000; }

  // Only works if no sound playing
  void mute(bool muted) {}
  bool isMuted() const  { return false; }
  
  // Only works if no sound playing. And slow.
  uint8_t setVolume204(int vol) { return vol; }
  uint8_t getVolume204() const  { return 200; }
  
private:
  void scanFiles();
  void addFile(const char* filename, int index);
  int calcSlot(const char* filename); // -1 if not a supported file

  // note: initialize to 255
  struct SFXLocation {
    uint8_t start;
    uint8_t count;
  
    const bool InUse() const { return start < 255 && count < 255; }
  };

  AudioPlayer* m_player;
  bool         m_bladeOn;
  int          m_numFilenames;
  int          m_currentSound;
  SFXLocation  m_location[NUM_SFX_TYPES];
  CStr<13>     m_filename[MAX_SFX_FILES];
  
  static SFX*  m_instance;
};

#endif // SFX_HEADER
