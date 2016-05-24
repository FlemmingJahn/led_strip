// Which pin on the Arduino is connected to the NeoPixels?
#define PIN      6
#define LED_CNT  150

#include "led_strip.h"

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  static LedStrip_c strip;
  static Section_cfg_c right_brace(0,29);
  static Section_cfg_c belt2(30, 59);
  static Section_cfg_c belt1(60, 89);
  static Section_cfg_c belt3(90,119);
  static Section_cfg_c left_brace(120, 149);

  static Section_cfg_c all(0, 149);
  static Section_cfg_c all1(0, 149);
  all.cfg.onEndPos = 0;
  all.cfg.slowness = 5;
  all.cfg.firstColor = 0xFF00;
  all.cfg.lastColor = 0xFF00;
  all.setFuncColorWipe(LEFT);

  all1.cfg.onEndPos = 0;
  all1.cfg.slowness = 5;
  all1.cfg.firstColor = 0xFF0000;
  all1.cfg.lastColor = 0xFF0000;
  all1.setFuncColorWipe(RIGHT);

  
  belt2.cfg.onEndPos = 30;
  belt2.cfg.slowness = 10;
  belt2.cfg.firstColor = 0x00FF;
  belt2.cfg.lastColor = 0xFF00;
  belt2.cfg.func = COLOR_WIPE;
  
  belt1.cfg.onEndPos = 69;
  belt1.cfg.slowness = 10;
  belt1.cfg.func = MOVE_RIGHT;
  
  belt3.cfg.onEndPos    = 99;
  belt3.cfg.slowness    = 10;
  belt3.cfg.func = MOVE_LEFT;

  belt1.cfg.slowness = 10;
  belt1.cfg.firstColor = 0x00FF;
  belt1.cfg.lastColor = 0xFF00;
  belt1.cfg.func = BOUNCE;

  belt3.cfg.slowness = 10;
  belt3.cfg.firstColor = 0x00FF;
  belt3.cfg.lastColor = 0xFF00;
  belt3.cfg.func = BOUNCE;
 
  right_brace.cfg.slowness        = 5; 
  right_brace.cfg.func            = MOVE_LEFT;
  right_brace.cfg.onEndPos        = 9;

  left_brace.cfg.slowness        = 5; 
  left_brace.cfg.func            = MOVE_RIGHT;
  left_brace.cfg.onStartPos      = 140;
  left_brace.cfg.onEndPos        = 149;
    
//  strip.setupsection(left_brace);
//  strip.setupsection(right_brace);
//  strip.setupsection(belt1);
//  strip.setupsection(belt2);
//  strip.setupsection(belt3);
 
  strip.setupsection(all);
  strip.setupsection(all1);
 

  while (1) {
    strip.timeTick(1000);
    strip.setupsection(all);
    strip.setupsection(all1);
  }
}




