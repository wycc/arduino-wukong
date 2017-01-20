// vim: ts=4 sw=4
#ifndef __ZWAVESLAVE_H
#define __ZWAVESLAVE_H
#include "Arduino.h"
/*****************************************/
//     Hardware specific definition
/*****************************************/
#ifdef MODEL_HC20
#define USE_UART2
#define DEBUG Serial
#endif

#ifdef MODEL_HC30
#define USE_UART0
#define DEBUG Serial1
#endif

#ifndef DEBUG
#define DEBUG Serial
#endif

#if defined(USE_UART1)
#define SERIAL Serial1
#else

#if defined(USE_UART2)
#define SERIAL Serial2
#else
#if defined(USE_UART0)
#define SERIAL Serial
#else
#error "Please define USE_UART1 or USE_UART2"
#endif
#endif
#endif

#define EEPROM_GROUP1 10
#define EEPROM_CONF 20
#define EEPROM_SWITCH_MULTILEVEL 30
#define EEPROM_SWITCH_MULTILEVEL_LEVEL 40
#define SENSOR_PORT 5

boolean g_sensor_state=false;


#define APPLICATIONCOMMANDHANDLER 0x04
#define APPLICATIONSLAVEUPDATE    0x49

#define COMMAND_CLASS_SWITCH_BINARY 0x25

#define COMMAND_CLASS_SWITCH_MULTILEVEL 0x26
#define SWITCH_MULTILEVEL_GET 2
#define SWITCH_MULTILEVEL_SET 1
#define SWITCH_MULTILEVEL_REPORT 3
#define SWITCH_MULTILEVEL_START 4
#define SWITCH_MULTILEVEL_STOP 5

#define COMMAND_CLASS_METER 0x32
#define METER_GET 1
#define METER_REPORT 2

#define COMMAND_CLASS_MANUFACTURE 0x72
#define MANUFACTURE_GET 4
#define MANUFACTURE_REPORT 5

#define COMMAND_CLASS_BASIC 0x20
#define COMMAND_CLASS_CONFIGURATION 0x70
#define CONFIGURATION_SET 4
#define CONFIGURATION_GET 5
#define CONFIGURATION_REPORT 6
#define CONFIGURATION_BULK_SET 7

#define COMMAND_CLASS_ASSOCIATION 0x85
#define ASSOCIATION_GET 2
#define ASSOCIATION_GROUPING_GET 5
#define ASSOCIATION_GROUPING_REPORT 6
#define ASSOCIATION_REMOVE 4
#define ASSOCIATION_REPORT 3
#define ASSOCIATION_SET 1

#define GENERIC_TYPE_SWITCH_BINARY   0x10
#define GENERIC_TYPE_SWITCH_MULTILEVEL  0x11
#define GENERIC_TYPE_SENSOR_BINARY   0x20
#define GENERIC_TYPE_SENSOR_MULTILEVEL   0x21
#define COMMAND_CLASS_SENSOR_BINARY  0x30
#define SENSOR_BINARY_GET            0x02
#define SENSOR_BINARY_REPORT         0x03


#define COMMAND_CLASS_SENSOR_MULTILEVEL 0x31
#define SENSOR_MULTILEVEL_GET 4
#define SENSOR_MULTILEVEL_REPORT 5
#define SENSOR_TYPE_TEMPERATURE 1
#define SENSOR_TYPE_HUMID 5
#define SENSOR_TYPE_POWER C4
#define SENSOR_SCALE_TEMPERATURE_F 0
#define SENSOR_SCALE_TEMPERATURE_C 1

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
#ifndef MAX_VALUE
#define MAX_VALUE 4
#endif

#ifndef MAX_CONF
#define MAX_CONF 10
#endif

#ifndef MAX_DIMMER
#define MAX_DIMMER 4
#endif

class ZWaveSlave {
private:  
  byte seq;          // Sequence number which is used to match the callback function
  int state;         // Current state
  int len;           // Length of the returned payload
  int type;          // 0: request 1: resposne 2: timeout
  int cmd;           // the serial api command number of the current payload
  byte g_basic_level;
  byte payload[64];  // The data of the current packet
  unsigned long learn_info_time,learn_stop_time;
  int i;            
  unsigned long expire;  // The expire time of the last command
  void (*f)(byte *,int); // The callback function registered by callback
  void (*cmd_handler)(byte *,int); // The callback function registered by callback
  void (*switch_binary_handler)(byte v); // The callback function registered by callback
  void (*nodeinfo)(byte *payload,int len);
  bool hasSensorBinary;
  bool hasSwitchBinary;
  bool hasSensorMultilevel;
  bool hasAssociation;
  bool hasMultilevelSwitch;
  byte generic,specific;
  byte sensorNum;
  byte sensorType[MAX_VALUE];
  byte sensorPrecision[MAX_VALUE];
  byte sensorScale[MAX_VALUE];
  float sensorValue[MAX_VALUE];
  byte confNum;
  byte numDimmer;
  bool debug;
public:
  byte multilevelswitchlevel[MAX_DIMMER];
  byte multilevelswitch[MAX_DIMMER];
  byte multilevelstep[MAX_DIMMER];
  bool hasMeter;
  byte meterType;
  byte meterValue;
  byte vendorID1,vendorID2;
  byte productID1,productID2;

