#include <EEPROM.h>
#include <ZWaveSlave.h>
ZWaveSlave zwave;

int max_data=0;
int start_time=0;
#define NONE 0
#define SHORT 1
#define LONG 2

#define IDLE 0
#define LEARN 1
#define LEARN_ON 2
#define LEARN_OFF 3

long led_time=0;
long learn_time=0;
byte btnact = NONE;
byte state = IDLE;
byte average_data=0;
byte threshold = 0;

void setup()
{
    Serial.begin(115200);
    zwave.init(GENERIC_TYPE_SENSOR_BINARY,0);
    zwave.enableBinarySensor();
    zwave.enableAssociation();
    pinMode(5,INPUT);
    digitalWrite(5, HIGH);
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
    threshold = (EEPROM.read(0) + EEPROM.read(1))/2;
}


void loop() 
{
    //Serial.println(digitalRead(5));
    if (digitalRead(5) == 0) {
      long s = millis();
      while(digitalRead(5)==0);
      if (millis() -s < 100) {
      } else if (millis() - s < 3000) {
        btnact = SHORT;
      } else if (millis() - s > 10000) {
        EEPROM.write(0,10);
        EEPROM.write(1,10);
        byte i;
        
        for(i=0;i<10;i++) {
          digitalWrite(13,digitalRead(13));
          delay(500);
        }
      } else {
        btnact = LONG;
      }
    }
    if (state == IDLE) {
      if (btnact == SHORT) {
        delay(100);
        zwave.learn(1);
        btnact = NONE;
        Serial.println("learn");
      } else if (btnact == LONG) {
        state = LEARN;
        led_time = millis();
        btnact = NONE;
      }
    } else if (state == LEARN) {
      if (btnact != NONE) {
        state = LEARN_ON;
        digitalWrite(13,HIGH);
        btnact = NONE;
        learn_time = millis();
      } else {
        if (led_time + 500 < millis()) {
          led_time = millis();
          digitalWrite(13,digitalRead(13)?LOW:HIGH);
        }
      }
    } else if (state == LEARN_ON) {
      if (btnact != NONE) {
        state = LEARN_OFF;
        digitalWrite(13,LOW);
        btnact = NONE;
        average_data = 0;
        learn_time = millis();
      }
      if (learn_time && millis() > learn_time + 1000) {
        EEPROM.write(0, average_data);
        learn_time = 0;
        Serial.println(average_data);
      }
    } else if (state == LEARN_OFF) {
      if (btnact != NONE) {
        state = IDLE;
        byte i=0;
        for(i=0;i<8;i++) {
          digitalWrite(13,digitalRead(13)?LOW:HIGH);
          delay(500);
        }
        digitalWrite(13,LOW);
        btnact = NONE;
      }
      if (learn_time && millis() > learn_time + 1000) {
        EEPROM.write(1, average_data);
        threshold = (EEPROM.read(0)+average_data)/2;
        learn_time = 0;
        Serial.println(average_data);
      }
    }
    
    if (start_time == 0) {
      start_time = millis();
    }
    int v = abs(analogRead(13)-512);
    if (max_data < v) {
      max_data = v;
    }
    if (start_time + 100 < millis()) {
      average_data = average_data/2 + max_data;
      max_data = 0;
      start_time = millis();
      if (average_data > threshold) {
        zwave.updateBinarySensor(1);
      } else {
        zwave.updateBinarySensor(0);
      }
    }
    zwave.mainloop();
}
