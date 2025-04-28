#ifndef vmsensords1820
#define vmsensords1820

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>

#include "vmsensora.h"

const  uint8_t DS1820_PIN =  D2;

#define NO_DS1820_SENSOR_VAL -128
#define NO_DS1820_SENSOR_DATA_VAL  -127
#define DS1820_FAKE_VAL 8500

class VmSensorDS1820 : public VmSensora {
private:
    uint8_t DS_PIN;
    OneWire oneWire;
    DallasTemperature ds_sensor;
    DeviceAddress tempDeviceAddress;

public:
    VmSensorDS1820(int id, uint8_t pin = DS1820_PIN)
        : VmSensora(id, TEMPERATURE_SENSOR, {TEMP_REGISTER_NAME}, TEMP_REGISTER_NAME, "ds1820_" + String(pin)),
          DS_PIN(pin),
          oneWire(pin),
          ds_sensor(&oneWire) 
    {
        set_register_multiplier(TEMP_REGISTER_NAME, MULTIPLIER_TEMP); // e.g., 23.45°C → 2345
    }

    void init() override{

        VmSensora::init(); // Adds device ID and sensor type to holder_registers
        init_ok = 0;
        cdebug("DS1820", "DS1820 starting init");

        ds_sensor.begin();

        if (ds_sensor.getAddress(tempDeviceAddress, 0)) {
            ds_sensor.setWaitForConversion(false);
            init_ok = 1;
            cdebug("DS1820", "Sensor address found");
        } else {
            cdebug("DS1820", "Sensor address NOT found");
        }

        cdebug("INIT_OK", String(init_ok));
    }

    void sensor_loop() override {
        ds_sensor.requestTemperatures();

        float temp = ds_sensor.getTempC(tempDeviceAddress);
        write_sensor_register(TEMP_REGISTER_NAME, temp);

        update_topics();
        fill_holder_registers();
    }

    int no_sensor_check(float val) override {
        if (val == NO_DS1820_SENSOR_DATA_VAL || val == DS1820_FAKE_VAL)
            return NO_SENSOR_DATA_STATE;
        if (val == NO_DS1820_SENSOR_VAL)
            return NO_SENSOR_STATE;
        return 0;
    }
};

#endif
