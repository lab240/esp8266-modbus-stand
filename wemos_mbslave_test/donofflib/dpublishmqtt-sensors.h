#ifndef dpublishermqttsensors_h
#define dpublishermqttsensors_h

#include "dpublishermqtt.h"


class DPublisherMqttSensors : public DPublisherMqtt
{
protected:
 
    
public:
  DPublisherMqttSensors(WMSettings *__s, PubSubClient *__c, uint _ext_mqtt=1): DPublisherMqtt(__s,__c, _ext_mqtt){
  };

  void init(Queue<pub_events> *_q){
      init_ok=0;
      DPublisherMqtt::init(_q);
      init_ok = 1;
    };


void virtual override publish_sensors(){
  publish_ds1820(vsensor* _vsensor);

} 

void virtual publish_ds1820(vsensor* _vsensor){
  
}

};

#endif