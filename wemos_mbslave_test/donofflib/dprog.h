#ifndef donoffprog_h
#define donoffprog_h

#include "Arduino.h"


//sorce debug
#define DSCOMMAND "COMMNAD"
#define DSEEPROM "EEPROM"
#define DSMAIN "MAIN"
#define DSHELP "HELP"
#define DSWIFI "WIFI"
#define DSERROR "ERROR"

#define DSENTER "DENTER" // same Serial.println("\n")

//out type of debug
#define TERROR 1 //ошибки будут при любом дебаге
#define TOUT 4   //обязательные сообщения, будут при любом дебаге
#define TINFO 3  //информационныен сообщения, не будет при дебаг=0
#define TWARN 2  //предупреждения не будут при дебаг=0
#define TDEBUG 0

enum debug_events{
  DTRACE,
  DTERROR,
  DTWARNING,
  DTINFO, 
  DTUNDEF,
};

#define DTOUT 1

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

class DProg {
  protected:
   int silent_mode=0; 

  public:
    int debug_level = 4;
  
    DProg() {};

  void virtual enable_silent(){silent_mode=1;};

  void virtual disable_silent(){silent_mode=0;};

  uint is_silent(){return silent_mode;};



  template <typename T>
  uint debug(String sourceStr, T debug,  int type=DTUNDEF, String preStr="", int strong_out_message=1) {
    //if silent mode, exit
    if(is_silent()) return 0;
    //if DSENTER, only print \n
    if(debug_level>0 && sourceStr==DSENTER){
      Serial.println("\n");
      return 1;
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
    return 2;
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


  uint virtual d_hour(){
    time_t tnow = time(nullptr);
    struct tm * timeinfo;
    timeinfo=localtime(&tnow);
    //timeinfo->tm_hour, 
    //timeinfo->tm_min, 
    //timeinfo->tm_sec, 
    //timeinfo->tm_mday, 
    //timeinfo->tm_mon, 
    //timeinfo->tm_year+1900);
    return timeinfo->tm_hour;
  };


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

  int mywifi_try_to_connect(){
  int c = 0;
  IPAddress ip;
  while (WiFi.status() != WL_CONNECTED && c < 20) {
    delay(500);
    Serial.print(".");
    c++;
  }

  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WIFI_CONNECTED");
      ip = WiFi.localIP();
      Serial.print("This is my ip: ");
      Serial.println(ip);
      return 1;
  }else{
      debug(DSWIFI, "WIFI_NOT_CONNECTED");
      return 1;
  }
};


SerialConfig get_serial_setings_num(uint serial_settings_num){
  switch(serial_settings_num){
    case 38: return SERIAL_8E1;
    case 6: return SERIAL_8N1;
  }
  return SERIAL_5N1;
};

String get_serial_settings_string(uint serial_settings_num){
  String outStr="";
  //debug("SERIAL", "serial_num->"+String(serial_settings_num));
  switch(serial_settings_num){
    case 6: return "SERIAL_8N1";
    case 38: return "SERIAL_8E1";
  }
  return "UNKNOWN";
};


    
};



#endif