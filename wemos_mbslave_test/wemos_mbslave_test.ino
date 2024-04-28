#include <ModbusRTU.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <ESP8266TrueRandom.h>
#include "mbslave.h"

#define DEBUG 1


#define DEFAULT_MB_RATE 115200        // скорость обмена по модбас
#define DEFAULT_MB_FC SERIAL_8E1      // параметры контроля передачи модбас

//#define LED_DATA D6

#define LED_DATA LED_BUILTIN

//константы адреса модбас регистра -1 для правильного отображения в mbpool
#define NUM_TRY 10

// #define EXTRAREGS 5 

#define MAXEXTRAREGS 255 

#define modbus_address settings.custom_level1
#define intregs_amount settings.custom_level2
#define coilregs_amount settings.custom_level3
#define serial_baudrate settings.custom_level4
#define serial_settings_num settings.custom_level_notify1

SerialConfig serial_settings=DEFAULT_MB_FC;

#define DEFAULT_ADDRESS 126
#define DEFAULT_INT_REGS 10
#define DEFAULT_COIL_REGS 10

#define R_ADDR 0
#define R_HOUR 1
#define R_MINS 2
#define R_SECS 3


ModbusRTU mb;                 // объект для взаимодействия с либой ModbusRTU

Ticker ticker;

WMSettings settings;

uint32_t softTimer = 0;       // переменная для захвата текущего времени
//uint8_t modbus_address=0;
uint32_t get_counter = 0;   
uint led_mode_setup;

uint16_t cbReadHreg(TRegister* reg, uint16_t numregs){
  digitalWrite(LED_DATA,HIGH);
  return reg->value;
}

void setup() {
 
  led_mode_setup =1;

  pinMode(LED_DATA, OUTPUT);
  
  ticker.attach(0.25,tickf);

  Serial.begin(115200, SERIAL_8N1);  
  delay(1000);

  eload(&settings);

  debug(DEEPROM, "Salt="+String(settings.salt)+"; Address="+String(modbus_address)+"; RegsI="+String(intregs_amount)+"; RegsC="+String(coilregs_amount));

  if (settings.salt != EEPROM_SALT || modbus_address > MAX_ID) {
    debug(DEEPROM, "Invalid settings in EEPROM, trying with defaults",TERROR);
    WMSettings defaults;
    settings = defaults;
    modbus_address=DEFAULT_ADDRESS; //по умолчанию пусть будет 126й адрес
    intregs_amount=DEFAULT_INT_REGS;
    coilregs_amount=DEFAULT_COIL_REGS;
    serial_baudrate=DEFAULT_MB_RATE;
    serial_settings=DEFAULT_MB_FC;

    debug(DEEPROM, "DEFAULTS: Salt="+String(settings.salt), TOUT);
    print_curr_settings(&settings);
    
  }else{
    serial_settings=get_serial_sttings_from_num(serial_settings_num); 
    debug(DEEPROM, "LOADED from EEPROM: Salt="+String(settings.salt), TOUT);
    print_curr_settings(&settings);
  }

  // инициализируем уарт с параметрами стандартного монитора порта

  debug(DENTER,0);
  print_welcome_help();
 
  debug(DMAIN, "--------------- Enter setup mode, to brake setup mode, send space<enter> or C<enter> -------------");
  debug(DENTER,0);

  String inCommandStr=""; 
  bool stop_commnads=0;
//цикл приема команд

  while (!stop_commnads){

    inCommandStr=get_command_str();
    Serial.println("\n");
    
    if(inCommandStr!=""){
      debug(DCOMMAND, "Received incoming string->"+String(inCommandStr));
     
      if( inCommandStr.length()<=3 && inCommandStr.charAt(0)=='C') {
        debug(DCOMMAND, "Command->"+String(inCommandStr.charAt(0))+"; Skip waiting command", TOUT);
        stop_commnads=1;

      }else if(inCommandStr.length()>3 && inCommandStr.indexOf('=')==-1){
        //commands 
        debug(DCOMMAND,"command recognized");
        if(inCommandStr.startsWith("help")){
          print_full_help();
        }
      
      }else if(inCommandStr.indexOf('=')!=-1){
        //settings set_parameter=<value>
        String cmdStr=  inCommandStr.substring(0,inCommandStr.indexOf('='));
        String numStr = inCommandStr.substring(inCommandStr.indexOf('=')+1,inCommandStr.length());
        debug(DCOMMAND, "Command->"+ cmdStr+", Value->"+numStr);
        if(!do_set_command(&settings, cmdStr, numStr)) {
          debug(DCOMMAND,"Wrong set parameter or value->"+cmdStr);
        }
      
      }else{
        debug(DCOMMAND, "Commnad is not recognized", TOUT);
      }
    
    }else{
      debug(DCOMMAND, "No incomming string");
      stop_commnads=1;
    }

  }
    
  debug(DENTER,0); // \n

  debug(DMAIN, "------ Start modbus emulation ------");
  print_curr_settings(&settings);
  debug(DMAIN, "Switching Serial port to hardware mode, finish serial input/output operations");
  debug(DMAIN, "-------------------------------------");

  ::delay(200);  // дожидаемся окончания передач в уарт
  

  //Serial.begin(serial_baudrate, (uint8_t) serial_settings_num); // инициализация уарт с настройками для Модбас
  Serial.begin(serial_baudrate, serial_settings); // инициализация уарт с настройками для Модбас
  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);
  Serial.swap();

  mb.begin(&Serial);  //указание порта для модбас
  mb.slave(modbus_address); // указание адреса устройства в протоколе модбас

  for(int h_reg=0; h_reg<intregs_amount; h_reg++){
    mb.addHreg(h_reg); //add register
    mb.Hreg(h_reg,0);  //add 0 to each reg
  }

   for(int c_reg=0; c_reg<coilregs_amount; c_reg++){
    mb.addCoil(c_reg); //add register
    mb.Coil(c_reg,0);  //add 0 to each reg
  }

//callback when request comes
  mb.onGetHreg(0,cbReadHreg,intregs_amount);

  led_mode_setup=0; //finish setup blinking
}

void loop() {

  mb.task(); // слушаем модбас
  //yield();   // отпускаем для обработки Wi-Fi
  if(softTimer<(millis())) update_regs(); // обновляем регистры по таймеру
}

void update_regs(){
  uint32_t sec = millis() / 1000ul;      // полное количество секунд со старта платы
  uint16_t timeHours = (sec / 3600ul);        // часы
  uint16_t timeMins = (sec % 3600ul) / 60ul;  // минуты
  uint16_t timeSecs = (sec % 3600ul) % 60ul;  // секунды

  // заполняем реги значениями времени
  mb.Hreg(R_ADDR, modbus_address); 
  mb.Hreg(R_HOUR, timeHours);
  mb.Hreg(R_MINS, timeMins);
  mb.Hreg(R_SECS, timeSecs);

  //next regs are random

  if(intregs_amount!=0)
   for(int h_reg=R_SECS+1; h_reg<intregs_amount; h_reg++){
      //mb.Hreg(h_reg,random(1,32000));
      mb.Hreg(h_reg,ESP8266TrueRandom.random(32000));
   }
  
  if(coilregs_amount!=0)
   for(int c_reg=0; c_reg<coilregs_amount; c_reg++){
     mb.Coil(c_reg,ESP8266TrueRandom.randomBit());
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
