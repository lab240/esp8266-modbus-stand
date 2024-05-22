#include <ModbusRTU.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <ESP8266TrueRandom.h>
#include <PubSubClient.h>
#include <Queue.h>
#include "mbslave.h"
#include "donofflib/dpublishmqtt.h"


#define DEBUG 1
#define WIFI_ENABLE 1
#define MQTT_ENABLE 1
#define POWER_PIN D1 //old version
//#define POWER_PIN D5 //new version

//#define LED_DATA D6

#define LED_DATA LED_BUILTIN

//константы адреса модбас регистра -1 для правильного отображения в mbpool
#define NUM_TRY 10

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
// #define serial_baudrate _s->custom_level4
// #define serial_settings_num _s->custom_level_notify1

SerialConfig serial_settings=DEFAULT_MB_FC;


#define R_ADDR 0
#define R_HOUR 1
#define R_MINS 2
#define R_SECS 3


ModbusRTU mbus_obj;                 // объект для взаимодействия с либой ModbusRTU

Ticker ticker;

WifiCreds wificreds;

//WMSettings settings;

WMSettings * _s;

WiFiClient espClient;
PubSubClient client(espClient);

DPublisherMQTT pubmqtt(_s, &client);

void callback(char* topic, byte* payload, unsigned int length);
Queue<pub_events> que_wanted= Queue<pub_events>(MAX_QUEUE_WANTED);

uint32_t softTimer = 0;       // переменная для захвата текущего времени
//uint8_t modbus_address=0;
uint32_t get_counter = 0;   
uint led_mode_setup;

uint16_t cbReadHreg(TRegister* reg, uint16_t numregs){
  digitalWrite(LED_DATA,HIGH);
  return reg->value;
}

void callback(char* topic, byte* payload, unsigned int length){
  // Serial.println("nature callback");
  pubmqtt.callback(topic,payload,length);
}

