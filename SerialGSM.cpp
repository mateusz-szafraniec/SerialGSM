// SerialGSM version 1.1
// by Meir Michanie
// meirm@riunx.com


// error codes
// http://www.developershome.com/sms/resultCodes2.asp
#include <SerialGSM.h>

SerialGSM::SerialGSM(byte rxpin,byte txpin, byte pwrkey, byte power):
SoftwareSerial(rxpin,txpin)
{
 verbose=false;
 this->power=power;
 this->pwrkey=pwrkey;
}

void SerialGSM::sendATCommand(char * command)
{
  this->flush();
  this->println(command);
  if (verbose) Serial.println(command);
}

void SerialGSM::FwdSMS2Serial(){
  this->sendATCommand("AT+CMGF=1"); // set SMS mode to text
  this->confirmAtCommand("\r\nOK", AT_TO);
  this->sendATCommand("AT+CNMI=2,2,0,0,0"); // set module to send SMS data to serial out upon receipt 
  this->confirmAtCommand("\r\nOK", AT_TO);
}

byte SerialGSM::answer(){
  this->sendATCommand("ATA");
  return this->confirmAtCommand("\r\nOK", AT_TO);
}

byte SerialGSM::echoOFF(){
  this->sendATCommand("ATE0");
  return this->confirmAtCommand("\r\nOK", AT_TO);
}

byte  SerialGSM::echoON(){
  this->sendATCommand("ATE1");
  return this->confirmAtCommand("\r\nOK", AT_TO);
}

byte  SerialGSM::clipON(){
  this->sendATCommand("AT+CLIP=1");
  return this->confirmAtCommand("\r\nOK", AT_TO);
}

byte SerialGSM::hangup(){
  this->sendATCommand("ATH");
  return this->confirmAtCommand("\r\nOK", CPBR_TO);
}

byte SerialGSM::DeleteAllSMS(){
  sendATCommand("AT+CMGD=1,4"); // delete all SMS
  return this->confirmAtCommand("\r\nOK", AT_TO);
}

byte SerialGSM::Reset(){
  this->sendATCommand(RESET_COMMAND); // Reset Modem
  return this->confirmAtCommand("\r\nOK", AT_TO);
}

/*
int SerialGSM::ReadLine(){
  static int pos=0;
  char nc;
  while (this->available()){
    nc=this->read();
    if (nc == '\n' or (pos > MAXMSGLEN) or ((millis()> lastrec + SERIALTIMEOUT)and (pos > 0)) ){
      nc='\0';
      lastrec=millis();
      inmessage[pos]=nc;
     pos=0;
     if (verbose) Serial.println(inmessage);
      return 1;
    }
    else if (nc=='\r') {
    }
    else{
      inmessage[pos++]=nc;
      lastrec=millis();
    }
  }
  return 0;
}
*/

/*
int SerialGSM::ReceiveSMS(){
  static boolean insms=0;
  if (this->ReadLine()){
  // Get the number of the sms sender in order to be able to reply
	if ( strstr(inmessage, "CMT: ") != NULL ){
	    insms=1;
	    int sf=6;
	    if(strstr(inmessage, "+CMT:")) sf++; 
		    for (int i=0;i < PHONESIZE;i++){
		      sendernumber[i]=inmessage[sf+i];
		    }
		sendernumber[PHONESIZE]='\0';
		return 0;
	 }else{ 
		if(insms) {
			insms=0;
			return 1;
		}
	}
  }
  return 0;
}
*/

boolean SerialGSM::Verbose(){
	return verbose;
}

void SerialGSM::Verbose(boolean var1){
	verbose=var1;
}

char * SerialGSM::Sender(){
	return sendernumber;
}


char * SerialGSM::Message(){
	return inmessage;
}


void SerialGSM::Sender(char * var1){
	sprintf(sendernumber,"%s",var1);
}

