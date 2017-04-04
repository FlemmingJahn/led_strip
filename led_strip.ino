// Which pin on the Arduino is connected to the NeoPixels?
#define PIN      6
#define LED_CNT  150


#include "led_strip.h"
#include "ledVest.h"

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
 T("Starting");
 static LedVest_c LedVestObj; // Made static in order to see how much dynamic memory we use at compile time.
 
 while (1) {
   LedVestObj.moveAll(1000);
   LedVestObj.bounce(1000);
   LedVestObj.move(1000);
  //  LedVestObj.constant(10000);
  }

}




