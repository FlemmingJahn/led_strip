#include <Adafruit_NeoPixel.h>

// Which type of behaviour shall the LEDs in the section do
typedef enum {
  MOVE_LEFT,          // Move to the left
  MOVE_RIGHT,         // Move to the right
  BOUNCE,             // Bounce back an forward
  COLOR_WIPE,         // Fill the LEDs one after the other
  CONSTANT,           // Keep LEDs in current possition
} sectionFunction_t;

// Which type of behaviour shall the LEDs in the section do
typedef enum {
  LEFT,          // Do movment in left direction
  RIGHT,         // Do movment in right direction
  BOUNCING,      // Bounce back an forward
} direction_t;

// Current configuration for the section
struct section_cfg_t {
  uint16_t sectionFirstPos;    // Position of the first LED in the section.
  uint16_t sectionLastPos;     // Position of the last LED in the section.

  uint16_t onStartPos;         // Position of the first LED of the LEDs that shall be turned on
  uint16_t onEndPos;           // Position of the last LED of the LEDs that shall be turned on

  // The LEDs color will be colored with colors going from firstColor to lastColor
  uint32_t firstColor;         // Color of the first LED that is turned on
  uint32_t lastColor;          // Color of the last LED that is turned on

  uint16_t slowness;           // The speed of the behaviour. The lower number the faster behaviour change
  sectionFunction_t func;      // The LEDs behaviour.

  // Variables
  direction_t direction;  // Current movement direction
};

//
//  Class for a section configuration
//
struct Section_cfg_c {
  public:
    Section_cfg_c(void);
    Section_cfg_c(int sectionFirstPos, int sectionLastPos);

    section_cfg_t cfg;
    size_t index;

    void setFuncMoveLeft(void);
    void setFuncMoveRight(void);
    void setFuncBounce(void);
    void setFuncColorWipe(direction_t dir = LEFT);
    void setFuncConstant(void);

  private:
    void doCfg(uint16_t sectionFirstPos, uint16_t sectionLastPos, uint16_t onStartPos);


};

// Class for doing the acton for a section
class Section_c {
  public:
    section_cfg_t cfg = {};          // Containing current configurations.

    void init();                     // Reset the setup to init positions.
    void updateLeds(uint32_t *leds); // Update LED array with the LEDs states in this section

  private:
    void bounce(void);                // Bounce back and forward within the section
    void moveLeft(uint16_t cnt = 1);  // Move LEDs "cnt" to the left
    void moveRight(uint16_t cnt = 1); // Move LEDs "cnt" to the right
    void colorWipe(direction_t dir);  // Fill the LEDs one after the other
    bool doMove();                    // Returns if movment shall happen based on current moveCnt.
    uint16_t delayCnt = 0;            // Current number of ticks until LED movment shall happen.
    uint16_t currentOnLedStartPos;    // Current position for the first LED that is turned on.
    uint16_t currentOnLedEndPos;      // Current position for the last LED that is turned on.
    bool withinRange(uint16_t pos);   // Returns true if "pos" is within the onLeds
    void incrPos(uint16_t &pos, uint16_t cnt);  // Increase a position with "cnt", keeps track of section bounderies
    void decrPos(uint16_t &pos, uint16_t cnt);  // Decrease a position with "cnt", keeps track of section bounderies

    // Returns a newColor when going from one color to another
    uint32_t newColor(const int32_t ledStart, //  Start color
                      const int32_t ledEnd,   //  End color
                      const int32_t onLen,    //  Number of LEDs that are turnon on
                      const int32_t cnt);     //  The LED number for which to get the newcolor for.

    bool currentBounceDirLeft(bool startLed = true); // Returns true if current direction for a bounce is left
};

//
// Class for the whole strip
//
#define STRIP_SECTIONS_MAX 10
class LedStrip_c {
  public:
    void setupsection(Section_cfg_c section);
    void timeTick(void);
    LedStrip_c(void);

  private:
    Section_c sections[STRIP_SECTIONS_MAX];
    uint8_t totalStripLength;
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_CNT, PIN, NEO_GRB + NEO_KHZ800);
};

/************************************************************************************************
   Implementation start
 *************************************************************************************************/
// Index for each section added
static size_t section_cfg_index = 0;

// Support function for constructor
void Section_cfg_c::doCfg(uint16_t sectionFirstPos, uint16_t sectionLastPos, uint16_t onStartPos) {
  index = section_cfg_index;
  section_cfg_index++;
  cfg.sectionFirstPos = sectionFirstPos;
  cfg.sectionLastPos   = sectionLastPos;
  cfg.onStartPos      = onStartPos;
  cfg.onEndPos        = onStartPos;

  //Default use the same color for all LEDs
  cfg.firstColor      = random(0xFFFFFF);
  cfg.lastColor       = cfg.firstColor;
  cfg.func            = CONSTANT;
}


Section_cfg_c::Section_cfg_c(int sectionFirstPos, int sectionLastPos) {
  doCfg(sectionFirstPos, sectionLastPos, sectionFirstPos);
}

Section_cfg_c::Section_cfg_c(void)  {
  doCfg(0, LED_CNT, 0);
}

void Section_cfg_c::setFuncMoveLeft(void) {
  cfg.func = MOVE_LEFT;
}

void Section_cfg_c::setFuncMoveRight(void) {
  cfg.func = MOVE_RIGHT;
}

void Section_cfg_c::setFuncBounce(void) {
  cfg.func = BOUNCE;
}

void Section_cfg_c::setFuncColorWipe(direction_t dir) {
  cfg.func = COLOR_WIPE;
  cfg.direction = dir;
}

