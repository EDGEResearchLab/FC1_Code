#include <Servo.h>

//pin definitions
const int VENT_PIN = 7; //from DO5 on xBee, controlled by arduino
const int CUT_PIN  = 8; //from DO6 on xBee, controlled by arduino

//servo controlling the gas valve
Servo ventServo;
const int ventClosed = 156;
const int ventOpen   = 115;

//servo controlling the cutdown mechanism
Servo cutServo;
const int cutClosed = 15;
const int cutOpen = 65;

//control vars
boolean cutConfirmed = false;

void setup()
{
  //set up the pins
  pinMode(VENT_PIN, INPUT);
  pinMode(CUT_PIN, INPUT);
  
  //set up the vent servo on pin 5
  ventServo.attach(5);
  ventServo.write(ventClosed);
  
  //set up the cut servo on pin 6
  cutServo.attach(6);
  cutServo.write(cutClosed);
  
  Serial.begin(4800);
}
void loop()
{
  //Debug stuff
  Serial.print("Vent Pin: ");
  Serial.print(digitalRead(VENT_PIN));
  Serial.print(", Cut Pin: ");
  Serial.println(digitalRead(CUT_PIN));
  
  //check for float command
  if(digitalRead(VENT_PIN)) 
  {
    Serial.println("Opening vent");
    ventServo.write(ventOpen);
  }
  else ventServo.write(ventClosed);
  
  //check for cut command
  if(!digitalRead(CUT_PIN))
  {
    cutServo.write(cutClosed);
    cutConfirmed = false;
  }
  else if (digitalRead(CUT_PIN) && !cutConfirmed)
  {
    Serial.println("Confirming...");
    cutServo.write(cutClosed);
    cutConfirmed = true;
    //give ourselves some time to correct an errant cut command
    delay(5000);
  }
  else if (digitalRead(CUT_PIN) && cutConfirmed)
  {
    Serial.println("Confirmed - cutting");
    cutServo.write(cutOpen);
  }
  
  //update every half-second
  delay(500);
}
