#include <Adafruit_NeoPixel.h>
#include "debug.h"

// A LED strip (LedStrip_c) is the main Class containing the whole LED strip information.
//
// The LED strip is split into a STRIP_SECTIONS_MAX number of sections (Section_c class). Ecah section
// can behave individual (e.g. one section moves left, while another section moves right).
// By default all sections have the LEDs turned of. 
// The sections can overlap each other. 



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
  uint32_t color1;             // Color for LED bits set to '1'
  uint32_t color2;             // Color for LED bits set to '0'
  uint8_t  slowness;           // The speed of the behaviour. 0 = fastest, 255 = slowest
  sectionFunction_t func;      // The LEDs behaviour.
 
};


//
//  Class for a section configuration
//
// Class for doing the acton for a section
class Section_c {
  public:
    void setLedColor(const uint16_t startLed, const uint16_t endLed, const colorId_t colorId, const uint32_t color);
    void setFunc(sectionFunction_t newFunc, uint8_t moveCnt = 1);
    void init();                     // Reset the setup to init positions.
    void updateLeds(uint32_t *leds); // Update LED array with the LEDs states in this section
    Section_c(void);
    void setSlowness(const uint8_t newSlowness); // Set a new speed
    
  private:
    section_cfg_t cfg = {};          // Containing current configurations.
    void Section_c::setCurrentLedPos(uint16_t pos, bool value);
    bool Section_c::getCurrentLedPos(uint16_t pos);
   
    void bounce(void);                // Bounce back and forward within the section
    void moveDir(direction_t direction);  // Move LEDs "cnt" either left or right
    void colorWipe(direction_t dir);  // Fill the LEDs one after the other
    bool doMove();                    // Returns if movment shall happen based on current moveCnt.
    uint16_t delayCnt = 0;            // Current number of ticks until LED movment shall happen.
    bool withinRange(const uint16_t pos, const belt_t &leds);   // Returns true if "pos" is within the onLeds
    bool currentBounceDirLeft(bool startLed = true); // Returns true if current direction for a bounce is left
         
};

//
// Class for the whole strip
//
#define STRIP_SECTIONS_MAX 5
class LedStrip_c {
  public:
    Section_c sections[STRIP_SECTIONS_MAX];
    //void setupsection(Section_cfg_c section);
    void timeTick(uint16_t cnt); // Do time tick "cmd" number of times;
    LedStrip_c(void); // Constructor - Initializing.

  private:
    
    uint8_t totalStripLength;
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_CNT, PIN, NEO_GRB + NEO_KHZ800);
};

/************************************************************************************************
   Implementation start
 *************************************************************************************************/
// Index for each section added
static size_t section_cfg_index = 0;
/*
// Support function for constructor
void Section_cfg_c::doCfg(uint16_t sectionFirstPos, uint16_t sectionLastPos, uint16_t onStartPos) {
  index = section_cfg_index;
  section_cfg_index++;
  cfg.sectionFirstPos  = sectionFirstPos;
  cfg.sectionLastPos   = sectionLastPos;
  cfg.onStartPos       = onStartPos;
  cfg.onEndPos         = onStartPos;

  //Default use the same color for all LEDs
  cfg.color1      = random(0xFFFFFF);
  cfg.color2       = cfg.color1;
  cfg.func         = CONSTANT;
  T("Init Done");
}

void Section_cfg_c::setColor(const color_id_t id, const belt_t &led_bits, const uint32_t color) {
   cfg.colors[id] = color;

   // Add the new LED bits
   for (auto i = 0; i < LED_ARRAY_SIZE; i++) {
      cfg.leds[i] |= led_bits[i];
      
      if (id == COLOR1) {
          cfg.currentLeds[i] |= led_bits[i]; // Set bits to '1' for all new bits for color1
      }

      if (id == COLOR2) {
          cfg.currentLeds[i] &= ~led_bits[i]; // Set bits to '0' for all new bits for color2
  }
   }
}
    

void Section_cfg_c::setFuncMoveRight(void) {
  cfg.func = MOVE_RIGHT;
}

void Section_cfg_c::setFuncBounce(void) {
  cfg.func = BOUNCE;
}


void Section_cfg_c::setLeds(uint16_t setLedNo) {
//  uint8_t arrIndex = pos/32;
//  uint8_t bitIndex = pos % 32;
  //cfg.leds[arrIndex] 
 // memcpy(&cfg.leds[0], newLeds, sizeof(cfg.leds));
}

void Section_cfg_c::setFuncColorWipe(direction_t dir) {
  cfg.func = COLOR_WIPE;
  cfg.direction = dir;
}

*/
Section_c::Section_c(void) {
 memset(&cfg, 0, sizeof(cfg));
 cfg.func = CONSTANT;
 cfg.moveCnt = 1;
 cfg.slowness = 0;
}

