#ifndef vmsensorrandom
#define vmsensorrandom

#define SENSOR_NAME "rnd"

#include "vmsensora.h"

class VmSensorRandom : public VmSensora {
private:
    int num_registers;

public:
    VmSensorRandom(int id, int num_regs=10)
        : VmSensora(id, RANDOM_SENSOR_8H, {}, "reg0", SENSOR_NAME+ String("_") + String(id)), num_registers(num_regs)
    {
        // Dynamically create num_regs registers: reg0, reg1, ..., regN
        for (int i = 0; i < num_registers; ++i) {
            String regName = "reg" + String(i);
            sensor_registers[regName] = 0;
            mqtt_topics[regName] = "";
            register_multipliers[regName] = 1;
            cdebug("RANDOM_SENSOR","Topic="+mqtt_topics[regName]);
        }

        // Set the main register to be the first one (reg0)
        name_main_register = "reg0";
    }

     void init() override{

        VmSensora::init(); // Fills device_id and sensor_type
        init_ok = 1;
    }

    void sensor_loop() override {
        // Randomize all registers
        for (auto& reg : sensor_registers) {
            reg.second = random(0, 1000); // Random values between 0 and 999
        }

        update_topics();
        fill_holder_registers();
    }

    int no_sensor_check(float val) override {
        // No error checking needed for random data
        return 0;
    }
};


#endif