void Section_cfg_c::setFuncConstant(void) {
  cfg.func = CONSTANT;
}

// Initialize to start posistion when starting a new section setup
void Section_c::init(void) {
  delayCnt = 0;
  currentOnLedStartPos = cfg.onStartPos;
  currentOnLedEndPos = cfg.onEndPos;
  /*  Serial.print("Start:");
    Serial.println(currentOnLedStartPos);
    Serial.print("End:");
    Serial.println(cfg.sectionLastPos);*/
}

// Returns true if "pos" is within the onLeds
bool Section_c::withinRange(uint16_t pos) {
  if (pos < cfg.sectionFirstPos || pos > cfg.sectionLastPos) {
    return false;
  }
  if (currentOnLedEndPos >= currentOnLedStartPos) {
    if (pos >= currentOnLedStartPos && pos <= currentOnLedEndPos) {
      return true;
    }
  } else {
    if (pos >= currentOnLedStartPos || pos <= currentOnLedEndPos) {
      return true;
    }
  }

  return false;
}

// See definition above
uint32_t Section_c::newColor(const int32_t ledStart, const int32_t ledEnd, const int32_t onLen, const int32_t cnt) {
  int32_t sub = ((ledStart - ledEnd) / onLen) * cnt;

  // Since the "complete on" and a "little on" if not that big, so we adjust to give a bigger differnece
  const int8_t adjust = 2;
  if (sub < 0) {
    sub /= adjust;
  } else {
    sub *= adjust;
  }

  int32_t led = ledStart -  sub;

  if (led > 255) {
    return 255;
  }

  if (led < 0) {
    return 0;
  }

  return (uint32_t)led;
}

// Do the LED behaviour and add the LEDs from this section to the "leds" array
void Section_c::updateLeds(uint32_t *leds) {
  if (delayCnt == 0) {
    switch (cfg.func) {
      case MOVE_RIGHT:
        moveRight(1);
        break;
      case MOVE_LEFT:
        moveLeft(1);
        break;
      case COLOR_WIPE:
        colorWipe(cfg.direction);
        break;
      case BOUNCE:
        bounce();
        break;
    }

    delayCnt = cfg.slowness;
  } else {
    delayCnt--;
  }

  uint16_t onLen = cfg.onEndPos - cfg.onStartPos + 1;

  uint32_t redLed;
  uint32_t greenLed;
  uint32_t blueLed;

  uint32_t redLedStart = (cfg.firstColor >> 16) & 0xFF;
  uint32_t greenLedStart = (cfg.firstColor >> 8) & 0xFF;
  uint32_t blueLedStart = cfg.firstColor & 0xFF;

  uint32_t redLedEnd = (cfg.lastColor >> 16) & 0xFF;
  uint32_t greenLedEnd = (cfg.lastColor >> 8) & 0xFF;
  uint32_t blueLedEnd = cfg.lastColor & 0xFF;

  uint16_t cnt = 1;
  for (uint16_t i = 0; i < LED_CNT; i++ ) {
    if (withinRange(i)) {
      redLed = newColor(redLedStart, redLedEnd, onLen, cnt);
      blueLed = newColor(blueLedStart, blueLedEnd, onLen, cnt);
      greenLed = newColor(greenLedStart, greenLedEnd, onLen, cnt);

      uint32_t currentColor = (redLed << 16) + (greenLed << 8) + blueLed;
      cnt++;

      leds[i] |= currentColor;
    }
  }
}

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

void Section_c::moveLeft(uint16_t cnt) {
  decrPos(currentOnLedStartPos, cnt);
  decrPos(currentOnLedEndPos, cnt);
}

// Fill the dots one after the other with a color (Idea from Adafruit_NeoPixel strandtest example)
void Section_c::colorWipe(direction_t dir) {

  // Bounce
  if (dir == BOUNCING) {
    static direction_t currentDir = LEFT;

    if (currentDir == LEFT) {
      if (currentOnLedStartPos == cfg.sectionLastPos) {
        currentDir = RIGHT;
      }
    } else {
      // Currentdir is RIGHT
      if (currentOnLedEndPos == cfg.sectionFirstPos) {
        currentDir = LEFT;
      }
    }
    dir = currentDir;
  }

  if (dir == RIGHT) {
    incrPos(currentOnLedEndPos, 1);
    return;
  }

  if (dir == LEFT) {
    decrPos(currentOnLedStartPos, 1);
    return;
  }
}


//
// LedStrip_c implementation
//
void LedStrip_c::timeTick(void) {
  // We start out with all LEDs off
  uint32_t leds[LED_CNT];
  memset(&leds[0], 0, sizeof(leds));

  // Loop through all sections and add their LEDs state
  for (auto i = 0; i < STRIP_SECTIONS_MAX; i++) {
    sections[i].updateLeds(leds);
  }

  // Turn on the LEDs
  for (uint16_t i = 0; i < LED_CNT; i++) {
    strip.setPixelColor(i, leds[i]);
  }
  delay(1);
  strip.show();
}

void LedStrip_c::setupsection(Section_cfg_c section) {

  if (section.cfg.sectionFirstPos > section.cfg.onStartPos) {
    section.cfg.onStartPos = section.cfg.sectionFirstPos;
  }

  if (section.cfg.sectionLastPos < section.cfg.onEndPos || section.cfg.sectionFirstPos > section.cfg.onEndPos  ) {
    section.cfg.onEndPos = section.cfg.sectionLastPos;
  }

  sections[section.index].cfg = section.cfg;
  sections[section.index].init();
}

LedStrip_c::LedStrip_c(void) {
  strip.begin();
}


