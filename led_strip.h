#include <Adafruit_NeoPixel.h>
#include "debug.h"

// A LED strip (LedStrip_c) is the main Class containing the whole LED strip information.
//
// The LED strip is split into a STRIP_SECTIONS_MAX number of sections (Section_c class). Ecah section
// can behave individual (e.g. one section moves left, while another section moves right).
// By default all sections have the LEDs turned of. 
// The sections can overlap each other. 


#define STRIP_SECTIONS_MAX 10

#define LED_ARRAY_SIZE LED_CNT/32 + 1
// Which type of behaviour shall the LEDs in the section do
typedef enum {
  MOVE_LEFT,          // Move to the left
  MOVE_RIGHT,         // Move to the right
  BOUNCE,             // Bounce back an forward
  COLOR_WIPE,         // Fill the LEDs one after the other
  CONSTANT,           // Keep LEDs in current possition
} sectionFunction_t;

// Id for selecting which color to work on.
typedef enum {
  COLOR1 = 0,
  COLOR2 = 1,
} colorId_t;


typedef enum {
   // 0 - 0xFFFFFF - RGB colors
   RANDOM = 0xFFFFFFFF
} color_t;

// Which type of behaviour shall the LEDs in the section do
typedef enum {
  LEFT,          // Do movment in left direction
  RIGHT,         // Do movment in right direction
  BOUNCING,      // Bounce back an forward
} direction_t;

typedef uint32_t belt_t[LED_ARRAY_SIZE];

// Current configuration for the section
struct section_cfg_t {
  belt_t leds;        // Containing which LEDs that is part of this section - One bit per LED. '1' Indicating that the LED is part of the section.
  belt_t currentLeds; // Containing the current value of the LEDs. '1' Indicating color2, '0' indcating color1 
  uint8_t moveCnt;    // Number of LEDs to move in each turn

  // The LEDs color will be colored with colors going from color1 to color2
  color_t color1;             // Color for LED bits set to '1'
  color_t color2;             // Color for LED bits set to '0'
  uint16_t  slowness;           // The speed of the behaviour. 0 = fastest, 255 = slowest
  sectionFunction_t func;      // The LEDs behaviour.
  uint8_t firstLedPos;         // The position of the first LED in this group
  uint8_t lastLedPos;          // The position of the last LED in this group
  direction_t currentBounceDir = LEFT; // The current direction of the movement.
};


//
//  Class for a section configuration
//
// Class for doing the acton for a section
class Section_c {
  public:
    void setLedColor(const uint16_t startLed, const uint16_t endLed, const colorId_t colorId, const color_t color);
    void setFunc(sectionFunction_t newFunc, uint8_t moveCnt = 1);
    void init(direction_t _currentBounceDir = LEFT);                     // Reset the setup to init positions.
    void updateLeds(uint32_t *leds); // Update LED array with the LEDs states in this section
    Section_c(void);
    void setSlowness(const uint16_t newSlowness); // Set a new speed
    
  private:
    section_cfg_t cfg = {};          // Containing current configurations.
    void Section_c::setCurrentLedPos(uint16_t pos, bool value);
    bool Section_c::getCurrentLedPos(uint16_t pos);
   
    void bounce(void);                // Bounce back and forward within the section
    uint32_t fade(uint32_t color, uint8_t fade_pct);             
 
    void moveDir(direction_t direction);  // Move LEDs "cnt" either left or right
    void colorWipe(direction_t dir);  // Fill the LEDs one after the other
    bool doMove();                    // Returns if movment shall happen based on current moveCnt.
    uint16_t delayCnt = 0;            // Current number of ticks until LED movment shall happen.
    bool withinRange(const uint16_t pos, const belt_t &leds);   // Returns true if "pos" is within the onLeds
};

//
// Class for the whole strip
//
class LedStrip_c {
  public:
    Section_c sections[STRIP_SECTIONS_MAX];
    void timeTick(uint16_t cnt); // Do time tick "cmd" number of times;
    LedStrip_c(void); // Constructor - Initializing.
    void sectionCntSet(uint8_t _sectionCnt);
  private:
    
    uint8_t totalStripLength;
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_CNT, PIN, NEO_GRB + NEO_KHZ800);
    uint8_t sectionCnt;     

};

/************************************************************************************************
   Implementation start
 *************************************************************************************************/
// Index for each section added
static size_t section_cfg_index = 0;
Section_c::Section_c(void) {
 memset(&cfg, 0, sizeof(cfg));
 cfg.func = CONSTANT;
 cfg.moveCnt = 1;
 cfg.slowness = 0;
}

void Section_c::setSlowness(const uint16_t newSlowness) {
  cfg.slowness = newSlowness;
}

