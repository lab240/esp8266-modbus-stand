#ifndef vmsensorbmp280
#define vmsensorbmp280

#include <Adafruit_BMP280.h> 
#include "vmsensora.h"


const char* BMP280_SENSOR_NAME="bmp280";
const char* BMP280_REG0_TEMP_NAME="temp";
const char* BMP280_REG1_PRESSURE_NAME="pressure";


const uint8_t BMP280_TYPE = 0x60;
const uint8_t BMP280_I2C_ADDRESS = 0x76;
const float APRESSURE=1013.25;

// Concrete implementation of vector_sensor for BMP280
class VmSensorBMP280 : public VmSensora {
private:
    Adafruit_BMP280 bmp;
public:
     VmSensorBMP280(int id, uint type=BMP280_TYPE)
        : VmSensora(id, BMP280_TYPE, {BMP280_REG0_TEMP_NAME, BMP280_REG1_PRESSURE_NAME}, BMP280_REG0_TEMP_NAME, String(BMP280_SENSOR_NAME) + "_"+ String(id)) {

            set_register_multiplier(BMP280_REG0_TEMP_NAME, MULTIPLIER_TEMP); // e.g., 23.45°C → 2345
            set_register_multiplier(BMP280_REG1_PRESSURE_NAME, MULTIPLIER_PRESSURE);      // Pressure in Pascals (int)
        }

      void init() override{

        VmSensora::init(); // Adds device ID and sensor type to holder_registers

        init_ok=0;

        if (!bmp.begin(BMP280_I2C_ADDRESS, BMP280_TYPE)) {  
            cdebug("BMP280", "Sensor not found!");
            init_ok-=0;
        } else {
            init_ok = 1; // Initialization successful
        }
        init_ok = 1; //temprorary
        cdebug("INIT_OK", String(init_ok));
    }

    void sensor_loop() override {
        float temp = bmp.readTemperature();      // В °C
        float pressure = bmp.readPressure();     // В Па

        //cdebug("BMP280", "TEMP="+ String(temp)+"   PRESS="+String(pressure));
        if(isnan(temp)){
             write_sensor_register(BMP280_REG0_TEMP_NAME, NO_SENSOR_DATA_VALUE);  
             //cdebug("BMP280", "TEMP="+ String(temp)+"   PRESS="+String(pressure));
        }
        else write_sensor_register(BMP280_REG0_TEMP_NAME, temp);

        if(isnan(pressure)) write_sensor_register(BMP280_REG1_PRESSURE_NAME, NO_SENSOR_DATA_VALUE);  
        else write_sensor_register(BMP280_REG1_PRESSURE_NAME, pressure);

        update_topics();
        fill_holder_registers();
    }
/*
    int no_sensor_check(float val) override {
      if(isnan(val))return NO_SENSOR_DATA_STATE;
      return 0;        
    }
*/
};

#endif
