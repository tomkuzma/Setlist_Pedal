#ifndef __SDCARD_H__
#define __SDCARD_H__

#include <Arduino.h>
#include "FS.h"
#include "SD.h"

/////////////////// listDir /////////////////////////////////////////////////////////////////////////
// returns list of filenames as a string. Deals with hidden files created from Apple/Linux systems.
// Args:    fs      - FS object
//          dirname - file path
//          levels  - # of levels to look into 
// Returns: list of filenames as a string.
/////////////////////////////////////////////////////////////////////////////////////////////////////
String listDir(fs::FS &fs, const char *dirname, uint8_t levels);

/////////////////// scanFile /////////////////////////////////////////////////////////////////////////
// Scans text file and indexes the position location of all the lines in the file.
// Args:    fs      - FS object
//          path    - file path
// Returns: number of lines in text file. -1 if file read error. 
/////////////////////////////////////////////////////////////////////////////////////////////////////
int scanFile(fs::FS &fs, const char *path);

/////////////////// readLines ///////////////////////////////////////////////////////////////////////
// Goes to position keyed from lineNum arg and returns 3 lines as a string from file. 
// Args:    fs          - FS object
//          path        - file path
//          lineNum     - line in file to start read
//          lineCount   - total lines to deal with scrolling max
// Returns: String of desired 3 lines from text file
/////////////////////////////////////////////////////////////////////////////////////////////////////
String readLines(fs::FS &fs, int lineNum, int lineCount, const char *path);

#endif // __SDCARD_H__