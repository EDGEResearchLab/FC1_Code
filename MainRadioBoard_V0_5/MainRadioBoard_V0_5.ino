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
  
  Serial.begin(4800); //This pipes to the USB port
  Serial1.begin(4800); //This is the TTL UART
}

void loop()
{
  if(Serial1.available() > 0) checkCommand(); //check the ttl uart for a commmand, and parse it if it's there
  delay(250); //wait for 0.25 second before checking again
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
