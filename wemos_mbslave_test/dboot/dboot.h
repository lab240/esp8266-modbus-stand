#ifndef __dboot__
#define __dboot__


#include "../donofflib/dbase.h"


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

public:
   DBootA(WMSettings * __s): DBase(__s) {};
  
    void init(){
        load();
        debug(DSEEPROM, "Rad setings from EEPROM, currect Salt="+ String(EEPROM_SALT)+ " EEPROM SALT=" + String(_s->salt));
        was_init=1;
    };

    void virtual print_curr_settings()=0;
    int virtual do_set_command(String cmdStr, String valStr)=0;

    void virtual print_welcome_help(){
        debug(DSHELP, "-------------- Avaliable commands (wait for "+String(NUM_TRY)+" secs) -----------------");
        debug(DSHELP, "Press <C+Enter> to skip Dboot");
        debug(DSHELP, "help - get full help");
    };

    void virtual print_full_help(){
        debug(DSHELP, "-------------- Avaliable commands (wait for "+String(NUM_TRY)+" secs) -----------------");
        debug(DSHELP, "Press <C+Enter> to skip Dboot");
    };

    //get command from terminal
    String read_command_from_serial_str(){
        int is_cmd=0;
        //int is_cmd_l=0;
        int count_try=0;
        uint32_t softTimer = 0; 
        char inCommandChr[50] = {0};    // заводим массив для захвата команды задания адреса
        uint8_t counterByte = 0;      // указатель для записи команды в массив

        while(is_cmd==0 && count_try<NUM_TRY){  // цикл таймаута для задания адреса

            Serial.print(count_try,DEC);
            Serial.print("...\t");  // печатаем секунды таймаута

            softTimer = millis() + 1000;                  // назначаем таймер на 1с

            int incoming_flg=0;
            while(softTimer > (millis())){                // крутимся в цикле пока время меньше заданного таймаута
            if (Serial.available() > 0) {               // если доступны данные из порта
                inCommandChr[counterByte]=Serial.read();      // принимаем в массив по указателю
                if(counterByte < 520) counterByte++;          // но не более 20 значений от 0 
                else counterByte = 0;
                incoming_flg=1;                      // иначе сбрасываем счетчик
            }
            }

            if(incoming_flg) is_cmd=1;
            count_try++;
        }
        return String(inCommandChr);
    };


    int do_boot_loop(){
        String inCommandStr=""; 
        bool stop_commnads=0;

        while (!stop_commnads){

            inCommandStr=get_command_str();
            
            debug(DSENTER,0);
            
            if(inCommandStr!=""){
            debug(DSCOMMAND, "Received incoming string->"+String(inCommandStr));
            
            //skip loop command
            if( inCommandStr.length()<=3 && inCommandStr.charAt(0)==SKIP_CHAR) {
                debug(DSCOMMAND, "Command->"+String(inCommandStr.charAt(0))+"; Skip waiting command", TOUT);
                stop_commnads=1;
                return 0;
            }

            if(inCommandStr.length()>3 && inCommandStr.indexOf('=')==-1){
                //commands 
                debug(DSCOMMAND,"command >" +String(inCommandStr) +"< incoming");
                if(inCommandStr.startsWith(CMD_CMD_HELP)){
                    print_full_help();
                }

                if(inCommandStr.startsWith(CMD_CMD_PRINT)){
                    print_curr_settings();
                }
            
            //<parameter>=<value>
            }else if(inCommandStr.indexOf('=')!=-1){  

                String cmdStr=  inCommandStr.substring(0,inCommandStr.indexOf('='));
                String numStr = inCommandStr.substring(inCommandStr.indexOf('=')+1,inCommandStr.length());
                debug(DSCOMMAND, "Command->"+ cmdStr+", Value->"+numStr);
                if(!do_set_command(cmdStr, numStr)) {
                debug(DSCOMMAND,"Wrong set parameter or value->"+cmdStr);
                }
            
            }else{
                debug(DSCOMMAND, "Commnad is not recognized", TOUT);
            }
            
            }else{
                debug(DSCOMMAND, "No incomming string");
                stop_commnads=1;
                return 0;
            }

        }
        if(stop_commnads) return 1; else return 0;
    };
};

#endif

