#ifndef dpublishermqtt_h
#define dpublishermqtt_h

//#include <TZ.h>
//#include <PubSubClient.h>
//#include <ESP8266WiFi.h>

#include "dcommands.h"
#include "dbase.h"
#include "dpublisher.h"

#include "dpubsetting.h"

#define MAX_CONNECT_ATTEMPTS_BEFORE_RESET 100
#define DDOS_MS 1000  //period in MS between to incoming commands



class DPublisherMqtt : public DPublisher
{
protected:
  int init_ok = 0;
  const int reconnect_period = 30;
  ulong last_connect_attempt;
  PubSubClient* _c1;
  int mqtt_not_configured=0;
  int time_synced=0;
  ulong m_incoming_ms=0;
  uint attempts=0;

  WiFiClient *_wifi_client;
  PubSubClient * _c;
    
public:
  DPublisherMqtt(WMSettings *__s, PubSubClient *__c, uint _ext_mqtt=1) : DPublisher(__s){
        
        if(_ext_mqtt){
          //get extarnal pointer to mqtt client
          _c=__c;  
        }else{
          //create instance of pubsub class inside (here)
          _wifi_client=new WiFiClient();
          _c = new PubSubClient();
          _c->setClient(*_wifi_client);
          using std::placeholders::_1;
          using std::placeholders::_2;
          using std::placeholders::_3;
          _c->setCallback(std::bind( &DPublisherMqtt::callback, this, _1,_2,_3));

        }

        if (String(_s->dev_id) == "empty_dev") mqtt_not_configured=1;
  };

