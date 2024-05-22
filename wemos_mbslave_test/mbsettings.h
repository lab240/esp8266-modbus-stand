#ifndef __mbsettings__
#define __mbsettings__

#include "Arduino.h"

#define EEPROM_SALT 1664

#define NUM_TRY 10   //waiting command pause

#define MAX_QUEUE_WANTED 10

enum pub_events { 
 PUBLISHER_WANT_SAVE,
 PUBLISHER_WANT_RESET,
 PUBLISHER_WANT_SAY_JUST_SYNCED,
};


typedef struct {
  
  // ** from web config
  int   salt = EEPROM_SALT;
  char  mqttServer[22]  = "mqtt.lab240.ru";
  char  mqttUser[12] = "";
  char  mqttPass[22]    = "";
  char  mqttPort[6]="1883";
  char  autooff[2] = "";
  char  dev_id[10]="empty_dev";

  // *** params config
  uint      autostop_sec=0;
  uint      autostop_sec2=0;
  uint      autooff_hours=0;
  bool      autoreboot_on_max_attempts=0; //reboot on MAX_CONNECT_ATTEMPTS_BEFORE_RESET (default 0)
  bool      notifyer_onoff=0;
  bool      current_check=1;
  uint      lscheme_num=0;
  uint      lscheme_num2=0;
  uint      time_zone=3; //3 for Moscow //2 for Riga
  bool      start_on=0;
//                             0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
  bool     custom_array1[24]={0,0,0,0,0,0,0,0,0,0,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
  bool     custom_array2[24]={1,1,1,1,1,1,1,1,1,1,1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  //uint      custom_scheme2=0B101010101010101010101010;
  int      custom_level_notify1=-128;
  bool     baselog=1;
  
// ** hotter \ cooler 
  uint8_t      hotter=0;
  bool      cooler=0;

// ** temp level
  uint      default_temp_level=20;
  uint      day_temp_level=18;
  uint      night_temp_level=21;
  /*     */

  uint      schm_onoff_num1=99;
  uint      schm_onoff_num2=0; //not use
  uint      level_delta=10;  //delta in Celsius*10

  bool      notifyer=0;
  char      email_notify[50]="dj.novikov@gmail.com";
  uint      hours_on_notify=6; //notify period in hours
  int      temp_low_level_notify=-128;
  int      analog_level_notify1=-128;
  int      analog_level_notify2=-128;
  int      temp_high_level_notify=-128;
  uint      autooff_hours2=0;
// ** 
  int      custom_level_notify2=-128;
  int      custom_level_notify3=-128;
  int      custom_level_notify4=-128;
  uint8_t      urgent_off=75;
  uint      tariff=500;      // копееек \ центов за 1квт\ч
  unsigned long      ucounter=0;  //счетчик потребления в рублях\долл\евр
  uint weekschm[7]={99,99,99,99,99,99,99};
  //*****
  int     custom_level3=-128;
  int     custom_level4=-128;
  int     custom_level1=-128;
  int     custom_level2=-128;
  //uint    hotter2=0;
  //int     custom_hotter2[24]={22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22};
  uint cb_schm1=0B000000000011111111111100;
  uint cb_schm2=0B111111111111100000000000;
  int     temp_matrix[24]={22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,21};
} WMSettings;

typedef struct {
	char ssid[20]="";
	char pass[20]="";
  uint ready_to_save=0;
} WifiCreds;


//mapping abstract settings structure members to our names
#define  mb_modbus_address       custom_level1
#define  mb_intregs_amount       custom_level2
#define  mb_coilregs_amount      custom_level3
#define  mb_serial_baudrate      custom_level4

#define  mb_serial_settings_num  custom_level_notify1
//*******************************************************


#define MIN_AUTOSTOP_SEC 0
#define MAX_AUTOSTOP_SEC 1200
#define MAX_AUTOOFF_HOURS 24
#define MAX_LSCHM_NUM 100
#define MAX_ANALOG_NOTIFY 50000
#define MAX_HOURS_NOTIFY 1000
#define MAX_PWR_TARIFF 10000
#define MAX_LEVEL_DELTA 50000
#define MIN_TEMP_LEVEL -5000
#define MAX_TEMP_LEVEL 10000
#define MAX_ANALOG_LEVEL 16000
#define MAX_NOTIFY_CUSTOM 5000

#define MIN_CUSTOM_LEVEL -32000
#define MAX_CUSTOM_LEVEL 32000


#endif