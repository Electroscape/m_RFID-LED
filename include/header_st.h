#pragma once

// -------------------- RFID --------------------

#define RFID_ENABLE

#define RFID_AMOUNT        3
#define RFID_DATABLOCK     1

const unsigned long rfidCheckInterval = 250;


// -------------------- LEDs --------------------

#define LED_ENABLE

#define LED_STRIP_COUNT          3
#define LEDS_PER_STRIP     3

#define LED_COLOR_ORDER    NEO_RGB


// -------------------- Brain --------------------

#define BRAIN_SLAVE_ADDR   0
#define BRAIN_FLAGS        (cmdFlags::ledFlag | cmdFlags::rfidFlag)