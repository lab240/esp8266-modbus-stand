#ifndef dmbsensorrandom
#define dmbsensorrandom
#include <Arduino.h>
#include "vsensora.h"

#include <ESP8266TrueRandom.h>



class vector_sensor_random : public vector_sensor {
public:
    // Constructor: Pass parameters to the base class constructor
    vector_sensor_random(int id, int num_sensor_registers, int num_coils, int main_register_index)
        : vector_sensor(id, num_sensor_registers, num_coils, main_register_index) {
        // Seed the random number generator
         randomSeed(millis());

        sensor_registers[1].value=RANDOM_SENSOR_8H_10C; 

    }

    // Override the sensor_loop to populate registers with random values
    void sensor_loop() override {
         // Update each input register with a random value
        for (size_t i = 2; i < sensor_registers.size(); ++i) {
            sensor_registers[i].value =random(1,32000); // Random 16-bit value

            //We can library
            //sensor_registers[i].value = ESP8266TrueRandom.random(32000); // Random 16-bit value
        }
        // Update each coil with a random state (true/false)
        for (size_t i = 0; i < coils.size(); ++i) {
            coils[i].state = rand() % 2; // Random boolean value (0 or 1)
        }

    }
};


#endif