void SerialGSM::switchGPRS_ON() 
{
  if (digitalRead(power)) {
    switchGPRS_OFF_SW();
    delay(PWR_OFF_ON_DELAY);
  }
  if (verbose) Serial.println(F("Switching GPRS ON"));
  digitalWrite(pwrkey, LOW);
  digitalWrite(power, HIGH);
  delay(PWR_ON_DELAY);
  digitalWrite(pwrkey, HIGH);
  delay(PWR_KEY_PRESS_DELAY);
  digitalWrite(pwrkey, LOW);
  this->confirmAtCommand(PWRON_MESSAGE, PWR_ON_DELAY2);
  byte attempt=0;
  while (!this->isGPRSReady())
  {
    attempt++;
    if (attempt>10) {
      if (verbose) Serial.println(F("Powering module OFF/ON"));
      switchGPRS_OFF_SW();
      delay(PWR_OFF_ON_DELAY);
      digitalWrite(power, HIGH);
      delay(PWR_ON_DELAY);
      digitalWrite(pwrkey, HIGH);
      delay(PWR_KEY_PRESS_DELAY);
      digitalWrite(pwrkey, LOW);
      delay(PWR_ON_DELAY2);
      attempt=0;
    }
  }
}

void SerialGSM::switchGPRS_OFF_SW() 
{
  if (verbose) Serial.println(F("Switching GPRS OFF"));
  digitalWrite(pwrkey, LOW);
  this->sendATCommand("AT+CPOWD=1");
  this->confirmAtCommand(PWRDOWN_MESSAGE, CPOWD_TO);
  digitalWrite(power, LOW);
}

void SerialGSM::switchGPRS_OFF_HW() 
{
  if (verbose) Serial.println(F("Switching GPRS OFF"));
  digitalWrite(pwrkey, LOW);
  this->flush();
  digitalWrite(pwrkey, HIGH);
  delay(PWR_KEY_PRESS_DELAY);
  digitalWrite(pwrkey, LOW);
  this->confirmAtCommand(PWRDOWN_MESSAGE, CPOWD_TO);
  digitalWrite(power, LOW);
}

boolean SerialGSM::isGPRSReady() 
{
  if (verbose) Serial.println(F("isGPRSReady?"));
  this->sendATCommand("AT");
  this->confirmAtCommand("\r\nOK", AT_TO);
  this->sendATCommand("AT");
  if (this->confirmAtCommand("\r\nOK", AT_TO)>0)
    return false;
  return true;
}

boolean SerialGSM::isNetworkAvailable()
{
  if (verbose) Serial.println(F("Checking GPRS"));
  if (!this->isGPRSReady()) {
    if (verbose) Serial.println(F("GPRS is: OFF"));
    this->switchGPRS_ON();
  }
  if (verbose) Serial.println(F("GPRS is: ON"));
  if (verbose) Serial.print(F("Checking network"));
  boolean ok = false;
  byte attempt=0;
  while (!ok) {
    if (this->checkNetworkRegistration() == 1) {
      Serial.print(".");
      delay(5000);
      attempt++;
      if (attempt>6) {
        if (verbose) Serial.println(F(" failed"));
        return false;
      }
    } else {
      ok = true;
    }
  }
  if (verbose) Serial.println("");
  if (verbose) Serial.println(F("NETWORK OK"));
  if (verbose) Serial.println(F("Checking signal quality"));
  ok = false;
  attempt=0;
  while (!ok) {
    if (this->signalQuality() == 0) {
      Serial.print(".");
      delay(5000);
      attempt++;
      if (attempt>10) {
        if (verbose) Serial.println(F(" failed"));
        return false;
      }
    } else {
      ok = true;
    }
  }
  if (verbose) Serial.println("");
  if (verbose) Serial.println(F("SIGNAL OK"));
  return true;
}

byte SerialGSM::sendSMS(char * number, char * message) {
  if (!this->isNetworkAvailable()) return false;
  if (verbose) Serial.println(F("Sending SMS"));
  this->sendATCommand("AT+CMGF=1");    //Because we want to send the SMS in text mode
  if (this->confirmAtCommand("\r\nOK", AT_TO)>0)
    return false;
  this->sendATCommand("AT+CSCS=\"GSM\"");
  if (this->confirmAtCommand("\r\nOK", AT_TO)>0) return false;
  this->print("AT+CMGS=\"");
  this->print(number);
  this->print("\"\r");    //Start accepting the text for the message
  //to be sent to the number specified.
  if (this->confirmAtCommand(">", CMGS1_TO)>0)
    return false;
  this->print(message);
  this->print("\r");   //The text for the message
    if (this->confirmAtCommand(">", CMGS1_TO)>0)
    return false;
  this->write(0x1A);  //Equivalent to sending Ctrl+Z
  return (this->confirmAtCommand("+CMGS:", CMGS2_TO)==0);
  
}

