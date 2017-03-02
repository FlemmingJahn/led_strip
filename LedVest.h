// To-do list
// 
// Bounce Cnt
// Add RANDOM posiblity
// Make Vest move showcase
// Mak Vest bouce showcase


#include "debug.h"

class LedVest_c {
  public:
    LedVest_c(void);
    void move(uint16_t cnt);
    void bounce(uint16_t cnt);
    
    private:
    LedStrip_c strip; // Made static in order to catch memmory usage at compile time.
 };

LedVest_c::LedVest_c(void) {
}
void LedVest_c::move(uint16_t cnt) {
 
 strip.sections[0].init();
 
 strip.sections[0].setLedColor(0, 9, COLOR1, 0xFF0000);
 strip.sections[0].setLedColor(20, 29, COLOR2, 0x00FF00);
// strip.sections[0].setFunc(MOVE_LEFT, 15);
 strip.sections[0].setSlowness(0);
 
 strip.sections[1].init();
 strip.sections[1].setLedColor(30, 49, COLOR1, 0x00FF00);
 strip.sections[1].setLedColor(50, 59, COLOR2, 0x0000FF);
 strip.sections[1].setFunc(MOVE_LEFT);
 strip.sections[1].setSlowness(0);

 strip.sections[3].init();
 strip.sections[3].setLedColor(60, 79, COLOR1, 0xFF0000);
 strip.sections[3].setLedColor(80, 89, COLOR2, 0x000000);
 strip.sections[3].setFunc(MOVE_RIGHT);
 strip.sections[3].setSlowness(0);
 
 
 strip.sections[2].init();
 strip.sections[2].setLedColor(10, 14, COLOR1, 0xFF0033);
 strip.sections[2].setLedColor(15, 19, COLOR2, 0x003333);
  for (auto i = 0; i < cnt; i++) {
   strip.timeTick(1);
 }
}

void LedVest_c::bounce(uint16_t cnt) {
 
 strip.sections[0].init();
 
 strip.sections[0].setLedColor(0, 2, COLOR1, 0xFF0000);
 strip.sections[0].setLedColor(3, 29, COLOR2, 0x000FF0);
 strip.sections[0].setFunc(BOUNCE);
 strip.sections[0].setSlowness(0);
 
  for (auto i = 0; i < cnt; i++) {
   strip.timeTick(1);
 }
}

