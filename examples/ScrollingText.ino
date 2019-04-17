#include <MAXLedMatrix.h>
//data pin, clock pin, chip select pin, number of displays
MAXLedMatrix lm = MAXLedMatrix(13, 14, 15, 4);

//If the orientation of the text is wrong add #define FLIP_ORIENTATION to MAXLedMatrix.h, see docs for more info!

void setup() {
  // put your setup code here, to run once:
  lm.setBrightness(1);//1-15 higher is brighter
  lm.setTextScrollDirection(ANIMATION_SCROLL_LEFT); //ANIMATION_NO_SCROLL, ANIMATION_SCROLL_RIGHT
  lm.setText(" WORLD CHIPS"); //Max 32 characters
}

void loop() {
  // put your main code here, to run repeatedly:
  //Must be called in a loop to update the animations / display
  lm.update();
}
