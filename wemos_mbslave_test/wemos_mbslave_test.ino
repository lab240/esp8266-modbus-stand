#include <ModbusRTU.h>
#include <EEPROM.h>
#include <Ticker.h>
#include "mbslave.h"

#define DEBUG 1


#define MB_RATE 115200        // скорость обмена по модбас
#define MB_FC SERIAL_8E1      // параметры контроля передачи модбас

//#define LED_DATA D6

#define LED_DATA LED_BUILTIN

//константы адреса модбас регистра -1 для правильного отображения в mbpool
#define NUM_TRY 10

#define REGNA 1-1
#define REGNH 2-1
#define REGNM 3-1
#define REGNS 4-1

#define EXTRAREGS 5 

int REGS[]={4,5,6,7,8,9};

#define modbus_address settings.custom_level1

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

  debug(DEEPROM, "Salt="+String(settings.salt)+"; Address="+String(modbus_address));

  if (settings.salt != EEPROM_SALT || modbus_address > MAX_ID) {
    debug(DEEPROM, "Invalid settings in EEPROM, trying with defaults",TERROR);
    WMSettings defaults;
    settings = defaults;
    modbus_address=126; //по умолчанию пусть будет 126й адрес
    debug(DEEPROM, "Salt="+String(settings.salt)+"; Address="+String(modbus_address), TDEBUG);
    
  }

  // инициализируем уарт с параметрами стандартного монитора порта

  debug(DENTER,0);

  debug(DMAIN, "-------------- Avaliable commnads (wait for "+String(NUM_TRY)+" secs) -----------------");
  debug(DMAIN, "seta=<ADDRESS> (1..127)");
  debug(DMAIN, "---------------------------------------------------------------------------------------");
  debug(DENTER,0);

  String inCommandStr=""; 
  bool stop_commnads=0;
//цикл приема команд

  while (!stop_commnads){

    inCommandStr=get_command_str();
    Serial.println("\n");
    
    if(inCommandStr!=""){
      debug(DCOMMAND, "Received incoming string->"+String(inCommandStr));
      if(inCommandStr.indexOf('=')!=-1){
        String cmdStr=  inCommandStr.substring(0,inCommandStr.indexOf('='));
        String numStr = inCommandStr.substring(inCommandStr.indexOf('=')+1,inCommandStr.length());
        debug(DCOMMAND, "Command->"+ cmdStr+", Value->"+numStr);
        do_command(&settings, cmdStr, numStr);
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
  debug(DMAIN, "Start programm with Modbus address->"+String(modbus_address), TOUT);
  debug(DMAIN, "Switching Serial port to hardware mode, finish serial input/output operations");
  debug(DMAIN, "-------------------------------------");

  ::delay(200);  // дожидаемся окончания передач в уарт
  

  Serial.begin(MB_RATE, MB_FC); // инициализация уарт с настройками для Модбас
  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);
  Serial.swap();

  mb.begin(&Serial);  //указание порта для модбас
  mb.slave(modbus_address); // указание адреса устройства в протоколе модбас

  mb.addHreg(REGNA); // добавление регистра с адресом устройства
  mb.addHreg(REGNH); // добавление регистра часов
  mb.addHreg(REGNM); // добавление регистра минут
  mb.addHreg(REGNS); // добавление регистра секунд

  mb.Hreg(REGNA, 0); //обнуление данных регистра адреса
  mb.Hreg(REGNH, 0); //обнуление данных регистра часов
  mb.Hreg(REGNM, 0); //обнуление данных регистра минут
  mb.Hreg(REGNS, 0); //обнуление данных регистра секунд

  for(int i=0; i<=EXTRAREGS; i++){
    mb.addHreg(REGS[i]);
    mb.Hreg(REGS[i],0);
  }

//callback when request comes
  mb.onGetHreg(0,cbReadHreg,4);

  led_mode_setup=0; //finish setup blinking
}

void loop() {

  mb.task(); // слушаем модбас
  //yield();   // отпускаем для обработки Wi-Fi
  if(softTimer<(millis()))regTime(); // обновляем регистры по таймеру
}

void regTime (void){
  uint32_t sec = millis() / 1000ul;      // полное количество секунд со старта платы
  uint16_t timeHours = (sec / 3600ul);        // часы
  uint16_t timeMins = (sec % 3600ul) / 60ul;  // минуты
  uint16_t timeSecs = (sec % 3600ul) % 60ul;  // секунды

  // заполняем реги значениями времени
  mb.Hreg(REGNA, modbus_address); 
  mb.Hreg(REGNH, timeHours);
  mb.Hreg(REGNM, timeMins);
  mb.Hreg(REGNS, timeSecs);


  for(int i=0; i<=EXTRAREGS; i++){
     mb.Hreg(REGS[i],random(1,32000));
    //mb.Hreg(regs5[i],5);
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
