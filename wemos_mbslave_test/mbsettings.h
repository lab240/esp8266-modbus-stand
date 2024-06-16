#ifndef __mbsettings__
#define __mbsettings__

#include "Arduino.h"
#include "donofflib/dsettings.h"

#define NUM_TRY 10   //waiting command pause

//default values
#define DEFAULT_ADDRESS 126           //default modbus address
#define DEFAULT_INT_REGS 10           //default hold regs
#define DEFAULT_COIL_REGS 10          //default coil regs

#define DEFAULT_MB_RATE 115200        // default baudrate
#define DEFAULT_MB_FC SERIAL_8E1      // dfault  serial settings

// MAX and MIN values
#define MAX_ID 127            // max modbus address
#define MAX_INT_REGS 255      
#define MIN_INT_REGS 4 

#define MAX_COIL_REGS 255
#define MIN_COIL_REGS 0

#define MAX_BAUDRATE 1500000
#define MIN_BAUDRATE 1200

#define MAX_SERIAL_VAR_NUM 62
//*****************************************************

#define SKIP_CHAR 'C'  //char ti skip waiting command

#endif