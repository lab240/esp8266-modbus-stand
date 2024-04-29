#ifndef __mbserial__
#define __mbserial__

#include "mbase.h"


#define NSERIAL_5N1 0x00
#define NSERIAL_6N1 0x02
#define NSERIAL_7N1 0x04
#define NSERIAL_8N1 0x06 //6
#define NSERIAL_5N2 0x08
#define NSERIAL_6N2 0x0A
#define NSERIAL_7N2 0x0C
#define NSERIAL_8N2 0x0E
#define NSERIAL_5E1 0x20
#define NSERIAL_6E1 0x22
#define NSERIAL_7E1 0x24
#define NSERIAL_8E1 0x26 //38
#define NSERIAL_5E2 0x28
#define NSERIAL_6E2 0x2A
#define NSERIAL_7E2 0x2C
#define NSERIAL_8E2 0x2E
#define NSERIAL_5O1 0x30
#define NSERIAL_6O1 0x32
#define NSERIAL_7O1 0x34
#define NSERIAL_8O1 0x36
#define NSERIAL_5O2 0x38
#define NSERIAL_6O2 0x3A
#define NSERIAL_7O2 0x3C
#define NSERIAL_8O2 0x3E //62

SerialConfig get_serial_sttings_from_num(uint serial_settings_num){
  switch(serial_settings_num){
    case 38: return SERIAL_8E1;
    case 6: return SERIAL_8N1;
  }
  return SERIAL_5N1;
};

String get_port_settings_string(uint serial_settings_num){
  String outStr="";
  //debug("SERIAL", "serial_num->"+String(serial_settings_num));
  switch(serial_settings_num){
    case 6: return "SERIAL_8N1";
    case 38: return "SERIAL_8E1";
  }
  return "UNKNOWN";
};



#endif