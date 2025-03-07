#ifndef __mbmbregsa__
#define __mbmbregsa__

#include <ModbusRTU.h>
#include "../dmbsensor/vsensora.h"


class modbus_regs: public DBase {
  private:
         
  protected:

   ModbusRTU * _mb;
   uint hregs_amount;
   uint cregs_amount;
   uint init_ok=0;
   vector_sensor* _vsensor=nullptr;
   

  public:
    modbus_regs(WMSettings * __s, ModbusRTU * __mb, vector_sensor* __vsensor): DBase(__s) {
        _mb=__mb;
        _vsensor=__vsensor;
    };

    void init() {

        hregs_amount=_vsensor->get_num_sensor_registers();
        cregs_amount=_vsensor->get_num_coils();

        debug("MBREGS", "Intit sensor-coil regs");

        init_sensor_regs();
        init_coil_regs();

        debug("MBRREGS", "sensor regs="+String(hregs_amount)+"; Coil regs="+String(cregs_amount));

        init_ok = 1;
    };

    void virtual loop(){
        update_regs();
    }

    void virtual update_hold_regs(){
        for(int h_reg=0; h_reg< hregs_amount; h_reg++){
            _mb->Hreg(h_reg,_vsensor->read_sensor_register(h_reg));
        }
    }
;
    void virtual update_coil_regs(){
        for(int c_reg=0; c_reg< cregs_amount; ++c_reg){
            _mb->Coil(c_reg,_vsensor->read_coil(c_reg));
        }
    };

    void virtual update_regs(){
       update_hold_regs();
       update_coil_regs();
    };

    void virtual print_sensor_regs(){
        String sregs="";
        for(int h_reg=0; h_reg< hregs_amount; h_reg++){
            sregs+="|"+String(h_reg)+"="+String(_vsensor->read_sensor_register(h_reg));
        }
        debug("MBREGS", "REGSH="+sregs);
    };

    void virtual init_sensor_regs(){
       
        for(int h_reg=0; h_reg<=hregs_amount; h_reg++){
            _mb->addHreg(h_reg); //add register
            _mb->Hreg(h_reg,0);  //add 0 to each reg
        }
        debug("MBREGS", "Iinit hregs->"+String(hregs_amount));
    };


    void virtual init_coil_regs(){
       
        for(int c_reg=0; c_reg<=cregs_amount; c_reg++){
            _mb->addCoil(c_reg); //add register
            _mb->Coil(c_reg,0);  //add 0 to each reg
        }
        debug("MBREGS", "Iinit coils->"+String(cregs_amount));
    };
};

#endif