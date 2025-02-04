#ifndef dmbsensora
#define dmbsensora

// Base class ModbusSensor without MQTT
class modbus_sensor {
protected:
    struct input_register_t {
        uint16_t value; // Value of the input register
    };

    struct coil_t {
        bool state; // State of the coil register (ON/OFF)
    };

    int device_id;                        // Device identifier
    std::vector<input_register_t> input_registers; // Vector of input registers
    std::vector<coil_t> coils;            // Vector of coil registers
    int main_register_index;         // Index of the main register

public:
    // Constructor: Initializes the device with the given parameters
    modbus_sensor(int id, int num_input_registers, int num_coils, int main_register_index)
        : device_id(id), input_registers(num_input_registers), coils(num_coils), main_register_index(main_register_index) {}

    void virtual sensor_loop(){}

       // Returns the number of input registers
    size_t get_num_input_registers() const {
        return input_registers.size();
    }

    // Returns the number of coils
    size_t get_num_coils() const {
        return coils.size();
    }

    // Reads the value of an input register by its index
    uint16_t read_input_register(int index) {
        if (index < 0 || index >= input_registers.size()) {
            Serial.println("Error Read: Invalid input register index.");
            return 0;
        }
        return input_registers[index].value;
    }

    // Writes a value to an input register by its index
    void write_input_register(int index, uint16_t value) {
        if (index < 0 || index >= input_registers.size()) {
            Serial.println("Error Write: Invalid input register index.");
            return;
        }
        input_registers[index].value = value;
    }

    // Reads the state of a coil by its index
    bool read_coil(int index) {
        if (index < 0 || index >= coils.size()) {
            Serial.println("Error Read: Invalid coil index.");
            return false;
        }
        return coils[index].state;
    }

    // Writes a state to a coil by its index
    void write_coil(int index, bool state) {
        if (index < 0 || index >= coils.size()) {
            Serial.println("Error Write: Invalid coil index.");
            return;
        }
        coils[index].state = state;
    }

    // Returns the device identifier
    int get_device_id() const {
        return device_id;
    }

    // Returns the value of the main register
    uint16_t get_value() const {
        if (main_register_index >= 0 && main_register_index < input_registers.size()) {
            return input_registers[main_register_index].value;
        } else {
            Serial.println("Error: Invalid main register index.");
            return 0;
        }
    }
};

#endif