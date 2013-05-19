/*************************************************************************
R.O.B.O.T.
Robot Onboard Binary Overlord Task

Based on example http://arduino.cc/en/Tutorial/SerialEvent
By Gunther Krueger (aka. DemonMidget) gunther@ozarkhomestead.com
*************************************************************************/
#define LEFT_CENTER 90
#define RIGHT_CENTER 90
#include <SoftwareSerial.h>

#define VREF 4.98
#define SENSOR_MAX 1024

#define XBEERX 5   
#define XBEETX 6

#define LEFTMOTORENABLE 9
#define RIGHTMOTORENABLE 10
#define LEFTFOREWARD 8
#define LEFTBACKWARD 11
#define RIGHTFOREWARD 12
#define RIGHTBACKWARD 13

#define MOTOR_ENABLE_PIN 4

#define BUMPER_PIN 3

#define MAXSPEED 90  //due to the way continuous rotation servos work maximum speed is reached at a much lower value than 90 (this value will change depending on your servos) (for Parallax servos)

#define DELAY_TIME 2500

SoftwareSerial xbeePort(XBEETX, XBEERX);

const int pingPin = 7;
volatile long count = 0;
volatile int bumper_flag=0;

unsigned long nextPrint;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete


  
void setup() {
 hbridgeSetup(); // adds the servos and prepares all SERB related variables
  // initialize serial:
  Serial.begin(57600);
  xbeePort.begin(57600);
  xbeePort.listen();
  Serial.println("# R.O.B.O.T v2 Mk.1");
  xbeePort.println("# R.O.B.O.T v2 Mk.1");
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  motorCommand("m+00+00");
  pinMode(MOTOR_ENABLE_PIN,OUTPUT);
  pinMode(BUMPER_PIN,INPUT); 
  pinMode(LEFTMOTORENABLE, OUTPUT);
  pinMode(RIGHTMOTORENABLE, OUTPUT);
  pinMode(LEFTFOREWARD, OUTPUT);
  pinMode(LEFTBACKWARD, OUTPUT);
  pinMode(RIGHTFOREWARD, OUTPUT);
  pinMode(RIGHTBACKWARD, OUTPUT);
  pinMode(XBEERX, INPUT);
  pinMode(XBEETX, OUTPUT);
  
  digitalWrite(BUMPER_PIN, HIGH);
  digitalWrite(MOTOR_ENABLE_PIN, HIGH);
  attachInterrupt(1,bumperHit,FALLING);
}

void motorCommand (String cmd)
{ 
 // Parsing the motor command string. 
 // example: 'm+90+90'
 
 
 char leftMotorCmd[4];
 leftMotorCmd[0]=cmd[1];
 leftMotorCmd[1]=cmd[2];
 leftMotorCmd[2]=cmd[3];
 leftMotorCmd[3]=0;
 char rightMotorCmd[4];
 rightMotorCmd[0]=cmd[4];
 rightMotorCmd[1]=cmd[5];
 rightMotorCmd[2]=cmd[6];
 rightMotorCmd[3]=0;
 int leftMotorSpeed = atoi(leftMotorCmd);
 int rightMotorSpeed = atoi(rightMotorCmd);
 //Serial.println(leftMotorCmd);
 //Serial.println(rightMotorCmd);
 if(digitalRead(BUMPER_PIN) == 0){
   leftMotorSpeed = min(leftMotorSpeed, 0);
   rightMotorSpeed = min(rightMotorSpeed, 0);
 }
 setSpeedLeft(leftMotorSpeed);
 setSpeedRight(rightMotorSpeed);
 
 
}

