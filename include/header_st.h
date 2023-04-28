#pragma once

const unsigned long rfidCheckInterval = 250;

#define RFID_AMOUNT         1

// if you change one of these do also set the brainflags accordingly
// #define ledDisable 1
#define rfidDisable 1
// #define relayDisable 1

const uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define RFID_DATABLOCK      1

#define ledRowCnt 1
// per row
#define ledCnt 1 
#define slaveIndex 2

