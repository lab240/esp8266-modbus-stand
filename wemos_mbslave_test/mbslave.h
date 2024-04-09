
#ifndef mbslave
#define mbslave 

#include "Arduino.h"
#include "mbase.h"


#define NUM_TRY 10

#define MAX_ID 127            // максимально допустимый адрес модбас



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

int do_command(WMSettings *_s, String cmdStr, String valStr){
  
  if (cmdStr == "seta"){ // проверяем первые 4 символа на соответствие команде 
            
      uint newAdress=0;
      if((newAdress=valStr.toInt()) == 0 ){
        debug(DCOMMAND, "Failed convert address to int value",1);
      }

      if (0 < newAdress && newAdress <= MAX_ID ){   // проверяем на соответствие диапазону 0...127
        _s->custom_level1=newAdress;
        debug(DCOMMAND, "Set new address ->" +String(newAdress));
        esave(_s);
        debug(DEEPROM, "New settings saved to EEPROM->"+String(_s->custom_level1));        
        return 0;             // подтверждаем запись
      }
      else{ 
        // не попали в диапазон - пишем ошибку
        debug(DCOMMAND, "Wrong address range !",1);    
        debug(DCOMMAND, "New address not selected or recognized, leave old address ->"+String(_s->custom_level1),4);
        return 2;
      }             
    }
    
    if (cmdStr == "setap"){
      debug(DCOMMAND, "Something do with->" + String(cmdStr));
      return 0;
    }
    
    if (cmdStr == "setp"){
       debug(DCOMMAND, "Something do with->" + String(cmdStr));
       return 0;
    }
    // если команды = не поступило
      debug(DCOMMAND, "Unknown command with = ->"+cmdStr+"="+valStr,TERROR);
      return 1;
  };

#endif

