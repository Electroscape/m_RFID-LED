#pragma once

const unsigned long rfidCheckInterval = 250;

#define RFID_AMOUNT         1

const uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define RFID_DATABLOCK      1

// --- LED settings --- 

/*
got moved into the library

#define STRIPE_CNT             1

// 140cm of 60 leds/m = 84 Leds to be safe bump it to 100
int ledCnts[STRIPE_CNT] = {100};
int ledPins[STRIPE_CNT] = {RFID_1_LED_PIN};
*/
