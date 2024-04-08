#include <ModbusRTU.h>
#include <EEPROM.h>
#include <Ticker.h>
#include "mbslave.h"

#define DEBUG 1

#define MAX_ID 127            // максимально допустимый адрес модбас
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


ModbusRTU mb;                 // объект для взаимодействия с либой ModbusRTU

Ticker ticker;

WMSettings settings;

uint32_t softTimer = 0;       // переменная для захвата текущего времени
//uint8_t modbus_address=0;
uint32_t get_counter = 0;   
uint led_mode_setup;

#define modbus_address settings.custom_level1

uint16_t cbReadHreg(TRegister* reg, uint16_t numregs){
  digitalWrite(LED_DATA,HIGH);
  return reg->value;
}

void setup() {
  uint8_t counterByte = 0;      // указатель для записи команды в массив
  char inCommand[50] = {0};    // заводим массив для захвата команды задания адреса
  String inCommandStr="";
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
    debug(DEEPROM, "Salt="+String(settings.salt)+"; Address="+String(modbus_address));
    
  }

  // инициализируем уарт с параметрами стандартного монитора порта

  Serial.println("\n");
  Serial.print("Address device - ");
  Serial.println(modbus_address, DEC);       // печатаем текущий адрес устройства

  Serial.println("");
  Serial.println("Wait new adress");
  Serial.println("* command for set new address: seta=xxx (001-127)");

  int is_cmd=0;
  //int is_cmd_l=0;
  int count_try=0;

  while(is_cmd==0 && count_try<NUM_TRY){  // цикл таймаута для задания адреса

    Serial.print(count_try,DEC);
    Serial.print("...\t");  // печатаем секунды таймаута

    softTimer = millis() + 1000;                  // назначаем таймер на 1с

    int incoming_flg=0;
    while(softTimer > (millis())){                // крутимся в цикле пока время меньше заданного таймаута
      if (Serial.available() > 0) {               // если доступны данные из порта
        inCommand[counterByte]=Serial.read();      // принимаем в массив по указателю
        if(counterByte < 520) counterByte++;          // но не более 20 значений от 0 
        else counterByte = 0;
        incoming_flg=1;                      // иначе сбрасываем счетчик
      }
    }

    if(incoming_flg) is_cmd=1;
    count_try++;
  }

  Serial.println("\n");

  if(is_cmd){
    debug(DCOMMAND, "Received incoming string->"+String(inCommand));
    //debug(DCOMMAND, "\n Commnad Recognized", TOUT);
    //debug(DCOMMAND, "incommand->"+String(inCommand));
    inCommandStr=String(inCommand);
    String cmdStr=  inCommandStr.substring(0,inCommandStr.indexOf('='));
    String numStr = inCommandStr.substring(inCommandStr.indexOf('=')+1,inCommandStr.length());
    debug(DCOMMAND, "Command->"+ cmdStr+", Value->"+numStr);

    if (cmdStr == "seta"){ // проверяем первые 4 символа на соответствие команде 
      //Serial.println("");
      //Serial.print("New adress set - ");
      //uint32_t newAdress = ((inCommand[4]-'0')*100)+((inCommand[5]-'0')*10)+(inCommand[6]-'0');  //переводим символы в цифры
           
      
      uint newAdress=0;
      if((newAdress=numStr.toInt()) == 0 ){
        debug(DCOMMAND, "Failed convert address to int value",1);
      }

      if (0 < newAdress && newAdress <= MAX_ID ){   // проверяем на соответствие диапазону 0...127
        modbus_address=newAdress;
        debug(DCOMMAND, "Set new address ->" +String(newAdress));
        esave(&settings);
        debug(DEEPROM, "New settings saved to EEPROM->"+String(modbus_address));                     // подтверждаем запись
      }
      else{ 
        // не попали в диапазон - пишем ошибку
        debug(DCOMMAND, "Wrong address range !",1);    
        debug(DCOMMAND, "New address not selected or recognized, leave old address ->"+String(modbus_address),4);
      }             
    }
    else if (cmdStr == "setap"){
      debug(DCOMMAND, "Something do with->" + String(cmdStr));
    }
    else if (cmdStr == "setp"){
       debug(DCOMMAND, "Something do with->" + String(cmdStr));
    }
    else{  
      // если команды = не поступило
      debug(DCOMMAND, "Comand is not recognized->"+cmdStr,TERROR);
    }
    
  }else{
    debug(DCOMMON, "No incomming string");
  }
  /*
  if(inCommand[0]){
    Serial.println(""); 
    for(uint8_t i = 0; i < 9; i++) Serial.print(inCommand[i]); // DEBUG - эхо ввода команды
  }
  */
  
  
  debug(DCOMMON, "Start programm with Modbus address->"+String(modbus_address), TOUT);
  debug(DCOMMON, "Switching Serial port to hardware mode, finish serial input/output operations");

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
