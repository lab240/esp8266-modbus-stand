#ifndef dmbsensorBH1750
#define dmbsensorBH1750
#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

#include "mbsensora.h"

const uint8_t BH1750_I2C_ADDRESS = 0x23;

#define DEBUG_BH1750 0

class modbus_sensor_bh1750 : public modbus_sensor {

protected:

    uint init_ok=0;
  
//Sensor porops    

    long  NO_SENSOR_VAL = -100; //sensor get -1.00, with MULTIPLIER is -100
    uint  MULTIPLIER = 100;     //25.24C we store as 2524

//*****

// Solid sensor object - BH1750
    BH1750 lightMeter;
// ****

public:
    // Constructor: Pass parameters to the base class constructor
    //id, num_hold_registers, num_coils, main_register
    modbus_sensor_bh1750(int id) :modbus_sensor(id, 5, 0, 4) {
      
      //registers[0] is filled in parent constructor
      hold_registers[1].value=LIGTH_SENSOR;
      hold_registers[2].value=MULTIPLIER;
     
    }

    void virtual init() {
      
      if(DEBUG_BH1750) debug("BH1750", "BH1750 starting init");

      Wire.begin(BH1750_I2C_ADDRESS);
      lightMeter.begin();
      //ds_sensor->setWaitForConversion(false);

      if(DEBUG_BH1750)  debug("BH1750", "BH1750 sensor ok");

      init_ok=1;

      debug("BH1750", "init ok");

      /***************/
      
    };


    int virtual nosensor_val(long val){
      if (val == NO_SENSOR_VAL)
        return 1;
      return 0;
    };

    // Override the sensor_loop to populate registers with ds1820

    void sensor_loop() override {
      if(!init_ok) {
        //init didnt run
        if(DEBUG_BH1750)  debug("BH1750", "BH1750::init run not_ok, cant start sensor loop");
        return;
      }

      float lux = lightMeter.readLightLevel();
      long lux_l=lux*MULTIPLIER;

      //if minus, convering 
      uint16_t u_test_result=static_cast<uint16_t>(lux_l);

      hold_registers[4].value = u_test_result;
      if (nosensor_val(lux_l)) hold_registers[3].value=SENSOR_STATE_OK; else hold_registers[3].value=SENSOR_STATE_FAIL;
      if(DEBUG_BH1750)  debug("BH1750", "BH1750 signed_val="+String(lux_l)+", unsigned_val="+String(u_test_result)+", "+String(millis()));
    };
        
};


#endif