#ifndef donoffbase_h
#define donoffbase_h

#include "Arduino.h"
#include "../mbsettings.h"

enum debug_events{
  DTRACE,
  DTERROR,
  DTWARNING,
  DTINFO, 
  DTUNDEF,
};

#define DTOUT 1


class DBase {
  protected:
    
    WMSettings * _s;
    
    //enable print out on default (if Serial is used for data tranfer, disable it by desable_print_debug())
    int silent_mode=0; 

  public:
    int debug_level = 4;
  

    DBase(WMSettings * __s) {
      _s = __s;
    };

  void enable_silent(){silent_mode=1;};

  void disable_silent(){silent_mode=0;};

  uint is_silent(){return silent_mode;};

  template <typename T>
  void debug(String sourceStr, T debug,  int type=DTUNDEF, String preStr="", int strong_out_message=1) {
    //if silent mode, exit
    if(is_silent()) return;
    //if DSENTER, only print \n
    if(debug_level>0 && sourceStr==DSENTER){
      Serial.println("\n");
      return;
    }

    int let_out=0;

    if(strong_out_message || debug_level>0){
         Serial.print(">");
    }
    
    if (debug_level > 0){
       switch(type){
        case DTRACE:
          if(debug_level>2) Serial.print("TRACE");
          break;
        case DTERROR:
          if(debug_level>0) Serial.print("<!ERROR!>");
          let_out=1;
          break;
        case DTWARNING:
           if(debug_level>2) Serial.print("<WARNING>");
           let_out=1;
          break;
        case DTINFO: 
           if(debug_level >3) Serial.print("<INFO>");
           let_out=1;
          break;
        case DTUNDEF: 
          //if(debug_level>0) Serial.print("<UNDEF>");
          let_out=1;
          break;
       }
       
       if(let_out || strong_out_message){
     //type!=4 ? Serial.print("DEBUG:") : Serial.print("OUTPUT:");
          Serial.print(sourceStr); 
          Serial.print(":");
          if(preStr!="") Serial.print(preStr+":");
          Serial.print(debug);
          Serial.println();
        }
    }
  };


  template <typename T>
  void debug1(String sourceStr, T debug,  int type=DTUNDEF, int strong_out_message=1) {
    debug(sourceStr,debug,type=DTUNDEF,"",strong_out_message)=1;
  };

  void debug_old(String sourceStr, String debugStr) {
      String debug_outStr = "DEBUG:" + sourceStr + ":" + debugStr;
      if (debug_level > 0) Serial.println(debug_outStr);
    };

  String get_time_str(unsigned long val) {
      //debug("BASETIMESTR", String(val));
      String outStr;
      ulong wt = val / 1000;
      uint h = wt / 3600;
      wt = wt % 3600;
      uint m = wt / 60;
      uint s = wt % 60;
      if(h<10) outStr="0"+String(h); else outStr=String(h);
      outStr+=":";
      if(m<10) outStr+="0"+String(m); else outStr+=String(m);
      outStr+=":";
      if(s<10) outStr+="0"+String(s); else outStr+=String(s);
      
      return outStr;
    };

 String get_uptime_str(){
        return get_time_str(millis());
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

String virtual s_get_timestamp(char c_b='T', char c_e='Z'){
    String s_timestamp;
    time_t tnow = time(nullptr);
    struct tm * _t;
    _t=localtime(&tnow);
    s_timestamp=String(_t->tm_year+1900);
    s_timestamp+="-";
    s_timestamp+= _t->tm_mon+1<10? "0"+String(_t->tm_mon+1): String(_t->tm_mon+1);
    s_timestamp+="-";
    s_timestamp+= _t->tm_mday<10? "0"+String(_t->tm_mday): String(_t->tm_mday);
    s_timestamp+=c_b;
    s_timestamp+=_t->tm_hour <10? "0"+String(_t->tm_hour): String(_t->tm_hour);
    s_timestamp+=":";
    s_timestamp+= _t->tm_min<10? "0"+String(_t->tm_min): String(_t->tm_min);
    s_timestamp+=":";
    s_timestamp+=_t->tm_sec<10? "0"+String(_t->tm_sec): String(_t->tm_sec);
    s_timestamp+=c_e;
    //debug("TIMESTAMP", s_timestamp);
    return s_timestamp;
};

void virtual reset(){
  ESP.restart();
}

uint virtual d_hour(){
  time_t tnow = time(nullptr);
  struct tm * timeinfo;
  timeinfo=localtime(&tnow);
  return timeinfo->tm_hour;
  //timeinfo->tm_hour, 
  //timeinfo->tm_min, 
  //timeinfo->tm_sec, 
  //timeinfo->tm_mday, 
  //timeinfo->tm_mon, 
  //timeinfo->tm_year+1900);


};
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