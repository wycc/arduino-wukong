#include <ZWaveSlave.h>
#include <EEPROM.h>
int g_seq = 0;
int last_node = 0;
int g_basic_level=0;



ZWaveSlave ZWave;

void displayStatus(byte *ret,int len)
{
  char buf[64];


  snprintf(buf,64,"Status=%d Node=%d\n", ret[1],ret[2]);
  Serial.write(buf);
}
void include1(byte *ret,int len)
{
  char buf[64];


  snprintf(buf,64,"Status=%d Node=%d\n", ret[1],ret[2]);
  Serial.write(buf);
  ZWave.callback(displayStatus);
  ZWave.includeAny();
}

void next(byte *ret,int len)
{
  Serial.write("Done\n");
  if (ZWave.getType() == 0) {
    if (ret[1] == 0) {
      Serial.write("ACK\n");
    } 
    else {
      Serial.write("No ACK\n");
    }
    ZWave.excludeAny();
    ZWave.callback(include1);
  } 
  else if (ZWave.getType() == 2) {
    Serial.write("Timeout\n");
    ZWave.excludeAny();
    ZWave.callback(include1);
  }
}

void offack(byte *b,int len)
{
  if (ZWave.getType() == 0) {
    delay(500);
    ZWave.callback(onack);
    ZWave.set(last_node,255,5);
  } 
  else if (ZWave.getType() == 2) {
    Serial.write("Timeout\n");
    ZWave.callback(onack);
    ZWave.set(last_node,255,5);
  } 
}

void onack(byte *b,int len)
{
  if (ZWave.getType() == 0) {
    delay(500);
    ZWave.callback(offack);

    ZWave.set(last_node,0,5);
  } 
  else if (ZWave.getType() == 2) {
    Serial.write("Timeout\n");
    ZWave.callback(offack);
    ZWave.set(last_node,0,5);
  }
}

void display_nif(byte *payload,int len) {
  char buf[128];

  snprintf(buf,64,"Status=%d Node=%d Device=%d:%d:%d\n", payload[0],payload[1],payload[3],payload[4],payload[5]);
  Serial.write(buf);
  last_node = payload[1];
}



void setup()
{
  byte b[1] = {
    0  };
  Serial.begin(115200);

  ZWave.init(GENERIC_TYPE_SENSOR_BINARY,0);
  Serial.write("Start\n");
  //ZWave.reset();
  //ZWave.callback(include1);
  //ZWave.excludeAny();
  //ZWave.LearnStop();
  //ZWave.includeAny();
  //ZWave.send(2,b,1);
  //offack(NULL,0);
  ZWave.callback_nif(display_nif);
}

unsigned char hex[] = { 
  '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

void include_cb(byte *b,int len)
{
  char buf[64];

  snprintf(buf,64,"Status=%d Node=%d\n", b[0],b[1]);
}

void exclude_cb(byte *b,int len)
{
  char buf[64];

  snprintf(buf,64,"Status=%d Node=%d\n", b[0],b[1]);
}

void help()
{
  Serial.write("l: turn on learn mode\n");
  Serial.write("L: turn off learn mode\n");
  Serial.write("n: send node info\n");
  Serial.write("r: reset the node\n");
  Serial.write("Press the program button of the node to change the current node\n");
}


boolean lastButtonState=true;
long lastDebounceTime=0;
void loop()
{
  if (ZWave.mainloop()) {

  }
  
  // Read button 5 with debounce
  boolean btn = digitalRead(5);
  if (lastButtonState == true) {
    if (lastDebounceTime > 0 && (lastDebounceTime+200 < millis())) {
      if (btn == false) {
        lastButtonState = false;
        lastDebounceTime = 0;
        ZWave.callback(0);
        ZWave.learn(0);
      }
    } else {
      if (btn == false) {
        lastDebounceTime = millis();
      }
    }
  } else {
    lastButtonState = btn;
  }
  //
  
  if (Serial.available()) {
    byte c = Serial.read();
    if (c == 'l') {
      ZWave.callback(0);
      ZWave.learn(1);      
    } 
    else if (c == 'L') {
      ZWave.callback(0);
      ZWave.learn(0);      
    } 
    else if (c == 'r') {
      ZWave.callback(0);
      ZWave.reset();      
    } 
    else if (c == 'N') {
      ZWave.init_nodeinfo();
    }
    else if (c == 'n') {
      ZWave.sendNodeInfo();
    }
    else {
      help();
    }
  }
}



