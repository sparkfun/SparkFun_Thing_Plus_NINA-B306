/*
  Written for NINA-B306 nRF52840
  Just test to see if we can read/write to an SD card over SPI
  
  This code is adapted from the SDFat Directory Functions example.
  https://github.com/greiman/SdFat/tree/master/examples
  DirectoryFunctions.ino

*/

#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"

// SD Card Chip Select
const uint8_t SD_CS_PIN = PIN_SD_CS;

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
//
// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(50)
//------------------------------------------------------------------------------

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_SCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_SCK)
#endif  // HAS_SDIO_CLASS

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
File root;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
File32 root;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
ExFile root;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
FsFile root;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

// Serial streams
ArduinoOutStream cout(Serial);

//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------
/* 
 *  void appendFile(fs::FS &fs, const char * path, const char * message) {
 *  void renameFile(fs::FS &fs, const char * path1, const char * path2) {
*/

void setup() {
  Serial.begin(9600);
  while(!Serial) {
    yield();
  }
  delay(1000);
  cout << F("Type any character to start\n");
  while(!Serial.available()) {
    yield();
  }

  // Initialize the SD Card.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  if(sd.exists("Folder1")
  || sd.exists("Folder1/file1.txt")
  || sd.exists("Folder1/File2.txt")) {
    error("Please remove existing Folder1, file1.txt, and File2.txt");
  }

  int rootFileCount = 0;
  if (!root.open("/")) {
    error("open root");
  }
  while (file.openNext(&root, O_RDONLY)) {
    if(!file.isHidden()) {
      rootFileCount++;
    }
    file.close();
    if(rootFileCount > 10) {
      error("Too many files in root. Please use an empty SD.");
    }
  }
  if (rootFileCount) {
    cout << F("\nPlease use an empty SD for best results.\n\n");
    delay(1000);
  }

  // Create a new folder.
  if (!sd.mkdir("Folder1")) {
    error("Create Folder1 failed");
  }
  cout << F("Created Folder1\n");

  // Create a file in Folder1 using a path.
  if (!file.open("Folder1/file1.txt", O_WRONLY | O_CREAT )) {
    error("create Folder1/file1.txt failed");
  }
  file.close();
  cout << F("Created Folder1/file1.txt\n");

  // Change volume working directory to Folder1.
  if (!sd.chdir("Folder1")) {
    error("chdir failed for Folder1.");
  }
  cout << F("chdir to Folder1\n");

  // Create File2.txt in current directory.
  if (!file.open("File2.txt", O_WRONLY | O_CREAT )) {
    error("create File2.txt failed.");
  }
  file.close();
  cout << F("Created File2.txt in current directory\n");

  cout << F("\nList of files on the SD.\n");
  sd.ls("/", LS_R);

  // Remove files from current directory.
  if (!sd.remove("file1.txt") || !sd.remove("File2.txt")) {
    error("remove files failed");
  }
  cout << F("\nfile1.txt and File2.txt removed.\n");

  // Change current directory to root.
  if (!sd.chdir()) {
    error("chdir to root failed.\n");
  }
  cout << F("\nList of files on the SD.\n");
  sd.ls(LS_R);

  // Remove Folder1
  if(!sd.rmdir("Folder1")){
    error("rmdir for Folder1 failed\n");
  }
  cout << F("\nFolder1 removed.\n");
  cout << F("\nList of files onthe SD.\n");
  sd.ls(LS_R);
  cout << F("Done!\n");
  
}

void loop() {

}
