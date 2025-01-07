#ifndef __mbmbregsa__
#define __mbmbregsa__

#include <ModbusRTU.h>


class MBRegsa: public DBase {
  private:
         
  protected:

   ModbusRTU * _mb;
   uint hregs_amount;
   uint cregs_amount;
   uint init_ok=0;
   

  public:
    MBRegsa(WMSettings * __s, ModbusRTU * __mb, uint __hregsa, uint __cregsa): DBase(__s) {
        _mb=__mb;
        hregs_amount=__hregsa;
        cregs_amount=__cregsa;
    };

    void init() {
        init_hold_regs(0,hregs_amount);
        init_coil_regs(0,cregs_amount);
        init_ok = 1;
    };

    void virtual update_regs()=0;

    void virtual init_hold_regs(int first_reg, int amount)=0;
 
    void virtual init_coil_regs(int first_reg, int amount)=0;


    void virtual fill_hold_regs(int first_reg, int amount)=0;
        
    void virtual fill_coil_regs(int first_reg, int amount)=0;
};

#endif