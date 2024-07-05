#ifndef __mbregs__
#define __mbregs__

#include <ModbusRTU.h>


#define R_ADDR 0
#define R_HOUR 1
#define R_MINS 2
#define R_SECS 3


void fill_hold_regs(ModbusRTU * _mb, int first_reg, int amount){
  
  for(int h_reg=first_reg; h_reg< amount; h_reg++){
      _mb->Hreg(h_reg,random(1,32000));
      //mb.Hreg(h_reg,ESP8266TrueRandom.random(32000));    //get too long answer, this is bad

      //_mb->Hreg(h_reg,5);
   }
   

}


void fill_coil_regs(ModbusRTU * _mb,int first_reg, int amount){
  for(int h_reg=first_reg; h_reg< amount; h_reg++){
      _mb->Coil(h_reg,random(0,1));
  }
}


void update_regs(ModbusRTU * _mb, int r_amount, int c_amount){
  uint32_t sec = millis() / 1000ul;      // полное количество секунд со старта платы
  uint16_t timeHours = (sec / 3600ul);        // часы
  uint16_t timeMins = (sec % 3600ul) / 60ul;  // минуты
  uint16_t timeSecs = (sec % 3600ul) % 60ul;  // секунды

  // заполняем реги значениями времени
  //_mb->Hreg(R_ADDR, modbus_address); 
  _mb->Hreg(R_HOUR, timeHours);
  _mb->Hreg(R_MINS, timeMins);
  _mb->Hreg(R_SECS, timeSecs);

  //next regs are random

  if(r_amount!=0) fill_hold_regs(_mb,R_SECS+1,r_amount-R_SECS);  

  if(c_amount!=0) fill_coil_regs(_mb, 0,c_amount); 

} 

void init_hold_regs(ModbusRTU * _mb, int first_reg, int amount){
  for(int h_reg=first_reg; h_reg<amount; h_reg++){
    _mb->addHreg(h_reg); //add register
    _mb->Hreg(h_reg,0);  //add 0 to each reg
  }
}


void init_coil_regs(ModbusRTU * _mb, int first_reg, int amount){
  for(int h_reg=first_reg; h_reg<amount; h_reg++){
    _mb->addCoil(h_reg); //add register
    _mb->Coil(h_reg,0);  //add 0 to each reg
  }
}

#endif