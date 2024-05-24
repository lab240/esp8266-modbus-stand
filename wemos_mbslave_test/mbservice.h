#ifndef __service__
#define __service__

#include <ESP8266WiFi.h>       
#include "Arduino.h"
#include "mbase.h"
#include "mbcommands.h"
#include "mbserial.h"
#include "donofflib/dpublishmqtt.h"

void print_curr_settings(WMSettings *_s){
  debug(DSHELP, "Mdbus address->"+String(_s->custom_level1), TOUT);
  debug(DSHELP, "HOLD REGS->"+String(_s->custom_level2), TOUT);
  debug(DSHELP, "COIL REGS->"+String(_s->custom_level3), TOUT);
  debug(DSHELP, "BAUDRATE->"+String(_s->custom_level4), TOUT);
  debug(DSHELP, "PORT SERIAL NUM->"+String(_s->custom_level_notify1), TOUT);
  debug(DSHELP, "PORT SETTINGS->"+get_port_settings_string(_s->custom_level_notify1));
  struct station_config stationConf;
  wifi_station_get_config(&stationConf);
  debug(DSCOMMAND,"WIFI CREDS(ssid|pass)->" + String((char*)stationConf.ssid) +"|" + String((char*)stationConf.password)); 
  debug(DSHELP, "MQTT SERVER->"+String(_s->mqttServer));
  debug(DSHELP, "MQTT USER->"+String(_s->mqttUser));
  debug(DSHELP, "MQTT PASS->"+String(_s->mqttPass));

}

void print_welcome_help(){
  debug(DSHELP, "-------------- Avaliable commnads (wait for "+String(NUM_TRY)+" secs) -----------------");
  debug(DSHELP, String(CMD_SET_ADDRESS) + "=<ADDRESS> (1..127), " + String(CMD_SET_INT_REGS_AMOUNT) + "=<NUM_INT_REGS>, "+ String(CMD_SET_COIL_REGS_AMOUNT) + "=<NUM_COILS>");
  debug(DSHELP, "help - get full help");
}

void print_full_help(){
  debug(DSHELP, "-------------- Avaliable commnads (wait for "+String(NUM_TRY)+" secs) -----------------");
  debug(DSHELP, String(CMD_SET_ADDRESS) + "=<ADDRESS> (1..127), ");
  debug(DSHELP, String(CMD_SET_INT_REGS_AMOUNT) + "=<NUM_INT_REGS>");
  debug(DSHELP, String(CMD_SET_COIL_REGS_AMOUNT) + "=<NUM_COILS>");
  debug(DSHELP, String(CMD_SET_BAUDRATE) + "=<BAUDRATE> baudrate for modbus, possible values: 115200, 9600, 19200, 57600... or custom baudrate");
  debug(DSHELP, String(CMD_SET_PORT_SETTINGS) + "=<NUM_OF_SETTNGS> portsettings for modbus, possible values: \"=6\"->SERIAL_8N1, \"=38\"->SERIAL_8E1");
}

int set_settings_val_int(WMSettings *_s, String _command, String _valStr, int* _setting_val,  int _min =0, int _max=255) {
      int test_val;
      bool recognize = 0;
      if (_valStr.startsWith("0") && _valStr.length() == 1) {
        test_val = 0;
        recognize = 1;
      }
      test_val = _valStr.toInt();
      if (recognize == 0 && test_val != 0) recognize = 1;
      if (recognize == 0) {
        //publish_to_info_topic("E:not set, INT wait");
        debug(DSCOMMAND, "Failed convert address to int value",1);
        return 0;
      }

      if (test_val >= _min && test_val <= _max) {
        *_setting_val = test_val;
        debug(DSCOMMAND, "Set new value ->" +String(*_setting_val));
        esave(_s);
        debug(DSEEPROM, "New settings saved to EEPROM");        
        return 1;
      }

      //publish_to_info_topic(String("E:expected " + String(_min) + "<>" + String(_max)).c_str());
      debug(DSCOMMAND, "Wrong range min-max",1);
      return 0;

    };

  int set_settings_val_bool(WMSettings *_s, String _command, String _valStr, bool* _setting_val) {
      bool recognize = 0;
      if (_valStr.startsWith("0") && _valStr.length() == 1) {
        *_setting_val = 0;
        recognize = 1;
      }
      if (_valStr.startsWith("1") && _valStr.length() == 1) {
        *_setting_val = 1;
        recognize = 1;
      }
      if (recognize) {
        //publish_to_info_topic(String("N:" + _command + "=" + String(*_setting_val)).c_str());
        debug(DSCOMMAND, "Set new value ->" +String(*_setting_val));
        esave(_s);
        debug(DSEEPROM, "New settings saved to EEPROM");        
        return 1;
      }
      debug(DSCOMMAND, "Wrong range 0-1",1);
      return 0;
    };

  int set_settings_val_str(WMSettings *_s, String _command, String _valStr, char* _setting_val_char_array, int _max_len) {
      bool recognize = 0;
     
      if (_valStr.length() <_max_len) {
        strcpy(_setting_val_char_array,_valStr.c_str());
        recognize = 1;
      }
      if (recognize) {
        debug(DSCOMMAND, "Set new value ->" +String(_setting_val_char_array));
        esave(_s);
        debug(DSEEPROM, "New settings saved to EEPROM");        
        return 1;
      }
      debug(DSCOMMAND, "Wrong range max len of string",1);
      return 0;
    };





#endif