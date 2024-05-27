#ifndef donoffsupply_h
#define donoffsupply_h

#include <queue.h>
#include "dpublishmqtt.h"

//#include "dqueue.h"

#if !defined(DOFFLINE)
    #define DOFFLINE 0
#endif

#define DEVICE_VERBOSE 1


#define MAX_LOOPS 10
#define MAX_SENSORS 10
#define MAX_LOOP_COUNTER 30
#define MS_LOOP_TIMING 200
#define MAX_SLOW_LOOP_COUNTER 600

class DDevice: public DBase {
  private:
  
    
 protected:
    String reasonStr = "";
    uint8_t  blink_loop = 0;
    uint current_blink_type = 0;


    int numrelays = 0;
   
    uint mycounter = 0;
    uint mycounter2=0;
    ulong mytimer = 0;
    int init_ok = 0;
    int m_just_synced=0;
    pub_events what_to_want;

    Queue<pub_events>* que_wanted;
    
    DPublisherMqtt* pub;
   
  public:
    DDevice(WMSettings * __s): DBase(__s) {};

    void init(DPublisherMqtt* _pub, Queue<pub_events>* _q) {

      pub = _pub;
      que_wanted=_q;
      mytimer = millis();
      init_ok = 1;

    };

    void supply_loop() {

      if ((millis() - mytimer) >= MS_LOOP_TIMING ) {
        //6 sec loop
        if(mycounter >=0 && mycounter<=MAX_LOOPS){
           //debug("SUPPLY_LOOP", "SLOW LOOP");
           slow_loop(mycounter);
           
        }

        if(mycounter >MAX_LOOPS && mycounter <=MAX_LOOPS+MAX_SENSORS){
          //debug("SUPPLY_LOOP", "SENSORS LOOP");
          sensors_loop(mycounter-MAX_LOOPS);
        }

        if(mycounter==MAX_LOOP_COUNTER) {
          //debug("VERY_SLOWLOOP", "counter="+String(mycounter2));
          very_slow_loop(mycounter2);
          mycounter2++;
        }

        mycounter++;
        //300 ms loop
       
        fast_loop();
        pub_wanted_loop();
               
        if (mycounter > MAX_LOOP_COUNTER) {
          mycounter = 0;
          if(m_just_synced==1) m_just_synced=0;
        }
        mytimer = millis();

      }

      if(mycounter2>MAX_SLOW_LOOP_COUNTER){
        mycounter2=0;
      }

      native_loop();
      pub->mqtt_loop();

    };

    void virtual one_minute_loop(){};
    void virtual ten_minutes_loop(){};
    void virtual thirty_minutes_loop(){};
    void virtual one_hour_loop(){};

    void virtual very_slow_loop(int counter){
          
          if(counter % 10 == 0 && counter!=0){ 
              if(DEVICE_VERBOSE) debug("SLOWLOOP", "1 MINUTE, counter="+String(counter));
              one_minute_loop();
          }


          if(counter % 100 == 1 && counter!=0){   
              if(DEVICE_VERBOSE) debug("SLOWLOOP", " 10 MINUTES, counter="+String(counter));
              ten_minutes_loop();
          }

          if((counter == 302 || counter==598) && counter!=0){           
              if(DEVICE_VERBOSE) debug("SLOWLOOP", "30 MINUTES, counter="+String(counter));
              thirty_minutes_loop();
          }

          if(counter==599 && counter!=0){           
              if(DEVICE_VERBOSE) debug("SLOWLOOP", " one_hour, counter="+String(counter));
              one_hour_loop();
          }

    };

    void virtual slow_loop(int mycounter){
        if (mycounter == 0) {
          //debug("SUPPLY_LOOP", "Reconnect loop");
          if(!DOFFLINE) reconnect_loop();
        }

        if (mycounter == 1) {
          service_loop();
        }

        if(mycounter==2){
          notifyer_loop();
        }

        if(mycounter==3){
          display_loop();
        }
    };

    int virtual notifyer_loop(){
      if(!_s->notifyer) return 0;
      return 1;
    };

    void virtual display_loop(){};

    int virtual pub_wanted_loop(){
      //debug("SUPPLY_QUEUE", "Loop queue wanted");

      if (que_wanted->count()==0) return 0;

      if(DEVICE_VERBOSE) debug("SUPPLY_QUEUE", "WANTED EVENT DETECTED:"+String(what_to_want));
      what_to_want=que_wanted->pop();

      if(what_to_want==PUBLISHER_WANT_SAY_JUST_SYNCED){
        m_just_synced=1;
      }

      do_want_event();
      return 1;
      
    };

    void virtual do_want_event(){
        if(what_to_want==PUBLISHER_WANT_SAVE){
        save();
        pub->publish_to_info_topic("N: saved");
      }
      
      if(what_to_want==PUBLISHER_WANT_RESET) {
        pub->publish_to_info_topic("I: OK, LET'S RESET");
        reset();
      }
    };
    
    void virtual sensors_loop(int mycounter){};
    void virtual fast_loop() {};
    void virtual native_loop() {};


    void reconnect_loop() {
      if (!pub->is_connected() || !pub->is_time_synced()) {
        pub->reconnect();
      }
    };

    void virtual service_loop() {

      if(DEVICE_VERBOSE) 
       debug("SHEDULER", "**Service loop->TIMESTAMP=" +  s_get_timestamp('<','>') + ", t_sync=" + String(pub->is_time_synced())+ 
           ", user="+ String(_s->mqttUser)+", dev_id=" + String(_s->dev_id)+" ,online="+String(pub->is_connected())+
           ", size_s="+String(sizeof(*_s))
       );

      if(pub->is_connected()){
        pub->publish_uptime();
        pub->publish_json();
      } 
      
   
    };

   
};





#endif