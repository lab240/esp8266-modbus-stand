#ifndef dpublisher_h
#define dpublisher_h


//#include <PubSubClient.h>
//#include <ArduinoJson.h>
#include "dcommands.h"
#include "dbase.h"
#include <Queue.h>
#include <TimeLib.h>


class DPublisherA: public DBase {
  protected:

    //String shStr;
    //WMSettings * _s;

    String incomingStr;
    int is_val_present = 0;
    int is_sh_present = 0;

    String shStr = "";
    String valStr = "";
    String cmdStr = "";

    String channel_uptime_str="0";

    int once_connected=0;

public:
    /* 
    int wanted_reset_hour_r1=0;
    int wanted_reset_hour_r1_m=0;
    int wanted_reset_hour_r2=0;
    int wanted_r1_on=0;
    int wanted_r1_off=0;
    int wanted_r2_on=0;
    int wanted_r2_off=0;
    int wanted_save=0;
    int wanted_reset=0;
    int wanted_r1_off_lschm0=0;
    */
    Queue<pub_events>* que_wanted;
   


  public:
    DPublisherA(WMSettings * __s): DBase(__s) {};

     void init( Queue<pub_events>* _q) {
      que_wanted= _q;
     }


    //int virtual publish_sensor(String channelStr, String dataStr){};
    
    int virtual publish_uptime()=0;
    
    int virtual publish_to_log_topic(String _valStr)=0;

    int virtual publish_to_topic(String _topic, String _valStr)=0;

    

    int virtual is_connected()=0;
    int virtual is_time_synced()=0;

    void virtual reconnect()=0;


    int virtual publish_sh_err(){
      if (!is_connected())
        return 0;    
      publish_to_info_topic("E: sh param not recognized");
      return 1;
    };

    void recognize_incoming_str(String _incomingStr) {
      bool autosave=0;
      int index_c=_incomingStr.indexOf(";");
      if (index_c != -1) {
        autosave=1;
        _incomingStr=_incomingStr.substring(0,_incomingStr.length()-1);
      }

      debug("PUBLISH_RECOGNIZE", "IncomingString"+incomingStr+", autosave:" + String(autosave));
      int  index_val = _incomingStr.indexOf("=");
      if (index_val == -1) is_val_present = 0; else is_val_present = 1;
      is_sh_present = _incomingStr.startsWith("sh ");

      //debug("RECOGNIZE", "is_val=" + String(is_val_present) + " is_sh=" + String(is_sh_present));
      if (is_val_present) {
        cmdStr = _incomingStr.substring(0, index_val);
        valStr = _incomingStr.substring(index_val + 1, _incomingStr.length());

        if(!set_parameters_loop()) publish_to_info_topic("E:Varable error");

        //debug("RECOGNIZE", "command=" + cmdStr + "; value=" + valStr);
        if(autosave) que_wanted->push(PUBLISHER_WANT_SAVE);
      }
      else if (is_sh_present) {

        shStr = _incomingStr.substring(3, _incomingStr.length());

        if(!show_parameters_loop()) publish_sh_err();
        //debug("RECOGNIZE", "sh val=" + shStr);
        
      }else {
        if(!cmd_loop(_incomingStr)) publish_to_info_topic ("no command");
      }
    };


    int virtual cmd_loop(String inS){
                  
        if (inS== D_SAVE) {
              que_wanted->push(PUBLISHER_WANT_SAVE);
              return 1;
        }
            
        if (inS==D_RESET) {
            que_wanted->push(PUBLISHER_WANT_RESET);
            return 1;
        }

        return 0;
            
    };


    int virtual show_parameters_loop() {

      if (is_sh_present == 0) {
        debug("SHOWPARAMSLOOP", "no sh recognized");
        return 0;
      }

      debug("SHOWPARAMS", shStr);

      if (shStr == C_CUSTOM_LEVEL1) {
        publish_sh_to_info_topic( shStr, String(_s->custom_level1));
        return 1;
      }

      if (shStr == C_CUSTOM_LEVEL2) {
        publish_sh_to_info_topic( shStr, String(_s->custom_level2));
        return 1;
      }

      if (shStr == C_CUSTOM_LEVEL3) {
        publish_sh_to_info_topic( shStr, String(_s->custom_level3));
        return 1;
      }

      if (shStr == C_CUSTOM_LEVEL4) {
        publish_sh_to_info_topic( shStr, String(_s->custom_level4));
        return 1;
      }

      if (shStr == C_CUSTOM1_NOTIFY_LEVEL) {
        publish_sh_to_info_topic( shStr, String(_s->custom_level_notify1));
        return 1;
      }

      if (shStr == C_CUSTOM2_NOTIFY_LEVEL) {
        publish_sh_to_info_topic( shStr, String(_s->custom_level_notify2));
        return 1;
      }
      
      if (shStr == C_EMAIL) {
        publish_sh_to_info_topic( shStr, String(_s->email_notify));
        return 1;
      }

      if (shStr == I_NET) {
        int rssdb = WiFi.RSSI();
        String ssid = WiFi.SSID();
        String ipstring = WiFi.localIP().toString();
        //Serial.println("rssdb="+String(rssdb)+", SSID="+String(ssid)+" ,ip="+ipstring);
        String outS = "SSID=" + ssid + ",rss=" + String(rssdb) + ",ip=" + String(ipstring);
        publish_to_info_topic(outS);
        return 1;
      }

      if (shStr == I_IP) {
        publish_to_info_topic( WiFi.localIP().toString());
        return 1;
      }

      if (shStr == I_TIME) {
         publish_to_info_topic("I:S="+String(is_time_synced())+",T="+String(hour())+":"+String(minute())+":"+String(year()));
         return 1;
      }

      if (shStr == I_SALT) {
         publish_sh_to_info_topic( shStr, String(_s->salt));
         return 1;
      }
      
      return 0;

    };

