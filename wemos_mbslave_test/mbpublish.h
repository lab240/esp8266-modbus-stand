#ifndef __mbpublish1__
#define __mbpublish1__

#include "mbcommands.h"
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


};

#endif