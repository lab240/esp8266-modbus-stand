#ifndef donoffbase_h
#define donoffbase_h

#include "Arduino.h"
#include "../mbsettings.h"

enum debug_events{
  DTUNDEF,
  DTERROR,
  DTWARNING,
  DTINFO,
  DTOUT
};


class DBase {
  protected:
    
    WMSettings * _s;
  public:
    int debug_level = 1;
    DBase(WMSettings * __s) {
      _s = __s;
    };


  template <typename T>
  void debug(String sourceStr, T debug,  int type=0, String preStr="") {
    //Serial.print("dl:"+String (debug_level));
    if (debug_level > 0){
       switch(type){
        case 0:
          Serial.print("<UDEF>");
          break;
        case 1:
          Serial.print("<ERROR>");
          break;
        case 2:
          Serial.print("<WARNING>");
          break;
        case 3: 
          Serial.print("<INFO>");
          break;
      }
      Serial.print("DEBUG:");
      Serial.print(sourceStr); 
      Serial.print(":");
      if(preStr!="") Serial.print(preStr+"->");
      Serial.print(debug);
     
      Serial.println();
      
     
    }
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
     debug("SAVE", "Lets' save");
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


};

#endif