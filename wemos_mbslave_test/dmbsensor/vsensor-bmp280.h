#ifndef dmbsensorBMP280
#define dmbsensorBMP280
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#include "vsensora.h"

const uint8_t BMP280_I2C_ADDRESS = 0x76;

#define DEBUG_BMP280 0

class vector_sensor_bmp280 : public vector_sensor {

protected:

    uint init_ok=0;
  
//Sensor porops    
    long  NO_SENSOR_VAL = -12700;
    uint  MULTIPLIER = 100;     //25.24C we store as 2524

//*****

// Solid sensor object - BMP280
    Adafruit_BMP280 bmp; 
// ****

public:
    // Constructor: Pass parameters to the base class constructor
    //id, num_sensor_registers, num_coils, main_register
    vector_sensor_bmp280(int id) :vector_sensor(id, 9, 0, 5) {
      
      //registers[0] is filled in parent constructor
      sensor_registers[1].value=BMP280_SENSOR;
      sensor_registers[2].value=MULTIPLIER;
     
    }

    void virtual init() {
    
      if(DEBUG_BMP280) debug("BMP280", "BMP280 starting init");

      uint status = bmp.begin(BMP280_I2C_ADDRESS);

       if (!status) 
        if(DEBUG_BMP280) debug("BMP280", "!!! Could not find a valid BMP280 sensor");

      bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

      if(DEBUG_BMP280)  debug("BMP280", "BMP280 sensor ok");

      init_ok=1;

      debug("BMP280", "init ok");

      /***************/
      
    };


    // Override the sensor_loop to populate registers with ds1820

    void sensor_loop() override {
      uint16_t high_word;
      uint16_t low_word;

      if(!init_ok) {
        //init didnt run
        if(DEBUG_BMP280)  debug("BMP280", "init run not_ok, cant start sensor loop");
        return;
      }

      
      int raw_result=bmp.readTemperature();

      if (!isnan(raw_result)){
        // if(DEBUG_BMP280)  debug("BMP280", "SENSOR_STATE_OK");
        sensor_registers[3].value=SENSOR_STATE_OK;
        sensor_registers[4].value=static_cast<uint16_t>(raw_result*MULTIPLIER);
        ulong raw_pressure =(ulong) bmp.readPressure()*MULTIPLIER;
        high_word = (raw_pressure >> 16) & 0xFFFF;  // hibyte
        low_word  = raw_pressure & 0xFFFF;          // lowbyte
/*
        sensor_registers[5].value = (raw_pressure >> 48) & 0xFFFF;  // Самый старший 16-битный блок
        sensor_registers[6].value = (raw_pressure >> 32) & 0xFFFF;
        sensor_registers[7].value = (raw_pressure >> 16) & 0xFFFF;
        sensor_registers[8].value = raw_pressure & 0xFFFF;
*/
        sensor_registers[5].value=high_word;
        sensor_registers[6].value=low_word;

        //sensor_registers[6].value=bmp.readAltitude(1013.25)*MULTIPLIER;
      }else{
        sensor_registers[3].value=SENSOR_STATE_FAIL;
        sensor_registers[4].value=static_cast<uint16_t>(NO_SENSOR_VAL);
        sensor_registers[5].value=static_cast<uint16_t>(NO_SENSOR_VAL);
        sensor_registers[6].value=static_cast<uint16_t>(NO_SENSOR_VAL); 
      } 



     //debug("BMP280", "status="+String(sensor_registers[3].value)+ \
                           ", t="+String(sensor_registers[4].value)+ \
                           ", p_hi="+String(sensor_registers[5].value)+ \
                           ", p_low="+String(sensor_registers[6].value)+ \
                           ", p_check="+String(((uint32_t)sensor_registers[5].value << 16) | sensor_registers[6].value)+ \
                           ", "+String(millis()));
    };
        
};


#endif