#ifndef vmsensorbh1750
#define vmsensorbh1750

#define MULTIPLIER_LUX 100

#include <Wire.h>
#include <BH1750.h>
#include "vmsensora.h"

// Concrete implementation of vector_sensor for BH1750 light sensor
class VmSensorBH1750 : public VmSensora {
private:
    BH1750 lightMeter;

public:
    VmSensorBH1750(int id)
        : VmSensora(
            id,                  // ID
            LIGTH_SENSOR,       // Sensor type (type ID for BH1750)
            {"lux"},           // Register name
            "lux",             // Main register name 
            "bh1750"    // Sensor name
        ) {

            set_register_multiplier("lux", MULTIPLIER_LUX);
        }

    void init() override {
        Wire.begin();
        lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

        // Fill constant values into holder_registers (ID and type)
        VmSensora::init();
    }

    void sensor_loop() override {
        float lux = lightMeter.readLightLevel();
        cdebug("RAW", String(lux));
        write_sensor_register("lux",lux);
        fill_holder_registers(); // append register value to holder_registers
        update_topics();
    }
};

#endif
