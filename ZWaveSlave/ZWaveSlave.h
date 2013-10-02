#ifndef __ZWAVESLAVE_H
#define __ZWAVESLAVE_H
#include "Arduino.h"
/*****************************************/
//     Hardware specific definition
/*****************************************/
#define EEPROM_GROUP1 10
#define SENSOR_PORT 5

boolean g_sensor_state=false;


#define APPLICATIONCOMMANDHANDLER 0x04
#define APPLICATIONSLAVEUPDATE    0x49




#define COMMAND_CLASS_BASIC 0x20

#define COMMAND_CLASS_ASSOCIATION 0x85
#define ASSOCIATION_GET 2
#define ASSOCIATION_GROUPING_GET 5
#define ASSOCIATION_GROUPING_REPORT 6
#define ASSOCIATION_REMOVE 4
#define ASSOCIATION_REPORT 3
#define ASSOCIATION_SET 1

#define GENERIC_TYPE_SENSOR_BINARY   0x20
#define COMMAND_CLASS_SENSOR_BINARY  0x30
#define SENSOR_BINARY_GET            0x02
#define SENSOR_BINARY_REPORT         0x03


#define COMMAND_CLASS_SENSOR_MULTILEVEL 0x31
#define SENSOR_MULTILEVEL_GET 4
#define SENSOR_MULTILEVEL_REPORT 5

#define APPLICATION_NODEINFO_NOT_LISTENING            0x00
#define APPLICATION_NODEINFO_LISTENING                0x01
#define APPLICATION_NODEINFO_OPTIONAL_FUNCTIONALITY   0x02
#define APPLICATION_FREQ_LISTENING_MODE_1000ms        0x10
#define APPLICATION_FREQ_LISTENING_MODE_250ms         0x20



#define REQUEST 0
#define RESPONSE 1
#define BASIC_SET 1
#define BASIC_GET 2
#define BASIC_REPORT 3

#define ST_SOF       1
#define ST_LEN       2
#define ST_TYPE      3
#define ST_CMD       4
#define ST_ID        5
#define ST_DATA      6
#define ST_CRC       7
#define ST_DONE      8

class ZWaveSlave {
private:  
  byte seq;          // Sequence number which is used to match the callback function
  int state;         // Current state
  int len;           // Length of the returned payload
  int type;          // 0: request 1: resposne 2: timeout
  int cmd;           // the serial api command number of the current payload
  byte g_basic_level;
  byte payload[64];  // The data of the current packet
  long learn_info_time,learn_stop_time;
  int i;            
  unsigned long expire;  // The expire time of the last command
  void (*f)(byte *,int); // The callback function registered by callback
  void (*cmd_handler)(byte *,int); // The callback function registered by callback
  void (*switch_binary_handler)(byte v); // The callback function registered by callback
  void (*nodeinfo)(byte *payload,int len);
  bool hasSensorBinary;
  bool hasAssociation;
  byte generic,specific;
public:
  ZWaveSlave() {
    byte buf[5];
    Serial2.begin(115200);
    Serial2.write(6);
    version();
    for(i=0;i<100;i++)
      mainloop();
    randomSeed(analogRead(0));
    seq = random(255);
    state = ST_SOF;
    expire = 0;
    nodeinfo=NULL;
	cmd_handler=NULL;
    f=NULL;
	g_basic_level = 0;
	learn_info_time = 0;
	learn_stop_time = 0;
	hasSensorBinary = false;
	hasAssociation = false;
  }
  void init(byte g,byte s) {
	generic = g;
	specific = s;
    init_nodeinfo();
  }
  int getType() {
    return type;
  }
  void DisplayNodeInfo() {
    char buf[128];

    snprintf(buf,64,"Status=%d Node=%d Device=%d:%d:%d\n", payload[0],payload[1],payload[3],payload[4],payload[5]);
    Serial.write(buf);
  }
  void enableSensorBinary() {
  	hasSensorBinary = true;
  }
  void enableAssociation() {
  	hasAssociation = true;
  }
  /*
  void checkSensor() {
    boolean s = digitalRead(SENSOR_PORT);
    if (s != g_sensor_state) {
      g_sensor_state = s;
      byte src = EEPROM.read(EEPROM_GROUP1);
      byte b[3];
      b[0] = COMMAND_CLASS_BASIC;
      b[1] = BASIC_SET;
      b[2] = s? 0xff:0;
      send(src,b,3,5);
    }
  }
  */
  void updateBinarySensor(byte v) {
    if (v != g_basic_level) {
	  	g_basic_level = v;
#ifdef EEPROM_h
      byte src = EEPROM.read(EEPROM_GROUP1);
      byte b[3];
      b[0] = COMMAND_CLASS_BASIC;
      b[1] = BASIC_SET;
      b[2] = v? 0xff:0;
      send(src,b,3,5);
#endif
	}
  }
  // Main event loop of the ZWave SerialAPI
  boolean mainloop() {
    unsigned long now = millis();
    //checkSensor();
    if (expire && (now > expire)) {
      expire = 0;
      type = 2;
      state = ST_SOF;
      if (f!=NULL) f(payload,i);
      return true;
    }
	if (state == ST_SOF) {
	    if (learn_info_time && now > learn_info_time) {
		    learn_info_time = 0;
		    sendNodeInfo();
			Serial.println("send info");
	    }
	    if (learn_stop_time && now > learn_stop_time) {
		    //learn(0);
		    learn_stop_time = 0;
	    }
	}
    if (Serial2.available()) {
      expire = now + 1000;
      byte c = Serial2.read();
      char buf[128];
      snprintf(buf,128,"c=%x state=%d\n", c, state);
      Serial.write(buf);
      if (state == ST_SOF) {
        if (c == 1) {
          state = ST_LEN;
          len = 0;
        } 
        else if (c == 0x15) {
        }
      } 
      else if (state == ST_LEN) {
        len = c-3;
        state = ST_TYPE;
      } 
      else if (state == ST_TYPE) {
        type = c;
        state = ST_CMD;
      } 
      else if (state == ST_CMD) {
        cmd = c;
        state = ST_DATA;
        i = 0;
      } 
      else if (state == ST_DATA) {
        payload[i++] = c;
        len--;
        if (len == 0) {
          state = ST_CRC;
        }
      } 
      else if (state == ST_CRC) {
        Serial2.write(6);
        state = ST_SOF;
        if (f!=NULL) f(payload,i);
        if (type == REQUEST) {
          if (cmd == APPLICATIONSLAVEUPDATE) {
            //if (nodeinfo) nodeinfo(payload,i);
          } else if (cmd == APPLICATIONCOMMANDHANDLER) {
            // ApplicationCommandHandler
            handleCommand(payload[1], i-3, payload+3);
          }
        } else {
        }
        return true;
      } 
    }
    return false;
  }

