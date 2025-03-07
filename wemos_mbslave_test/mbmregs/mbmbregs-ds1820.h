#ifndef __mbmbregsds1820__
#define __mbmbregsds1820__

#include <ModbusRTU.h>
#include "../dmbsensor/vsensora.h"
#include "mbmbregsa.h"


class modbus_regs_ds1820: public modbus_regs {
  private:
         
  protected:

  public:
   modbus_regs_ds1820(WMSettings * __s, ModbusRTU * __mb, vector_sensor* __vsensor): modbus_regs(__s,__mb,__vsensor) {};

//do static_cast for temperature ds1820 (minus), for main register with temp

   void virtual update_hold_regs(){
    
        for(int h_reg=0; h_reg< hregs_amount; h_reg++){
            
         
            if(h_reg ==_vsensor->get_main_register_index()){
                //debug("DS1820_REGS", "main_register="+String(_vsensor->get_main_register_index())+", vlue="+String(_vsensor->read_sensor_register(h_reg)));
                _mb->Hreg(h_reg, static_cast<uint16_t>(_vsensor->read_sensor_register(h_reg)));
                // _mb->Hreg(h_reg,_vsensor->read_sensor_register(h_reg));
            }else{
                _mb->Hreg(h_reg,_vsensor->read_sensor_register(h_reg));
            }        
        }
    
    };

   };


#endif