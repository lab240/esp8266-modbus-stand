
#include <Ticker.h>
#include <PubSubClient.h>
#include <Queue.h>

#include <ESP8266WiFi.h>
#include <ModbusRTU.h>

#include "donofflib/dpublishmqtt.h"
#include "donofflib/ddevice.h"
#include "mbmb/mbpublish.h"
#include "dboot/dbootmodbus.h"

/*
#include "mbmregs/mbmbregsa.h"
#include "mbmregs/mbmbregs-ds1820.h"
#include "mbmregs/mbmbregs-bmp280.h"
*/

/*
#include "dmbsensor/vsensor-random.h"
#include "dmbsensor/vsensor-ds1820.h"
#include "dmbsensor/vsensor-bh1750.h"
#include "dmbsensor/vsensor-bmp280.h"
*/

#include "dmbsensor/vmsensor-bh1750.h" 

#define DEBUG 1
#define WIFI_ENABLE 0
#define MQTT_ENABLE 0

#define DEBUG_VERBOSE_MODE_NOMODBUS_OUT

#ifdef DEBUG_VERBOSE_MODE_NOMODBUS_OUT
  #define SWAPSERIAL 0
  #define SILENT_SERIAL_MODE 0
#else
  #define SWAPSERIAL 1
  #define SILENT_SERIAL_MODE 1
#endif


//if all sensors not present, we use random sensor

#define DS1820_SENSOR_PRESENTS 0
#define BMP280_SENSOR_PRESENTS 0
#define BH1750_SENSOR_PRESENTS 1

//#define POWER_PIN D1 //old version
#define POWER_PIN D5 //new version

#define LED_DATA2 D0

#define LED_DATA LED_BUILTIN

// #define EXTRAREGS 5 

#define MAXEXTRAREGS 255 

// #define modbus_address settings.custom_level1
// #define intregs_amount settings.custom_level2
// #define coilregs_amount settings.custom_level3
// #define serial_baudrate settings.custom_level4
// #define serial_settings_num settings.custom_level_notify1

// #define modbus_address  _s->custom_level1
// #define intregs_amount  _s->custom_level2
// #define coilregs_amount _s->custom_level3
// #define  _s->custom_level4
// #define serial_settings_num _s->custom_level_notify1

SerialConfig serial_settings=DEFAULT_MB_FC;

ModbusRTU mbus_obj;                

Ticker ticker;

WifiCreds wificreds;

WMSettings * _s;
DBootEspMqttModbus * dboot;

WiFiClient espClient;
PubSubClient client(espClient);
DProg dprogramm;

DPublisherMqtt* publisher_mqtt;
DDevice* mb_dev;
//modbus_regs* mb_regs;
//vector_sensor* mbsensor;
//vector_sensor_ds1820* ds1820sensor;

VmSensora* mbsensor;


//void callback(char* topic, byte* payload, unsigned int length);
Queue<pub_events> que_wanted= Queue<pub_events>(MAX_QUEUE_WANTED);

uint32_t softTimer = 0;       // переменная для захвата текущего времени
//uint8_t modbus_address=0;
uint32_t get_counter = 0;   
uint led_mode_setup;

uint16_t cbReadHreg(TRegister* reg, uint16_t numregs){
  digitalWrite(LED_DATA,HIGH);
  return reg->value;
}

/*
void callback(char* topic, byte* payload, unsigned int length){
  publisher_mqtt->callback(topic,payload,length);
}
*/

