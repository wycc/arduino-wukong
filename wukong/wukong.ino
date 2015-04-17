#include <WukongVM.h>

Wukong wukong;
void setup() {
  Serial.begin(115200);
  DDRK |= (1<<7);
  PORTK &= ~(1<<7);
  PORTK |= (1<<7);
  delay(10);
  DDRA &= ~(1<<1);
  PORTA |= (1<<1);
  wukong.begin();
  Serial.println("init done");
}

void loop() {
  // put your main code here, to run repeatedly:
  wukong.loop();
}
