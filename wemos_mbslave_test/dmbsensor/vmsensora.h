#ifndef vmsensora
#define vmsensora

#include <map>
#include <vector>
#include <Arduino.h>


#define NO_SENSOR_VALUE -12700
#define NO_SENSOR_DATA_VALUE   -12800
#define NO_SENSOR_STATE -1
#define NO_SENSOR_DATA_STATE  -2

enum sensor_types { 
 UNDEF_SENSOR_TYPE,
 RANDOM_SENSOR_8H_10C,
 TEMPERATURE_SENSOR,
 LIGTH_SENSOR,
 BMP280_SENSOR
 };


// A base class for sensors that store register values by name and expose them via MQTT topics.
class VmSensora : public DProg {
public:
    // Stores sensor register values by their name
    std::map<String, int32_t> sensor_registers;
    
    // Stores sensor multipliers values by their name
    std::map<String, uint16_t> register_multipliers;

    // Stores MQTT topics for each register
    std::map<String, String> mqtt_topics;

    // Stores the values of registers in a vector for batch processing
    std::vector<int16_t> holder_registers;

    // The name of the main register used for retrieving the primary sensor value
    String name_main_register;

    // The name of the sensor
    String sensor_name;

    // Device identifier
    int device_id;

    // Sensor type identifier
    int sensor_type;

    int init_ok=0;

public:
    // Constructor initializes registers, topics, and sensor metadata
    VmSensora(int id, int type, const std::vector<String>& register_names, const String& main_register_name, const String& sensor_name)
        : device_id(id), sensor_type(type), name_main_register(main_register_name), sensor_name(sensor_name) {

        for (const auto& name : register_names) {
            sensor_registers[name] = 0;
            mqtt_topics[name] = sensor_name + "-" + name;
            register_multipliers[name] = 1; // default multiplier is 1
        }
    }

    // Abstract function for sensor loop logic
    virtual void sensor_loop() = 0;

    // should return: 0 - ok, -1 - no sensor, -2 - no data
    virtual int no_sensor_check(float val) = 0;


    int cdebug(const String& source, const String& message) {
        if (silent_mode) return 0;
        //Serial.print("[");
        Serial.print(source);
        Serial.print(": ");
        Serial.println(message);
        return 1;
    }

    // Optional initialization hook
    virtual void init() {
        // Initialize holder_registers with constant values: id and type
        holder_registers.clear();
        holder_registers.push_back(static_cast<int16_t>(device_id));
        holder_registers.push_back(static_cast<int16_t>(sensor_type));

        init_ok=1;

    };

    // Set multiplier for converting float to int for a register
    void set_register_multiplier(const String& name, uint16_t multiplier) {
        register_multipliers[name] = multiplier;
    }

    // Read scaled (float) value from a register
    float read_scaled_sensor_register(const String& name) {
        if (sensor_registers.find(name) == sensor_registers.end()) return 0;
        uint16_t multiplier = register_multipliers[name];
        if (multiplier == 0) return 0;
        return static_cast<float>(sensor_registers[name]) / multiplier;
    }


    // Returns the number of defined sensor registers
    size_t get_num_sensor_registers() const {
        return sensor_registers.size();
    }

    // Alias for get_num_sensor_registers
    int get_register_count() const {
        return sensor_registers.size();
    }

    // Reads a register value by name, logs an error if not found
    int32_t read_sensor_register(const String& name) {
        if (sensor_registers.find(name) == sensor_registers.end()) {
            Serial.println("Error Read: Invalid input register name.");
            return 0;
        }
        return sensor_registers[name];
    }

    // Writes a value to a register by name and updates the MQTT topic
     void write_sensor_register(const String& name, float value) {
        if (sensor_registers.find(name) == sensor_registers.end()) {
            Serial.println("Error Write: Invalid input register name.");
            return;
        }

        int check = no_sensor_check(value);
        if (check == NO_SENSOR_STATE || init_ok==0) {
            sensor_registers[name] = NO_SENSOR_VALUE;
        } else if (check == NO_SENSOR_DATA_STATE) {
            sensor_registers[name] = NO_SENSOR_DATA_VALUE;
        } else {
            uint16_t multiplier = register_multipliers[name];
            sensor_registers[name] = static_cast<int32_t>(value * multiplier);
        }
    }


    // Retrieves the MQTT topic associated with a register
    String get_mqtt_topic(const String& name) {
        if (mqtt_topics.find(name) == mqtt_topics.end()) {
            Serial.println("Error: Invalid register name for MQTT topic.");
            return "";
        }
        return mqtt_topics[name];
    }

    // Updates all MQTT topics with the current register values
    void update_topics() {
        for (auto& topic : mqtt_topics) {
            update_topic(topic.first);
        }
    }

    // Updates a specific MQTT topic with the current value of its register
    void update_topic(const String& name) {
        if (sensor_registers.find(name) != sensor_registers.end()) {
            uint16_t multiplier = register_multipliers[name];
            if (multiplier == 1 || sensor_registers[name]==NO_SENSOR_VALUE || sensor_registers[name]==NO_SENSOR_DATA_VALUE ) {
                mqtt_topics[name] = String(sensor_registers[name]);
            } else {
                float scaled_value = read_scaled_sensor_register(name);
                mqtt_topics[name] = String(scaled_value, 2);  // two decimal places
            }
        }
    }




    // Fills the holder_registers vector with only dynamic register values
    void fill_holder_registers() {
        if (holder_registers.size() < 2) {
            holder_registers.resize(2);
        }
        holder_registers.resize(2);
        for (const auto& reg : sensor_registers) {
            const String& name = reg.first;
            int16_t mult = static_cast<int16_t>(register_multipliers[name]);
            int16_t val = static_cast<int16_t>(reg.second);
            holder_registers.push_back(mult);
            holder_registers.push_back(val);
        }
    }

    
    // Gets the device ID
    int get_id() const {
        return device_id;
    }

    // Sets the device ID
    void set_id(int id) {
        device_id = id;
    }

    // Gets the sensor type
    int get_type() const {
        return sensor_type;
    }

    // Sets the sensor type
    void set_type(int type) {
        sensor_type = type;
    }

    // Gets the sensor name
    String get_name() const {
        return sensor_name;
    }

    // Sets the sensor name and updates MQTT topics accordingly
    void set_name(const String& name) {
        sensor_name = name;
        update_topics();
    }

    // Gets the value from the main register; logs an error if not found
    int32_t get_value()  {
        if (sensor_registers.find(name_main_register) != sensor_registers.end()) {
            return sensor_registers[name_main_register];
        } else {
            Serial.println("Error: Invalid main register name.");
            return 0;
        }
    }

    void print_registers() {
        for (const auto& reg : sensor_registers) {
            cdebug("REG", reg.first + "=" + String(reg.second));
        }
    }

    void print_holdregisters() {
        String line = "";
        for (size_t i = 0; i < holder_registers.size(); ++i) {
            line += String(holder_registers[i]);
            if (i != holder_registers.size() - 1) line += ", ";
        }
        cdebug("HREG", line);
    }

    void print_mqtt() {
        for (const auto& topic : mqtt_topics) {
            cdebug("MQTT", topic.first + " -> " + topic.second);
        }
    }

};

#endif
