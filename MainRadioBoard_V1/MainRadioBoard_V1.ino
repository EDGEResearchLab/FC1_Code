#include <Servo.h> 

/*TODO:
***Add data mux
***Document radio I/O Pins
*/

//Define pins that get passed around via the xBee
const int BALLAST_PIN = 16; //to DI4 on xBee
const int CUT_PIN = 15; //to DI6 on xBee
const int VENT_PIN = 14; //to DI5 on xBee
const int RESERVE_PIN = 10; //to DI7 on xBee - spare pin for future expansion

//Define pins that are used to control the data mux
const int GPS_PIN = 2;
const int MYDATA_PIN = 3;

char cmdByte = 0x31;
char oldCmdByte = 0x31;


void setup()
{
  //set up the ballast valve signal pin, and make sure it's low
  digitalWrite(BALLAST_PIN, LOW); 
  pinMode(BALLAST_PIN, OUTPUT);
  digitalWrite(BALLAST_PIN, LOW);
  
  //set up the cut mechanism signal pin, and make sure it's low
  digitalWrite(CUT_PIN, LOW);
  pinMode(CUT_PIN, OUTPUT);
  digitalWrite(CUT_PIN, LOW);
  
  //set up the float valve signal pin, and make sure it's low
  digitalWrite(VENT_PIN, LOW);
  pinMode(VENT_PIN, OUTPUT);
  digitalWrite(VENT_PIN, LOW);
  
  //set up the reserve signal pin, and make sure it's low
  digitalWrite(RESERVE_PIN, LOW);
  pinMode(RESERVE_PIN, OUTPUT);
  digitalWrite(RESERVE_PIN, LOW);
  
  //set up the mux pin for the local sensor data, and make sure it's low
  digitalWrite(MYDATA_PIN, LOW);
  pinMode(MYDATA_PIN, OUTPUT);
  digitalWrite(MYDATA_PIN, LOW);
  
  //set up the mux pin for the GPS, and make sure it's high (so the GPS can start sending data immediately)
  digitalWrite(GPS_PIN, LOW);
  pinMode(GPS_PIN, OUTPUT);
  digitalWrite(GPS_PIN, HIGH);
  
  Serial.begin(4800); //This pipes to the USB port
  Serial1.begin(4800); //This is the TTL UART
}

void loop()
{
  //GPS Passthrough Mode
  sendGPS();

  for(int i=0; i<120; i++) //we'll be checking for commands every 0.25 second, so this will leave the GPS feed on for approx. 30 seconds
  {
    if(Serial1.available() > 0) checkCommand(); //check the ttl uart for a commmand, and parse it if it's there
    //else Serial.println("No Commmand Received..."); //we won't see this from the balloon, it's just there for on-the-ground debugging
    delay(250); //wait for 0.25 second before checking again
  }

  //Sensor Reporting Mode
  sendMYDATA();
  
  for(int i=0; i<3; i++) //send the sensor data three times @ 0.5 second intervals, checking for commands in between
  {
    sensorsToNMEA();
    if(Serial1.available() > 0) checkCommand(); //check the ttl uart for a commmand, and parse it if it's there
    //else Serial.println("No Commmand Received..."); //we won't see this from the balloon, it's just there for on-the-ground debugging
    delay(500); //wait for 0.5 second before checking again
  }
  flightModeToNMEA();
}

