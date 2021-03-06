#include <EEPROM.h>
#include <ZWaveSlave.h>
ZWaveSlave zwave;
void setup()
{
    Serial.begin(115200);
    zwave.init(GENERIC_TYPE_SENSOR_BINARY,0);
    zwave.enableBinarySensor();
    zwave.enableAssociation();
    pinMode(5,INPUT);
    digitalWrite(5, HIGH);
    digitalWrite(4, HIGH);
}

void loop() 
{
    if (digitalRead(4) == 0) {
      while(digitalRead(4)==0);
      delay(100);
      zwave.learn(1);
    }
    //Serial.println(digitalRead(4));
    zwave.mainloop();
    zwave.updateBinarySensor(digitalRead(5));
}
