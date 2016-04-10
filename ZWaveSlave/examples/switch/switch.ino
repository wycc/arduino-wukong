#define USE_UART2
#include <EEPROM.h>
#include <ZWaveSlave.h>
ZWaveSlave zwave;

void switch_binary_handle(byte v)
{
    Serial.println(v);
    if (v)
      PORTK &= 0xf0;
    else
      PORTK |= 0xf;
}

void setup()
{
    Serial.begin(115200);
    DDRK = 0xf;
    zwave.enableBinarySwitch();
    zwave.setSwitchBinaryHandler(switch_binary_handle);
    zwave.init(GENERIC_TYPE_SWITCH_BINARY,0);
    pinMode(5,INPUT);
    digitalWrite(5, HIGH);
    pinMode(13,OUTPUT);
}

void loop() 
{
    //Serial.println(digitalRead(4));
    if (digitalRead(5) == 0) {
      while(digitalRead(5)==0);
      delay(100);
      zwave.learn(1);
    }
    //Serial.println(digitalRead(4));
    zwave.mainloop();
      
}
