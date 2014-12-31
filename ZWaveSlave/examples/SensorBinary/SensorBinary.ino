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
    pinMode(4,INPUT);
}

void loop() 
{
    zwave.mainloop();
    bool b = digitalRead(5);
    zwave.updateBinarySensor(b);
    if (digitalRead(4)==0) {
      while(digitalRead(4)==0);
      zwave.enterLearn();
    }
}
