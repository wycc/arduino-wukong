#include "WuKong.h"
void WuKong::loop()
{
   if (state == ST_INIT && waitNode && waitNodeNext < millis()) {
   	  Serial2.print("GET /w255,0 \n\n");
	  waitNodeNext += 1000;
	  state = ST_O;
   }
   if (Serial2.available()) {
      char c = Serial2.read();
      if (debug) {
        char buf[20];
        snprintf(buf,20,"c=%c(%x) s=%d\n", c, c,state);
        Serial.print(buf);
      }
	  if (state == ST_INIT) {
	  } else if (state == ST_O) {
	  	if (c == 'O') {
			state = ST_K;
		} else {
			state = ST_INIT;
		}
	  } else if (state == ST_K) {
	  	if (c == 'K') {
			state = ST_C;
		}
	  } else if (state == ST_C) {
	  	if (c == 'C')
			state = ST_L;
		else if (c == '\n')
			;
		else 
			state = ST_INIT;
	  } else if (state == ST_L) {
	  	if (c == 'L')
			state = ST_COLON;
		else
			state = ST_INIT;
	  } else if (state == ST_COLON) {
	  	if (c == ':') {
			state = ST_LEN;
			payload_len = 0;
			payload_ptr = 0;
		} else {
			state = ST_INIT;
		}
	  } else if (state == ST_LEN) {
	  	if (c >='0' && c <='9')
		  	payload_len = payload_len*10+(c-'0');
		else if (c == '\n')
			state = ST_EOL2;
	  } else if (state == ST_EOL1) {
	  	if (c == '\n')
			state = ST_EOL2;
	  } else if (state == ST_EOL2) {
	  	if (c == '\n')
			state = ST_CONTENT;
		else
			state = ST_EOL1;
	  } else if (state == ST_CONTENT) {
	    Serial.print(payload_len);
	  	payload_len--;
		payload[payload_ptr++] = c;
		if (payload_len == 0) {
			state = ST_INIT;
			if (waitNode) {
				Serial.print("ptr=");
			    Serial.print(payload_ptr);
				Serial.print("\n");
		  		if (payload_ptr == 6) {
				  	int state = payload[1];
					int mode = payload[3];
					int id = hex(payload+4);
					if (state == '5') {
      					Serial2.print("GET /r");
						Serial2.print(waitNodeCommand);
						Serial2.print(",0 \n\n");
						wait();
						Serial2.print("GET /w0,0 \n\n");
						wait();
						waitNode = 0;
						return;
					}
			  	}
			}
		}
	  }
   }
}
