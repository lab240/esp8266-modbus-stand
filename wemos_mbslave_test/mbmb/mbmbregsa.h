#ifndef __mbmbregsa__
#define __mbmbregsa__

#include <ModbusRTU.h>
#include "../dmbsensor/mbsensora.h"


class MBRegsA: public DBase {
  private:
         
  protected:

   ModbusRTU * _mb;
   uint hregs_amount;
   uint cregs_amount;
   uint init_ok=0;
   modbus_sensor* _mbsensor=nullptr;
   

  public:
    MBRegsA(WMSettings * __s, ModbusRTU * __mb, modbus_sensor* __mbsensor): DBase(__s) {
        _mb=__mb;
        _mbsensor=__mbsensor;
    };

    void init() {

        hregs_amount=_mbsensor->get_num_input_registers()+1;
        cregs_amount=_mbsensor->get_num_coils()+1;

        init_hold_regs();
        init_coil_regs();

        init_ok = 1;
    };

    void virtual loop(){
        update_regs();
    }

    void virtual update_regs(){

        _mb->Hreg(0,_s-> mb_modbus_address);

        _mbsensor->sensor_loop();

        for(int h_reg=1; h_reg< hregs_amount; h_reg++){
            _mb->Hreg(h_reg,_mbsensor->read_input_register(h_reg));
        }

        for(int c_reg=1; c_reg< cregs_amount; c_reg++){
            _mb->Hreg(c_reg,_mbsensor->read_coil(c_reg));
        }

    };

    void virtual init_hold_regs(){
        for(int h_reg=0; h_reg<=hregs_amount; h_reg++){
            _mb->addHreg(h_reg); //add register
            _mb->Hreg(h_reg,0);  //add 0 to each reg
        }
    };


    void virtual init_coil_regs(){
        for(int c_reg=0; c_reg<=cregs_amount; c_reg++){
            _mb->addCoil(c_reg); //add register
            _mb->Coil(c_reg,0);  //add 0 to each reg
        }
    };
};

#endif