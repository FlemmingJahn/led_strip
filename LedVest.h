// To-do list
// 
// Make Fade function
// Make vest splt

#include "debug.h"

class LedVest_c {
  public:
    LedVest_c(void);
    void showCase(uint16_t cnt, sectionFunction_t func);
    void move(uint16_t cnt);
    void bounce(uint16_t cnt);
    void moveAll(uint16_t cnt);
    void constant(uint16_t cnt);
    
    private:
    LedStrip_c strip; // Made static in order to catch memmory usage at compile time.
 };

LedVest_c::LedVest_c(void) {
}
color_t getRandomColor(void) {
 const color_t maxIntensity = 0x40; // Turn down the intensity, because if all LEDs are turned on with highest intensity, it can be a problem to supply the last LEDs in the string
 color_t red   = random(maxIntensity);
 color_t blue  = random(maxIntensity);
 color_t green = random(maxIntensity);
 return (red << 16) + (blue << 8) + green;
}

sectionFunction_t invertMove(sectionFunction_t func) {
  if (func == MOVE_LEFT) {
    return MOVE_RIGHT;
  }
  if (func == MOVE_RIGHT) {
    return MOVE_LEFT;
  }

  return func;
}

void colorSwap(color_t *c1, color_t *c2) {
  color_t tmp = *c1;
  *c1 = *c2;
  *c2 = tmp;
}

void LedVest_c::showCase(uint16_t cnt, sectionFunction_t func) {
 color_t color1 = getRandomColor();
 color_t color2 = getRandomColor();
 
 uint8_t cSize = random(29);  
 uint8_t mCnt = 1;
 uint16_t speed = random(5) + 1;
 strip.sectionCntSet(5);
 strip.sections[0].init();
 strip.sections[0].setLedColor(0, cSize, COLOR1, color1);
 strip.sections[0].setLedColor(cSize + 1, 29, COLOR2, color2);
 strip.sections[0].setFunc(func, mCnt);
 strip.sections[0].setSlowness(speed);

 // Left seen from back
 strip.sections[4].init();
 strip.sections[4].setLedColor(120, 149 - cSize -1, COLOR1, color2);
 strip.sections[4].setLedColor(149 - cSize, 149, COLOR2, color1);
 strip.sections[4].setFunc(invertMove(func), mCnt);
 strip.sections[4].setSlowness(speed);

 // Middle Horiz
 strip.sections[1].init();
 strip.sections[1].setLedColor(30, 59 - cSize -1, COLOR1, color2);
 strip.sections[1].setLedColor(59 - cSize, 59, COLOR2, color1);
 strip.sections[1].setFunc(func, mCnt);
 strip.sections[1].setSlowness(speed);

  // Top Horiz.
 strip.sections[3].init();
 strip.sections[3].setLedColor(60, 60 + cSize, COLOR1, color1);
 strip.sections[3].setLedColor(60 + cSize + 1, 89, COLOR2, color2);
 strip.sections[3].setFunc(invertMove(func), mCnt);
 strip.sections[3].setSlowness(speed);

 // Button Horiz
 strip.sections[2].init();
 strip.sections[2].setLedColor(90, 119 - cSize -1, COLOR1, color2);
 strip.sections[2].setLedColor(119 - cSize, 119, COLOR2, color1);
 strip.sections[2].setFunc(func, mCnt);
 strip.sections[2].setSlowness(speed);

  for (auto i = 0; i < cnt; i++) {
   strip.timeTick(1);
 }
}

void LedVest_c::move(uint16_t cnt) {
  showCase(cnt, MOVE_RIGHT);
  showCase(cnt, MOVE_LEFT);
}

void LedVest_c::bounce(uint16_t cnt) {
 color_t color1 = getRandomColor();
 color_t color2 = getRandomColor();

 sectionFunction_t func = BOUNCE;
 
 uint8_t cSize  = random(29);  
 uint8_t mCnt   = 1;
 uint16_t speed = random(5) + 1;  
 strip.sectionCntSet(5);
 strip.sections[0].init();
 strip.sections[0].setLedColor(0, cSize, COLOR2, color1);
 strip.sections[0].setLedColor(cSize + 1, 29, COLOR1, color2);
 strip.sections[0].setFunc(func, mCnt);
 strip.sections[0].setSlowness(speed);

 // Left seen from back
 strip.sections[4].init();
 strip.sections[4].setLedColor(120, 149 - cSize -1, COLOR1, color2);
 strip.sections[4].setLedColor(149 - cSize, 149, COLOR2, color1);
 strip.sections[4].setFunc(invertMove(func), mCnt);
 strip.sections[4].setSlowness(speed);

 // Middle Horiz
 strip.sections[1].init();
 strip.sections[1].setLedColor(30, 59 - cSize -1, COLOR1, color2);
 strip.sections[1].setLedColor(59 - cSize, 59, COLOR2, color1);
 strip.sections[1].setFunc(func, mCnt);
 strip.sections[1].setSlowness(speed);

  // Top Horiz.
 strip.sections[3].init(RIGHT);
 strip.sections[3].setLedColor(60, 60 + cSize, COLOR2, color1);
 strip.sections[3].setLedColor(60 + cSize + 1, 89, COLOR1, color2);
 strip.sections[3].setFunc(invertMove(func), mCnt);
 strip.sections[3].setSlowness(speed);

 // Button Horiz
 strip.sections[2].init();
 strip.sections[2].setLedColor(90, 119 - cSize -1, COLOR1, color2);
 strip.sections[2].setLedColor(119 - cSize, 119, COLOR2, color1);
 strip.sections[2].setFunc(func, mCnt);
 strip.sections[2].setSlowness(speed);

  for (auto i = 0; i < cnt; i++) {
   strip.timeTick(1);
 }
}

 void LedVest_c::constant(uint16_t cnt) {
 strip.sectionCntSet(1);
 for (auto i = 0; i < 1;i ++) {
  color_t color1 = 0xFF0000; // getRandomColor();
  color_t color2 = 0x00FF00; //getRandomColor();
 
  uint8_t cSize = random(LED_CNT);  
  uint8_t mCnt = 1;
  
  strip.sections[i].init();
  strip.sections[i].setLedColor(0, cSize, COLOR1, color1);
  strip.sections[i].setLedColor(cSize + 1, LED_CNT, COLOR2, color2);
  strip.sections[i].setFunc(CONSTANT, mCnt);
  }
  for (auto u = 0; u < cnt; u++) {
    strip.timeTick(1);
  }
}



void LedVest_c::moveAll(uint16_t cnt) {
 strip.sectionCntSet(1);
 sectionFunction_t func;
  switch (random(5)) {
  case 0: 
    func = MOVE_LEFT;
    break;
  case 1: 
    func = MOVE_LEFT;
    break;
  default:
    func = BOUNCE;
    break; 
 }
 for (auto i = 0; i < 1;i ++) {
  color_t color1 = getRandomColor();
  color_t color2 = getRandomColor();
 
  uint8_t cSize = random(LED_CNT);  
  uint8_t mCnt = 1;
  uint16_t speed = random(5) + 1;
  
  strip.sections[i].init();
  strip.sections[i].setLedColor(0, cSize, COLOR1, color1);
  strip.sections[i].setLedColor(cSize + 1, LED_CNT, COLOR2, color2);
  strip.sections[i].setFunc(func, mCnt);
  strip.sections[i].setSlowness(speed);
  }
  for (auto u = 0; u < cnt; u++) {
    strip.timeTick(1);
  }
}