void loop() {
  serialPoll();
  // print the string when a newline arrives:
  if(bumper_flag==1)
    driverFail();
  if (stringComplete) { // there is at least one newline in inputString
    debug_dump(inputString, '%');
    int pos = inputString.indexOf('\n');
    while(pos != -1)
    {
      debug_dump(inputString, '$');
      ++pos; // pos points 1 past the newline
      String messageString = inputString.substring(0, pos);
      if(inputString.length() > pos)
        inputString = inputString.substring(pos);
      else
        inputString = "";
      pos = inputString.indexOf('\n');
      debug_dump(messageString, '@');
      char inByte = messageString[0];
      switch (inByte) {
      case 'm':    
        motorCommand(messageString);
        break;
      case 'r':    
        relayControl(messageString[1]);
        break;
      case 's':
        sensorRead(messageString[1]);
        break;
      default:
        break;
        }

    }
    // clear the string:
    //inputString = "";
    stringComplete = false;
  }
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialPoll() {
  debug_dump("poll", '~');
  while (xbeePort.available()) {
    // get the new byte:
    char inChar = (char)xbeePort.read();
    debug_dump("read", '~');
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      debug_dump("complete", '~');

    }
  }
}
/*
 * sets up your arduino to address your SERB using the included routines
*/
void hbridgeSetup(){
setSpeedLeft(0);
setSpeedRight(0);
}




void setSpeedRight(int rightSpeed){
  int speedval = RIGHT_CENTER - rightSpeed;
  Serial.print("right=");
  Serial.println(speedval);
  //rightServo.write(speedval);
  //rightServo.write(RIGHT_CENTER - rightSpeed);             //sends the new value to the servo
}


void setSpeedLeft(int leftSpeed){
  int speedval = LEFT_CENTER + leftSpeed;
  Serial.print("left=");
  Serial.println(speedval);
  //leftServo.write(speedval);
}


void countPulse(){
  detachInterrupt(0);
  count++;
  while(digitalRead(2)==0){
  }
  //Serial.println(count);                           
  attachInterrupt(0,countPulse,FALLING);
}


void relayControl(char cmd)
    {
      if (cmd == '1')
        {
            digitalWrite(MOTOR_ENABLE_PIN, HIGH);
            Serial.println("# Servo-power on.");
        }
      
      else
          {
              digitalWrite(MOTOR_ENABLE_PIN, LOW);
              Serial.println("# Servo-power off.");
          }  
    }
    
void bumperHit()
{
   bumper_flag = 1;
}

void driverFail(){
  if(digitalRead(BUMPER_PIN)==LOW)
  {
    motorCommand("m+00+00");   //stops robot RIGHT NOW!!! BEFORE THE GIEGER TUBE BREAKS!!!
    Serial.println("# Driver Fail!");
  }
  bumper_flag = 0;
}


void sensorRead(char cmd)
    {
      int Value = 0;
      float Voltage;
      switch(cmd){
        case '0': // servo battery
            Value = analogRead(A0);
            Voltage = ((Value * VREF) / SENSOR_MAX) * 2.0;
            Serial.print("d0-");
            Serial.println(Voltage);
            break;
        case '1': // arduino battery
            Value = analogRead(A1);
            Voltage = ((Value * VREF) / SENSOR_MAX) * 2.0;
            Serial.print("d1-");
            Serial.println(Voltage);
            break;
        case '2': // US sensor
            pingSense();
            break;
        case '3': // bump sensor
            Value = digitalRead(BUMPER_PIN);
            Serial.print("d3-");
            Serial.println(Value);
            break;

          
      } 
}


void pingSense()
{
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration=0;
  long inches;
  //long cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  //cm = microsecondsToCentimeters(duration);
 
  Serial.print("d2-");
  Serial.println(inches);
 
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}


char hex[] = "0123456789abcdef";

void debug_dump(String data, char prompt)
{
  Serial.print(prompt);
  int i;
  for(i=0;i<data.length();i++)
  {
    char d = data[i];
    if(d<' ' ||  d>'~') 
    {
       Serial.print('[');
       Serial.print(hex[d>>4]);
       Serial.print(hex[d & 0x0f]);
       Serial.print(']');
    }
    else
      Serial.print(d);
  }
  Serial.println(prompt);
}

