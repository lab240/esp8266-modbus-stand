#ifndef __dbootesp__
#define __dbootesp__


#include "dboot.h"
#include <ESP8266WiFi.h>



class DBootEsp : public DBootA
{
protected:

public:
   DBootEsp(WMSettings * __s): DBootA(__s) {};
  
   void virtual init() override{
    DBootA::init();
    set_defaults_if_need();
   };

   int virtual set_defaults_if_need()=0; //change it if SALT is wrong and you need set default values


   void virtual print_curr_settings() override {

        DBootA::print_curr_settings();
    
        struct station_config stationConf;
        wifi_station_get_config (&stationConf);
        debug(DSHELP, "WIFI CREDS(ssid|pass)->" + String((char*)stationConf.ssid) +"|" + String((char*)stationConf.password)); 
   
    };

    void virtual print_welcome_help() override{
        DBootA::print_welcome_help();

    };

    void virtual print_full_help() override{
        DBootA::print_full_help();

        debug(DSHELP, String(CMD_SSID) + "=<SSID|PASSWORD> wifi creds");
    };

    
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

        return 0;  
    }; 

};



#endif

