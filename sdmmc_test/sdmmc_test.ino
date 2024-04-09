#define DEBUG_ON
#define READ_BUF_SECTORS 8 // that many sectors per read operation
/*
Just for your information
Of what I have tested with some random 16GB card:

sectors per read  |  reading speed, MB/s
------------------+---------------------
        1         |        0.90
        2         |        1.57
        4         |        2.82
        8         |        5.00
       16         |        7.79
       32         |       11.12
       64         |       13.85
      128         |       15.75

 * SPI access is SIGNIFICANTLY SLOWER than SD_MMC (like 1/3 of SD_MMC speed), 
 * so 4-bit MMC is the only possible variant for playing back samples from SD_MMC.
 * Arduino ESP32 core libs are able of using 4-bit bus, and it's a good thing.
 * Thus D1 and D2 are mandatory to have a good speed
 * Be aware that the vast majority of micro-SD breakout boards are produced
 * with D1 and D2 NOT CONNECTED
 *
 * pin 1 - D2                |  Micro SD card     |
 * pin 2 - D3                |                   /
 * pin 3 - CMD               |                  |__
 * pin 4 - VDD (3.3V)        |                    |
 * pin 5 - CLK               | 8 7 6 5 4 3 2 1   /
 * pin 6 - VSS (GND)         | - - - - - - - -  /
 * pin 7 - D0                | - - - - - - - - |
 * pin 8 - D1                |_________________|
 *                             ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦
 *                     г=======- ¦ ¦ ¦ ¦ ¦ ¦ L=========¬
 *                     ¦         ¦ ¦ ¦ ¦ ¦ L======¬    ¦
 *                     ¦   г=====- ¦ ¦ ¦ L=====¬  ¦    ¦
 * Connections for     ¦   ¦   г===¦=¦=¦===¬   ¦  ¦    ¦
 * full-sized          ¦   ¦   ¦   г=- ¦   ¦   ¦  ¦    ¦
 * SD card             ¦   ¦   ¦   ¦   ¦   ¦   ¦  ¦    ¦
 * ESP32-S3 DevKit  | 21  47  GND  39 3V3 GND  40 41  42  |
 * ESP32-S3-USB-OTG | 38  37  GND  36 3V3 GND  35 34  33  |
 * ESP32            |  4   2  GND  14 3V3 GND  15 13  12  |
 * Pin name         | D1  D0  VSS CLK VDD VSS CMD D3  D2  |
 * SD pin number    |  8   7   6   5   4   3   2   1   9 /
 *                  |                                  =/
 *                  |__=___=___=___=___=___=___=___=___/
 * WARNING: ALL data pins must be pulled up to 3.3V with an external 10k Ohm resistor!
 * Note to ESP32 pin 2 (D0): Add a 1K Ohm pull-up resistor to 3.3V after flashing
 *
 * SD Card | ESP32
 *    D2       12
 *    D3       13
 *    CMD      15
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      14
 *    VSS      GND
 *    D0       2  (add 1K pull up after flashing)
 *    D1       4
 *
 *    For more info see file README.md in this library or on URL:
 *    https://github.com/espressif/arduino-esp32/tree/master/libraries/SD_MMC
 *    
 *  
 */
// debug macros
    #define DEB(...) USBSerial.print(__VA_ARGS__) 
    #define DEBF(...) USBSerial.printf(__VA_ARGS__) 
    #define DEBUG(...) USBSerial.println(__VA_ARGS__) 
/*
#ifdef DEBUG_ON
  #if  !defined(CONFIG_IDF_TARGET_ESP32S3) 
  #else
    #define DEB(...) Serial.print(__VA_ARGS__) 
    #define DEBF(...) Serial.printf(__VA_ARGS__) 
    #define DEBUG(...) Serial.println(__VA_ARGS__) 
  #endif
#else
  #define DEB(...)
  #define DEBF(...)
  #define DEBUG(...)
#endif
*/
#include <Arduino.h>
#include "sdmmc.h"
#include "sdmmc_file.h"

SDMMC_FAT32 Card;

SET_LOOP_TASK_STACK_SIZE(READ_BUF_SECTORS*BYTES_PER_SECTOR+10000); // if you want some more on stack during setup() or loop() instead of creating separate tasks

void setup() {
  btStop(); // we don't want bluetooth to consume our precious cpu time 



delay(500);
    USBSerial.begin(115200);  
delay(500);
/*
  #if !defined(CONFIG_IDF_TARGET_ESP32S3) 
    USBSerial.begin(115200);  
  #else
    Serial.begin(115200);  
  #endif
  */

  DEBUG(">>>Init SD_MMC Card");
  Card.begin();
  DEBUG("<<<SD_MMC Init done\r\n");
  
  DEBUG(">>>Testing card for random access sector-aligned reading speed");
  Card.testReadSpeed(READ_BUF_SECTORS /*sectors per read*/, 8 /*total MBytes*/);
  DEBUG("<<<Testing done\r\n");

  // List root folder
  Card.setCurrentDir("/");
  Card.printCurrentDir();

  DEBUG();

  // Scan root directory and list first available dir
  entry_t* entry;
  Card.rewindDir();
  entry = Card.nextEntry();
  while (!entry->is_end) {
    if (entry->is_dir) {
      Card.setCurrentDir(entry->name);
    //  Card.printCurrentDir();
      break;
    }
    entry = Card.nextEntry();
  }
delay(1000);


  Card.setCurrentDir("/");
  Card.setCurrentDir("/drums1/");
 // Card.printCurrentDir();
  SDMMC_FileReader Reader(&Card);

  str_max_t str="";
  Reader.open("sampler.ini"); 
  
  while (Reader.available()) {
    Reader.read_line(str);
    DEBUG(str.c_str());
  }
  Reader.close();

  Card.end();
}


void loop() {
  vTaskDelete(NULL);
}
