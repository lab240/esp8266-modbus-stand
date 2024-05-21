#ifndef __mbslave__
#define __mbslave__

#include <ESP8266WiFi.h>       
#include "Arduino.h"
#include "mbase.h"
#include "commands.h"
#include "mbserial.h"
#include "mbservice.h"


//default values
#define DEFAULT_ADDRESS 126           //default modbus address
#define DEFAULT_INT_REGS 10           //default hold regs
#define DEFAULT_COIL_REGS 10          //default coil regs

#define DEFAULT_MB_RATE 115200        // default baudrate
#define DEFAULT_MB_FC SERIAL_8E1      // dfault  serial settings

// MAX and MIN values
#define MAX_ID 127            // max modbus address
#define MAX_INT_REGS 255      
#define MIN_INT_REGS 4 

#define MAX_COIL_REGS 255
#define MIN_COIL_REGS 0

#define MAX_BAUDRATE 1500000
#define MIN_BAUDRATE 1200

#define MAX_SERIAL_VAR_NUM 62
//*****************************************************

#define SKIP_CHAR 'C'  //char ti skip waiting command

#define MODE_SETUP 1  //tech

int mywifi_try_to_connect(){
  int c = 0;
  IPAddress ip;
  while (WiFi.status() != WL_CONNECTED && c < 20) {
    delay(500);
    Serial.print(".");
    c++;
  }

  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WIFI_CONNECTED");
      ip = WiFi.localIP();
      Serial.print("This is my ip: ");
      Serial.println(ip);
      return 1;
  }else{
      debug(DWIFI, "WIFI_NOT_CONNECTED");
      return 1;
  }
};

//get command from terminal
String get_command_str(){
  int is_cmd=0;
  //int is_cmd_l=0;
  int count_try=0;
  uint32_t softTimer = 0; 
  char inCommandChr[50] = {0};    // заводим массив для захвата команды задания адреса
  uint8_t counterByte = 0;      // указатель для записи команды в массив

  while(is_cmd==0 && count_try<NUM_TRY){  // цикл таймаута для задания адреса

    Serial.print(count_try,DEC);
    Serial.print("...\t");  // печатаем секунды таймаута

    softTimer = millis() + 1000;                  // назначаем таймер на 1с

    int incoming_flg=0;
    while(softTimer > (millis())){                // крутимся в цикле пока время меньше заданного таймаута
      if (Serial.available() > 0) {               // если доступны данные из порта
        inCommandChr[counterByte]=Serial.read();      // принимаем в массив по указателю
        if(counterByte < 520) counterByte++;          // но не более 20 значений от 0 
        else counterByte = 0;
        incoming_flg=1;                      // иначе сбрасываем счетчик
      }
    }

    if(incoming_flg) is_cmd=1;
    count_try++;
  }
  return String(inCommandChr);
}



int do_set_command(WMSettings *_s, String cmdStr, String valStr){
  
  if (cmdStr == CMD_SET_ADDRESS)
      if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_modbus_address, 0,MAX_ID)) return 1;
  
  if (cmdStr == CMD_SET_INT_REGS_AMOUNT)
      if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_intregs_amount, 4,MAX_INT_REGS)) return 1;
  
  if (cmdStr == CMD_SET_COIL_REGS_AMOUNT)
      if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_coilregs_amount, 0,MAX_COIL_REGS)) return 1;
  
  if (cmdStr == CMD_SET_BAUDRATE)
      if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_serial_baudrate, MIN_BAUDRATE, MAX_BAUDRATE )) return 1;
  
  if (cmdStr == CMD_SET_PORT_SETTINGS)
      if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_serial_settings_num, 1, MAX_SERIAL_VAR_NUM)) return 1;
  if (cmdStr == CMD_SSID){
      debug(DCOMMAND, "Wifi Creds, push ssid="+valStr);
      
      if(valStr.indexOf('|')!=-1){  
        String ssidStr=  valStr.substring(0,valStr.indexOf('|'));
        String passStr = valStr.substring(valStr.indexOf('|')+1,valStr.length());
        debug(DCOMMAND, "Command->"+ ssidStr+", Value->"+passStr);
        debug(DCOMMAND, "Writing creds...");

        //wifi begin without connect
        WiFi.begin(ssidStr.c_str(), passStr.c_str(), 0, NULL, false);
        struct station_config stationConf;
        
        //os_memset(stationConf.ssid, 0, sizeof(stationConf.ssid));
	      //os_memset(stationConf.password, 0, sizeof(stationConf.password));
	      //snprintf(stationConf.ssid, sizeof(stationConf.ssid), "%s", ssidStr.c_str());
	      //snprintf(stationConf.password, sizeof(stationConf.password), "%s", passStr.c_str());
        
        //trick to save wifi creds to EEPROM
        wifi_station_get_config(&stationConf);
        debug(DCOMMAND,"new saved ssid|pass=|" + String((char*)stationConf.ssid) +"|" + String((char*)stationConf.password)+"|"); 
        wifi_station_set_config(&stationConf);
       
        return 1;
      }
      debug(DCOMMAND,"Can't find `|` symbol in wifi|pass format");
  } 

  if(cmdStr==CMD_MQTT_USER){
    if (set_settings_val_str(_s,cmdStr,valStr,_s->mqttUser,12))return 1;
  }
  if(cmdStr==CMD_MQTT_PASS){
    if (set_settings_val_str(_s,cmdStr,valStr,_s->mqttPass,22))return 1;
  }
    if(cmdStr==CMD_MQTT_SERVER){
    if (set_settings_val_str(_s,cmdStr,valStr,_s->mqttServer,22))return 1;
  }

  return 0;

};



