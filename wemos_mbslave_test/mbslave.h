#ifndef __mbslave__
#define __mbslave__

#include "Arduino.h"
#include "mbase.h"
#include "commands.h"


#define NUM_TRY 10

#define MODE_SETUP 1

#define MAX_ID 127            // максимально допустимый адрес модбас
#define MAX_INT_REGS 255
#define MAX_COIL_REGS 255


String get_command_str(){
  int is_cmd=0;
  //int is_cmd_l=0;
  int count_try=0;
  uint32_t softTimer = 0; 
  char inCommand[50] = {0};    // заводим массив для захвата команды задания адреса
  uint8_t counterByte = 0;      // указатель для записи команды в массив

  while(is_cmd==0 && count_try<NUM_TRY){  // цикл таймаута для задания адреса

    Serial.print(count_try,DEC);
    Serial.print("...\t");  // печатаем секунды таймаута

    softTimer = millis() + 1000;                  // назначаем таймер на 1с

    int incoming_flg=0;
    while(softTimer > (millis())){                // крутимся в цикле пока время меньше заданного таймаута
      if (Serial.available() > 0) {               // если доступны данные из порта
        inCommand[counterByte]=Serial.read();      // принимаем в массив по указателю
        if(counterByte < 520) counterByte++;          // но не более 20 значений от 0 
        else counterByte = 0;
        incoming_flg=1;                      // иначе сбрасываем счетчик
      }
    }

    if(incoming_flg) is_cmd=1;
    count_try++;
  }
  return String(inCommand);
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
        debug(DCOMMAND, "Failed convert address to int value",1);
        return 0;
      }

      if (test_val >= _min && test_val <= _max) {
        *_setting_val = test_val;
        debug(DCOMMAND, "Set new value ->" +String(*_setting_val));
        esave(_s);
        debug(DEEPROM, "New settings saved to EEPROM");        
        return 1;
      }

      //publish_to_info_topic(String("E:expected " + String(_min) + "<>" + String(_max)).c_str());
      debug(DCOMMAND, "Wrong range min-max",1);
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
        debug(DCOMMAND, "Set new value ->" +String(*_setting_val));
        esave(_s);
        debug(DEEPROM, "New settings saved to EEPROM");        
        return 1;
      }
      debug(DCOMMAND, "Wrong range 0-1",1);
      return 0;
    };

  int set_settings_val_str(WMSettings *_s, String _command, String _valStr, char const* _setting_val_char_array, int _max_len) {
      bool recognize = 0;
     
      if (strlen(_setting_val_char_array)<_max_len) {
        _setting_val_char_array = _valStr.c_str();
        recognize = 1;
      }
      if (recognize) {
        debug(DCOMMAND, "Set new value ->" +String(_setting_val_char_array));
        esave(_s);
        debug(DEEPROM, "New settings saved to EEPROM");        
        return 1;
      }
      debug(DCOMMAND, "Wrong range max len of string",1);
      return 0;
    };



int do_command(WMSettings *_s, String cmdStr, String valStr){
  
  if (cmdStr == CMD_SET_ADDRESS)
      if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->custom_level1, 0,MAX_ID)) return 1;
  
  if (cmdStr == CMD_SET_INT_REGS_AMOUNT)
      if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->custom_level2, 4,MAX_INT_REGS)) return 1;
  
  if (cmdStr == CMD_SET_COIL_REGS_AMOUNT)
      if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->custom_level3, 0,MAX_COIL_REGS)) return 1;
  return 0;

};

void print_curr_settings(WMSettings *_s){
  debug(DHELP, "Mdbus address->"+String(_s->custom_level1), TOUT);
  debug(DHELP, "HOLD REGS->"+String(_s->custom_level2), TOUT);
  debug(DHELP, "COIL REGS->"+String(_s->custom_level3), TOUT);
}

void print_welcome_help(){
  debug(DHELP, "-------------- Avaliable commnads (wait for "+String(NUM_TRY)+" secs) -----------------");
  debug(DHELP, String(CMD_SET_ADDRESS) + "=<ADDRESS> (1..127), " + String(CMD_SET_INT_REGS_AMOUNT) + "=<NUM_INT_REGS>, "+ String(CMD_SET_COIL_REGS_AMOUNT) + "=<NUM_COILS>");
  debug(DHELP, "help - get full help");
}

void print_full_help(){
  debug(DHELP, "-------------- Avaliable commnads (wait for "+String(NUM_TRY)+" secs) -----------------");
  debug(DHELP, String(CMD_SET_ADDRESS) + "=<ADDRESS> (1..127), ");
  debug(DHELP, String(CMD_SET_INT_REGS_AMOUNT) + "=<NUM_INT_REGS>");
  debug(DHELP, String(CMD_SET_COIL_REGS_AMOUNT) + "=<NUM_COILS>");
  debug(DHELP, String(CMD_SET_BAUDRATE) + "=<BAUDRATE> baudrate for modbus, possible values: 115200, 9600, 19200, 57600... or custom baudrate");
  debug(DHELP, String(CMD_SET_PORT_SETTINGS) + "=<NUM_OF_SETTNGS> portsettings for modbus, possible values: \"=6\"->SERIAL_8N1, \"=38\"->SERIAL_8E1");
}
#endif