void Section_c::setFunc(sectionFunction_t newFunc, uint8_t moveCnt = 1) {
  cfg.func = newFunc;
  cfg.moveCnt = moveCnt;
}

bool Section_c::getCurrentLedPos(uint16_t pos) {
  auto bindex = pos / 32;
  auto b = pos % 32;
  if (cfg.currentLeds[bindex] & ((uint32_t) 1 << b)) {
      return true;
  }
  return false; 
}

void Section_c::setCurrentLedPos(uint16_t pos, bool value) {
  auto bindex = pos / 32;
  auto b = pos % 32;

  if (value) {
    cfg.currentLeds[bindex] |= (uint32_t) 1 << b;
  } else {
    cfg.currentLeds[bindex] &= ~((uint32_t) 1 << b);
  }
}

void Section_c::setLedColor(uint16_t startLed, uint16_t endLed, colorId_t colorId, color_t color) {
  if (startLed > endLed) {
    T("Error");
    return;
  }
  if (cfg.firstLedPos > startLed) {
    cfg.firstLedPos = startLed;
  }

  if (cfg.lastLedPos < endLed) {
    cfg.lastLedPos = endLed;
  }

  if (color == RANDOM) {
    color = random(0x1000000);  
  }
  
  for (auto index = startLed; index <= endLed; index++) {
    auto bindex = index / 32;
    auto b = index % 32;
    cfg.leds[bindex] |= (uint32_t) 1 << b;
  //  T_V("Bindex", bindex);
    
    if (colorId == COLOR1) {
  //    T_V("b", b);  
      cfg.currentLeds[bindex] |= (uint32_t) 1 << b;
      cfg.color1 = color;
    } else {
   //   T_V("b", b);  
      cfg.currentLeds[bindex] &= ~((uint32_t) 1 << b);
      cfg.color2 = color;
    }
  }
}
// Initialize to start posistion when starting a new section setup
void Section_c::init(direction_t _currentBounceDir = LEFT) {
  T("init section");
  memset(&cfg, 0, sizeof(cfg));
  T("init section");

  delayCnt = 0;
  T("init section");
  cfg.firstLedPos = LED_CNT;
  cfg.func = CONSTANT;
  cfg.currentBounceDir = _currentBounceDir;
  T("init section Done");
}

// Returns true if "pos" is within the onLeds
bool Section_c::withinRange(const uint16_t pos, const belt_t &leds) {
  uint8_t arrIndex = pos/32;
  uint8_t bitIndex = pos % 32;
 /* T_V("f", ((uint32_t)1 << bitIndex));
  T_V("g", leds[arrIndex] & ((uint32_t)1 << bitIndex));
  T_V("leds[arrIndex]", leds[arrIndex]);
  */
  /*
  T_V("bitIndex:", bitIndex);
  T_V("arrIndex:", arrIndex);*/
//  T_V("pos:", pos);
 
  if (leds[arrIndex] & ((uint32_t)1 << bitIndex)) {
  //  T("true");
    return true; 
  } 
 // T("false");
  return false; 
}

// Do the LED behaviour and add the LEDs from this section to the "leds" array
void Section_c::updateLeds(uint32_t *leds) {
    //T_V("delayCnt:", delayCnt);
    if (delayCnt == 0) {
      switch (cfg.func) {
      case MOVE_RIGHT:
        moveDir(RIGHT);
        break;
      case MOVE_LEFT:
        //T("Left");
        moveDir(LEFT);
        break;
      case COLOR_WIPE:
        T("Wipte");
        break;
      case BOUNCE:
        //T("Bounce");
        bounce();
        break;
      case CONSTANT:
        //T("Constant");
        break;
     }

      delayCnt = cfg.slowness; 
    } else {
       delayCnt--;
    }

  uint8_t fade_pct = 90;
  for (uint16_t i = 0; i < LED_CNT; i++ ) {
    if (withinRange(i, cfg.leds)) {
      if (withinRange(i, cfg.currentLeds)) {
   //     T(cfg.color1);
        
  //       leds[i] |= fade(cfg.color1, fade_pct);

          leds[i] |= cfg.color1;
      } else {
          leds[i] |= cfg.color2;
    //          leds[i] |= fade(cfg.color2, fade_pct);
    //    T(cfg.color2);
      }
      fade_pct = (fade_pct * 60) / 100;
     // T(fade_pct); 
    }
    
  }
}
/*
// Increase a position with "cnt", keeps track of section bounderies
void Section_c::incrPos(uint16_t &pos, uint16_t cnt) {
  if (pos + cnt > cfg.sectionLastPos) {
    pos = cnt - (cfg.sectionLastPos - pos) - 1 + cfg.sectionFirstPos;
  } else {
    pos += cnt;
  }
}

// Decrease a position with "cnt", keeps track of section bounderies
void Section_c::decrPos(uint16_t &pos, uint16_t cnt) {
  int16_t nextPos = pos - cnt;
  if (nextPos < (int16_t)cfg.sectionFirstPos) {
    pos = (cfg.sectionLastPos + (pos - cfg.sectionFirstPos) + 1) - cnt;
  } else {
    pos -= cnt;
  }
}
*/
/*
// See class definition
// In : startLed - If true then bounce when startLed reaches section boundery.
//                 If false then bounce when endLed reaches section boundery.
bool Section_c::currentBounceDirLeft(bool startLed) {
  static bool leftDirection = true;
  uint32_t testLed = currentOnLedEndPos;
  if (startLed) {
    testLed = currentOnLedStartPos;
  }

  if (testLed == cfg.sectionLastPos) {
    leftDirection = true;
  }

  if (testLed == cfg.sectionFirstPos) {
    leftDirection = false;
  }
  return leftDirection;
}
*/
  
