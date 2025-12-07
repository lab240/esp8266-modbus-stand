#ifndef vmsensorbh1750
#define vmsensorbh1750

#include <Wire.h>
#include <BH1750.h>
#include "vmsensora.h"

const char* BH1750_SENSOR_NAME="bh1750";
const char* BH1750_REG0_NAME="lux";

// Concrete implementation of vector_sensor for BH1750 light sensor
class VmSensorBH1750 : public VmSensora {
private:
    BH1750 lightMeter;

public:
    VmSensorBH1750(int id)
        : VmSensora(
            id,                  // ID
            LIGTH_SENSOR,       // Sensor type (type ID for BH1750)
            {BH1750_REG0_NAME},           // Register name
            BH1750_REG0_NAME,             // Main register name 
            String(BH1750_SENSOR_NAME) + "_"+ String(id)    // Sensor name
        ) {

            set_register_multiplier("lux", MULTIPLIER_LUX);
        }

    void init() override{
        Wire.begin();
        lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

        // Fill constant values into holder_registers (ID and type)
        VmSensora::init();
        init_ok=1;
    }

    void sensor_loop() override {
        float lux = lightMeter.readLightLevel();
        cdebug("RAW", String(lux));
        write_sensor_register("lux",lux);
        fill_holder_registers(); // append register value to holder_registers
        update_topics();
    }

/*
    int no_sensor_check(float val) override {
        // BH1750 returns < 0.0 if sensor is not responding or not connected
        if (val == -2.00) return NO_SENSOR_DATA_STATE;          // No data
        if (val == -1.00) return NO_SENSOR_STATE;   // no sensor
        return 0;                        // O
    }
*/
};



#endif
