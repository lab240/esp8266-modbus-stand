#ifndef __mbpublish1__
#define __mbpublish1__


#include "../donofflib/dbase.h"


class DBootA : public DBase
{
protected:
  int init_ok = 0;
  const int reconnect_period = 30;
  ulong last_connect_attempt;
  PubSubClient* _c;
  int not_configured=0;
  int time_synced=0;
  ulong m_incoming_ms=0;
  uint attempts=0;
  uint was_init=0;

public:
   DBootA(WMSettings * __s): DBase(__s) {};
  
   void init(){
    load();
    debug(DSEEPROM, "Rad setings from EEPROM, currect Salt="+ String(EEPROM_SALT)+ " EEPROM SALT=" + String(_s->salt));
    was_init=1;
   };

   

    void virtual print_curr_settings(WMSettings *_s){
/*        
        debug(DSHELP, "custom_level1->"+String(_s->custom_level1), TOUT);
        debug(DSHELP, "custom_Level2->"+String(_s->custom_level2), TOUT);
        debug(DSHELP, "custom_level3->"+String(_s->custom_level3), TOUT);
        debug(DSHELP, "customlevel4->"+String(_s->custom_level4), TOUT);
        debug(DSHELP, "custom_level_notify1->"+String(_s->custom_level_notify1), TOUT);
 */
        debug(DSCOMMAND,"WIFI CREDS(ssid|pass)->" + String((char*)stationConf.ssid) +"|" + String((char*)stationConf.password)); 
        debug(DSHELP, "MQTT SERVER->"+String(_s->mqttServer));
        debug(DSHELP, "MQTT USER->"+String(_s->mqttUser));
        debug(DSHELP, "MQTT PASS->"+String(_s->mqttPass));

        }

    void print_welcome_help(){
        debug(DSHELP, "-------------- Avaliable commands (wait for "+String(NUM_TRY)+" secs) -----------------");
        debug(DSHELP, "Press <C+Enter> to skip Dboot"
        debug(DSHELP, "help - get full help");

        debug(DSHELP, "MQTT SERVER->"+String(_s->mqttServer));
        debug(DSHELP, "MQTT USER->"+String(_s->mqttUser));
        debug(DSHELP, "MQTT PASS->"+String(_s->mqttPass));
    }

    void print_full_help(){
        debug(DSHELP, "-------------- Avaliable commands (wait for "+String(NUM_TRY)+" secs) -----------------");
        debug(DSHELP, "Press <C+Enter> to skip Dboot");
        debug(DSHELP, String(CMD_SSID) + "=<SSID|PASSWORD> wifi creds");
        debug(DSHELP, String(CMD_MQTT_SERVER) + "=<mqtt_serer>, "+ String(CMD_MQTT_USER)+"=<mqtt_user>, "+String(CMD_MQTT_PASS)+"=<mqtt_pass>, "+String(CMD_MQTT_PORT)+"=<mqtt_port>");
        }


    //get command from terminal
    String read_command_from_serial_str(){
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
    };


    int do_espboot_loop(){
        String inCommandStr=""; 
        bool stop_commnads=0;

        while (!stop_commnads){

            inCommandStr=get_command_str();
            
            debug(DSENTER,0);
            
            if(inCommandStr!=""){
            debug(DSCOMMAND, "Received incoming string->"+String(inCommandStr));
            
            //skip loop command
            if( inCommandStr.length()<=3 && inCommandStr.charAt(0)==SKIP_CHAR) {
                debug(DSCOMMAND, "Command->"+String(inCommandStr.charAt(0))+"; Skip waiting command", TOUT);
                stop_commnads=1;
                return 0;
            }

            if(inCommandStr.length()>3 && inCommandStr.indexOf('=')==-1){
                //commands 
                debug(DSCOMMAND,"command >" +String(inCommandStr) +"< incoming");
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
                debug(DSCOMMAND, "Command->"+ cmdStr+", Value->"+numStr);
                if(!do_set_command(_s, cmdStr, numStr)) {
                debug(DSCOMMAND,"Wrong set parameter or value->"+cmdStr);
                }
            
            }else{
                debug(DSCOMMAND, "Commnad is not recognized", TOUT);
            }
            
            }else{
                debug(DSCOMMAND, "No incomming string");
                stop_commnads=1;
                return 0;
            }

        }
        if(stop_commnads) return 1; else return 0;
    };


    int do_set_command(WMSettings *_s, String cmdStr, String valStr){
      
        if (cmdStr == CMD_SSID){
            debug(DSCOMMAND, "Wifi Creds, push ssid="+valStr);
            
            if(valStr.indexOf('|')!=-1){  
                String ssidStr=  valStr.substring(0,valStr.indexOf('|'));
                String passStr = valStr.substring(valStr.indexOf('|')+1,valStr.length());
                debug(DSCOMMAND, "Command->"+ ssidStr+", Value->"+passStr);
                debug(DSCOMMAND, "Writing creds...");

                //wifi begin without connect
                WiFi.begin(ssidStr.c_str(), passStr.c_str(), 0, NULL, false);
                struct station_config stationConf;
                 
                //trick to save wifi creds to EEPROM
                wifi_station_get_config(&stationConf);
                debug(DSCOMMAND,"new saved ssid|pass=|" + String((char*)stationConf.ssid) +"|" + String((char*)stationConf.password)+"|"); 
                wifi_station_set_config(&stationConf);
            
                return 1;
            }
            debug(DSCOMMAND,"Can't find `|` symbol in wifi|pass format");
            return 0;
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

        if(cmdStr==CMD_MQTT_DEV){
            if (set_settings_val_str(_s,cmdStr,valStr,_s->dev_id,10))return 1;
        }

        return 0;  
    }; 

};



#endif

