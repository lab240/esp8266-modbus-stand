#ifndef __mbmbregsbmp280__
#define __mbmbregsbmp280__

#include <ModbusRTU.h>
#include "../dmbsensor/vsensora.h"
#include "mbmbregsa.h"

#define DEBUG_MBREGS 0


class modbus_regs_bmp280: public modbus_regs {
  private:
         
  protected:

  public:
   modbus_regs_bmp280(WMSettings * __s, ModbusRTU * __mb, vector_sensor* __vsensor): modbus_regs(__s,__mb,__vsensor) {};

//do static_cast for temperature bmp280 (minus), for main register with temp

   void virtual update_hold_regs(){
    uint16_t high_word;
    uint16_t low_word;
    int32_t raw_value;


   //map to registers 0-address, 1-type, 2-multiplier, 3-state
        for(int h_reg=0; h_reg<=3; h_reg++){
             _mb->Hreg(h_reg,_vsensor->read_sensor_register(h_reg));
        }

        //if bad state
        if(_vsensor->read_sensor_register(3) == 0){
            for(int h_reg=4; h_reg < hregs_amount; h_reg++){
             _mb->Hreg(h_reg,static_cast<uint16_t>(_vsensor->read_sensor_register(h_reg)));
            }
            return;
        }

        //temperature (maybe minus)
        _mb->Hreg(4,static_cast<uint16_t>(_vsensor->read_sensor_register(4)));

        //pressure is too large, lets separate to 2 registers

        _mb->Hreg(5,_vsensor->read_sensor_register(5)); //MULTIPLIER for pressure 
        raw_value=_vsensor->read_sensor_register(6);    //Value of pressure

        high_word = (raw_value >> 16) & 0xFFFF;  // hibyte
        low_word  = raw_value & 0xFFFF;     

        _mb->Hreg(6,high_word);
        _mb->Hreg(7,low_word);

         //altitude is too large, lets separate to 2 registers
        _mb->Hreg(8,_vsensor->read_sensor_register(7)); //MULTIPLIER of Altitude
        raw_value=0;
        raw_value=_vsensor->read_sensor_register(8);    //value of altitude
        _mb->Hreg(9,raw_value);

        if(DEBUG_MBREGS) debug("MBP280REGS", "reg5="+String(_vsensor->read_sensor_register(5))+", reg6="+String(_vsensor->read_sensor_register(6)));
        
    };

   };


#endif