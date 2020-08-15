#ifndef Functions_h
#define Functions_h
#include <Globalvariables.h>
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;

enum 
{LOG_DISABLED           =0, 
 LOG_INIT               =1, 
 LOG_ALL                =2, 
 LOG_TORQUE             =3,
 LOG_TORQUE_FILTERED    =4
};


extern void printHEX(uint8_t n, bool PAen, uint8_t SEP); //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '
extern void printHEXBT(uint8_t n, bool PAen, uint8_t SEP); //n= Value to print, PA = Preamble (0x), Seperator 0:disabled, 1:',', 2:' '


/******************** Serial Handling  **************************/
extern void printTimeBT(unsigned long time, bool dhh, bool dmm);
extern bool PrintHelpBT();
extern bool handleserialBT();

#endif