// Bounce back and forward
void Section_c::bounce(void) {  
  if (cfg.currentBounceDir == RIGHT) {
     T_V("cfg.lastLedPos:", cfg.lastLedPos);  
     if (withinRange(cfg.lastLedPos, cfg.currentLeds)) {
        cfg.currentBounceDir = LEFT;
     }
   } else {
     T_V("cfg.firstLedPos:", cfg.firstLedPos);
      if (withinRange(cfg.firstLedPos, cfg.currentLeds)) {
          cfg.currentBounceDir = RIGHT;
      }
   }
   moveDir(cfg.currentBounceDir);
}

uint32_t Section_c::fade(uint32_t color, uint8_t fade_pct) {
    uint32_t blue = (((color >> 16) & 0xFF) * fade_pct) / 100;
    uint32_t green = (((color >> 8) & 0xFF) * fade_pct) / 100;
    uint32_t red = ((color & 0xFF) * fade_pct) / 100;
    T(color);
    T_V("red:", red);
    T(blue);
    T(green);
   
    return (red << 16) + (green<< 8) + blue;

}

void Section_c::moveDir(direction_t dir) {
  uint16_t prevPos  = 0;
  bool     prevVal  = false;
  bool     firstVal = false;
  uint16_t lastPos = 0;

  
  // Default is set to move LEFT
  int16_t startPos = 0;
  int16_t endPos = LED_CNT;
  int8_t  stepCnt = 1;
  
  if (dir == RIGHT) {
    startPos = LED_CNT-1;
    endPos = -1;
    stepCnt = -1;
  }

  for (auto i = 0; i < cfg.moveCnt; i++) {
  bool first = true;

  for (int16_t currentPos = startPos; currentPos != endPos; currentPos += stepCnt) {
    if (withinRange(currentPos, cfg.leds)) {
   //    T_V("currentPos:", currentPos);
       if (first) {
         firstVal = getCurrentLedPos(currentPos);
         first = false;
       } else {
        /* T_V("prevPos", prevPos);
         T_V("prevVal", prevVal);
      */
         setCurrentLedPos(prevPos, getCurrentLedPos(currentPos));
       }
       
       prevVal = getCurrentLedPos(currentPos);
       prevPos = currentPos;
       lastPos = currentPos;
    }
    
  }
  setCurrentLedPos(lastPos, firstVal);
  }
}    
//
// LedStrip_c implementation
//
void LedStrip_c::timeTick(uint16_t cnt) {
  
    // We start out with all LEDs off
    uint32_t led_colors[LED_CNT];
    memset(&led_colors[0], 0, sizeof(led_colors));
    // Loop through all sections and add their LEDs state
    for (auto i = 0; i < sectionCnt; i++) {
        // T_V("Strip", i);
        sections[i].updateLeds(led_colors);
    }

    // Turn on the LEDs
    for (auto i = 0; i < LED_CNT; i++) {
       //Serial.print("i:");
       //Serial.print(i);
       //Serial.print(" c:"); 
       //  T(led_colors[i]);
       strip.setPixelColor(i, led_colors[i]);
    }

    strip.show();
}

LedStrip_c::LedStrip_c(void) {
  strip.begin();
}

void LedStrip_c::sectionCntSet(uint8_t _sectionCnt) {
  if (_sectionCnt < STRIP_SECTIONS_MAX) {
    sectionCnt = _sectionCnt;
  } else {
    sectionCnt = STRIP_SECTIONS_MAX;
  }
}


