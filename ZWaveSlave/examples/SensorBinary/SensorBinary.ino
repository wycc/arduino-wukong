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
}

void loop() 
{
    zwave.mainloop();
    zwave.updateBinarySensor(digitalRead(5));
}