  // Register for the callback function when we receive packet from the Z-Wave module
  void callback(void (*func)(byte *,int)) {
    f = func;
  }
  void setCommandLoop(void (*func)(byte *,int)) {
    cmd_handler = func;
  }

  void callback_nif(void (*func)(byte *,int)) {
    nodeinfo = func;
  }

  // Send ZWave command to another node. This command can be used as wireless repeater between 
  // two nodes. It has no assumption of the payload sent between them.
  void send(byte id, byte *b,byte l,byte option) {
    int k;
    byte crc;
    byte buf[24];
    byte ll=l+7;

    Serial.write("Send\n");
    buf[0] = 1;
    buf[1] = l+7;
    buf[2] = 0;
    buf[3] = 0x13;
    buf[4] = id;
    buf[5] = l;
    for(k=0;k<l;k++) 
      buf[k+6] = b[k];
    buf[l+6] = option;
    buf[l+7] = seq;

    crc = 0xff;
    Serial2.write(buf[0]);
    for(k=0;k<l+7;k++) {
      Serial2.write(buf[k+1]);
      Serial.println(buf[k+1]);
      crc = crc ^ buf[k+1];
    }
    seq++;
    Serial2.write(crc);
    expire = millis()+1000;
  }

  // Include or exclude node from the network
  void networkIncludeExclude(byte t,byte m) {
    byte b[10];
    int k;

    b[0] = 1;
    b[1] = 5;
    b[2] = 0;
    b[3] = t;
    b[4] = m;
    b[5] = seq;
    b[6] = 0xff^5^0^t^m^seq;
    seq++;
    for(k=0;k<7;k++)
      Serial2.write(b[k]);
  }

  // Reset the ZWave module to the factory default value. This must be called carefully since it will make
  // the network unusable.
  void reset() {
    byte b[10];
    int k;

    b[0] = 1;
    b[1] = 4;
    b[2] = 0;
    b[3] = 0x42;
    b[4] = seq;
    b[5] = 0xff^4^0^0x42^seq;
    seq++;
    for(k=0;k<6;k++)
      Serial2.write(b[k]);

  }
  void version() {
    byte b[10];
    int k;

    b[0] = 1;
    b[1] = 4;
    b[2] = 0;
    b[3] = 0x15;
    b[4] = seq;
    b[5] = 0xff^4^0^0x15^seq;
    seq++;
    for(k=0;k<6;k++)
      Serial2.write(b[k]);

  }

