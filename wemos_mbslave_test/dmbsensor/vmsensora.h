#include <map>
#include <vector>
#include <Arduino.h>

// A base class for sensors that store register values by name and expose them via MQTT topics.
class vector_sensor : public DProg {
protected:
    // Stores sensor register values by their name
    std::map<String, int32_t> sensor_registers;

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

public:
    // Constructor initializes registers, topics, and sensor metadata
    vector_sensor(int id, int type, const std::vector<String>& register_names, const String& main_register_name, const String& sensor_name)
        : device_id(id), sensor_type(type), name_main_register(main_register_name), sensor_name(sensor_name) {

        for (const auto& name : register_names) {
            sensor_registers[name] = 0;
            mqtt_topics[name] = sensor_name + "/" + name;
        }
    }

    // Abstract function for sensor loop logic
    virtual void sensor_loop() = 0;

    // Optional initialization hook
    virtual void init() {
        // Initialize holder_registers with constant values: id and type
        holder_registers.clear();
        holder_registers.push_back(static_cast<int16_t>(device_id));
        holder_registers.push_back(static_cast<int16_t>(sensor_type));
    };

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
    void write_sensor_register(const String& name, int32_t value) {
        if (sensor_registers.find(name) == sensor_registers.end()) {
            Serial.println("Error Write: Invalid input register name.");
            return;
        }
        sensor_registers[name] = value;
        update_topic(name);
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
            mqtt_topics[name] = sensor_name + "/" + name + "=" + String(sensor_registers[name]);
        }
    }

    // Fills the holder_registers vector with only dynamic register values
    void fill_holder_registers() {
        // Keep the first two elements (id and type) intact
        if (holder_registers.size() < 2) {
            holder_registers.resize(2);
        }

        // Clear everything after the first two entries
        holder_registers.resize(2);

        // Add register values
        for (const auto& reg : sensor_registers) {
            holder_registers.push_back(static_cast<int16_t>(reg.second));
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
    int32_t get_value() const {
        if (sensor_registers.find(name_main_register) != sensor_registers.end()) {
            return sensor_registers[name_main_register];
        } else {
            Serial.println("Error: Invalid main register name.");
            return 0;
        }
    }
};