byte SerialGSM::readSMSfromSIM(byte position) {
  if (verbose) Serial.println(F("Reading SMS"));
  this->sendATCommand("AT+CMGF=1");    //Because we want to read the SMS in text mode
  if (this->confirmAtCommand("\r\nOK", AT_TO)>0) return false;
  this->sendATCommand("AT+CSCS=\"GSM\"");
  if (this->confirmAtCommand("\r\nOK", AT_TO)>0) return false;
  this->print("AT+CMGR=");
  this->println(position);
  if (this->confirmAtCommand("\r\nOK", CMGF_TO)>0) return false;
  char *ptr = buffer;
  char *str = NULL;
  char *str2 = NULL;
  byte linijka = 0;
  bool znalazl=false;
  ptr = strtok_r(ptr, "\n", &str);
  while ((linijka<8)&&(!znalazl)){
    linijka++;
    ptr = strtok_r(NULL, "\n", &str);
    znalazl=strcasestr(ptr, "+CMGR:");
  }
  if (znalazl) {
    if (verbose) Serial.print(F("Message from:"));
    if (verbose) Serial.println(ptr);
    str2=str;
    ptr = strtok_r(ptr, " ", &str);
    ptr = strtok_r(NULL, ",", &str);
    strcpy(sendernumber,strtok_r(NULL, ",", &str));
    removeChar(sendernumber, '"');
    if (verbose) Serial.print("Phone number: ");
    if (verbose) Serial.println(sendernumber);
    if (verbose) Serial.print("Message:");
    strcpy(inmessage,strtok_r(NULL, "\r", &str2));
    if (verbose) Serial.println(inmessage);
    return true;
  }
  return false;
}


byte SerialGSM::dial(char * number) {
  if (!this->isNetworkAvailable()) return false;
  if (verbose) Serial.print(F("Calling: "));
  if (verbose) Serial.println(number);
  this->write("ATD");
  this->write(number);
  this->println(";");
  return (this->confirmAtCommand("\r\nOK", CMGS1_TO)==0);
}

void SerialGSM::clearBufferArray()              // function to clear buffer array
{
  for (byte i = 0; i < BUFFER_SIZE; i++)
  {
    buffer[i] = NULL;
  }                               // clear all index of array with command NULL
  count = 0;
}

byte SerialGSM::readDataToBuffer() 
{
  if (this->available())              // if date is comming from softwareserial port ==> data is comming from gprs shield
  {
    while (this->available())          // reading data into char array
    {
      buffer[count++] = this->read();     // writing data into array
      if (count == BUFFER_SIZE)
        break;
    }
    return true;
  } else {
    return false;
  }
}

void SerialGSM::printBufferContent() 
{
  if (count>0)              // if date is comming from softwareserial port ==> data is comming from gprs shield
  {
    Serial.write(buffer, count);            // if no data transmission ends, write buffer to hardware serial port
  }
}

/*
void SerialGSM::printSerialContent() 
{
  if (this->available())              // if date is comming from softwareserial port ==> data is comming from gprs shield
  {
    while (this->available())          // reading data into char array
    {
      buffer[count++] = this->read();     // writing data into array
      if (count == BUFFER_SIZE)
        break;
    }
    Serial.write(buffer, count);            // if no data transmission ends, write buffer to hardware serial port
    this->clearBufferArray();              // call clearBufferArray function to clear the storaged data from the array
  }
}
*/

