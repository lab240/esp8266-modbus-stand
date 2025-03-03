#ifndef dmbsensorrandom
#define dmbsensorrandom
#include <Arduino.h>
#include "mbsensora.h"

#include <ESP8266TrueRandom.h>



class modbus_sensor_random : public modbus_sensor {
public:
    // Constructor: Pass parameters to the base class constructor
    modbus_sensor_random(int id, int num_hold_registers, int num_coils, int main_register_index)
        : modbus_sensor(id, num_hold_registers, num_coils, main_register_index) {
        // Seed the random number generator
         randomSeed(millis());

        hold_registers[1].value=RANDOM_SENSOR_8H_10C; 

    }

    // Override the sensor_loop to populate registers with random values
    void sensor_loop() override {
         // Update each input register with a random value
        for (size_t i = 2; i < hold_registers.size(); ++i) {
            hold_registers[i].value =random(1,32000); // Random 16-bit value

            //We can library
            //hold_registers[i].value = ESP8266TrueRandom.random(32000); // Random 16-bit value
        }
        // Update each coil with a random state (true/false)
        for (size_t i = 0; i < coils.size(); ++i) {
            coils[i].state = rand() % 2; // Random boolean value (0 or 1)
        }

    }
};


#endif