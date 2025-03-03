#ifndef dmbsensords1820
#define dmbsensords1820
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#include "mbsensora.h"


#define DS1820_PIN D2

#define DEBUG_DS1820 0

class modbus_sensor_ds1820 : public modbus_sensor {

protected:

    uint init_ok=0;

//Sensor porops    

    uint  TYPE=TEMPERATURE_SENSOR;
    long  NO_SENSOR_VAL = -12700;
    long  NOT_READY_VAL = -12800;
    ulong START_DELAY = 0;
    uint  MULTIPLIER = 100;     //25.24C we store as 2524
    uint  FAKE_VAL=8500;

//*****

// Solid sensor object - DS1820
    DallasTemperature* ds_sensor;
    DeviceAddress tempDeviceAddress;
    OneWire* oneWire;
// ****

public:
    // Constructor: Pass parameters to the base class constructor
    //id, num_hold_registers, num_coils, main_register
    modbus_sensor_ds1820(int id) :modbus_sensor(id, 4, 0, 3) {
        // Seed the ds1820
    oneWire = new OneWire(DS1820_PIN);
    ds_sensor = new DallasTemperature(oneWire);

    }

    void virtual init() {
      
      if(DEBUG_DS1820) debug("DS1820", "DS1820 dallas starting init");

      ds_sensor->begin();
      ds_sensor->getAddress(tempDeviceAddress, 0);
      //ds_sensor->setWaitForConversion(false);

      debug("DS1820", "DS1820 dallas sensor ok");

      hold_registers[0].value=device_id;
      hold_registers[1].value=TYPE;

      init_ok=1;

      debug("DS1820", "init ok");

      /***************/
      
    };


    int virtual nosensor_val(long val){
      if (val == NO_SENSOR_VAL)
        return 1;
      return 0;
    };

     int virtual fake_val(long val){
      if (val == FAKE_VAL)
        return 1;
      return 0;
    };

    // Override the sensor_loop to populate registers with ds1820

    void sensor_loop() override {
      if(!init_ok) {
        //init didnt run
        if(DEBUG_DS1820)  debug("DS1820", "DS1820::init run not_ok, cant start sensor loop");
        return;
      }
      ds_sensor->requestTemperatures();
      int test_result=ds_sensor->getTempC(tempDeviceAddress)*MULTIPLIER;

      //if minus, convering 
      uint16_t u_test_result=static_cast<uint16_t>(test_result);

      hold_registers[3].value = u_test_result;
      if (nosensor_val(test_result) || fake_val(test_result)) hold_registers[2].value=SENSOR_STATE_OK; else hold_registers[2].value=SENSOR_STATE_FAIL;
      if(DEBUG_DS1820)  debug("DS1820", "DS1820 signed_val="+String(test_result)+", unsigned_val="+String(u_test_result)+", "+String(millis()));
    };
        
};


#endif