void checkCommand()
{
  oldCmdByte = cmdByte; //buffer the old value in case the received command is invalid
  cmdByte = char(Serial1.read());
  Serial.print("Received: ");
  Serial.println(cmdByte);
  if (cmdByte == 0x31) //normal flight: valve closed, ballast closed, burn wire off
  {
    Serial.println("Normal Flight: Mode1");
    digitalWrite(VENT_PIN,LOW);
    digitalWrite(BALLAST_PIN, LOW);
    digitalWrite(CUT_PIN, LOW);
  }
  else if (cmdByte == 0x32) //venting flight: valve open, ballast closed, burn wire off
  {
    Serial.println("Venting Flight: Mode2");
    digitalWrite(VENT_PIN,HIGH);
    digitalWrite(BALLAST_PIN, LOW);
    digitalWrite(CUT_PIN, LOW);
  }
  else if (cmdByte == 0x33) //ballast dump: valve closed, ballast open, burn wire off
  {
    Serial.println("Ballast Dump: Mode3");
    digitalWrite(VENT_PIN,LOW);
    digitalWrite(BALLAST_PIN, HIGH);
    digitalWrite(CUT_PIN, LOW);
  }
  else if (cmdByte == 0x35) //cutdown: valve closed, ballast closed, burn wire on
  {
    Serial.println("Cutdown: Mode5");
    digitalWrite(VENT_PIN, LOW);
    digitalWrite(BALLAST_PIN, LOW);
    digitalWrite(CUT_PIN, HIGH);
  }
  else if (cmdByte == 0x39) //emergency descent: valve open, ballast closed, burn wire on
  {
    Serial.println("Emergency: Mode9");
    digitalWrite(VENT_PIN, HIGH);
    digitalWrite(BALLAST_PIN, LOW);
    digitalWrite(CUT_PIN, HIGH);
  }
  else
  {
    Serial.println("Unrecognized command");
    cmdByte = oldCmdByte; //the received command was invalid; reset the value of the reported varible to its previous state
  }
}

void sendGPS()
{
  delay(500); //this is needed to ensure that the radio times out and packetizes whatever has been sent to it before going back to GPS
  digitalWrite(MYDATA_PIN, LOW); //Turn off the data mux for the local sensor data line
  delay(500); //Wait for just a moment to allow everything to settle out (otherwise, we'll get some corrupt data)
  digitalWrite(GPS_PIN, HIGH); //Turn on the data mux for the GPS feed
}

void sendMYDATA()
{
  digitalWrite(GPS_PIN, LOW); //Turn off the data mux for the GPS feed
  digitalWrite(MYDATA_PIN, HIGH); //Turn on the data mux for the local sensor data line
  delay(500); //Wait for just a moment to allow everything to settle out (otherwise, we'll get some corrupt data)
  Serial1.println();//Throw in a println in case we didn't cut off at an endline
}

void sensorsToNMEA()
{
  //Format: $PBSEN,<battery>,B,<temperature>,T,<pressure>,P,<humidity>,H*<checksum>
  //Build the string out, omitting the '$' and '*' since they're not counted for the checksum.  We'll add them later.
  String sentence = String("PBSEN,")+String(analogRead(A0))+String(",B,")+String(analogRead(A1))+String(",T,")+String(analogRead(A2))+String(",P,")+String(analogRead(A3))+String(",H");
//  Serial.println(sentence); //Only use this for debugging purposes...
  
  int XORVal, i; //variable initialization for the following calculation
  //Calculate the checksum
  for(XORVal = 0, i = 0; i<sentence.length(); i++)
  {
    int c = (unsigned char)sentence[i];
    XORVal ^= c;
  }
  
  //Put it all together and send it
  Serial1.print("$"); //the missing leading dollar sign
  Serial1.print(sentence);
  Serial1.print("*"); //the missing astrisk
  Serial1.println(XORVal, HEX); //the sentence checksum
  
  //Do the same thing, but over the debug port
  /*
  Serial.print("$"); //the missing leading dollar sign
  Serial.print(sentence);
  Serial.print("*"); //the missing astrisk
  Serial.println(XORVal, HEX); //the sentence checksum
  */
}

void flightModeToNMEA()
{
  //Format: $PBMOD,<mode>,FM*<checksum>
  //Build the string out, omitting the '$' and '*' since they're not counted for the checksum.  We'll add them later.
  String sentence = String("PBMOD,")+String((cmdByte - 0x30),HEX)+String(",FM");
  
  int XORVal, i; //variable initialization for the following calculation
  //Calculate the checksum
  for(XORVal = 0, i = 0; i<sentence.length(); i++)
  {
    int c = (unsigned char)sentence[i];
    XORVal ^= c;
  }
  
  //Put it all together and send it
  Serial1.print("$"); //the missing leading dollar sign
  Serial1.print(sentence);
  Serial1.print("*"); //the missing astrisk
  Serial1.println(XORVal, HEX); //the sentence checksum
}

