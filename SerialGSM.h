#ifndef _SerialGSM_H
#define _SerialGSM_H
#include "Arduino.h"
#include <M590.h>
#include <SoftwareSerial.h>

#define BUFFER_SIZE 255

#define SERIALTIMEOUT 2000
#define PHONESIZE 13
#define MAXMSGLEN 160
#define BOOTTIME 25

#define PWR_OFF_ON_DELAY 3000L

#define AT_TO      500L
#define CPBR_TO     1000L
#define CMGR_TO     300L
#define CMGD_TO     15000L
#define CMGDA_TO    10000L
#define CSQ_TO      30000L
#define CSCLK_TO    2000L
#define CPOWD_TO    10000L
#define CMEE_TO     10000L
#define IPR_TO      10000L
#define CMGF_TO     10000L
#define CNMI_TO     10000L
#define CREG_TO     30000L
#define CPMS_TO     10000L
#define CPIN_TO     10000L
#define CMGS1_TO    2000L
#define CMGS2_TO    20000L
#define UDPREPLY_TO   5000L

class SerialGSM : public SoftwareSerial {
public:
  SerialGSM(byte rxpin,byte txpin, byte pwrkey, byte power);
  char buffer[BUFFER_SIZE]; // buffer array for data recieve over serial port
  byte count;     // counter for buffer array
  void sendATCommand(char * command);
  void FwdSMS2Serial();
  byte echoOFF();
  byte echoON();
  byte clipON();
  byte DeleteAllSMS();
  byte Reset();
  byte dial(char * number);
  byte answer();
  byte hangup();

  //int ReadLine();
  //int ReceiveSMS();
  void Verbose(boolean var1);
  boolean Verbose();
  void Sender(char * var1);
  char * Sender();
  char * Message();

  void switchGPRS_ON();
  void switchGPRS_OFF_SW();
  void switchGPRS_OFF_HW();
  void switchM590_ON();
  void switchM590_OFF_SW();
  void switchM590_OFF_HW();
  boolean isGPRSReady();
  boolean isNetworkAvailable();
  byte sendSMS(char * number, char * message);
  byte readSMSfromSIM(byte position);
  byte readSMS();
  
  void clearBufferArray();
  byte readDataToBuffer();
  void printBufferContent();
  void printSerialContent();
  byte confirmAtCommand(char* searchString, unsigned long timeOut);
  byte waitForResult(char* searchString, unsigned long timeOut);
  byte findInBuffer(char* text);
  bool checkNetworkRegistration();
  byte waitForNetwork(unsigned long timeOut);
  uint8_t signalQuality();
  boolean selectSIMPhoneBook();
  char * readPhoneNumber(uint8_t position);
  boolean deletePhoneNumber(uint8_t position);
  boolean storePhoneNumber(uint8_t position, char * phoneNumber, char * name);
  
  void GPRSWriteByte(char c);
  byte SerialGSM::GPRSReadByte();
  bool isGPRSDataAvailable();
  
  void removeChar(char *str, char garbage);

  bool getPSUTTZ();
  byte parseCMTI();

  boolean verbose;
  char sendernumber[PHONESIZE + 1];
  char inmessage[160];
  byte DST;
  char *psuttz;
  byte pwrkey;
  byte power;
  
protected:
  unsigned long lastrec;
  
};

#endif /* not defined _SerialGSM_H */

