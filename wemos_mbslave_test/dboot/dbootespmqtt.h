#ifndef __dbootespmqtt__
#define __dbootespmqtt__


#include "dbootesp.h"
#include <ESP8266WiFi.h>



class DBootEspMqtt : public DBootEsp
{
protected:

public:
   DBootEspMqtt(WMSettings * __s): DBootEsp(__s) {};
  
   void virtual init() override{
    DBootEsp::init();
    set_defaults_if_need();
   };

   int virtual set_defaults_if_need() =0; //change it if SALT is wrong and you need set default values

   void virtual print_curr_settings() override {

        DBootEsp::print_curr_settings();
    
        debug(DSHELP, "MQTT SERVER->"+String(_s->mqttServer));
        debug(DSHELP, "MQTT USER->"+String(_s->mqttUser));
        debug(DSHELP, "MQTT PASS->"+String(_s->mqttPass));
    
    };

    void virtual print_welcome_help() override{
        DBootEsp::print_welcome_help();

        debug(DSHELP, "MQTT SERVER->"+String(_s->mqttServer));
        debug(DSHELP, "MQTT USER->"+String(_s->mqttUser));
        debug(DSHELP, "MQTT PASS->"+String(_s->mqttPass));
    };

    void virtual print_full_help() override{
        DBootEsp::print_full_help();

        debug(DSHELP, String(CMD_MQTT_SERVER) + "=<mqtt_serer>, "+ String(CMD_MQTT_USER)+"=<mqtt_user>, "+String(CMD_MQTT_PASS)+"=<mqtt_pass>, "+String(CMD_MQTT_PORT)+"=<mqtt_port>");
    };

      int do_set_command(String cmdStr, String valStr){
        
        if(DBootEsp::do_set_command(cmdStr,valStr)) return 1;

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