/*************************************************************
  Procedure to search data returned from AT commands.  All
  data up to and including searchString is stored in a global
  array to be used if needed.
  ARGUMENTS
  searchString  pointer to string to be searched
  timeOut     timeout in milliseconds

  RETURN:
  0       searchString found successfully
  1       ERROR string encountered, potential problem
  2       Buffer full, searchString not found
  3       Timeout reached before searchString found
**************************************************************/
byte SerialGSM::confirmAtCommand(char* searchString, unsigned long timeOut)
{
  this->clearBufferArray();
  unsigned long tOut = millis();
  while ((millis() - tOut) <= timeOut)
  {
    if (this->available())
    {
      buffer[count] = this->read();
      count++;
      buffer[count] = '\0';
      if (strstr(buffer, searchString) != NULL) {
        if (verbose) Serial.println(F("OK"));
        return 0;
        }
      if (strstr(buffer, "ERROR") != NULL) {
        if (verbose) Serial.println(F("ERROR"));
        return 1;
        }
      if (count >= (BUFFER_SIZE - 1)) {
        if (verbose) Serial.println(F("ERROR (not found)"));
        return 2;
        }
    }
  }
  if (verbose) Serial.println(F("ERROR (timeout)"));
  return 3;
}

/*************************************************************
  Procedure to search data returned from AT commands.  All
  data up to and including searchString is stored in a global
  array to be used if needed.
  ARGUMENTS
  searchString  pointer to string to be searched
  timeOut     timeout in milliseconds

  RETURN:
  0       searchString found successfully
  1       ERROR string encountered, potential problem
  2       Buffer full, searchString not found
  3       Timeout reached before searchString found
**************************************************************/
byte SerialGSM::waitForResult(char* searchString, unsigned long timeOut)
{
  unsigned long tOut = millis();
  while ((millis() - tOut) <= timeOut)
  {
    if (this->available())
    {
      buffer[count] = this->read();
      count++;
      buffer[count] = '\0';
      if (strstr(buffer, searchString) != NULL) {
        if (verbose) Serial.println(F("OK"));
        return 0;
        }
      if (strstr(buffer, "ERROR") != NULL) {
        if (verbose) Serial.println(F("ERROR"));
        return 1;
        }
      if (count >= (BUFFER_SIZE - 1)) {
        if (verbose) Serial.println(F("ERROR (not found)"));
        return 2;
        }
    }
  }
  if (verbose) Serial.println(F("ERROR (timeout)"));
  return 3;
}

byte SerialGSM::findInBuffer(char* text) 
{
  if (strcasestr(buffer, text)) {
    return true;
  } else {
    return false;
  }
}

byte SerialGSM::waitForNetwork(unsigned long timeOut)
{
  unsigned long tOut = millis();
  byte result;
  while ((millis() - tOut) <= timeOut)
  {
	result = checkNetworkRegistration();
	if (result == 0) {
		if (verbose) Serial.println(F("OK"));
        return 0;
	}
	delay (500);
  }
  if (verbose) Serial.println(F("ERROR (timeout)"));
  return 1;
}

/*************************************************************
  Procedure to check if GSM is registered to the network.
  RETURN:
  0   GSM is registered to the network
  1   GSM is not registered to the network
**************************************************************/
bool SerialGSM::checkNetworkRegistration()
{
  this->sendATCommand("AT+CREG?");
  this->confirmAtCommand("\r\nOK", CREG_TO);
  if ((strstr(buffer, ",1") != NULL) || (strstr(buffer, ",5") != NULL))
    return 0;
  return 1;
}

/*************************************************************
  Procedure to check the GSM signal quality. The higher the
  number the better the signal quality.
  RETURN:
    0     Error sending AT command/No signal
    1 - 99    RSSI signal strength

**************************************************************/
uint8_t SerialGSM::signalQuality()
{
  this->sendATCommand("AT+CSQ");
  if (this->confirmAtCommand("\r\nOK", CSQ_TO)>0)
    return 0;
  char *ptr = buffer;
  char *str = NULL;
  ptr = strtok_r(ptr, " ", &str);
  ptr = strtok_r(NULL, ",", &str);
  if (verbose) Serial.print(F("Signal LEVEL: "));
  if (verbose) Serial.println(ptr);
  return (atoi(ptr));
}

