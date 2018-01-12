#include <SerialGSM.h>
#include <SoftwareSerial.h>
SerialGSM cell(8,9);

boolean sendonce=true;
void setup(){  
  Serial.begin(57600);
  cell.begin(57600);
  cell.Verbose(true);
  //cell.switchGPRS_ON();
  cell.echoOFF;
  cell.DeleteAllSMS();
  cell.FwdSMS2Serial();
 }


void loop(){
  if (cell.ReceiveSMS()){
	 Serial.print("Sender: ");
	 Serial.println(cell.Sender());
	 Serial.print("message: ");
	 Serial.println(cell.Message());
	 cell.DeleteAllSMS();
  }
}

