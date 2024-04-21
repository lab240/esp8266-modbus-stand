#ifndef __mbase__
#define __mbase__

#include "Arduino.h"
#include "mbsettings.h"


#define MEM_SIZE 512
#define debug_level 1

#define DCOMMAND "COMMNAD"
#define DERROR "ERROR"
#define DEEPROM "EEPROM"
#define DMAIN "MAIN"
#define DENTER "DENTER" // same Serial.println("\n")

#define TERROR 1 //ошибки будут при любом дебаге
#define TOUT 4   //обязательные сообщения, будут при любом дебаге
#define TINFO 3  //информационныен сообщения, не будет при дебаг=0
#define TWARN 2  //предупреждения не будут при дебаг=0
#define TDEBUG 0

void eload(WMSettings* _s){
    EEPROM.begin(MEM_SIZE);
    EEPROM.get(0, *_s);
    EEPROM.end();
};

template <typename T>
void debug(String sourceStr, T debug,  int type=4, String preStr="") {
    //Serial.print("dl:"+String (debug_level));
    if(sourceStr==DENTER){
      Serial.println("\n");
      return;
    }
    if (debug_level > 0){
       switch(type){
        case 0:
          Serial.print("DEBUG");
          break;
        case 1:
          Serial.print("<!ERROR!>");
          break;
        case 2:
          Serial.print("<WARNING>");
          break;
        case 3: 
          Serial.print("<INFO>");
          break;
        case 4: 
          Serial.print("->");
          break;
      }
    
      }
      //type!=4 ? Serial.print("DEBUG:") : Serial.print("OUTPUT:");
      Serial.print(sourceStr); 
      Serial.print(":");
      if(preStr!="") Serial.print(preStr+"->");
      Serial.print(debug);
      Serial.println();
};


void esave(WMSettings* _s){
     debug("SAVE", "Lets' save");
     EEPROM.begin(MEM_SIZE);
     EEPROM.put(0, *_s);
     EEPROM.commit();
     EEPROM.end();     
};

#endif
