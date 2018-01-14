  #define PWR_ON_DELAY 300L  //wait for voltage stabilize
  #define PWR_OFF_DELAY 5000L //wait for network log off
  
  #define PWR_KEY_PRESS_DELAY 1000L //power key press time
  
  #define PWR_ON_DELAY2 5000L //wait for MODEM:STARTUP message
  
  #define PWRON_MESSAGE "MODEM:STARTUP" 
  #define PWRDOWN_MESSAGE "POWER DOWN" //brak oficjalnych danych
  
  #define RESET_COMMAND "AT+CFUN=16" //SW reset command
  #define POWER_DOWN_COMMAND "AT+CPWROFF" //power down