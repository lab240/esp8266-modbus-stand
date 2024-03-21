#include <ModbusRTU.h>
#include <EEPROM.h>
#include <Ticker.h>

#define DEBUG 1

#define MAX_ID 127            // максимально допустимый адрес модбас
#define MB_RATE 115200        // скорость обмена по модбас
#define MB_FC SERIAL_8E1      // параметры контроля передачи модбас

#define LED_DATA D6

//#define LED_DATA LED_BUILTIN

//константы адреса модбас регистра -1 для правильного отображения в mbpool

#define REGNA 1-1
#define REGNH 2-1
#define REGNM 3-1
#define REGNS 4-1

ModbusRTU mb;                 // объект для взаимодействия с либой ModbusRTU

Ticker ticker;

uint32_t softTimer = 0;       // переменная для захвата текущего времени
uint8_t adrSlave=0;
uint32_t get_counter = 0;   
uint led_mode_setup;

uint16_t cbReadHreg(TRegister* reg, uint16_t numregs){
  digitalWrite(LED_DATA,HIGH);
  return reg->value;
}

void setup() {
  uint8_t conterByte = 0;      // указатель для записи команды в массив
  char inCommand[10] = {0};    // заводим массив для захвата команды задания адреса
  led_mode_setup =1;

  pinMode(LED_DATA, OUTPUT);
  
  ticker.attach(0.25,tickf);

  EEPROM.begin(4);                    // инициализируем eeprom
  
  adrSlave = EEPROM.read(0);  // считываем адес устройства из EEPROM ячейки 0.
  
  if(adrSlave > MAX_ID) {             // если в EEPROM был слишком большой мусор скидываем в 0
    adrSlave = 0;
    EEPROM.write(0, (uint8_t)adrSlave);
    EEPROM.commit();
  }
  
  Serial.begin(115200, SERIAL_8N1);     // инициализируем уарт с параметрами стандартного монитора порта

  Serial.println("\n");
  Serial.print("Address device - ");
  Serial.println(adrSlave, DEC);       // печатаем текущий адрес устройства

  Serial.println("");
  Serial.println("Wait new adress");
  Serial.println("* command for set new address: set-xxx (001-127)");


  for(uint8_t i = 0; i < 6; i++){  // цикл таймаута для задания адреса

  Serial.print(i,DEC);
  Serial.print("...\t");  // печатаем секунды таймаута

  softTimer = millis() + 1000;                  // назначаем таймер на 1с
  while(softTimer > (millis())){                // крутимся в цикле пока время меньше заданного таймаута
    if (Serial.available() > 0) {               // если доступны данные из порта
      inCommand[conterByte]=Serial.read();      // принимаем в массив по указателю
      if(conterByte < 8) conterByte++;          // но не более 9 значений от 0 
      else conterByte = 0;                      // иначе сбрасываем счетчик
    }
  }
  }

  if (inCommand[0]=='s' && inCommand[1]=='e' && inCommand[2]=='t' && inCommand[3]=='-'){ // проверяем первые 4 символа на соответствие команде 
    Serial.println("");
    Serial.print("New adress set - ");
    uint32_t newAdress = ((inCommand[4]-'0')*100)+((inCommand[5]-'0')*10)+(inCommand[6]-'0');  //переводим символы в цифры
    
    if (0 < newAdress && newAdress <= MAX_ID ){   // проверяем на соответствие диапазону 0...127
      adrSlave = newAdress;                         // пишем новый адрес в переменную
      EEPROM.write(0, (uint8_t)newAdress);          // и в EEPROM
      EEPROM.commit();                              // подтверждаем запись
      Serial.println(newAdress, DEC);
    }
    else Serial.println("Error");                 // не попали в диапазон - пишем ошибку
  }
  else{  // если команды set- не поступило
    Serial.println("");
    Serial.println("New address not selected");
  }
  if(inCommand[0]){
    Serial.println("");
    for(uint8_t i = 0; i < 9; i++) Serial.print(inCommand[i]); // DEBUG - эхо ввода команды
  }
  Serial.println("");
  Serial.println("Start programm with Modbus address->"+String(adrSlave));
  ::delay(200);  // дожидаемся окончания передач в уарт

  Serial.begin(MB_RATE, MB_FC); // инициализация уарт с настройками для Модбас
  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);
  Serial.swap();

  mb.begin(&Serial);  //указание порта для модбас
  mb.slave(adrSlave); // указание адреса устройства в протоколе модбас

  mb.addHreg(REGNA); // добавление регистра с адресом устройства
  mb.addHreg(REGNH); // добавление регистра часов
  mb.addHreg(REGNM); // добавление регистра минут
  mb.addHreg(REGNS); // добавление регистра секунд

  mb.Hreg(REGNA, 0); //обнуление данных регистра адреса
  mb.Hreg(REGNH, 0); //обнуление данных регистра часов
  mb.Hreg(REGNM, 0); //обнуление данных регистра минут
  mb.Hreg(REGNS, 0); //обнуление данных регистра секунд


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
  mb.Hreg(REGNA, adrSlave); 
  mb.Hreg(REGNH, timeHours);
  mb.Hreg(REGNM, timeMins);
  mb.Hreg(REGNS, timeSecs);
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