    int virtual set_parameters_loop() {

      if (is_val_present == 0) {
        debug("SETPARAMSLOOP", "no cmd=val recognized");
        return 0;
      }

      //String _cStr=cmdStr;
      //String _vStr=valStr;

      if (cmdStr == C_TEST_B) {
        bool a;
        set_settings_val_bool(cmdStr, valStr, &a);
        return 1;

      }
      if (cmdStr == C_TEST_I) {
        int i;
        set_settings_val_int( cmdStr, valStr, &i, -30, 10);
        return 1;
      }

      if (cmdStr == C_CUSTOM_LEVEL1) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level1, MIN_CUSTOM_LEVEL, MAX_CUSTOM_LEVEL);
        return 1;
      }

      if (cmdStr == C_CUSTOM_LEVEL2) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level2, MIN_CUSTOM_LEVEL, MAX_CUSTOM_LEVEL);
        return 1;
      }

      if (cmdStr == C_CUSTOM_LEVEL3) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level3, MIN_CUSTOM_LEVEL, MAX_CUSTOM_LEVEL);
        return 1;
      }

      if (cmdStr == C_CUSTOM_LEVEL4) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level4, MIN_CUSTOM_LEVEL, MAX_CUSTOM_LEVEL);
        return 1;
      }

      if (cmdStr == C_CUSTOM1_NOTIFY_LEVEL) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level_notify1, -128, MAX_NOTIFY_CUSTOM);
        return 1;
      }

      if (cmdStr == C_CUSTOM2_NOTIFY_LEVEL) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level_notify2, -128, MAX_NOTIFY_CUSTOM);
        return 1;
      }

      if (cmdStr == C_CUSTOM3_NOTIFY_LEVEL) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level_notify3, -128, MAX_NOTIFY_CUSTOM);
        return 1;
      }

      if (cmdStr == C_CUSTOM4_NOTIFY_LEVEL) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level_notify4, -128, MAX_NOTIFY_CUSTOM);
        return 1;
      }

      return 0;

    };
/*
    int commands_loop(){
      
      if (incomingStr == C_CUSTOM4_NOTIFY_LEVEL) {
        set_settings_val_int( cmdStr, valStr, (int*) &_s->custom_level_notify4, -128, MAX_NOTIFY_CUSTOM);
        return 1;
      }
    };
*/
    int set_settings_val_bool(String _command, String _valStr, bool* _setting_val) {
      bool recognize = 0;
      if (_valStr.startsWith("0") && _valStr.length() == 1) {
        *_setting_val = 0;
        recognize = 1;
      }
      if (_valStr.startsWith("1") && _valStr.length() == 1) {
        *_setting_val = 1;
        recognize = 1;
      }
      if (recognize) {
        publish_to_info_topic(String("N:" + _command + "=" + String(*_setting_val)).c_str());
        return 1;
      }
      publish_to_info_topic("E:not set, 01 wait");
      return 0;
    };

    int set_settings_val_int(String _command, String _valStr, int* _setting_val,  int _min =0, int _max=255) {
      int test_val;
      bool recognize = 0;
      if (_valStr.startsWith("0") && _valStr.length() == 1) {
        test_val = 0;
        recognize = 1;
      }
      test_val = _valStr.toInt();
      if (recognize == 0 && test_val != 0) recognize = 1;
      if (recognize == 0) {
        publish_to_info_topic("E:not set, INT wait");
        return 0;
      }

      if (test_val >= _min && test_val <= _max) {
        *_setting_val = test_val;
        publish_to_info_topic(String("N:" + _command + "=" + String(test_val)).c_str());
        return 1;
      }

      publish_to_info_topic(String("E:expected " + String(_min) + "<>" + String(_max)).c_str());
      return 0;

    };

    int virtual publish_sh_to_info_topic(String shStr, String _valStr) =0;

    int virtual publish_to_info_topic(String _valStr) =0;

    void virtual clear_info_channel(){
      publish_to_info_topic("                          ");
      
    };

};

#endif
