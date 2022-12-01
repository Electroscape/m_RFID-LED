/**
 * @file BREAKOUT.ino
 * @author Martin Pek (martin.pek@web.de)
 * @brief 
 * @version 1.6.2
 * @date 30.06.2022
 * build with lib_arduino v0.6.2
 */


#include "header_st.h"

#include <stb_common.h>
#include <avr/wdt.h>

#include <stb_rfid.h>
#include <stb_oled.h>
#include <stb_brain.h>
#include <stb_led.h>

// #define ledDisable 1
#define rfidDisable 1
// #define relayDisable 1

STB_BRAIN Brain;

#ifndef ledDisable 
    STB_LED LEDS;
#endif


// for software SPI use (PN532_SCK, PN532_MISO, PN532_MOSI, RFID_SSPins[0])
#ifndef rfidDisable
    Adafruit_PN532 RFID_0(PN532_SCK, PN532_MISO, PN532_MOSI, RFID_1_SS_PIN);
    Adafruit_PN532 RFID_READERS[1] = {RFID_0};
    uint8_t data[16];
    unsigned long lastRfidCheck = millis();
#endif


void setup() {
    Brain.begin();
    Brain.setSlaveAddr(3);
    // https://jeelabs.org/2011/11/09/fixing-the-arduinos-pwm-2/index.html
    // bitSet(TCCR1B, WGM12);
    Brain.dbgln(F("WDT endabled"));
    wdt_enable(WDTO_8S);
    wdt_reset();

    /*
    Serial.println(F("ReceiveFlags")); Serial.flush();
    Brain.receiveFlags();
    Serial.println(F("ReceiveSettings")); Serial.flush();
    Brain.receiveSettings();
    */

   // ledCount may aswell be one row 
    for (int i=0; i<4; i++) {
        // col 0 is the cmd type 0 is for setLedamount aka settingCmds::ledCount;
        Brain.settings[i][0] = settingCmds::ledCount;
        // col 1 is the PWM index
        Brain.settings[i][1] = i;
        // col 2 is the amount of leds
        Brain.settings[i][2] = 60;
    }

    Brain.settings[4][0] = settingCmds::ledClrOrder;
    Brain.settings[4][1] = NEO_BRG;
    Brain.settings[4][2] = NEO_BRG;
    Brain.settings[4][3] = NEO_BRG;
    Brain.settings[4][4] = NEO_BRG;

    Brain.flags = ledFlag;

    // Brain.receiveSetup();



#ifndef rfidDisable
    if (Brain.flags & rfidFlag) {
        STB_RFID::RFIDInit(RFID_0);
        wdt_reset();
    }
#endif

#ifndef ledDisable
    if (Brain.flags & ledFlag) {
        LEDS.ledInit(Brain.settings);
        
       /*  wdt_disable();
        while (true) {

            LEDS.setStripToClr(0, LEDS.Strips[0].Color(75, 0, 0));
            delay(1000);
            LEDS.setStripToClr(1, LEDS.Strips[0].Color(0, 75, 0));
            delay(1000);
            LEDS.setStripToClr(2, LEDS.Strips[0].Color(0, 0, 0));
            delay(1000);
            LEDS.setStripToClr(3, LEDS.Strips[0].Color(0, 0, 75));
            delay(1000);
            LEDS.setAllStripsToClr( LEDS.Strips[0].Color(25, 25, 25));
            delay(1000);
        } */
        
    }
#endif

    wdt_reset();

    Brain.STB_.printSetupEnd();
}

void loop() {
    //Serial.println(millis());
    #ifndef rfidDisable
    if (Brain.flags & rfidFlag) {
        rfidRead();
    }
    #endif
    
    if (Brain.flags & ledFlag && Brain.slaveRespond()) {
        Serial.println("slave got pushed");
        Serial.println(Brain.STB_.rcvdPtr);
        ledReceive();
    }
  
    LEDS.LEDloop(Brain);
    wdt_reset();
    
}


#ifndef rfidDisable
void rfidRead() {
    if (millis() - lastRfidCheck < rfidCheckInterval) {
        return;
    }

    lastRfidCheck = millis();
    char message[32] = "!RFID";

    Serial.println(F("RFID..."));
    Serial.flush();

    for (int readerNo = 0; readerNo < RFID_AMOUNT; readerNo++) {
        if (STB_RFID::cardRead(RFID_READERS[0], data, RFID_DATABLOCK)) {
            Serial.println(F("RFID read succees"));
            Serial.flush();
            strcat(message, "_");
            strcat(message, (char*) data);
        }
    }

    Brain.oledClear();
    // STB.defaultOled.println(message);
    Brain.addToBuffer(message);

    Serial.println(F("RFID end"));
    Serial.flush();
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