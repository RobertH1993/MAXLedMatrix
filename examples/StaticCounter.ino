#include <MAXLedMatrix.h>


MAXLedMatrix lm = MAXLedMatrix(13, 14, 15, 4); //data pin, clock pin, chip-select, number of displays

void setup() {
  // put your setup code here, to run once:
  lm.setBrightness(1);
}

uint64_t last_millis = 0;
uint64_t t = 0;
void loop() {
  // put your main code here, to run repeatedly:
  lm.update();

  if(millis() - last_millis > 1000){
    t += 1;
    char st[4];
    itoa(t, st, 10);
    lm.staticDrawCharacter(0, st[0]);
    lm.staticDrawCharacter(1, st[1]);
    lm.staticDrawCharacter(2, st[2]);
    lm.staticDrawCharacter(3, st[3]);
    last_millis = millis();
  }
}
