#ifndef __TFT_H__
#define __TFT_H__

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"


/////////////////// printLines ///////////////////////////////////////////////////////////////////////
// Prints string to TFT 
// Args:    printString - string to pring
//          txtColour - TFT_eSPI colour constant
//          bgColour - TFT_eSPI colour constant
// Returns: none
/////////////////////////////////////////////////////////////////////////////////////////////////////
void printLines(String printString, int txtColour, int bgColour);

void tftSetup();


#endif // __TFT_H__