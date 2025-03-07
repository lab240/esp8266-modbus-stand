#ifndef dmbsensorBMP280
#define dmbsensorBMP280
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#include "vsensora.h"

const uint8_t BMP280_I2C_ADDRESS = 0x76;

const float APRESSURE=1013.25;

#define DEBUG_BMP280 1

class vector_sensor_bmp280 : public vector_sensor {

protected:

    uint init_ok=0;
  
//Sensor porops    
    long  NO_SENSOR_VAL = -12700;
    uint  MULTIPLIER =  100;     //25.24C we store as 2524
    uint  MULTIPLIER2 = 1;     //25.24C we store as 2524
    uint  MULTIPLIER3 = 1;     //25.24C we store as 2524
    uint  init_status=0;
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
          
    }

    void virtual init() {
    
      if(DEBUG_BMP280) debug("BMP280", "BMP280 starting init");

      init_status = bmp.begin(BMP280_I2C_ADDRESS);

       if (!init_status) 
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

    int virtual nosensor_val(long val){
      if (val == 176 || val == 178 || val==177)
        return 1;
      return 0;
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
      
      float raw_result=bmp.readTemperature();

      //no sensor or no values from sensor
      if (isnan(raw_result)|| init_status==0 || nosensor_val(raw_result)){
        sensor_registers[2].value=SENSOR_STATE_FAIL;
        sensor_registers[3].value=MULTIPLIER;
        sensor_registers[4].value=NO_SENSOR_VAL;
        sensor_registers[3].value=MULTIPLIER2;
        sensor_registers[5].value=NO_SENSOR_VAL;
        sensor_registers[3].value=MULTIPLIER3;
        sensor_registers[6].value=NO_SENSOR_VAL; 

        //sensor_registers[6].value=bmp.readAltitude(1013.25)*MULTIPLIER;
      }else{

         // if(DEBUG_BMP280)  debug("BMP280", "SENSOR_STATE_OK");
        sensor_registers[2].value=SENSOR_STATE_OK;
        sensor_registers[3].value=MULTIPLIER;
        sensor_registers[4].value=(int32_t)raw_result*MULTIPLIER;
        sensor_registers[5].value=MULTIPLIER2;
        sensor_registers[6].value=(int32_t)bmp.readPressure()*MULTIPLIER2;
        sensor_registers[7].value=MULTIPLIER3;
        sensor_registers[8].value=(int32_t)bmp.readAltitude(APRESSURE)*MULTIPLIER3;

      } 

      if(DEBUG_BMP280) debug("BMP280", "ok_status="+String(sensor_registers[3].value)+ \
                           ", t="+String(sensor_registers[4].value)+ \
                           ", p="+String(sensor_registers[6].value)+ \
                           ", altitude="+String(sensor_registers[8].value)+ \
                           ", "+String(millis()));


    //if no sensor lets try to init it 
      if(init_status==0) {
        init_status = bmp.begin(BMP280_I2C_ADDRESS);
      }

  };
        
};


#endif