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

STB STB;
STB_BRAIN BRAIN;
STB_LED LEDS;


// for software SPI use (PN532_SCK, PN532_MISO, PN532_MOSI, RFID_SSPins[0])
#ifndef rfidDisable
    Adafruit_PN532 RFID_0(PN532_SCK, PN532_MISO, PN532_MOSI, RFID_1_SS_PIN);
    Adafruit_PN532 RFID_READERS[1] = {RFID_0};
    uint8_t data[16];
    unsigned long lastRfidCheck = millis();
#endif


char ledKeyword[] = "!LED";


void setup() {

    STB.begin();
    STB.rs485SetSlaveAddr(0);

    STB.dbgln(F("WDT endabled"));
    wdt_enable(WDTO_8S);
    wdt_reset();

    STB.i2cScanner();
    wdt_reset();
    STB.defaultOled.setScrollMode(SCROLL_MODE_AUTO);

    BRAIN.receiveFlags(STB);
    BRAIN.receiveSettings(STB);

#ifndef rfidDisable
    if (BRAIN.flags[rfidFlag]) {
        STB_RFID::RFIDInit(RFID_0);
        wdt_reset();
    }
#endif

#ifndef ledDisable
    if (BRAIN.flags[ledFlag]) {
        LEDS.ledInit(BRAIN.settings);
    }
#endif

    wdt_reset();

    STB.printSetupEnd();
}


void loop() {

    // if (Serial.available()) { Serial.write(Serial.read()); }

    #ifndef rfidDisable
    if (BRAIN.flags[rfidFlag]) {
        rfidRead();
    }
    #endif

    STB.rs485SlaveRespond();

    while (STB.rcvdPtr != NULL) {
        
        if (strncmp((char *) ledKeyword, STB.rcvdPtr, 4) == 0) {
            
            char *cmdPtr = strtok(STB.rcvdPtr, "_");
            cmdPtr = strtok(NULL, "_");

            int i = 0;
            int values[3] = {0,0,0};

            while (cmdPtr != NULL && i < 3) {
                // STB.dbgln(cmdPtr);
                sscanf(cmdPtr,"%d", &values[i]);
                //STB.dbgln(String(values[i]));
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
                STB.rs485SendAck();
                #endif
            }
            
        }
       
        STB.rs485RcvdNextLn();
    }

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

    STB.defaultOled.clear();
    STB.defaultOled.println(message);
    STB.rs485AddToBuffer(message);

    Serial.println(F("RFID end"));
    Serial.flush();
}
#endif
