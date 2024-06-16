#ifndef __mbpublish1__
#define __mbpublish1__

#include "mbcommands.h"
#include "mbserial.h"
#include "mbsettings.h"
#include "donofflib/dpublishmqtt.h"

class DPublisherMqttMBstand : public DPublisherMqtt
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

public:
  DPublisherMqttMBstand(WMSettings *__s, PubSubClient *__c, uint _ext_mqtt=1) : DPublisherMqtt(__s, __c, _ext_mqtt){
        
  };

  
  void init(Queue<pub_events> *_q){
      DPublisherMqtt::init(_q);
      init_ok = 1;
    };


  int virtual show_parameters_loop() override {

      if(DPublisherMqtt::show_parameters_loop()){
        //debug("PUBLISH", "Mother class returns 1");
        return 1;
      }

      if (shStr == C_HOLDREG) {
        publish_sh_to_info_topic( shStr, String(_s->mb_intregs_amount));
        return 1;
      }

      if (shStr == C_COILS) {
        publish_sh_to_info_topic( shStr, String(_s->mb_coilregs_amount));
        return 1;
      }
      
      if (shStr == C_MBADDRESS) {
        publish_sh_to_info_topic( shStr, String(_s->mb_modbus_address));
        return 1;
      }
      
      if (shStr == C_SERIALPORT) {
        String resultStr=String(_s->mb_serial_baudrate)+"|";
        resultStr+=get_serial_settings_string(_s->mb_serial_settings_num);
        publish_sh_to_info_topic( shStr, resultStr);
        return 1;
      }

      return 0;
      
    }

    int virtual publish_custom_help() override {
      if (!is_connected()) return 0;    
      publish_to_info_topic("H:"+String(C_HOLDREG)+"|"+String(C_COILS)+"|"+String(C_MBADDRESS)+"|"+String(C_SERIALPORT)+"|"+String(C_SERIAL_BAUDRATE));
      return 1;
    };

   
   //return serialized String for /out/json channel
   String virtual form_json_channel_string() override{
      StaticJsonDocument <200> root;
      String jsonStr="";
      root["dev"] = _s->dev_id;
      root["user"] = _s->mqttUser; 
      root["mbaddress"] = _s->mb_modbus_address; 
      root["mbhregs"] = _s->mb_intregs_amount;
      root["mbcoils"] = _s->mb_coilregs_amount;
      root["baudrate"] = _s->mb_serial_baudrate;
      root["serialsettings"] = get_serial_settings_string(_s->mb_serial_settings_num);
      root["timestamp"] = s_get_timestamp();

      serializeJson(root, jsonStr);
      debug("JSON", "Serialise:"+jsonStr);
      return jsonStr;
   }

   //end class
};




#endif