void setup() {

  Serial.begin(115200, SERIAL_8N1);  
  delay(1000);

  Serial.println("************ Starting Modbus Emulator 0.1b ********************");

  delay(1000);
  
  //init espboot
  _s=new(WMSettings);

    Serial.println("************ Starting DBOOT ********************");
  
  dboot=new DBootEspMqttModbus(_s);

  // Serial.println("************ Starting DBOOT\INIT ********************");
  dboot->init();
  dboot->print_curr_settings();
  
  mb_dev=new DDevice(_s); 
  
  if(MQTT_ENABLE){

    publisher_mqtt=new DPublisherMqttMBstand(_s, nullptr, 0);
    publisher_mqtt->init(&que_wanted);
    mb_dev->init(publisher_mqtt, &que_wanted);
  }else{
    dprogramm.debug(DSMAIN,"Init mb_dev with NO MQTT MODE");
    mb_dev->init(nullptr,nullptr);
  }


  led_mode_setup =1;

  pinMode(LED_DATA, OUTPUT);
  pinMode(LED_DATA2, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);

  ticker.attach(0.25,tickf);
 
  if(WIFI_ENABLE){
    dprogramm.debug(DSMAIN, "Init wifi settings");
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
  }
  
  dprogramm.debug(DSMAIN, "-------------- Welcome  ------------------------------------------------------------------------");

  // инициализируем уарт с параметрами стандартного монитора порта

  dprogramm.debug(DSENTER,0);
  dboot->print_welcome_help();
 
  dprogramm.debug(DSMAIN, "--------------- Enter setup mode, to brake setup mode, send space<enter> or C<enter> -------------");
  dprogramm.debug(DSENTER,0);
 
  //loop of setup boot
  dboot->do_boot_loop();

  dprogramm.debug(DSENTER,0); // \n

  dprogramm.debug(DSMAIN, "------ Start modbus emulation with parameters ------");
  // print_curr_settings(_s);

  dboot->print_curr_settings();

  delete(dboot);


  if(WIFI_ENABLE){
    ticker.attach(0.15,tickf);
    struct station_config stationConf;
    wifi_station_get_config (&stationConf);
    dprogramm.debug(DSWIFI,"Try with ssid|pass=|" + String((char*)stationConf.ssid) +"|" + String((char*)stationConf.password)+"|");
    WiFi.begin();
    dprogramm.mywifi_try_to_connect();
    ticker.attach(0.25,tickf);
  }

  pinMode(POWER_PIN, OUTPUT);


  dprogramm.debug(DSMAIN, "Run new Serial.begin");

  //Serial.end();   // Остановить текущий Serial
  
  //Serial.setDebugOutput(false); 

  dprogramm.debug(DSMAIN, "Write HIGH to enable RS485");

  //Power relay to enable RS485 RX\TX
  digitalWrite(POWER_PIN,HIGH);
  
  if(SWAPSERIAL==1){
    //Swap hardware serial to D7,D8    
    dprogramm.debug(DSMAIN, "Swap Serial to D7-D8");
    dprogramm.debug(DSMAIN, "Setting silent mode, bye bye console ");
    Serial.flush();
    delay(200);
    Serial.begin( _s->mb_serial_baudrate, serial_settings);  
    digitalWrite(POWER_PIN,HIGH);
    Serial.swap();    
  }else{
    dprogramm.debug(DSMAIN, "NO Swap serial");
   
    if(SILENT_SERIAL_MODE){
      Serial.flush();
      Serial.end();
      delay(200);
      Serial.begin( _s->mb_serial_baudrate, serial_settings);  
    }
    digitalWrite(POWER_PIN,HIGH);
  }

  if(SILENT_SERIAL_MODE){
    mb_dev->enable_silent();
    dprogramm.enable_silent();
  }

  mbus_obj.begin(&Serial);  //указание порта для модбас
  mbus_obj.slave(_s->mb_modbus_address); // указание адреса устройства в протоколе модбас

  dprogramm.debug(DSMAIN, "Starting sensor");

   if (DS1820_SENSOR_PRESENTS){

    // mbsensor=new vector_sensor_ds1820(_s->mb_modbus_address);
    // mbsensor->init();
    // dprogramm.debug(DSMAIN, "ds1820 sensor init done");
    // mb_regs=new modbus_regs_ds1820(_s,&mbus_obj,mbsensor);

   }else if (BH1750_SENSOR_PRESENTS){
   
    mbsensor=new VmSensorBH1750(_s->mb_modbus_address);
    mbsensor->init();
    dprogramm.debug(DSMAIN, "bh1750 sensor init done");
    //mb_regs=new modbus_regs(_s,&mbus_obj,mbsensor);
   
   }else if (BMP280_SENSOR_PRESENTS){
   
    //  mbsensor=new vector_sensor_bmp280(_s->mb_modbus_address);
    //  mbsensor->init();
    //  dprogramm.debug(DSMAIN, "bmp280 sensor init done");
    //  mb_regs=new modbus_regs_bmp280(_s,&mbus_obj,mbsensor);
   
   }else{
   
    // mbsensor=new vector_sensor_random(_s->mb_modbus_address,10,10,1);
    // dprogramm.debug(DSMAIN, "Random sensor create done");
    // mbsensor->init();
    // dprogramm.debug(DSMAIN, "Random sensor init done");
    // mb_regs=new modbus_regs(_s,&mbus_obj,mbsensor);
   
   }

 

  if(SILENT_SERIAL_MODE){
    //mb_regs->enable_silent();
    //mbsensor->enable_silent();
  }

  //mb_regs->init();

  
  dprogramm.debug(DSMAIN,"Modbus init regs... enabled");

//callback when request comes

  mbus_obj.onGetHreg(0,cbReadHreg,_s->mb_intregs_amount);

  dprogramm.debug(DSMAIN,"Callback fot modbus regs... enabled");

  led_mode_setup=0; //finish setup blinking

  
}

// loop without modbus, only sensor check
/*
void loop(){
 if(softTimer<(millis())) {
     ds1820sensor->sensor_loop();
     //mb_regs->update_regs(); // обновляем регистры по таймеру softTimer= millis() + 500;
     //mb_regs->print_hold_regs();
     digitalWrite(LED_DATA2, !digitalRead(LED_DATA2));
     softTimer=millis()+500;
  }
}
  
*/

void loop() {

  //digitalWrite(LED_DATA2, HIGH);
  mbus_obj.task(); // слушаем модбас
  //if (client.connected()) client.loop();
  mb_dev->supply_loop();
  
  //yield();   // отпускаем для обработки Wi-Fi
  if(softTimer<(millis())) {
     mbsensor->sensor_loop();
     //mb_regs->update_regs(); 
     //mb_regs->print_hold_regs();
     //expose_to_modbus(&mbus_obj, mbsensor);
     //digitalWrite(LED_DATA2, !digitalRead(LED_DATA2));

     mbsensor->print_registers();
     mbsensor->print_holdregisters();
     mbsensor->print_mqtt();

     softTimer=millis()+1000;
  }
  
}


void tickf(){
  if(led_mode_setup) {
    digitalWrite(LED_DATA, !digitalRead(LED_DATA));
  }else{
  //digitalWrite(LED_DATA, !digitalRead(LED_DATA));
    digitalWrite(LED_DATA, LOW);
  }
}

void expose_to_modbus( ModbusRTU * __mb, VmSensora* sensor) {
    const std::vector<int16_t>& regs = sensor->holder_registers;
    for (size_t i = 0; i < regs.size(); ++i) {
        __mb->Hreg(i, regs[i]);
    }
}



