#include <EEPROM.h>
#include <ZWaveSlave.h>
ZWaveSlave zwave;

void myhandle(byte v)
{
    zwave.multilevelswitch[0] = v;
    if ((v != 0) && (v != 255)) 
      zwave.multilevelswitchlevel[0] = v;
}

void setup()
{
    Serial.begin(115200);
    zwave.enableMultilevelSwitch(1);
    zwave.setBasicHandler(myhandle);
    zwave.init(GENERIC_TYPE_SWITCH_MULTILEVEL,0);
    pinMode(4,INPUT);
    digitalWrite(4, HIGH);
    pinMode(2,OUTPUT);
}



long next_update=0;
void loop() 
{
    //Serial.println(digitalRead(4));
    if (digitalRead(4) == 0) {
      while(digitalRead(4)==0);
      delay(100);
      zwave.learn(1);
    }
    //Serial.println(digitalRead(4));
    zwave.mainloop();
    
    if (next_update < millis()) {
      next_update = millis()+100;
      if (zwave.multilevelstep[0] == 0x20) {
        if (zwave.multilevelswitch[0]<99) {
          zwave.multilevelswitch[0]++;
          zwave.updateBinarySwitch(zwave.multilevelswitch[0]);
        }
      } else if (zwave.multilevelstep[0] == 0x60) {
        if (zwave.multilevelswitch[0]>0) {
          zwave.multilevelswitch[0]--;
          zwave.updateBinarySwitch(zwave.multilevelswitch[0]);
        }
      }
    }
    if (zwave.multilevelswitch[0] == 0)
      analogWrite(2, 0);
    else {
      analogWrite(2, zwave.multilevelswitchlevel[0]*255/99);
    }
}
