#ifndef __mqttsens__
#define __mqttsens__

#include "../dmbsensor/vsensora.h"
#include "../donofflib/dpublishmqtt.h"

char* const CHANNEL1="ds_out";

class mqtt_sensor_ds1820: public DBase {
  private:
         
  protected:

   uint hregs_amount;
   uint cregs_amount;
   uint init_ok=0;
   vector_sensor* _vsensor=nullptr;
   DPublisherMqtt* pub;
   

  public:
    mqtt_sensor_ds1820(WMSettings * __s, PublisherMqtt* _pub, vector_sensor* __vsensor): DBase(__s) {
        _mb=__mb;
        _vsensor=__vsensor;
    };

    void virtual publish_mqtt_sensor(){
        long value=_vsensor->get_value();
        debug("MQTTREGS", "main_sensor_val="+String(value));
        pub->publish_to_topic(String(CHANNEL1),String(value));
    };
   
};

#endif