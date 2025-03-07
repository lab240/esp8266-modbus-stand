#ifndef dmbsensords1820b
#define dmbsensords1820
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#include "vsensora.h"


#define DS1820_PIN D2

#define DEBUG_DS1820 0

class vector_sensor_ds1820 : public vector_sensor {

protected:

    uint init_ok=0;

//Sensor porops    

    long  NO_SENSOR_VAL = -12700;
    long  NOT_READY_VAL = -12800;
    ulong START_DELAY = 0;
    uint  MULTIPLIER = 100;     //25.24C we store as 2524
    uint  FAKE_VAL=8500;
    uint  vector_index=1;

//*****

// Solid sensor object - DS1820
    DallasTemperature* ds_sensor;
    DeviceAddress tempDeviceAddress;
    OneWire* oneWire;
// ****

public:
    // Constructor: Pass parameters to the base class constructor
    //id, num_sensor_registers, num_coils, main_register
    vector_sensor_ds1820(int id) :vector_sensor(id, 5, 0, 4) {
        // Seed the ds1820
    
    sensor_registers[1].value=TEMPERATURE_SENSOR;
    sensor_registers[2].value=MULTIPLIER;

    oneWire = new OneWire(DS1820_PIN);
    ds_sensor = new DallasTemperature(oneWire);

    }

    void virtual init() {
      
      if(DEBUG_DS1820) debug("DS1820", "DS1820 dallas starting init");

      ds_sensor->begin();
      ds_sensor->getAddress(tempDeviceAddress, 0);
      //ds_sensor->setWaitForConversion(false);

      debug("DS1820", "Sensor ok, main_register_index="+String(main_register_index));

      init_ok=1;

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
      int raw_result=ds_sensor->getTempC(tempDeviceAddress)*MULTIPLIER;

      sensor_registers[4].value = raw_result;
      if (nosensor_val(raw_result) || fake_val(raw_result)) sensor_registers[3].value=SENSOR_STATE_FAIL; else sensor_registers[3].value=SENSOR_STATE_OK;
      if(DEBUG_DS1820) debug("DS1820", "val="+String(raw_result)+", "+String(millis()));
    };
        
};


#endif