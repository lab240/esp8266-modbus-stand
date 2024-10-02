#ifndef donoffbase_h
#define donoffbase_h

#include <EEPROM.h>

#include "dprog.h"
#include "dsettings.h"


#define D_MEM_SIZE 512

class DBase: public DProg {
  protected:
    
    WMSettings * _s;
    
    //enable print out on default (if Serial is used for data tranfer, disable it by desable_print_debug())
  public:
    
    DBase(WMSettings * __s): DProg()  {
      _s = __s;
    };
    

void virtual save(){
     debug("SAVE", "Lets' save EEPROM");
     EEPROM.begin(D_MEM_SIZE);
     EEPROM.put(0, *_s);
     EEPROM.commit();
     EEPROM.end();     
};

void load(){
    EEPROM.begin(D_MEM_SIZE);
    EEPROM.get(0, *_s);
    EEPROM.end();
}


void virtual reset(){
  ESP.restart();
}


int set_settings_val_int(String _command, String _valStr, int *_setting_val, int _min, int _max, int autosave=1)
{
  int test_val;
  bool recognize = 0;
  if (_valStr.startsWith("0") && _valStr.length() == 1)
  {
    test_val = 0;
    recognize = 1;
  }
  test_val = _valStr.toInt();
  if (recognize == 0 && test_val != 0)
    recognize = 1;
  if (recognize == 0)
  {
    // publish_to_info_topic("E:not set, INT wait");
    debug(DSCOMMAND, "Failed convert address to int value", 1);
    return 0;
  }

  if (test_val >= _min && test_val <= _max)
  {
    *_setting_val = test_val;
    debug(DSCOMMAND, "Set new value ->" + String(*_setting_val));
    if(autosave) save();
    debug(DSEEPROM, "New settings saved to EEPROM");
    return 1;
  }

  // publish_to_info_topic(String("E:expected " + String(_min) + "<>" + String(_max)).c_str());
  debug(DSCOMMAND, "Wrong range min-max", 1);
  return 0;
};

int set_settings_val_bool(String _command, String _valStr, bool *_setting_val,  int autosave=1)
{
  bool recognize = 0;
  if (_valStr.startsWith("0") && _valStr.length() == 1)
  {
    *_setting_val = 0;
    recognize = 1;
  }
  if (_valStr.startsWith("1") && _valStr.length() == 1)
  {
    *_setting_val = 1;
    recognize = 1;
  }
  if (recognize)
  {
    // publish_to_info_topic(String("N:" + _command + "=" + String(*_setting_val)).c_str());
    debug(DSCOMMAND, "Set new value ->" + String(*_setting_val));
    if(autosave) save();
    debug(DSEEPROM, "New settings saved to EEPROM");
    return 1;
  }
  debug(DSCOMMAND, "Wrong range 0-1", 1);
  return 0;
};

int set_settings_val_str(String _command, String _valStr, char *_setting_val_char_array, int _max_len, int autosave=1)
{
  bool recognize = 0;

  if (_valStr.length() < _max_len)
  {
    strcpy(_setting_val_char_array, _valStr.c_str());
    recognize = 1;
  }
  if (recognize)
  {
    debug(DSCOMMAND, "Set new value ->" + String(_setting_val_char_array));
    if(autosave) save();
    debug(DSEEPROM, "New settings saved to EEPROM");
    return 1;
  }
  debug(DSCOMMAND, "Wrong range max len of string", 1);
  return 0;
};

};

#endif