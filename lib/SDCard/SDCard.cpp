#include "SDCard.h"

#define MAX_LINES 100   // max songs in setlist

static int lineLocation[MAX_LINES]; // array for keying location value of each line

/////////////////// listDir /////////////////////////////////////////////////////////////////////////
// returns list of filenames as a string. Deals with hidden files created from Apple/Linux systems.
// Args:    fs      - FS object
//          dirname - file path
//          levels  - # of levels to look into 
// Returns: list of filenames as a string.
/////////////////////////////////////////////////////////////////////////////////////////////////////
String listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    //Serial.printf("Listing directory: %s\n", dirname);

    // open root
    File root = fs.open(dirname);

    // error catch for invalid directory
    if (!root)
    {
        Serial.println("Failed to open directory");
        return "Failed to open directory";
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return "Not a directory";
    }

    // open next file or dir in directory
    File file = root.openNextFile();

    String fileName, fileList;
    while (file)
    {
            // Get filename
            fileName = file.name();

            // Ommit OSX directory hidden files
            if (!fileName.startsWith(".")) {
                fileList += (fileName + '\n'); // add filename to return string
            }
        // move to next file or dir
        file = root.openNextFile();
    }

    return fileList;
} // end listDir

/////////////////// scanFile /////////////////////////////////////////////////////////////////////////
// Scans text file and indexes the position location of all the lines in the file.
// Args:    fs      - FS object
//          path    - file path
// Returns: number of lines in text file. -1 if file read error. 
/////////////////////////////////////////////////////////////////////////////////////////////////////
int scanFile(fs::FS &fs, const char *path){

    // init char position in file and line count
    int position = 0;
    int lineCount = 0;
    memset(lineLocation, 0, MAX_LINES); // clear previous line location indexes

    File file = fs.open(path);  // open filepath
    
    // invalid filename catch. returns -1 on error.
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return -1;
    }

    // index positions of lines
    while (file.available())
    {
        if (file.read() == '\n')
        {
            lineCount++;
            lineLocation[lineCount] = position + 1;
        }
        position++;
    }
    
    file.close();   // close file

    Serial.printf("Line Count: %d\n", lineCount);

    return lineCount;
} // end scanFile

/////////////////// readLines ///////////////////////////////////////////////////////////////////////
// Goes to position keyed from lineNum arg and returns 3 lines as a string from file. 
// Args:    fs          - FS object
//          path        - file path
//          lineNum     - line in file to start read
//          lineCount   - total lines to deal with scrolling max
// Returns: String of desired 3 lines from text file
/////////////////////////////////////////////////////////////////////////////////////////////////////
String readLines(fs::FS &fs, int lineNum, int lineCount, const char *path)
{
    // open file
    File file = fs.open(path);

    // invalid filename catch.
    if (!file)
    {
        // Serial.println("Failed to open file for reading");   
        return "Failed to open file for reading";
    }

    String line = "";   // 

    // go to starting position of desired line
    file.seek(lineLocation[lineNum]);

    if (lineNum >= lineCount - 3) // if starting within last 3 lines
    {
        // read lines until EOF
        while (file.available())
        {
            line += (char)file.read();  // build return string
        }
            // Add End of File tag
            line.setCharAt(line.length(), '\0');
            line += "\nEND OF SETLIST";
    }
    else
    {
        // get 3 lines from starting position
        while (file.position() < lineLocation[lineNum + 3])
        {
            line += (char)file.read();  // build return string
        }
    }

    // print desired lines read from file
    // Serial.println(line);

    file.close();   // close file

    return line; 
}
