#include <stdio.h>
#include "Arduino.h"
#include <HardwareSerial.h>
class WuKong {
  private:
    int state;
  public:
    int debug;
    const static int ST_INIT=0;
    const static int ST_O=2;
    const static int ST_K=3;
    const static int ST_C=4;
    const static int ST_L=5;
    const static int ST_COLON=6;
    const static int ST_LEN=7;
    const static int ST_EOL1=8;
    const static int ST_EOL2=9;
    const static int ST_CONTENT=10;

    const static int SWITCH=1;
    const static int SENSOR=4;
    const static int CURTAIN=5;
    const static int SW2=10;
    const static int IR=14;
	int payload_len;
	int payload_ptr;
	char payload[20];
	int waitNode;
	int waitNodeCommand;
	long int waitNodeNext;
    void init() {
      Serial2.begin(115200);
      debug=0;
	  waitNode=0;
    }
    void include() {
      Serial2.print("GET /00,0 \n\n");
	  wait();
    }
  
    void exclude() {
      Serial2.print("GET /10,0 \n\n");
	  wait();
    }
  
    void command(int p,int d,int v) {
      long int num = v;
      num = num*256+d;
      num = num*256+p;
      Serial2.print("GET /r");
      Serial2.print(num);
      Serial2.print(",0 \n\n");
	  wait();
    }
	int hex(char *s) {
		int v=0;
		if (*s>='0'&&*s<='9')
			v = *s-'0';
		else if (*s>='a'&&*s<='f')
			v = *s-'a'+10;
		s++;
		if (*s>='0'&&*s<='9')
			v = v*16+*s-'0';
		else if (*s>='a'&&*s<='f')
			v = v*16+*s-'a'+10;
		return v;
	}
    void learn(const int mode,int port,int device) {
      char buf[32];
      snprintf(buf,32,"GET /w%d,0 \n\n", mode);
      Serial2.print(buf);
      Serial.print(buf);
	  wait();
	  waitNode=1;
	  waitNodeNext = millis()+1000;
	  waitNodeCommand = device;
	  waitNodeCommand = waitNodeCommand*256+port;
    }
    void join() {
      Serial2.print("GET /p0,0 \n\n");
	  wait();
    }
    void reset() {
      Serial2.print("GET /s0,0 \n\n");
	  wait();
    }
	void wait() {
	  state = ST_O;
	  while(state != ST_INIT)
	  	loop();
	}
    void getStatus() {
      Serial2.print("GET /w255,0  \n\n");
	  wait();
    }
    void stop() {
      Serial2.print("GET /90,0 \n\n");
	  wait();
    }
    void dump() {
      Serial2.print("GET /y0,0 \n\n");
	  wait();
    }
    void loop();
};

