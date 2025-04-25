#ifndef vmsensorbmp280
#define vmsensorbmp280

#define MULTIPLIER_TEMP 100

#include <Adafruit_BMP280.h> 
#include "vmsensora.h"

// Concrete implementation of vector_sensor for BH1750 light sensor
class VmSensorBMP280 : public VmSensora {
private:
    Adafruit_BMP280 bmp;
public:
     VmSensorBMP280(int id)
        : VmSensora(id, 280, {"temperature", "pressure"}, "temperature", "bmp280") {

            set_register_multiplier("temperature", MULTIPLIER_TEMP); // e.g., 23.45°C → 2345
            set_register_multiplier("pressure", 1);      // Pressure in Pascals (int)
        }

    void init() override {

        VmSensora::init(); // Adds device ID and sensor type to holder_registers

        init_ok=0;

        if (!bmp.begin(0x76)) {  
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

        write_sensor_register("temperature", temp);
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