  void init(Queue<pub_events> *_q){
      DPublisher::init(_q);
      configTime(_s->time_zone * 3600,  0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
      sync_time();
      try_connect();
      m_incoming_ms=millis();
      init_ok = 1;
    };

    int virtual is_connected()
    {
      //Serial.print(WiFi.status());
      if (mqtt_not_configured)
      {
        //Serial.println("CONNECT: not_configured dtected, dev_id empty");
        return 0;
      }
      if (_c->connected())
      {
        //debug("PUBLISHER", "MQTT CONNECTED");
      }
      else
      {
        // debug("PUBLISHER", "ERROR MQTT, ERR_NUM:");
        // debug("PUBLISHER", String(_c->state()));
      }

      if (_c->connected() && WiFi.status() == WL_CONNECTED)
        return 1;
      if (!_c->connected() && WiFi.status() == WL_CONNECTED)
        return 0; //later add what to do if mqtt disconnected
      else
        return 0;
    };


    int virtual sync_time(int forced=0){
        if(forced==0 && time_synced==1) return 0;

        time_t tnow = time(nullptr);
        struct tm * timeinfo;

        debug("TIMESYNC", "TIME FROM SERVER:"+String(ctime(&tnow)), DTINFO);

        timeinfo=localtime(&tnow);

        //setTime(tnow); //this is set wrong hour, without time zone

        //instead we use manual setTime
        // debug("TIMESYNC", "SET HOUR:"+String(timeinfo->tm_hour));
        // debug("TIMESYNC", "SET MIN:"+String(timeinfo->tm_min));
        // debug("TIMESYNC", "SET DAY:"+String(timeinfo->tm_mday));
        // debug("TIMESYNC", "SET YEAR:"+String(timeinfo->tm_year+1900));

        setTime(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year+1900);

        if(year()==1970){ //default date
          debug("TIMESYNC", "FAIL TO SYNC TIME :(", DTERROR);
          time_synced=0;
          return 0;
        }else{
          debug("TIMESYNC", "TIME SYNCED:"+String(d_hour())+":"+String(minute())+":"+String(year())+":"+String(month())+"; push event");
          que_wanted->push(PUBLISHER_WANT_SAY_JUST_SYNCED);
          time_synced=1;
          return 1;
        }
    };

   int virtual is_time_synced()
    {

      return time_synced;
    };
 
    int virtual try_connect(){

        debug("RECONNECT", "Try attemps:" + String(attempts));
        if(WiFi.status() == WL_CONNECTED){
            debug("RECONNECT", "WIFI CONNECTED", DTINFO);
            debug("RECONNECT",  WiFi.localIP(), DTINFO, "LOCAL IP");
            debug("RECONNECT",  WiFi.gatewayIP(), DTINFO, "GATEWAY");
            //debug("RECONNECT",  WiFi.dnsIP(), DTINFO, "DNS");
        }
       
        if(attempts>MAX_CONNECT_ATTEMPTS_BEFORE_RESET && _s->autoreboot_on_max_attempts) reset();
      
        if (WiFi.status() != WL_CONNECTED) {
            debug("RECONNECT", "NO WIFI CONNECTION->"+ String(WiFi.status()), DTERROR);
            return 0;
        }
        
        if (String(_s->dev_id) == "empty_dev") mqtt_not_configured=1; else mqtt_not_configured=0;
        if (mqtt_not_configured){
          debug("RECONNECTMQTT", "MQTT IS NOT CONFIGRED (maybe dev_id=empty_dev)");
          return 0;
        }
        /*
        //4if dev_id is not configured, exit
        if(String(_s->dev_id) == "empty_dev"){
          debug("RECONNECTMQTT", "MQTT DEV is empty", DTERROR, "MQTT_IP");
          return 0;
        }

        */

        //check if name of mqtt correct
        IPAddress result;
        int err = WiFi.hostByName(_s->mqttServer, result) ;
       
        if(err !=1){
          debug("RECONNECTMQTT", "Cant resolve mqtt server");
          debug("RECONNECTMQTT", WiFi.dnsIP(), DTERROR, "DNS");
          return 0;
        }else{ 
          debug("RECONNECTMQTT", result, DTINFO, "MQTT_IP");
        }
      

        int port = atoi(_s->mqttPort);
        _c->setServer(_s->mqttServer, port);

        debug("PUBLISHER", "Attempting MQTT...");
        // Create a random client ID
        String clientId = "donoff-" + String(_s->dev_id);
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        debug("PUBLISHER", "Server=" + String(_s->mqttServer) + "| User:" + String(_s->mqttUser) + "| Pass:" + String(_s->mqttPass) + "| port=" + String(_s->mqttPort));
        //if (client.connect(clientId.c_str(),"xvjlwxhs","_k7d9m2yt0hn")) {
        if (_c->connect(clientId.c_str(), _s->mqttUser, _s->mqttPass)){
          debug("PUBLISHER", "MQTT CONNECTED", DTINFO);
          attempts=0;
          subscribe_all();
        }else{
          debug("PUBLISHER", "MQTT FAILED TO CONNECT", DTERROR);
        }
        return 1;   
        
    };

    String form_full_topic(const char* short_topic){
        return "/"+String(_s->mqttUser)+"/"+String(_s->dev_id)+String(short_topic);
    };


    String form_full_topic(String short_topic){
        return "/"+String(_s->mqttUser)+"/"+String(_s->dev_id)+short_topic;
    };


    void subscribe(const char* short_topic){
        
        String formed_topic;
        formed_topic="/"+String(_s->mqttUser)+"/"+String(_s->dev_id)+String(short_topic);
        //debug("MQTT", "Subscribe:"+formed_topic);
        _c->subscribe(formed_topic.c_str());
    };

    void subscribe_all(){
        subscribe(PARAMS_CHANNEL);
    };

    void reconnect(){
        if(millis() - last_connect_attempt > reconnect_period * 1000){
          if (!is_connected()){
            attempts++;
            try_connect();
          }
          if(!is_time_synced()){
            sync_time();
          }

          last_connect_attempt = millis();

        }
      
    };

    void mqtt_loop(){
        if (_c->connected()) _c->loop();
    }

    void callback(char* topic, byte* payload, unsigned int length){
        //debug("CALLBACK", String(topic)+"::"+String((char*)payload));
        char message[100];
        
        //antispam, check time between commands
        if(millis()-m_incoming_ms<DDOS_MS){
          publish_to_info_topic("DDOS CHECK");
          m_incoming_ms=millis();
          return;
        }
        m_incoming_ms=millis();


        String inS="";
        for (int i = 0; i < length; i++)
        {
          inS = inS + (char)payload[i];
        }
        //inS = String(message);
        //inS=inS.substring(0, length);
        String topic_params_full = form_full_topic(PARAMS_CHANNEL);
        if (String(topic) == topic_params_full) {
          debug("CALLBACK_PARAMS", "Params incoming msg is detected-->"+inS);
          recognize_incoming_str(inS);
          
        }

    };

    int virtual publish_sh_to_info_topic(String shStr, String _valStr)
    {
      if (!is_connected())
        return 0;
      
      String sht=form_full_topic(INFO_CHANNEL);
      debug("PUBLISH_SH_INFO","TOPIC="+sht);
      _c->publish(form_full_topic(INFO_CHANNEL).c_str(), (shStr+"="+_valStr).c_str());
      return 1;

    };

    int virtual publish_to_info_topic(String _valStr)
    {
      if (!is_connected()) return 0;

      // String sht=form_full_topic(INFO_CHANNEL);
      // debug("PUBLISH_CLEAR_INFO","TOPIC="+sht);
     _c->publish(form_full_topic(INFO_CHANNEL).c_str(), _valStr.c_str());
     return 1;
    };

    int virtual publish_to_log_topic(String _valStr)
    {
      debug("PUBLISH_LOG", "Publish to log topic->"+form_full_topic(LOG_CHANNEL)+", msg:"+_valStr);
      if (!is_connected()) return 0;
      _c->publish(form_full_topic(LOG_CHANNEL).c_str(), _valStr.c_str());
      return 1;

    };

    int virtual publish_uptime()
    {
      if (!is_connected()) return 0;
      _c->publish(form_full_topic(UPTIME_CHANNEL).c_str(), get_time_str(millis()).c_str());
      return 1;

    };

    int virtual publish_to_topic(String _topic, String _valStr){
        _c->publish(form_full_topic(_topic).c_str(), _valStr.c_str());
        return 1;
    };

  };



/*void pubsub_callback(char* topic, byte* payload, unsigned int length) {
    DPublisherMqtt::callback(topic, payload,length);
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i=0;i<length;i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
 }
 */


#endif