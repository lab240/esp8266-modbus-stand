#ifndef __dbootmodbus__
#define __dbootmodbus__


#include "dboot.h"


class DBootModbus : public DBootA
{
protected:
 

public:
   DBootModbus(WMSettings * __s): DBootA(__s) {};

    void virtual print_curr_settings(WMSettings *_s){
        DBootA::print_curr_settings();
        
        debug(DSHELP, "Mdbus address->"+String(_s->custom_level1), TOUT);
        debug(DSHELP, "HOLD REGS->"+String(_s->custom_level2), TOUT);
        debug(DSHELP, "COIL REGS->"+String(_s->custom_level3), TOUT);
        debug(DSHELP, "BAUDRATE->"+String(_s->custom_level4), TOUT);
        debug(DSHELP, "PORT SERIAL NUM->"+String(_s->custom_level_notify1), TOUT);
        debug(DSHELP, "PORT SETTINGS->"+get_serial_settings_string(_s->custom_level_notify1));


    }

    void print_welcome_help(){
        DBootA::print_welcome_help();

    }

    void print_full_help(){
        DBootA::print_full_help();

        debug(DSHELP, String(CMD_SET_ADDRESS) + "=<ADDRESS> (1..127), ");
        debug(DSHELP, String(CMD_SET_INT_REGS_AMOUNT) + "=<NUM_INT_REGS>");
        debug(DSHELP, String(CMD_SET_COIL_REGS_AMOUNT) + "=<NUM_COILS>");
        debug(DSHELP, String(CMD_SET_BAUDRATE) + "=<BAUDRATE> baudrate for modbus, possible values: 115200, 9600, 19200, 57600... or custom baudrate");
        debug(DSHELP, String(CMD_SET_PORT_SETTINGS) + "=<NUM_OF_SETTNGS> portsettings for modbus, possible values: \"=6\"->SERIAL_8N1, \"=38\"->SERIAL_8E1");
    }

    int do_espboot_loop(){
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
                print_curr_settings(_s);
                }
            
            //<parameter>=<value>
            }else if(inCommandStr.indexOf('=')!=-1){  

                String cmdStr=  inCommandStr.substring(0,inCommandStr.indexOf('='));
                String numStr = inCommandStr.substring(inCommandStr.indexOf('=')+1,inCommandStr.length());
                debug(DSCOMMAND, "Command->"+ cmdStr+", Value->"+numStr);
                if(!do_set_command(_s, cmdStr, numStr)) {
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


    int do_set_command(WMSettings *_s, String cmdStr, String valStr){
        
        if(DBootA::do_set_command(_s,cmdStr,valStr)) return 1;

        if (cmdStr == CMD_SET_ADDRESS)
             if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_modbus_address, 0,MAX_ID)) return 1;
  
        if (cmdStr == CMD_SET_INT_REGS_AMOUNT)
            if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_intregs_amount, 4,MAX_INT_REGS)) return 1;
        
        if (cmdStr == CMD_SET_COIL_REGS_AMOUNT)
            if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_coilregs_amount, 0,MAX_COIL_REGS)) return 1;
        
        if (cmdStr == CMD_SET_BAUDRATE)
            if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_serial_baudrate, MIN_BAUDRATE, MAX_BAUDRATE )) return 1;
        
        if (cmdStr == CMD_SET_PORT_SETTINGS)
            if (set_settings_val_int(_s,cmdStr,valStr,(int*) &_s->mb_serial_settings_num, 1, MAX_SERIAL_VAR_NUM)) return 1;

        return 0;  
    }; 

};



#endif

