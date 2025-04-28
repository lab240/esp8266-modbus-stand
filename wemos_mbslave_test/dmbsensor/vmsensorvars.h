#ifndef vmsensorvars
#define vmsensorvars



#define NO_SENSOR_VALUE -12700
#define NO_SENSOR_DATA_VALUE   -12800


enum sensor_types { 
 UNDEF_SENSOR_TYPE,
 RANDOM_SENSOR_8H,
 TEMPERATURE_SENSOR,
 LIGTH_SENSOR,
 BMP280_SENSOR
 };

#define MULTIPLIER_TEMP 100
#define MULTIPLIER_PRESSURE 1
#define MULTIPLIER_LUX 100


const char* TEMP_REGISTER_NAME="temp";
const char* PRESSURE_REGISTER_NAME="pressure";

#define NO_SENSOR_STATE -1
#define NO_SENSOR_DATA_STATE  -2

#define ENABLE_SPLIT_IN_TO_WORDS 1
#define ENABLE_SPLIT_IN_TO_WORDS 1

#endif