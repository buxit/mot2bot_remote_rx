//#define TAVBOT

#ifdef TAVBOT
#define NODE_ID 5
#define RF69_COMPAT 0 // define this to use the RF69 driver i.s.o. RF12
#else
#define NODE_ID 3
#define RF69_COMPAT 1
#endif

#include <JeeLib.h>

#define MASSAGE_PARSER_BUFFERSIZE 48
#define MASSAGE_PACKER_BUFFERSIZE 48

#include <SlipMassagePacker.h>
#include <SlipMassageParser.h>


#define LED_PIN 9

#define MAGIC_REMOTE 6435437

struct RemoteData {
  long magic;
  short h1;
  short v1;
  short h2;
  short v2;
  short mode;
};

struct RobotData {
  long magic = 6435437;
  float bat_percent;
  float bat_voltage;
  int8_t rssi;
  char ee = 0xc0;
};

RemoteData state;
RobotData rdata;

SlipMassageParser inbound;
SlipMassagePacker outbound;

MilliTimer sendTimer;

void setup() {
  Serial.begin(115200);
  delay(100);
  //rf12_configSilent();
  rf12_initialize((0x80 | NODE_ID), (0x80 | NODE_ID) >> 6, 212, 1600);
  delay(100);
  rdata.bat_percent = -1.0;
  rdata.bat_voltage = -1.0;
}

short needToSend = 0;

void loop() {
  if (rf12_recvDone() && rf12_crc == 0) {
    memcpy(&state, (void*)rf12_data, sizeof(RemoteData));
    /*
      Serial.print((char)rf12_data[0]);
      Serial.print((char)rf12_data[1]);
      Serial.print((char)rf12_data[2]);
      Serial.println(rf12_data[3]);
    */
    if (state.magic == MAGIC_REMOTE)
    {
      Serial.print(state.h1);
      Serial.print(" ");
      Serial.print(state.v1);
      Serial.print(" ");
      Serial.print(state.h2);
      Serial.print(" ");
      Serial.print(state.v2);
      Serial.print(" ");
      Serial.print(state.mode);
      Serial.println();
    } else {
      Serial.print("Wrong magic: ");
      Serial.println(state.magic);
    }
    //needToSend = 1;
  }

  if (sendTimer.poll(1000)) {
    needToSend = 1;
  }

  //delay(10);
  if (needToSend == 1 && rf12_canSend()) {
    activityLed(1);
    byte header = RF12_HDR_DST | (NODE_ID+1);
    rdata.magic = 6435437;
    rdata.rssi = -(RF69::rssi >> 1);
    rf12_sendStart(header, &rdata, sizeof(RobotData));
    needToSend = 0;
    activityLed(0);
  }
//  if ( inbound.parseStream( &Serial ) ) {
    while(Serial.available()) {
      if(inbound.parse( Serial.read() )) {
        //Serial.print("parseStream: ");
        if ( inbound.fullMatch("status") ) {
          /*
          byte byteValue = inbound.nextByte();
          int intValue = inbound.nextInt();
          long longValue = inbound.nextLong();
          */
          float bat_percent = inbound.nextFloat();
          float bat_voltage = inbound.nextFloat();
          if(bat_percent != rdata.bat_percent)
          {
            rdata.bat_percent = bat_percent;
            needToSend = 1;
          }
          if(bat_voltage != rdata.bat_voltage)
          {
            rdata.bat_voltage = bat_voltage;
            needToSend = 1;
          }
          /*
          Serial.print(bat_percent);
          Serial.print(" ");
          Serial.print(bat_voltage);
          */
        }
      //Serial.println(".");
    }
  }
}

static void activityLed (byte on) {
#ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, !on);
#endif
}

