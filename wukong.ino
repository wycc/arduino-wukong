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
    void init() {
      Serial2.begin(115200);
      debug=0;
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
    void loop() {
      if (Serial2.available()) {
        char c = Serial2.read();
        if (1) {
          char buf[20];
          snprintf(buf,20,"c=%c s=%d\n", c, state);
          Serial.print(buf);
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
      wukong.stop();
    } else if (c == 'y') {
      wukong.dump();
    }
  }
}
