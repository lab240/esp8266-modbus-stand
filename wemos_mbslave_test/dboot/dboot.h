#ifndef __dboot__
#define __dboot__


#include "../donofflib/dbase.h"

#define DEFAULT_NUM_TRYES 10   //waiting command pause

class DBootA : public DBase
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
  uint was_init=0;

  //number of seconds when we are waitng the command
  uint num_tryes=DEFAULT_NUM_TRYES;

public:
   DBootA(WMSettings * __s): DBase(__s) {
   };
  
    void virtual init(){
        debug(DSEEPROM,"Loading EEPROM...");
        load();
        debug(DSEEPROM, "Read setings from EEPROM, currect Salt="+ String(EEPROM_SALT)+ " EEPROM SALT=" + String(_s->salt));
        was_init=1;
    };


    int virtual do_set_command(String cmdStr, String valStr)=0;

    void virtual print_curr_settings(){
        debug(DSHELP, "-------------- Current Settings -----------------");
    };

    void virtual print_welcome_help(){
        debug(DSHELP, "-------------- Avaliable commands (wait for "+String(num_tryes)+" secs) -----------------");
        debug(DSHELP, "Press <C+Enter> to skip Dboot");
        debug(DSHELP, "help - get full help");
    };

    void virtual print_full_help(){
        debug(DSHELP, "-------------- Avaliable commands (wait for "+String(num_tryes)+" secs) -----------------");
        debug(DSHELP, "Press <C+Enter> to skip Dboot");
    };

    //get command from terminal
    String read_command_from_serial_str(){
        int is_cmd=0;
        //int is_cmd_l=0;
        int count_try=0;
        uint32_t softTimer = 0; 
        char inCommandChr[50] = {0};    // incomming array of chars
        uint8_t counterByte = 0;      //  byte pointer

        while(is_cmd==0 && count_try < num_tryes){  // waiting command loop

            Serial.print(count_try,DEC);
            Serial.print("/");
            Serial.print(num_tryes);
            Serial.print("...\t");  // печатаем секунды таймаута

            softTimer = millis() + 1000;                  // set timer 1sec

            int incoming_flg=0;
            while(softTimer > (millis())){                
            if (Serial.available() > 0) {                 // if data avalaible
                inCommandChr[counterByte]=Serial.read();      // read bytes from serial
                if(counterByte < 520) counterByte++;          
                else counterByte = 0;
                incoming_flg=1;                      // иначе сбрасываем счетчик
            }
            }

            if(incoming_flg) is_cmd=1;
            count_try++;
        }
        return String(inCommandChr);
    };

    int virtual do_run_command(String inStr){
        if(inStr.startsWith(CMD_HELP)){
            print_full_help();
            return 1;
        }

        if(inStr.startsWith(CMD_PRINT)){
            print_curr_settings();
            return 1;
        }

        if(inStr.startsWith(CMD_LONG)){
            num_tryes+=20;
            return 1;
        }

        

        return 0;

    };


    int do_boot_loop(){

        String inCommandStr=""; 
        bool stop_command=0;

        while (!stop_command){

            inCommandStr=read_command_from_serial_str();
            debug(DSCOMMAND, "INCSTRING="+inCommandStr);
            
            if(inCommandStr!=""){
                debug(DSCOMMAND, "Received incoming string->"+String(inCommandStr));
            
                 //skip loop command
                if( inCommandStr.length()<3 && inCommandStr.charAt(0)==SKIP_CHAR) {
                    debug(DSCOMMAND, "Command->"+String(inCommandStr.charAt(0))+"; Skip waiting command", TOUT);
                    stop_command=1;
                    return 0;
                }

            //<parameter>=<value> string
                if(inCommandStr.indexOf('=')!=-1){  
                    String cmdStr=  inCommandStr.substring(0,inCommandStr.indexOf('='));
                    String numStr = inCommandStr.substring(inCommandStr.indexOf('=')+1,inCommandStr.length());
                    debug(DSCOMMAND, "Command->"+ cmdStr+", Value->"+numStr);
                    if(!do_set_command(cmdStr, numStr)) {
                        //wrong varable or value
                        debug(DSCOMMAND,"Wrong set parameter or value->"+cmdStr);
                    }else{
                        if(num_tryes<20) num_tryes+=20;
                    }
                
                //command string, lets do something and continue
                 }else if(inCommandStr.length()>=3){
                    debug(DSCOMMAND,"command->" +String(inCommandStr));
                    if(!do_run_command(inCommandStr)){              
                        debug(DSCOMMAND, "Commnad is not recognized", TOUT);
                    }else{
                        if(num_tryes<20) num_tryes+=20;
                    }
                //no incoming string, finish loop
                // }else{
                //    debug(DSCOMMAND, "Can not recognize string");
                // }    
               
                }else{
                     debug(DSCOMMAND, "Something strange in incomig string", TOUT);
                }
                      

            }else{
                //no incoming str, finish listen incoming
                debug(DSCOMMAND, "Finish boot loop, bye bye bye bye", TOUT);
                stop_command=1; 
            }           
        }
        if(stop_command) return 1; else return 0;
    };
};

#endif

