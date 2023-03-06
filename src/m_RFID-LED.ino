/**
 * @file RFID_LED.ino for Analyzer Pod83
 * @author Christian Walter (pauseandplay@cybwalt.de)
 * @brief 
 * @version 1.6.2
 * @date 04.02.2023
 * build with lib_arduino v0.6.2
 */


#include "header_st.h"

#include <stb_common.h>
#include <avr/wdt.h>

#include <stb_rfid.h>
#include <stb_brain.h>
#include <stb_led.h>
#include <stb_servo.h>

#define ledDisable 1
#define rfidDisable 1
#define relayDisable 1
//#define ServoDisable 1


STB_BRAIN Brain;

#ifndef ledDisable 
    STB_LED LEDS;
#endif

#ifndef ServoDisable 
    STB_SERVO Servos;
#endif


#ifndef rfidDisable
    Adafruit_PN532 RFID_0(PN532_SCK, PN532_MISO, PN532_MOSI, RFID_1_SS_PIN);
       Adafruit_PN532 RFID_READERS[1] = {RFID_0};
    STB_RFID RFIDS;
    unsigned long lastRfidCheck = millis();
    size_t  length_message = 0;
#endif


#ifndef ServoDisable 
    void servoRecieve() {

        while (Brain.STB_.rcvdPtr != NULL) {
            Servos.evaluateCmds(Brain);
            Brain.nextRcvdLn();
        }
    }
#endif

void setup() {
    Brain.begin();
    Brain.setSlaveAddr(1);
    Brain.dbgln(F("WDT endabled"));
    wdt_enable(WDTO_8S);
    wdt_reset();

#ifndef ledDisable 
   // ledCount may aswell be one row 
    for (int i=0; i<ledCnt; i++) {
        // col 0 is the cmd type 0 is for setLedamount aka settingCmds::ledCount;
        Brain.settings[i][0] = settingCmds::ledCount;
        // col 1 is the PWM index
        Brain.settings[i][1] = i;
        // col 2 is the amount of leds
        Brain.settings[i][2] = 6;
    }

    Brain.settings[ledCnt][0] = settingCmds::ledClrOrder;
    Brain.settings[ledCnt][1] = NEO_GRB;
#endif

    Brain.flags = servoFlag;

#ifndef ledDisable
    if (Brain.flags & ledFlag) {
        LEDS.ledInit(Brain.settings); 
        Serial.println("Color Test");       
        LEDS.setAllStripsToClr(LEDS.Strips[0].Color(255, 0, 0));
        delay(2000);
        LEDS.setAllStripsToClr(LEDS.Strips[0].Color(0, 255, 0));
        delay(2000);
        LEDS.setAllStripsToClr(LEDS.Strips[0].Color(0, 0, 255));
        delay(2000);
        LEDS.setAllStripsToClr(LEDS.Strips[0].Color(255, 255, 255));
        delay(2000);
        //Serial.println(F("Color Test finished"));        
    }
#endif

#ifndef rfidDisable
    if (Brain.flags & rfidFlag) {
        STB_RFID::RFIDInit(RFID_0);
        STB_RFID::RFIDInit(RFID_1);
        STB_RFID::RFIDInit(RFID_2);
        STB_RFID::RFIDInit(RFID_3);
        wdt_reset();
    }
#endif
    wdt_reset();
}

void loop() {
    //Serial.println(millis());
#ifndef rfidDisable
    if (Brain.flags & rfidFlag) {
        rfidRead();
    }
#endif
    
#ifndef ledDisable
    if (Brain.flags & ledFlag && Brain.slaveRespond()) {
        Serial.println("slave got pushed");
        Serial.println(Brain.STB_.rcvdPtr);
        ledReceive();
    } 
  
    LEDS.LEDloop(Brain);      
#endif

#ifndef ServoDisable 
    if (Brain.flags & servoFlag && Brain.slaveRespond()) {
        Serial.println("slave got pushed");
        Serial.println(Brain.STB_.rcvdPtr);
        servoRecieve();
    }    
#endif

    wdt_reset();
    
}


#ifndef rfidDisable
void rfidRead() {
    if (millis() - lastRfidCheck < rfidCheckInterval) {
        return;
    }

    lastRfidCheck = millis();

    char message[32];

    message[32]= RFIDS.allRFID_Message(RFID_READERS,RFID_AMOUNT, RFID_DATABLOCK);
    
   // if (strlen(message) != length_message){
        //Serial.println(message);
        Brain.addToBuffer(message);  
        length_message = strlen(message);
    //}
}
#endif


#ifndef ledDisable
void ledReceive() {

    while (Brain.STB_.rcvdPtr != NULL) {
        LEDS.evaluateCmds(Brain);
        Brain.nextRcvdLn();
    }
}
#endif
