#ifndef __dbootwithwifi__
#define __dbootwithwifi__


#include "dboot.h"
#include <ESP8266WiFi.h>



class DBootWithWifi : public DBootA
{
protected:

public:
   DBootWithWifi(WMSettings * __s): DBootA(__s) {};
  
   void init(){
    DBootA::init();
    was_init=1;
   };

   

    void virtual print_curr_settings() override {
        struct station_config stationConf;
        wifi_station_get_config (&stationConf);
        debug(DSCOMMAND,"WIFI CREDS(ssid|pass)->" + String((char*)stationConf.ssid) +"|" + String((char*)stationConf.password)); 
        debug(DSHELP, "MQTT SERVER->"+String(_s->mqttServer));
        debug(DSHELP, "MQTT USER->"+String(_s->mqttUser));
        debug(DSHELP, "MQTT PASS->"+String(_s->mqttPass));

    }

    void virtual print_welcome_help() override{
        DBootWithWifi::print_welcome_help();

        debug(DSHELP, "MQTT SERVER->"+String(_s->mqttServer));
        debug(DSHELP, "MQTT USER->"+String(_s->mqttUser));
        debug(DSHELP, "MQTT PASS->"+String(_s->mqttPass));
    }

    void virtual print_full_help() override{
        DBootWithWifi::print_full_help();

        debug(DSHELP, String(CMD_SSID) + "=<SSID|PASSWORD> wifi creds");
        debug(DSHELP, String(CMD_MQTT_SERVER) + "=<mqtt_serer>, "+ String(CMD_MQTT_USER)+"=<mqtt_user>, "+String(CMD_MQTT_PASS)+"=<mqtt_pass>, "+String(CMD_MQTT_PORT)+"=<mqtt_port>");
    }

    
   int virtual do_set_command(String cmdStr, String valStr) override {
      
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
            if (set_settings_val_str(cmdStr,valStr,_s->mqttUser,12))return 1;
        }

        if(cmdStr==CMD_MQTT_PASS){
            if (set_settings_val_str(cmdStr,valStr,_s->mqttPass,22))return 1;
        }

        if(cmdStr==CMD_MQTT_SERVER){
            if (set_settings_val_str(cmdStr,valStr,_s->mqttServer,22))return 1;
        }

        if(cmdStr==CMD_MQTT_DEV){
            if (set_settings_val_str(cmdStr,valStr,_s->dev_id,10))return 1;
        }

        return 0;  
    }; 

};



#endif

