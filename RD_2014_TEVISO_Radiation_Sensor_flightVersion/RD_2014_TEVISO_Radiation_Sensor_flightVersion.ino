/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */
unsigned int duration;
const int radpin = 4;

unsigned long elapsedTime = 0;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(57600);
  pinMode(radpin, INPUT);
  
  //Set up guard pins on around the radiation pin
  pinMode(radpin-2, OUTPUT);
  pinMode(radpin-1, OUTPUT);
  pinMode(radpin+1, OUTPUT);
  pinMode(radpin+2, OUTPUT);
  digitalWrite(radpin-2, LOW);
  digitalWrite(radpin-1, LOW);
  digitalWrite(radpin+1, LOW);
  digitalWrite(radpin+2, LOW);
  
  //Wait for a second to allow stuff to settle out
  delay(1000);
  Serial.println("Header Data");
  Serial.println("Header Data");
  //Write the header to the log file
  Serial.println("Time,Pressure,PulseWidth");
}

// the loop routine runs over and over again forever:
void loop() {
  //wait for a pulse from the sensor, and read how long it is
  duration = pulseIn(radpin, HIGH, 4294967295);
  //capture the elapsed time
  elapsedTime = micros();
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // print out the value you read:
  Serial.print(elapsedTime);
  Serial.print(",");
  Serial.print(sensorValue);
  Serial.print(",");
  Serial.println(duration);
}
