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


STB_BRAIN Brain;

#ifdef LED_ENABLE
    STB_LED LEDS;
#endif


#ifdef RFID_ENABLE

    uint8_t data[16];
    Adafruit_PN532 RFID_0(PN532_SCK, PN532_MISO, PN532_MOSI, RFID_1_SS_PIN);

    #if RFID_AMOUNT > 1
        Adafruit_PN532 RFID_1(PN532_SCK, PN532_MISO, PN532_MOSI, RFID_2_SS_PIN);
    #endif

    #if RFID_AMOUNT > 2
        Adafruit_PN532 RFID_2(PN532_SCK, PN532_MISO, PN532_MOSI, RFID_3_SS_PIN);
    #endif

    #if RFID_AMOUNT > 3
        Adafruit_PN532 RFID_3(PN532_SCK, PN532_MISO, PN532_MOSI, RFID_4_SS_PIN);
    #endif

    // hardcoded to 4 as the maximum of the hardware capability
    #if RFID_AMOUNT == 1

        Adafruit_PN532 RFID_READERS[1] = {
            RFID_0
        };

    #elif RFID_AMOUNT == 2

        Adafruit_PN532 RFID_READERS[2] = {
            RFID_0,
            RFID_1
        };

    #elif RFID_AMOUNT == 3

        Adafruit_PN532 RFID_READERS[3] = {
            RFID_0,
            RFID_1,
            RFID_2
        };

    #elif RFID_AMOUNT == 4

        Adafruit_PN532 RFID_READERS[4] = {
            RFID_0,
            RFID_1,
            RFID_2,
            RFID_3
        };

    #else
        #error "RFID_AMOUNT must be between 1 and 4"
    #endif

    STB_RFID RFIDS;
    unsigned long lastRfidCheck = millis();
#endif


void setup() {
    Brain.begin();
    Brain.setSlaveAddr(BRAIN_SLAVE_ADDR);
    Brain.dbgln(F("WDT endabled"));
    wdt_enable(WDTO_8S);
    wdt_reset();
    Brain.flags = BRAIN_FLAGS;

    #ifdef LED_ENABLE

        for (int i = 0; i < LED_STRIP_COUNT; i++) {
            Brain.settings[i][0] = settingCmds::ledCount;
            Brain.settings[i][1] = i;
            Brain.settings[i][2] = LEDS_PER_STRIP;
        }

        Brain.settings[LED_STRIP_COUNT][0] = settingCmds::ledClrOrder;
        for (int i = 1; i <= LED_STRIP_COUNT; i++) {
            // i think ledStripcount is used because the prior slots are used up by teh led config, this is a row of all the clr orders ... technically not yet individually configurable as of yet
            Brain.settings[LED_STRIP_COUNT][i] = LED_COLOR_ORDER;
        }
        

        if (Brain.flags & ledFlag) {
            LEDS.ledInit(Brain.settings);        
            LEDS.setAllStripsToClr(LEDS.Strips[0].Color(255, 0, 0));
            delay(1000);
            LEDS.setAllStripsToClr(LEDS.Strips[0].Color(0, 255, 0));
            delay(1000);
            LEDS.setAllStripsToClr(LEDS.Strips[0].Color(0, 0, 255));
            delay(1000);
            LEDS.setAllStripsToClr(LEDS.Strips[0].Color(0, 0, 0));
            wdt_disable();
        //Serial.println(F("Color Test finished"));        
        }

    #endif


    #ifndef rfidDisable
        if (Brain.flags & rfidFlag) {
            for (int i = 0; i < RFID_AMOUNT; i++) {
                STB_RFID::RFIDInit(RFID_READERS[i]);
            }
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