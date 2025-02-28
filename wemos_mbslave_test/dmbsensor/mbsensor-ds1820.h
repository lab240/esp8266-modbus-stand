#ifndef dmbsensords1820
#define dmbsensords1820
#include <Arduino.h>
#include <DallasTemperature.h>
#include "mbsensora.h"



class modbus_sensor_ds1820 : public modbus_sensor {

protected:

    long  NO_SENSOR_VAL = -12700;
    long NOT_READY_VAL = -12800;
    ulong  START_DELAY = 0;
    int NEED_ASKING = 0;
    int NEED_FILTERED = 0;
    int REQUEST_CIRCLE = 750;
    int MULTIPLIER = 1;
    int NEED_ASK_WHILE_WATING=0;
    int TYPE=0; //0 - default type of sensor


    OneWire oneWire;
    DallasTemperature ds_sensor;

public:
    // Constructor: Pass parameters to the base class constructor
    modbus_sensor_ds1820(int id) :modbus_sensor(id, 1, 0, 0) {
        // Seed the ds1820


    }

    void virtual init(String _name, int _type) {
      
      //ds_1820 needs asking before get results 
      NEED_ASKING = 1;
      REQUEST_CIRCLE = 750;
      TYPE=_type;

      //25.24C we store as 2524
      MULTIPLIER = 100;

      DeviceAddress tempDeviceAddress;
      ds_sensor.begin();
      ds_sensor.getAddress(tempDeviceAddress, 0);
      ds_sensor.setWaitForConversion(false);

      debug("DS1820", "DS1820::init ok");

      /***************/
      
    };

    // Override the sensor_loop to populate registers with ds1820

    void sensor_loop() override {
        
    };
        
};


#endif