int do_espboot_loop(WMSettings * _s){
  String inCommandStr=""; 
  bool stop_commnads=0;

  while (!stop_commnads){

    inCommandStr=get_command_str();
    
    debug(DENTER,0);
    
    if(inCommandStr!=""){
      debug(DCOMMAND, "Received incoming string->"+String(inCommandStr));
     
      //skip loop command
      if( inCommandStr.length()<=3 && inCommandStr.charAt(0)==SKIP_CHAR) {
        debug(DCOMMAND, "Command->"+String(inCommandStr.charAt(0))+"; Skip waiting command", TOUT);
        stop_commnads=1;
        return 0;
      }

     if(inCommandStr.length()>3 && inCommandStr.indexOf('=')==-1){
        //commands 
        debug(DCOMMAND,"command >" +String(inCommandStr) +"< incoming");
        if(inCommandStr.startsWith(CMD_CMD_HELP)){
          print_full_help();
        }

         if(inCommandStr.startsWith(CMD_CMD_PRINT)){
          print_curr_settings(_s);
        }
      
       //<parameter>=<value>
      }else if(inCommandStr.indexOf('=')!=-1){  

        String cmdStr=  inCommandStr.substring(0,inCommandStr.indexOf('='));
        String numStr = inCommandStr.substring(inCommandStr.indexOf('=')+1,inCommandStr.length());
        debug(DCOMMAND, "Command->"+ cmdStr+", Value->"+numStr);
        if(!do_set_command(_s, cmdStr, numStr)) {
          debug(DCOMMAND,"Wrong set parameter or value->"+cmdStr);
        }
      
      }else{
        debug(DCOMMAND, "Commnad is not recognized", TOUT);
      }
    
    }else{
      debug(DCOMMAND, "No incomming string");
      stop_commnads=1;
      return 0;
    }

  }
 if(stop_commnads) return 1; else return 0;
}

int correction_to_default_if_need(WMSettings *_s){
    int was_corrected=0;
    if(_s->mb_modbus_address>MAX_ID) {
      debug(DEEPROM, "Modbus address is corrected to default", TOUT);
      _s->mb_modbus_address=DEFAULT_ADDRESS;
      was_corrected=1;
    }
    if(_s->mb_intregs_amount<MIN_INT_REGS || _s->mb_intregs_amount>MAX_INT_REGS) {
      debug(DEEPROM, "Int Regs amount is corrected to default", TOUT);
      _s->mb_intregs_amount=DEFAULT_INT_REGS;
      return 1;
    }
    if(_s->mb_coilregs_amount<MIN_COIL_REGS || _s->mb_coilregs_amount>MAX_COIL_REGS){
      debug(DEEPROM, "COIL Regs amount is corrected to default", TOUT);
      _s->mb_coilregs_amount=DEFAULT_COIL_REGS;
      was_corrected=1;
    } 
    if(_s->mb_serial_baudrate<MIN_BAUDRATE || _s->mb_serial_baudrate > MAX_BAUDRATE){
      debug(DEEPROM, "Serial BAUDRATE is corrected to default", TOUT);
      _s->mb_serial_baudrate=DEFAULT_MB_RATE;
      was_corrected=1;
    }  
    if(_s->mb_serial_settings_num<0 || _s->mb_serial_settings_num > MAX_SERIAL_VAR_NUM) {
      debug(DEEPROM, "Serial SETTINGS NUM is corrected to default", TOUT);
      _s->mb_serial_settings_num=NSERIAL_8E1;
      was_corrected=1;
    }
    if(was_corrected)return 1;
    return 0;
}

#endif

