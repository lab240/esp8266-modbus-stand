#ifndef vmsensorbmp280
#define vmsensorbmp280

#include <Adafruit_BMP280.h> 
#include "vmsensora.h"

const uint8_t BMP280_TYPE = 0x60;
const uint8_t BMP280_I2C_ADDRESS = 0x76;
const float APRESSURE=1013.25;


// Concrete implementation of vector_sensor for BMP280
class VmSensorBMP280 : public VmSensora {
private:
    Adafruit_BMP280 bmp;
public:
     VmSensorBMP280(int id, uint type=BMP280_TYPE)
        : VmSensora(id, BMP280_TYPE, {TEMP_REGISTER_NAME, PRESSURE_REGISTER_NAME}, TEMP_REGISTER_NAME, "bmp280"+String(BMP280_TYPE)) {

            set_register_multiplier(TEMP_REGISTER_NAME, MULTIPLIER_TEMP); // e.g., 23.45°C → 2345
            set_register_multiplier(PRESSURE_REGISTER_NAME, MULTIPLIER_PRESSURE);      // Pressure in Pascals (int)
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
        cdebug("INIT_OK", String(init_ok));
    }

    void sensor_loop() override {
        float temp = bmp.readTemperature();      // В °C
        float pressure = bmp.readPressure();     // В Па

        write_sensor_register(TEMP_REGISTER_NAME, temp);
        write_sensor_register("pressure", pressure);

        update_topics();
        fill_holder_registers();
    }

    int no_sensor_check(float val) override {
       if (val == 176 || val == 178 || val==177)
            return NO_SENSOR_DATA_STATE;
      return 0;        
    }

};

#endif
