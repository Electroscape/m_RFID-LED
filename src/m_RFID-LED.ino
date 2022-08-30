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
// #define rfidDisable 1
// #define relayDisable 1

STB_BRAIN BRAIN;

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
    BRAIN.begin();
    BRAIN.setSlaveAddr(0);
    BRAIN.dbgln(F("WDT endabled"));
    wdt_enable(WDTO_8S);
    wdt_reset();

    Serial.println(F("ReceiveFlags")); Serial.flush();
    BRAIN.receiveFlags();
    Serial.println(F("ReceiveSettings")); Serial.flush();
    BRAIN.receiveSettings();
    
    
    /*
    BRAIN.flags[rfidFlag] = 1;

    // col 0 is the cmd type 0 is for setLedamount aka settingCmds::ledCount;
    BRAIN.settings[0][0] = settingCmds::ledCount;
    // col 1 is the PWM index
    BRAIN.settings[0][1] = 0;
    // col 2 is the amount of leds
    BRAIN.settings[0][2] = 3;
    BRAIN.flags[ledFlag] = 1;
    */



#ifndef rfidDisable
    if (BRAIN.flags[rfidFlag]) {
        STB_RFID::RFIDInit(RFID_0);
        wdt_reset();
    }
#endif

#ifndef ledDisable
    if (BRAIN.flags[ledFlag]) {
        LEDS.ledInit(BRAIN.settings);
        LEDS.setAllStripsToClr(LEDS.Strips[0].Color(75, 0, 0));
    }
#endif

    wdt_reset();

    BRAIN.STB_.printSetupEnd();
}


void loop() {

    #ifndef rfidDisable
    if (BRAIN.flags[rfidFlag]) {
        rfidRead();
    }
    #endif
    
    #ifndef ledDisable
    if (BRAIN.flags[ledFlag]) {
        ledReceive();
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

    BRAIN.oledClear();
    // STB.defaultOled.println(message);
    BRAIN.addToBuffer(message);

    Serial.println(F("RFID end"));
    Serial.flush();
}
#endif


#ifndef ledDisable
void ledReceive() {
    Serial.println("ledReceive");
    BRAIN.slaveRespond();

    while (BRAIN.STB_.rcvdPtr != NULL) {
        
        // strncmp((char *) ledKeyword, BRAIN.STB_.rcvdPtr, 4) == 0
        if (true) {
            
            char *cmdPtr = strtok(BRAIN.STB_.rcvdPtr, "_");
            cmdPtr = strtok(NULL, "_");

            int i = 0;
            int values[3] = {0,0,0};

            while (cmdPtr != NULL && i < 3) {
                // STB.dbgln(cmdPtr);
                sscanf(cmdPtr,"%d", &values[i]);
                // STB.dbgln(String(values[i]));
                cmdPtr = strtok(NULL, "_");
                i++;
            }

          
            if (i == 3) {
                // STB.dbgln("I == 2");
                #ifndef ledDisable
                // double check this since the led stripes for testing may not be identical
                if (BRAIN.flags[ledFlag]) {
                    long int setClr = LEDS.Strips[0].Color(values[0],values[2],values[1]);
                    LEDS.setAllStripsToClr(setClr);
                }
                BRAIN.sendAck();
                #endif
            }
            
        }
       
        BRAIN.nextRcvdLn();
    }
}
#endif