void setup() {

  if(MQTT_ENABLE){
    client.setCallback(callback);
    pubmqtt.init(&que_wanted);
  }

  led_mode_setup =1;

  pinMode(LED_DATA, OUTPUT);
  
  ticker.attach(0.25,tickf);

  _s=new(WMSettings);

  Serial.begin(115200, SERIAL_8N1);  
  delay(1000);

  eload(_s);

   debug(DEEPROM, "Setings from EEPROM, currect Salt="+ String(EEPROM_SALT)+ " EEPROM SALT=" + String(_s->salt),  TOUT);
   print_curr_settings(_s);

  debug(DEEPROM, "Salt="+String(_s->salt)+"; Address="+String(_s->mb_modbus_address)+"; RegsI="+String(_s->mb_intregs_amount)+"; RegsC="+String(_s->mb_coilregs_amount));
  //debug(DEEPROM, "Salt="+String(_s->salt)+"; Address="+String(_s->custom_level1)+"; RegsI="+String(_s->custom_level2)+"; RegsC="+String(_s->custom_level3));

  if (_s->salt != EEPROM_SALT) {
    debug(DEEPROM, "Invalid settings in EEPROM, trying with defaults",TERROR);
    WMSettings defaults;
    *_s = defaults;
    _s->mb_modbus_address=DEFAULT_ADDRESS; //по умолчанию пусть будет 126й адрес
    _s->mb_intregs_amount=DEFAULT_INT_REGS;
    _s->mb_coilregs_amount=DEFAULT_COIL_REGS;
    _s->mb_serial_baudrate=DEFAULT_MB_RATE;
    serial_settings=DEFAULT_MB_FC;
    _s->mb_serial_settings_num=NSERIAL_8E1; //SERIAL_8E1

    debug(DEEPROM, "DEFAULTS: Salt="+String(_s->salt), TOUT);
    print_curr_settings(_s);
    
  }else{
    //check each parameter
    int is_corrected=correction_to_default_if_need(_s);
 
    serial_settings=get_serial_sttings_from_num(_s->mb_serial_settings_num); 
    if(is_corrected) {
      debug(DEEPROM, "LOADED from EEPROM with correction", TOUT);
      print_curr_settings(_s);
    }
  }

  if(WIFI_ENABLE){
    debug(DMAIN, "Init wifi settings");
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
  }
 

  // инициализируем уарт с параметрами стандартного монитора порта

  debug(DENTER,0);
  print_welcome_help();
 
  debug(DMAIN, "--------------- Enter setup mode, to brake setup mode, send space<enter> or C<enter> -------------");
  debug(DENTER,0);
 
  //loop of setup progs
  do_espboot_loop(_s);  

  debug(DENTER,0); // \n

  debug(DMAIN, "------ Start modbus emulation ------");
  print_curr_settings(_s);

  if(WIFI_ENABLE){
    ticker.attach(0.15,tickf);
    struct station_config stationConf;
    wifi_station_get_config (&stationConf);
    debug(DWIFI,"Try with ssid|pass=|" + String((char*)stationConf.ssid) +"|" + String((char*)stationConf.password)+"|");
    WiFi.begin();
    mywifi_try_to_connect();
    ticker.attach(0.25,tickf);
  }
  
  debug(DMAIN, "-------------------------------------");
  debug(DMAIN, "Switching Serial port to hardware mode, finish serial input/output operations");
  debug(DMAIN, "-------------------------------------");

  ::delay(200);  // дожидаемся окончания передач в уарт
  

  //Serial.begin(serial_baudrate, (uint8_t) serial_settings_num); // инициализация уарт с настройками для Модбас
  Serial.begin(_s->mb_serial_baudrate, serial_settings); // инициализация уарт с настройками для Модбас
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  Serial.swap();

  mbus_obj.begin(&Serial);  //указание порта для модбас
  mbus_obj.slave(_s->mb_modbus_address); // указание адреса устройства в протоколе модбас

  for(int h_reg=0; h_reg<_s->mb_intregs_amount; h_reg++){
    mbus_obj.addHreg(h_reg); //add register
    mbus_obj.Hreg(h_reg,0);  //add 0 to each reg
  }

   for(int c_reg=0; c_reg<_s->mb_coilregs_amount; c_reg++){
    mbus_obj.addCoil(c_reg); //add register
    mbus_obj.Coil(c_reg,0);  //add 0 to each reg
  }

//callback when request comes
  mbus_obj.onGetHreg(0,cbReadHreg,_s->mb_intregs_amount);

  led_mode_setup=0; //finish setup blinking
}

void loop() {

  mbus_obj.task(); // слушаем модбас
  //yield();   // отпускаем для обработки Wi-Fi
  if(softTimer<(millis())) update_regs(); // обновляем регистры по таймеру
}

void update_regs(){
  uint32_t sec = millis() / 1000ul;      // полное количество секунд со старта платы
  uint16_t timeHours = (sec / 3600ul);        // часы
  uint16_t timeMins = (sec % 3600ul) / 60ul;  // минуты
  uint16_t timeSecs = (sec % 3600ul) % 60ul;  // секунды

  // заполняем реги значениями времени
  mbus_obj.Hreg(R_ADDR, _s->mb_modbus_address); 
  mbus_obj.Hreg(R_HOUR, timeHours);
  mbus_obj.Hreg(R_MINS, timeMins);
  mbus_obj.Hreg(R_SECS, timeSecs);

  //next regs are random

  if(_s->mb_intregs_amount!=0)
   for(int h_reg=R_SECS+1; h_reg<_s->mb_intregs_amount; h_reg++){
      //mbus_obj.Hreg(h_reg,random(1,32000));
      mbus_obj.Hreg(h_reg,ESP8266TrueRandom.random(32000));
   }
  
  if(_s->mb_coilregs_amount!=0)
   for(int c_reg=0; c_reg<_s->mb_coilregs_amount; c_reg++){
     mbus_obj.Coil(c_reg,ESP8266TrueRandom.randomBit());
   }


  softTimer= millis() + 500; // перевзводим на полсекунды
} 

void tickf(){
  if(led_mode_setup) {
    digitalWrite(LED_DATA, !digitalRead(LED_DATA));
  }else{
  //digitalWrite(LED_DATA, !digitalRead(LED_DATA));
    digitalWrite(LED_DATA, LOW);
  }
}


