#pragma once

// taken from nrage's PakIO.h
// TODO: does not support actual pak functions right now
// only enough to pass key data over ReadController for netplay

// Raw data
// Byte 1 = Number of bytes to send
// Byte 2 = Number of bytes to receive
// Byte 3 = Command type

// Get status
#define RD_GETSTATUS        0x00
// Read button values
#define RD_READKEYS         0x01
// Read from controller pak
#define RD_READPAK          0x02
// Write to controller pak
#define RD_WRITEPAK         0x03
// Reset controller
#define RD_RESETCONTROLLER  0xff
// Read EEPROM
#define RD_READEEPROM       0x04
// Write EEPROM
#define RD_WRITEEPROM       0x05

// Codes for retrieving status
// 0x010300 - A1B2C3FF

// A1
// Default controller
#define RD_ABSOLUTE         0x01
#define RD_RELATIVE         0x02
// Default controller
#define RD_GAMEPAD          0x04

// B2
#define RD_EEPROM           0x80
#define RD_NOEEPROM         0x00

// C3
// No plugin in controller
#define RD_NOPLUGIN         0x00
// Plugin in controller (memory pak, rumble pak, etc.)
#define RD_PLUGIN           0x01
// Pak interface was uninitialized before the call
#define RD_NOTINITIALIZED   0x02
// Address of last pak I/O was invalid
#define RD_ADDRCRCERR       0x04
// EEPROM busy
#define RD_EEPROMBUSY       0x80

// The error values are as follows:
// 0x01ER00 - ........

// ER
// No error, operation successful
#define RD_OK               0x00
// Error, device not present for specified command
#define RD_ERROR            0x80
// Error, unable to send/receive the number of bytes for command type
#define RD_WRONGSIZE        0x40

// The address where rumble-commands are sent to
// This is really 0xC01B but our addressing code truncates the last several bits
#define PAK_IO_RUMBLE       0xC000

// 32KB memory pak
#define PAK_MEM_SIZE        32*1024
#define PAK_MEM_DEXOFFSET   0x1040