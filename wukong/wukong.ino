class WuKong {
  private:
    int state;
  public:
    int debug;
    const static int ST_INIT=0;
    const static int ST_O=2;
    const static int SWITCH=1;
    const static int SENSOR=4;
    const static int CURTAIN=5;
    const static int SW2=10;
    const static int IR=14;

    const static int INIT = 0;
    const static int ID = 1;
    const static int LEN = 2;
    const static int CMD = 3;
    const static int STATUS = 4;
    const static int PORT = 5;
    const static int DEVICE = 6;
    const static int VALUE = 7;
    const static int VALUE6 = 7;
    const static int END = 8;
    void init() {
      Serial2.begin(115200);
      debug=0;
      state = INIT;
    }
    void include() {
    }
  
    void exclude() {
      Serial2.print("GET /10,0 \n\n");
    }
  
    void command(int p,int d,int v) {
      long int num = v;
      num = num*256+d;
      num = num*256+p;
      Serial2.print("GET /r");
      Serial2.print(num);
      Serial2.print(",0 \n\n");
    }
    void learn(const int mode) {
      char buf[32];
      snprintf(buf,32,"GET /w%d,0 \n\n", mode);
      Serial2.print(buf);
      Serial.print(buf);
    }
    void join() {
      Serial2.print("GET /q0,0 \n\n");
    }
    void reset() {
      Serial2.print("GET /s0,0 \n\n");
    }
    void getStatus() {
      Serial2.print("GET /40,0  \n\n");
    }
    void stop() {
      Serial2.print("GET /90,0 \n\n");
    }
    void dump() {
      Serial2.print("GET /y0,0 \n\n");
    }
    int port,device;
    int payload_len;
    
    void loop() {
      char buf[32];
      if (Serial2.available()) {
        byte c = Serial2.read();
        if (0) {
          snprintf(buf,20,"c=%02x(%c) s=%d\n", c,c, state);
          Serial.print(buf);
        } else if (state == INIT && debug) {
          if (c != 0xc0)
            Serial.print(c);
        }
        if (state == INIT) {
          if (c == 0xc0) {
            state = ID;
          } else {
          }
        } else if (state == ID) {
          state = LEN;
        } else if (state == LEN) {
          state = CMD;
        } else if (state == CMD) {
          if (c == 0x49) {
            state = STATUS;
          } else {
            state = END;
          }
        } else if (state == STATUS) {
          state = PORT;
        } else if (state == PORT) {
          port = c;
          state = DEVICE;
        } else if (state == DEVICE) {
          if (c == 0xc1) {
            state = INIT;
          } else {
            device = c;
            state = VALUE;
          }
        } else if (state == VALUE) {
          if (device == 0x44 || device == 0x45) {
            if (c <= 5) {
              state = VALUE6;
              payload_len = 5;
              return;
            }
          }
          state = PORT;
          if (debug) {
            snprintf(buf,sizeof(buf),"%02x %02x %02x", port,device,c);
            Serial.println(buf);
          }
        } else if (state == VALUE6) {
          payload_len--;
          if (payload_len == 0) {
            state = PORT;
          }
        } else if (state == END) {
          if (c == 0xc1) {
            state = INIT;
          }
        }
      }
    }
};

WuKong wukong;
int room;
int device;
int deviceStart;
void setup()
{
  wukong.init();
  wukong.debug=1;
  Serial.begin(115200);
}
int state =0;

#define HELP \
"'a': Include a device\n"\
"'d': Exclude a device\n"\
"'s': Stop include or exclude\n"\
"'L': send a Light command\n"\
"'C': send a curtain command\n"\
"'A': send a air consditioner command\n"\
"'V': send a AV command\n"\
"'z': reset the device\n"\
"'c': Check the status\n"


char buf[32];

void help()
{
  Serial.print(HELP);
}
void loop()
{
  wukong.loop();
  if (Serial.available()) {
    char c = Serial.read();
    snprintf(buf,32,"state = %d\n", state);
    Serial.print(buf);
    if (state == ' ' || state == '\n') return;
    if (state == 4) {
      if (c >='1' && c <='4') {
        room = c-'0';
        state--;
      } else {
        state = 0;
      }
      return;
    } else if (state == 3) {
      if (c >='1' && c <= '8') {
        device=c-'0';
        state--;
      } else {
        state = 0;
      }
      return;
    } else if (state == 2) {
      if (c=='o') {
        state--;
      } else {
        state = 0;
      }
      return;
    } else if (state == 1) {
      if (c == 'n') {
        wukong.command(room, device + deviceStart, 255);
      } else {
        wukong.command(room, device + deviceStart, 0);
      }
      state--;
      return;
    } else if (state == 11) {
      if (c == 'L') {
        Serial.print("Learn a switch\n");
        wukong.learn(wukong.SWITCH);
      } else if (c == 'C') {
        wukong.learn(wukong.CURTAIN);
      } else if (c == 'A') {
        wukong.learn(wukong.IR);
      } else if (c == 'V') {
        wukong.learn(wukong.IR);
      }
      state = 0;
      return;
    } else if (state == 21) {
      if (c == 'L') {
        wukong.exclude();
      }
      state = 0;
    }
    if (c == 'h') {
      help();
    } else if (c == 'a') {
      state = 11;
      Serial.print("Select type\n");
    } else if (c == 'd') {
      Serial.print("Select type\n");
      state = 21;
    } else if (c == 'L') {
      state = 4;
      deviceStart = 0x11;
    } else if (c == 'C') {
      state = 4;
      deviceStart = 0x21;
    } else if (c == 'A') {
      state = 4;
      deviceStart = 0x31;
    } else if (c == 'V') {
      state = 4;
      deviceStart=0xa;
    } else if (c == 'j') {
      wukong.join();
    } else if (c == 'z') {
      wukong.reset();
    } else if (c == 'c') {
      wukong.getStatus();
    } else if (c == 's') {
      wukong.learn(0);
    } else if (c == 'y') {
      wukong.dump();
    } else if (c == 'x') {
      wukong.learn(255);
    } 
  }
}
