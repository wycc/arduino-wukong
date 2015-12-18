#include <EEPROM.h>

// The ZWave module is in the COM1
#define USE_UART1
#include <ZWaveSlave.h>
ZWaveSlave zwave;
unsigned long expire=0;
byte v = 0;
void setup()
{
    Serial.begin(115200);
    zwave.init(GENERIC_TYPE_SENSOR_BINARY,0);
    zwave.enableBinarySensor();
    zwave.enableAssociation();
    zwave.enableDebug();
    
    pinMode(5,INPUT);
    digitalWrite(5, HIGH);
    pinMode(2,INPUT);
    digitalWrite(2, HIGH);
    Serial.println("init done");
}

void loop() 
{
    if (digitalRead(2) == 0) {
      while(digitalRead(2) == 0);
      delay(100);
      zwave.learn(1);
    }
    if (expire < millis()) {
      zwave.set(0,v,0);
      Serial.println("Send");
      expire = millis()+1000;
      v = 255 - v;
    }
    zwave.mainloop();
    zwave.updateBinarySensor(digitalRead(5));
}


