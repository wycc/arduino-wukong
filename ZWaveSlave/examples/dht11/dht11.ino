#include <DHT.h>

#include <Ultrasonic.h>

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

#define TRIGGER_PIN A0
#define ECHO_PIN A1

DHT dht(10,DHT11);

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
void setup()
{
    Serial.begin(115200);
    zwave.enableBinarySensor();
    zwave.enableMultilevelSensor(2);
    zwave.enableAssociation();
    zwave.setupSensorType(0,1,0,1);
    zwave.setupSensorType(1,0,0,5);
    zwave.init(0x21,0);      
    
    digitalWrite(5,HIGH);
    pinMode(11,OUTPUT);
    digitalWrite(11, HIGH);
    pinMode(10,INPUT);
    digitalWrite(10, HIGH);
    pinMode(9,INPUT);
    digitalWrite(9, LOW);
    pinMode(8,OUTPUT);
    pinMode(13,OUTPUT);
    digitalWrite(13,8);
    threshold = (EEPROM.read(0) + EEPROM.read(1))/2;
}


int readData()
{
    long microsec = ultrasonic.timing();
    float cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
    return cmMsec;
}

int readData_DHT11()
{
    static int last_t=0;
    static int last_h=0;
    static long next_report=0;
    static int next_type=0;
    byte b[10];
    int t = dht.readTemperature();
    int h = dht.readHumidity();
    //Serial.print("h = ");
    //Serial.print(h);
    //Serial.print(" t = ");
    //Serial.println(t);
    zwave.updateMultilevelSensor(0,t);
    zwave.updateMultilevelSensor(1,h);
    if (last_t != t) {
      Serial.print("last_t=");Serial.print(last_t);Serial.print(" t=");Serial.println(t);
      last_t = t;
      zwave.sendSensorReport(0,0);      
    }
    if (last_h != h) {
      Serial.print("last_h=");Serial.print(last_h);Serial.print(" h=");Serial.println(h);
      last_h=h;
      zwave.sendSensorReport(0,1);      
    }
    if (millis() > next_report) {
      zwave.sendSensorReport(0,next_type);
      next_report = millis() + 5000;
      next_type = 1- next_type;
      Serial.print("send report");
      if (next_type == 0) {
        Serial.print(" t=");Serial.println(last_t);
      } else {
        Serial.print(" h=");Serial.println(last_h);
      }
    }
    return t;
    
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
    int v = readData_DHT11();
    if (max_data < v) {
      max_data = v;
    }
    if (start_time + 100 < millis()) {
      average_data = average_data/2 + max_data;
      Serial.println(average_data);
      max_data = 0;
      start_time = millis();
      if (average_data > threshold) {
        zwave.updateBinarySensor(1);
        Serial.println("1");
      } else {
        zwave.updateBinarySensor(0);
        Serial.println("0");
      }
    }
    zwave.mainloop();
}