  byte configurations[MAX_CONF];
  ZWaveSlave() {
	byte i;
	DEBUG.begin(115200);
    SERIAL.begin(115200);
    SERIAL.write(6);
    //version();
	//mainloop();
    for(i=0;i<30;i++) {
       mainloop();
	}
    for(i=0;i<30;i++) {
       mainloop();
	}
    //randomSeed(analogRead(0));
    seq = 0;
    state = ST_SOF;
    expire = 0;
    nodeinfo=NULL;
	cmd_handler=NULL;
    f=NULL;
	g_basic_level = 0;
	learn_info_time = 0;
	learn_stop_time = 0;
	hasSensorBinary = false;
	hasSwitchBinary = false;
	hasAssociation = false;
	hasMultilevelSwitch = false;
	hasSensorMultilevel=false;
	numDimmer = 0;
	confNum=0;
	vendorID1 = 0x01;
	vendorID2 = 0x62;
	productID1 = 0x30;
	productID2 = 0;
	debug = false;
  }
  void init(byte g,byte s) {
	generic = g;
	specific = s;
#ifdef MODEL_HC20	
    DDRK |= (1<<7);
	PORTK &= ~(1<<7);
	delay(100);
	PORTK |= (1<<7);
	DDRA &= ~(1<<1);
	PORTA |= (1<<1);
#endif	
    init_nodeinfo();
  }
  void enableDebug() {debug = true;}
  void disableDebug() {debug = false;}
  int getType() {
    return type;
  }
  void DisplayNodeInfo() {
    char buf[128];

    snprintf(buf,64,"Status=%d Node=%d Device=%d:%d:%d\n", payload[0],payload[1],payload[3],payload[4],payload[5]);
    DEBUG.write(buf);
  }
  void enableBinarySensor() {
  	hasSensorBinary = true;
  }
  void enableBinarySwitch() {
  	hasSwitchBinary = true;
  }
  void setBasicHandler(void (*handler)(byte v)) {
	  switch_binary_handler = handler;
  }
  void setSwitchBinaryHandler(void (*handler)(byte v)) {
	  switch_binary_handler = handler;
  }
  void enableMultilevelSwitch(byte ins) {
	byte i;
	hasMultilevelSwitch = true;
	for(i=0;i<ins;i++) {
#ifdef EEPROM_h
		multilevelswitch[i] = EEPROM.read(EEPROM_SWITCH_MULTILEVEL+i);
		multilevelswitchlevel[i] = EEPROM.read(EEPROM_SWITCH_MULTILEVEL_LEVEL+i);
#else
		multilevelswitch[i] = 0;
		multilevelswitchlevel[i] = 0;
#endif
		multilevelstep[i] = 0;
	}
  }
  void enableConfiguration(byte num) {
	if (num >= 0 && num < 32) {
		byte n;

	  	confNum = num;
		for(n=0;n<confNum;n++) {
			configurations[n] = EEPROM.read(EEPROM_CONF+n);
		}
	}
  }
  void enableAssociation() {
  	hasAssociation = true;
  }
  void enableMultilevelSensor(byte n) {
  	hasSensorMultilevel = true;
	sensorNum = n;
	for(byte i=0;i<sensorNum;i++) {
		sensorType[i] = 0;
		sensorPrecision[i] = 0;
		sensorScale[i] = 0;
		sensorValue[i] = 0;
	}
  }
  void enableMeter(byte type) {
  	hasMeter = true;
	meterType = type;
  }
  void updateMeter(byte value) {
	meterValue = value;
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
  void sendToGroup(byte group, byte v) {
        if (group == 1) {
#ifdef EEPROM_h
            byte src = EEPROM.read(EEPROM_GROUP1);
	    byte b[3];
	    b[0] = COMMAND_CLASS_BASIC;
	    b[1] = BASIC_SET;
	    b[2] = v+0xa0;
	    send(src,b,3,5);
#endif
        }
  }
  void setupSensorType(byte i, byte pre, byte scale,byte type) {
  	if (i < sensorNum) {
	  	sensorType[i] = type;	
		sensorPrecision[i] = pre;
		sensorScale[i] = scale;
	}
  }
  void updateMultilevelSensor(byte i, float v) {
  	if (i < sensorNum)
		sensorValue[i] = v;
  }
  void updateMultilevelSwitch(byte i, byte v) {
  	if (i < numDimmer)
		sensorValue[i] = v;
  }
  void updateBinarySwitch(bool v) {
	  g_basic_level = v;
  }
  void updateBinarySensor(bool v) {
	byte newv = v?255:0;
    if (newv != g_basic_level) {
	  	g_basic_level = newv;
#ifdef EEPROM_h
      byte src = EEPROM.read(EEPROM_GROUP1);
      byte b[3];
      b[0] = COMMAND_CLASS_BASIC;
      b[1] = BASIC_SET;
      b[2] = g_basic_level;
      send(src,b,3,5);
#endif
	}
  }
  void updateManufacture(byte vid1, byte vid2, byte pid1, byte pid2) {
	  vendorID1 = vid1;
	  vendorID2 = vid2;
	  productID1 = pid1;
	  productID2 = pid2;
  }
  void updateConfiguration(byte n, byte value) {
	  if (n <= confNum)
		  configurations[n-1] = value;
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
			DEBUG.println("send info");
	    }
	    if (learn_stop_time && now > learn_stop_time) {
		    //learn(0);
		    learn_stop_time = 0;
	    }
	}
    if (SERIAL.available()) {
      expire = now + 1000;
      byte c = SERIAL.read();
	  if (debug) {
      	char buf[128];
	    snprintf(buf,128,"-> c=%x state=%d\n", c, state);
	    DEBUG.write(buf);
	  }
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
        SERIAL.write(6);
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

    DEBUG.write("Send\n");
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
    SERIAL.write(buf[0]);
    for(k=0;k<l+7;k++) {
      SERIAL.write(buf[k+1]);
      //Serial.println(buf[k+1]);
      crc = crc ^ buf[k+1];
    }
    seq++;
    SERIAL.write(crc);
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
      SERIAL.write(b[k]);
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
      SERIAL.write(b[k]);

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
      SERIAL.write(b[k]);

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
      SERIAL.write(b[k]);

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
      SERIAL.write(b[k]);
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
	ptr = 8;
	b[ptr++] = COMMAND_CLASS_BASIC;
	if (hasSwitchBinary)
	    b[ptr++] = COMMAND_CLASS_SWITCH_BINARY;
	if (hasSensorBinary)
	    b[ptr++] = COMMAND_CLASS_SENSOR_BINARY;
	if (hasMultilevelSwitch)
	    b[ptr++] = COMMAND_CLASS_SWITCH_MULTILEVEL;	    
	if (hasAssociation)
	    b[ptr++] = COMMAND_CLASS_ASSOCIATION;
	if (hasSensorMultilevel)
	    b[ptr++] = COMMAND_CLASS_SENSOR_MULTILEVEL;
	if (hasMeter)
	    b[ptr++] = COMMAND_CLASS_METER;
	if (confNum)
	    b[ptr++] = COMMAND_CLASS_CONFIGURATION;
	b[7] = ptr-8;
	b[1] = ptr-1;
    b[ptr] = 0xff;
	for(i=1;i<ptr;i++) {
		b[ptr] ^= b[i];
	}
    for(byte k=0;k<ptr+1;k++)
      SERIAL.write(b[k]);
	DEBUG.print("ptr=");
	DEBUG.println(ptr);
  }
  