void Section_c::setSlowness(const uint8_t newSlowness) {
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

void Section_c::setLedColor(uint16_t startLed, uint16_t endLed, colorId_t colorId, uint32_t color) {
  if (startLed > endLed) {
    T("Error");
    return;
  }
  
  for (auto index = startLed; index <= endLed; index++) {
    auto bindex = index / 32;
    auto b = index % 32;
    cfg.leds[bindex] |= (uint32_t) 1 << b;
   // T_V("Bindex", bindex);
   // T_V("b", b);  
    if (colorId == COLOR1) {
      cfg.currentLeds[bindex] |= (uint32_t) 1 << b;
      cfg.color1 = color;
    } else {
      cfg.currentLeds[bindex] &= ~((uint32_t) 1 << b);
      cfg.color2 = color;
    }
  }
}
// Initialize to start posistion when starting a new section setup
void Section_c::init(void) {
  T("init section");
  memset(&cfg, 0, sizeof(cfg));
  T("init section");

  delayCnt = 0;
  T("init section");

  cfg.func = CONSTANT;
  T("init section Done");
}

// Returns true if "pos" is within the onLeds
bool Section_c::withinRange(const uint16_t pos, const belt_t &leds) {
  uint8_t arrIndex = pos/32;
  uint8_t bitIndex = pos % 32;
  // T_V("f", ((uint32_t)1 << bitIndex));
 // T_V("g", leds[arrIndex] & ((uint32_t)1 << bitIndex));
 // T_V("leds[arrIndex]", leds[arrIndex]);
  if (leds[arrIndex] & ((uint32_t)1 << bitIndex)) {
  //T("true");
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
        T("Bounce");
      //  bounce();
        break;
      case CONSTANT:
        //T("Constant");
        break;
     }

      delayCnt = cfg.slowness; 
    } else {
       delayCnt--;
    }
     
  for (uint16_t i = 0; i < LED_CNT; i++ ) {
    if (withinRange(i, cfg.leds)) {
      if (withinRange(i, cfg.currentLeds)) {
   //     T(cfg.color1);
         leds[i] |= cfg.color1;
      } else {
         leds[i] |= cfg.color2;
    //    T(cfg.color2);
      }
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
/*
// Bounce back and forward
void Section_c::bounce(void) {
  bool leftDirection = currentBounceDirLeft();

  if (leftDirection) {
    moveLeft(1);
  } else {
    moveRight(1);
  }
}

void Section_c::moveRight(uint16_t cnt) {
  incrPos(currentOnLedStartPos, cnt);
  incrPos(currentOnLedEndPos, cnt);
}
*/
void Section_c::moveDir(direction_t dir) {
  uint16_t prevPos  = 0;
  bool     prevVal  = false;
  bool     firstVal = false;
  uint16_t lastPos = 0;

  
  // Default is set to move LEFT
  int16_t startPos = 0;
  int16_t endPos = LED_CNT -1;
  int8_t  stepCnt = 1;
  
  if (dir == RIGHT) {
    startPos = LED_CNT-1;
    endPos = 0;
    stepCnt = -1;
  }

  for (auto i = 0; i < cfg.moveCnt; i++) {
  bool first = true;

  for (uint16_t currentPos = startPos; currentPos != endPos; currentPos += stepCnt) {
    if (withinRange(currentPos, cfg.leds)) {
       if (first) {
         firstVal = getCurrentLedPos(currentPos);
         first = false;
       } else {
       //  T_V("prevPos", prevPos);
       //  T_V("prevVal", prevVal);
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
    for (auto i = 0; i < STRIP_SECTIONS_MAX; i++) {
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
/*
void LedStrip_c::setupsection(Section_cfg_c section) {

  if (section.cfg.sectionFirstPos > section.cfg.onStartPos) {
    section.cfg.onStartPos = section.cfg.sectionFirstPos;
  }

  if (section.cfg.sectionLastPos < section.cfg.onEndPos || section.cfg.sectionFirstPos > section.cfg.onEndPos) {
    section.cfg.onEndPos = section.cfg.sectionLastPos;
  }

  sections[section.index].cfg = section.cfg;
  sections[section.index].init();
}
*/
LedStrip_c::LedStrip_c(void) {
  T("Init");
  strip.begin();
}


