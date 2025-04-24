#include <Wire.h>
#include <BH1750.h>

// Concrete implementation of vector_sensor for BH1750 light sensor
class BH1750Sensor : public vector_sensor {
private:
    BH1750 lightMeter;

public:
    BH1750Sensor()
        : vector_sensor(
            1,                  // ID
            1750,               // Sensor type (custom type ID for BH1750)
            {"lux"},           // Register names (was "illuminance")
            "lux",             // Main register name (was "illuminance")
            "bh1750_sensor"    // Sensor name
        ) {}

    void init() override {
        Wire.begin();
        lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

        // Fill constant values into holder_registers (ID and type)
        vector_sensor::init();
    }

    void sensor_loop() override {
        float lux = lightMeter.readLightLevel();
        write_sensor_register("lux", static_cast<int32_t>(lux));
        fill_holder_registers(); // append register value to holder_registers
    }
};
