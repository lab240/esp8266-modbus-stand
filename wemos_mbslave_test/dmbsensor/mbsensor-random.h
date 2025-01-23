#ifndef dmbsensorrandom
#define dmbsensorrandom
#include <cstdlib> // For random numbers
#include <ctime>   // For seeding the random number generator
#include "mbsensora.h"

class modbus_sensor_random : public modbus_sensor {
public:
    // Constructor: Pass parameters to the base class constructor
    modbus_sensor_random(int id, int num_input_registers, int num_coils, int main_register_index)
        : modbus_sensor(id, num_input_registers, num_coils, main_register_index) {
        // Seed the random number generator
        srand(time(NULL));
    }

    // Override the sensor_loop to populate registers with random values
    void sensor_loop() override {
        // Update each input register with a random value
        for (size_t i = 0; i < input_registers.size(); ++i) {
            input_registers[i].value = rand() % 65536; // Random 16-bit value
        }
        // Update each coil with a random state (true/false)
        for (size_t i = 0; i < coils.size(); ++i) {
            coils[i].state = rand() % 2; // Random boolean value (0 or 1)
        }

    }
};


#endif