boolean SerialGSM::selectSIMPhoneBook() {
  this->sendATCommand("AT+CPBS=\"SM\"");
  return (this->confirmAtCommand("\r\nOK", AT_TO)==0);
}

char* SerialGSM::readPhoneNumber(uint8_t position)
{
  this->print("AT+CPBR=");
  this->println(position);
  if (this->confirmAtCommand("\r\nOK", CPBR_TO)>0)
    return "NULL";
  char *ptr = buffer;
  char *str = NULL;
  ptr = strtok_r(ptr, " ", &str);
  ptr = strtok_r(NULL, ",", &str);
  if (atoi(ptr) != position) return "NULL";
  ptr = strtok_r(NULL, ",", &str);
  removeChar(ptr, '"');
  if (verbose) Serial.print(F("Phone number: "));
  if (verbose) Serial.println(ptr);
  return ptr;
}

boolean SerialGSM::deletePhoneNumber(uint8_t position)
{
  this->print("AT+CPBW=");
  this->println(position);
  return (this->confirmAtCommand("\r\nOK", CPBR_TO)==0);
}

boolean SerialGSM::storePhoneNumber(uint8_t position, char * phoneNumber, char * name)
{
  this->print("AT+CPBW=");
  this->print(position);
  this->print(",");
  this->print("\"");
  this->print(phoneNumber);
  this->print("\"");
  this->print(",145,");
  this->print("\"");
  this->print(name);
  this->sendATCommand("\"");
  return (this->confirmAtCommand("\r\nOK", CPBR_TO)==0);
}

void SerialGSM::GPRSWriteByte(char c)
{
  this->write(c);
}

byte SerialGSM::GPRSReadByte()
{
  return this->read();
}

bool SerialGSM::isGPRSDataAvailable()
{
  return (this->available()>0);
}

bool SerialGSM::getPSUTTZ()
{
  char *ptr = buffer;
  char *str = NULL;
  char *str2 = NULL;
        byte linijka = 0;
        bool znalazl=false;
        ptr = strtok_r(ptr, "\n", &str);
        while ((linijka<8)&&(!znalazl)){
          linijka++;
          ptr = strtok_r(NULL, "\n", &str);
          znalazl=strcasestr(ptr, "*PSUTTZ: ");
        }
        if (!znalazl) return false;
        psuttz = strtok_r(NULL, "*PSUTTZ:", &ptr);
        psuttz++;
        znalazl=false;
        while ((linijka<8)&&(!znalazl)){
          linijka++;
          ptr = strtok_r(NULL, "\n", &str);
          znalazl=strcasestr(ptr, "DST:");
        }
        if (znalazl) {
          ptr = strtok_r(NULL, "DST: ", &ptr);
          DST=atoi(ptr);
        }
        return true;
}

byte SerialGSM::readSMS()
{
        char *ptr = buffer;
        char *str = NULL;
        char *str2 = NULL;
        byte linijka = 0;
        bool znalazl=false;
        ptr = strtok_r(ptr, "\n", &str);
        while ((linijka<8)&&(!znalazl)){
          linijka++;
          ptr = strtok_r(NULL, "\n", &str);
          znalazl=strcasestr(ptr, "+CMT:");
        }
        if (znalazl) {
          if (verbose) Serial.print("nadawca:");
          ptr++;
          ptr++;
          ptr++;
          ptr++;
          ptr++;
          ptr++;
          ptr = strtok(ptr, ",");
          removeChar(ptr,'\"');
          strcpy(sendernumber,ptr);
          if (verbose) Serial.println(sendernumber);
          str=strtok_r(NULL, "\r", &str);
          strcpy(inmessage,str);
          if (verbose) Serial.println("wiadomosc");
          if (verbose) Serial.println(inmessage);
          return true;
        }
        return false;
}

byte SerialGSM::parseCMTI()
{
    char *ptr = buffer;
    char *str = NULL;
    ptr = strtok_r(ptr, " ", &str);
    ptr = strtok_r(NULL, ",", &str);
    ptr = strtok_r(NULL, ",", &str);
    return atoi(ptr);
}


void SerialGSM::removeChar(char *str, char garbage) 
{
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}