  // Reset the ZWave module to the factory default value. This must be called carefully since it will make
  // the network unusable.
  void enterLearn() {
  	learn(1);
	learn_info_time = millis()+500;
	learn_stop_time = millis()+3000;
  }
  void learn(int onoff) {
    byte b[10];
    int k;

    b[0] = 1;
    b[1] = 5;
    b[2] = 0;
    b[3] = 0x50;
    b[4] = onoff;
    b[5] = seq;
    b[6] = 0xff^5^0^0x50^onoff^seq;
    seq++;
    for(k=0;k<7;k++)
      Serial2.write(b[k]);

  }
  void sendNodeInfo() {
    byte b[10];
    byte k;
    
    b[0] = 1;
    b[1] = 6;
    b[2] = 0;
    b[3] = 0x12;
    b[4] = 0xff;
    b[5] = 1;
    b[6] = seq;
    b[7] = 0xff^6^0^0x12^0xff^1^seq;
    seq++;
    for(k=0;k<8;k++)
      Serial2.write(b[k]);
  }

  // Start inclusion procedure to add a new node
  void includeAny() {
    networkIncludeExclude(0x4A,1);
  }

  // Stop inclusion/exclusion procedure
  void LearnStop() {
    networkIncludeExclude(0x4A,5);
  }

  // Start exclusion procedure
  void excludeAny() {
    networkIncludeExclude(0x4B,1);
  }

  // Set the value of a node
  void set(byte id,byte v,byte option) {
    byte b[3];

    b[0] = 0x20;
    b[1] = 1;
    b[2] = v;
    send(id,b,3,option);
  }
  void init_nodeinfo() {
    byte b[12];
	byte ptr=8;
	byte i;
	delay(2000);
    
    b[0] = 1;
    b[1] = 10;
    b[2] = 0;
    b[3] = 3;
    b[4] = APPLICATION_NODEINFO_LISTENING;
    b[5] = generic;
    b[6] = 1;
    b[7] = 3;
    b[8] = COMMAND_CLASS_SENSOR_BINARY;
	ptr = 9;
	if (hasSensorBinary)
	    b[ptr++] = COMMAND_CLASS_SENSOR_BINARY;
	if (hasAssociation)
	    b[ptr++] = COMMAND_CLASS_ASSOCIATION;
	b[7] = ptr-8;
	b[1] = ptr-1;
    b[ptr] = 0xff;
	for(i=1;i<ptr;i++) {
		b[ptr] ^= b[i];
	}
    for(byte k=0;k<ptr+1;k++)
      Serial2.write(b[k]);
	Serial.print("ptr=");
	Serial.println(ptr);
  }
  
  void handleCommand(int src, int len, byte command[]) {
    byte b[10];
    byte cls = command[0];
    byte cmd = command[1];
    Serial.print("Receive command\n");
    Serial.println(cls);
    Serial.println(cmd);
	if (cmd_handler) cmd_handler(command,len);
    if (cls == COMMAND_CLASS_BASIC) {
        switch(cmd) {
          case BASIC_SET:
              Serial.print("Receive Set command\n");
              g_basic_level = command[2];
			  if (switch_binary_handler)
			  	switch_binary_handler(g_basic_level);
              break;
          case BASIC_GET:
              b[0] = cls;
              b[1] = BASIC_REPORT;
              b[2] = g_basic_level;
              send(src,b,3,5);
              break;
          
        }
    } else if (cls == COMMAND_CLASS_ASSOCIATION) {
      byte n;
      switch(cmd) {
        case ASSOCIATION_GET:
#ifdef EEPROM_h
          n = EEPROM.read(EEPROM_GROUP1);
          b[0] = COMMAND_CLASS_ASSOCIATION;
          b[1] = ASSOCIATION_REPORT;
          b[2] = command[2];
          b[3] = b[2]==1? 1:0;
          b[4] = 0;
          if (b[2] != 1 || n == 0) {
            send(src,b,5,5);
          } else {
            b[5] = n;
            send(src,b,6,5);
          }
#endif		  
          break;
        case ASSOCIATION_REMOVE:
#ifdef EEPROM_h
          EEPROM.write(EEPROM_GROUP1,0);
#endif
          break;
        case ASSOCIATION_GROUPING_GET:
#ifdef EEPROM_h
          b[0] = COMMAND_CLASS_ASSOCIATION;
          b[1] = ASSOCIATION_GROUPING_REPORT;
          b[2] = 1;
          send(src,b,3,5);
		  #endif
          break;
        case ASSOCIATION_SET:
#ifdef EEPROM_h		
          if (command[2] == 1)
            EEPROM.write(EEPROM_GROUP1,command[2]);
#endif			
          break;
      }
    } else if (cls == COMMAND_CLASS_SENSOR_MULTILEVEL) {
        if (cmd == SENSOR_MULTILEVEL_GET) {
        }
    } else if (cls == COMMAND_CLASS_SENSOR_BINARY) {
        if (cmd == SENSOR_BINARY_GET) {
            b[0] = COMMAND_CLASS_SENSOR_BINARY;
            b[1] = SENSOR_BINARY_REPORT;
            b[2] = g_basic_level? 0xff:0;
            send(src,b,3,5);
        }
    }
  }
};

#endif