  void sendSensorReport(byte src,byte ch) {
	byte b[10];
	float v=sensorValue[ch];
	b[0] = COMMAND_CLASS_SENSOR_MULTILEVEL;
	b[1] = SENSOR_MULTILEVEL_REPORT;
	b[2] = sensorType[ch];
	for(byte j=0;j<sensorPrecision[ch];j++) {
		v = v * 10;
	}
	if (src == 0) 
		src = EEPROM.read(EEPROM_GROUP1);
	long l = v;
	byte size=1;
	if (l >= (1L<<15))
		size = 4;
	else if (l >= (1L<<7))
		size = 2;

	b[3] = (sensorPrecision[ch]<< 5) | (sensorScale[ch]<<3) | size;
	if (size == 1) {
		b[4] = l&0xff;
		send(src,b,5,5);
	} else if (size == 2) {
		b[4] = (l>>8)&0xff;
		b[5] = (l)&0xff;
		send(src,b,6,5);
	} else if (size == 4) {
		b[4] = (l>>24)&0xff;
		b[5] = (l>>16)&0xff;
		b[6] = (l>>8)&0xff;
		b[7] = (l)&0xff;
		send(src,b,8,5);
	}
  }
  

  void handleCommand(int src, int len, byte command[]) {
    byte b[10];
    byte cls = command[0];
    byte cmd = command[1];
    byte n;

    DEBUG.print("Receive command\n");
    DEBUG.println(cls);
    DEBUG.println(cmd);
	if (cmd_handler) cmd_handler(command,len);
    if (cls == COMMAND_CLASS_BASIC || cls == COMMAND_CLASS_SWITCH_BINARY) {
        switch(cmd) {
          case BASIC_SET:
              DEBUG.print("Receive Set command\n");
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
	  if (hasAssociation==false) return;
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
    } else if (cls == COMMAND_CLASS_SWITCH_MULTILEVEL) {
	    if (hasMultilevelSwitch == false) return;
	    if (cmd == SWITCH_MULTILEVEL_GET) {
		    b[0] = COMMAND_CLASS_SWITCH_MULTILEVEL;
		    b[1] = SWITCH_MULTILEVEL_REPORT;
		    b[2] = (multilevelswitch[0]==255)? multilevelswitchlevel[0]:multilevelswitch[0];
		    send(src,b,3,5);
	    } else if (cmd == SWITCH_MULTILEVEL_SET) {
		    multilevelswitch[0] = command[2];
			if ((multilevelswitch[0] != 0) && (multilevelswitch[0] != 255))
				multilevelswitchlevel[0] = multilevelswitch[0];
#ifdef EEPROM_h		
            EEPROM.write(EEPROM_SWITCH_MULTILEVEL,multilevelswitch[0]);
            EEPROM.write(EEPROM_SWITCH_MULTILEVEL_LEVEL,multilevelswitchlevel[0]);
#endif			
	    } else if (cmd == SWITCH_MULTILEVEL_START) {
			multilevelstep[0] = command[2];
		} else if (cmd == SWITCH_MULTILEVEL_STOP) {
			multilevelstep[0] = 0;
		}
    } else if (cls == COMMAND_CLASS_SENSOR_MULTILEVEL) {
		if (hasSensorMultilevel==false) return;
		byte i=0;

		if (len > 2) {
			byte t = command[2];
			byte scale = (command[3]>>3)&0x3;
			DEBUG.print("search ");
			DEBUG.print(command[3]);
			DEBUG.print(" ");
			DEBUG.println(scale);
			for(i=0;i<sensorNum;i++) {
				DEBUG.print(sensorType[i]);
				DEBUG.print(' ');
				DEBUG.println(sensorScale[i]);
				if (t == sensorType[i] && sensorScale[i] == scale ) break;
			}
			if (i == sensorNum) return;
		}

        if (cmd == SENSOR_MULTILEVEL_GET) {
			sendSensorReport(src, 1);
        }
    } else if (cls == COMMAND_CLASS_SENSOR_BINARY) {
		if (hasSensorBinary==false) return;
        if (cmd == SENSOR_BINARY_GET) {
            b[0] = COMMAND_CLASS_SENSOR_BINARY;
            b[1] = SENSOR_BINARY_REPORT;
            b[2] = g_basic_level? 0xff:0;
            send(src,b,3,5);
        }
    } else if (cls == COMMAND_CLASS_CONFIGURATION) {
		if (cmd == CONFIGURATION_GET) {
			DEBUG.print("confNum=");DEBUG.print(confNum);DEBUG.print(" command[2]=");DEBUG.println(command[2]);
			if (command[2] > confNum) return;
			b[0] = cls;
			b[1] = CONFIGURATION_REPORT;
			b[2] = command[2];
			b[3] = 1;
			b[4] = configurations[command[2]-1];
			send(src,b,5,5);
		} else if (cmd == CONFIGURATION_SET) {
			int size = command[3]&0x7;
			if (command[2] > confNum) return;
			byte v;
			if (size == 1)
				v = command[4];
			else if (size == 2) 
				v = command[5];
			else
				v = command[7];
			EEPROM.write(EEPROM_CONF+command[2]-1, v);
			configurations[command[2]-1] = v;
		}
	} else if (cls == COMMAND_CLASS_METER) {
		if (cmd == METER_GET) {
			b[0] = cls;
			b[1] = METER_REPORT;
			b[2] = 1;
			b[3] = (meterType<<3) | 1;
			b[4] = meterValue;
			send(src,b,5,5);
		}
	} else if (cls == COMMAND_CLASS_MANUFACTURE) {
		if (cmd == MANUFACTURE_GET) {
			b[0] = cls;
			b[1] = MANUFACTURE_REPORT;
			b[2] = vendorID1;
			b[3] = vendorID2;
			b[4] = 0;
			b[5] = 0;
			b[6] = productID1;
			b[7] = productID2;
			send(src,b,8,5);
		}
	}
  }
};

#endif
