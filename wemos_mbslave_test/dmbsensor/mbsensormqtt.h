#ifndef dmbsensormqtt
#define dmbsensormqtt

#include <PubSubClient.h>

// Derived class ModbusSensor with MQTT support and connection check
class modbus_sensor_mqtt : public modbus_sensor {
private:
    PubSubClient &client; // Reference to the MQTT client

    // Checks if the MQTT client is connected
    bool is_connected() {
        if (!client.connected()) {
            Serial.println("Error: MQTT broker not connected.");
            return false;
        }
        return true;
    }

public:
    // Constructor: Initializes the MQTT-enabled Modbus sensor
    modbus_sensor_mqtt(int id, int num_input_registers, int num_coils, int baud_rate, int port, int sensitive_register_index, PubSubClient &mqtt_client)
        : modbus_sensor(id, num_input_registers, num_coils, baud_rate, port, sensitive_register_index), client(mqtt_client) {}

    // Publishes the value of an input register to an MQTT topic
    void publish_input_register_to_mqtt(int index) {
        if (!is_connected()) return; // Check MQTT connection

        if (index < 0 || index >= input_registers.size()) {
            Serial.println("Error: Invalid input register index.");
            return;
        }

        uint16_t value = input_registers[index].value;

        // Generate MQTT topic based on device ID and input register index
        String topic = "device/" + String(device_id) + "/input_register/" + String(index);

        char msg[50];
        snprintf(msg, 50, "%u", value); // Convert the register value to a string

        if (client.publish(topic.c_str(), msg)) {
            Serial.println("Input register value published to MQTT: " + String(msg) + " on topic: " + topic);
        } else {
            Serial.println("Error publishing input register to MQTT.");
        }
    }

    // Publishes the state of a coil to an MQTT topic
    void publish_coil_to_mqtt(int index) {
        if (!is_connected()) return; // Check MQTT connection

        if (index < 0 || index >= coils.size()) {
            Serial.println("Error: Invalid coil index.");
            return;
        }

        bool state = coils[index].state;

        // Generate MQTT topic based on device ID and coil index
        String topic = "device/" + String(device_id) + "/coil/" + String(index);

        char msg[50];
        snprintf(msg, 50, "%s", state ? "ON" : "OFF"); // Convert the coil state to a string

        if (client.publish(topic.c_str(), msg)) {
            Serial.println("Coil state published to MQTT: " + String(msg) + " on topic: " + topic);
        } else {
            Serial.println("Error publishing coil state to MQTT.");
        }
    }
};

#endif