#ifndef dmbsensora
#define dmbsensora

//#include <cstddef.hpp>
//#include <stddef.h>

// Base class vectorSensor without MQTT
class vector_sensor: public DProg{
protected:
    struct sensor_register_t {
        int32_t value; // Value of the input register
    };

    struct state_t {
        bool state; // State of the coil register (ON/OFF)
    };

    int device_id;                        // Device identifier
    std::vector<sensor_register_t> sensor_registers; // Vector of input registers
    std::vector<state_t> coils;            // Vector of coil registers
    int main_register_index=0;         // Index of the main register

public:
    // Constructor: Initializes the device with the given parameters
    vector_sensor(int id, int num_sensor_registers, int num_coils, int main_register_index)
        : device_id(id), sensor_registers(num_sensor_registers), coils(num_coils), main_register_index(main_register_index) {
         //register 1 - getAddress (id)
         //register 2 - sensor type
         sensor_registers[0].value=device_id;
         sensor_registers[1].value=UNDEF_SENSOR_TYPE;
        };

    void virtual sensor_loop()=0;

     void virtual init() {};

       // Returns the number of input registers
    size_t get_num_sensor_registers() const {
        return sensor_registers.size();
    };

    // Returns the number of coils
    size_t get_num_coils() const {
        return coils.size();
    };

    // Reads the value of an input register by its index
    uint16_t read_sensor_register(int index) {
        if (index < 0 || index >= sensor_registers.size()) {
            Serial.println("Error Read: Invalid input register index.");
            return 0;
        }
        return sensor_registers[index].value;
    };

    // Writes a value to an input register by its index
    void write_sensor_register(int index, uint16_t value) {
        if (index < 0 || index >= sensor_registers.size()) {
            Serial.println("Error Write: Invalid input register index.");
            return;
        }
        sensor_registers[index].value = value;
    };

    // Reads the state of a coil by its index
    bool read_coil(int index) {
        if (index < 0 || index >= coils.size()) {
            Serial.println("Error Read: Invalid coil index.");
            return false;
        }
        return coils[index].state;
    };

    // Writes a state to a coil by its index
    void write_coil(int index, bool state) {
        if (index < 0 || index >= coils.size()) {
            Serial.println("Error Write: Invalid coil index.");
            return;
        }
        coils[index].state = state;
    };

    // Returns the device identifier
    int get_device_id() const {
        return device_id;
    };


    // Returns the main register index
    int get_main_register_index() const {
        return main_register_index;
    };


    // Returns the value of the main register
    uint16_t get_value() const {
        if (main_register_index >= 0 && main_register_index < sensor_registers.size()) {
            return sensor_registers[main_register_index].value;
        } else {
            Serial.println("Error: Invalid main register index.");
            return 0;
        }
    };
};

#endif