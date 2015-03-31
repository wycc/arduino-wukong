#include <EEPROM.h>
#include <ZWaveSlave.h>
#include <avr/wdt.h>
ZWaveSlave zwave;

unsigned long max_data=0;
unsigned long max_count = 0;
long start_time=0;
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
long next_update_time=0;
byte seq=10;
void simple_av_hanlder(byte v)
{
    byte b[10];
    
    b[0] = 0x94;
    b[1] = 1;
    b[2] = seq++;
    b[3] = 1;
    b[4] = 0;
    b[5] = 0;
    b[6] = 0;
    b[7] = 39;
    if ((average_data >= threshold) && (v == 0)) {
      zwave.send(1,b,8,5);
    } else if ((average_data < threshold)&& (v == 0xff)) {
      zwave.send(zwave.configurations[4],b,8,5);
    }
}

void setup()
{
    Serial.begin(115200);
    zwave.enableBinarySensor();
    zwave.enableAssociation();
    zwave.enableConfiguration(4);
    zwave.enableMeter(2);
    zwave.setBasicHandler(simple_av_hanlder);
    zwave.init(GENERIC_TYPE_SENSOR_BINARY,0);
    pinMode(4,INPUT);
    digitalWrite(4, HIGH);
    pinMode(5,INPUT);
    digitalWrite(5, HIGH);
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
    Serial.println("start");
    Serial.println("start");
    Serial.println("start");
    Serial.println("start");
    Serial.println("start");
    wdt_enable(WDTO_1S);
}

unsigned long learn_timeout = 0;

void loop() 
{
    threshold = (zwave.configurations[0] + zwave.configurations[1])/2;
    if (learn_timeout && (learn_timeout < millis())) {
      learn_timeout = 0;
      zwave.learn(0);
      Serial.println("learn off");
    }
    if (digitalRead(4) == 0) {
      long s = millis();
      Serial.println("press");
      
      while(digitalRead(4)==0) {
        //Serial.println("hold");
        wdt_reset();
      }
      
      Serial.println("click");
      if (millis() -s < 100) {
      } else if (millis() - s < 3000) {
        Serial.println("short click");
        btnact = SHORT;
      } else if (millis() - s > 10000) {
        EEPROM.write(0,10);
        EEPROM.write(1,10);
        byte i;
        Serial.println("Reset");

        for(i=0;i<10;i++) {
          digitalWrite(13,digitalRead(13));
          delay(500);
        }
        Serial.println("Done");
      } else {
        btnact = LONG;
      }
    }
    if (state == IDLE) {
      if (btnact == SHORT) {
        btnact = NONE;
        Serial.println("learn");
        zwave.learn(1);
        learn_timeout = millis()+1000;
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
    //long s = micros();
    int v = abs(analogRead(13)-512);
    //Serial.println(micros()-s);
#if 0
    if (max_data < v) {
      max_data = v;
    }
#else    
    max_data = max_data + v;
    max_count = max_count+1;
#endif
    wdt_reset();

    if (start_time + 1000 < millis()) {
      int w;
      average_data = average_data/2 + max_data/max_count;
      w = average_data*19/13;
      zwave.updateMeter(w);
      Serial.print("m=");
      Serial.println(w);
      max_data = 0;
      max_count = 0;
      start_time = millis();
      if (millis() > next_update_time) {
        next_update_time = millis() + 1000;
        zwave.updateConfiguration(3,w);
      }
      if (average_data > threshold) {
        zwave.updateBinarySensor(1);
      } else {
        zwave.updateBinarySensor(0);
      }
    }
    zwave.mainloop();
}
