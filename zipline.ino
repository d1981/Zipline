// 10-13-2016
// Josh Kapple
#include "IRremote.h"
#include <Servo.h>
#include <math.h>

Servo esc;
int buttonState = 0;
int fwd  = 1;
int rev  = 2;
int nuet = 0;
int receiver = 6; // Signal Pin of IR receiver to Arduino Digital Pin 6
int led = 13;     // arduino led 
int mode = 2;     // Start in regular throttle mode. 1 indicates esc training, 2 regular throttle. 
int throttle = 0;
int mydirection = 0;
int isPaused = 1;
int lastSign = 1;
unsigned long time;
unsigned long timeToWait;

IRrecv irrecv(receiver);    // create instance of 'irrecv'
decode_results results;     // create instance of 'decode_results'

void setup() 
{
  // put your setup code here, to run once:
  esc.attach(9, 1000, 2500);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(led,OUTPUT);
  Serial.begin(9600);
  while (! Serial); 
  Serial.println(String(fwd + " for full forward,"));
  int throttle = 1250;
  time=millis();
  timeToWait = 0;
  randomSeed(analogRead(0));
}

void loop() 
{
  if (mode == 2){
    throttle = 1250;
  }
  checkIRInput();
  Serial.println(throttle, 1);
  esc.writeMicroseconds(1250);
  delay(500);
  time = millis();
  if (time > timeToWait){
    // Pause is over do something 
    // Randomly wait and go in different direction for a random amount of time
    int randomTime = random(0, 1500);
    if (lastSign == 1){
      lastSign = 2;
    }
    else {
      lastSign = 1;
    }
    if (isPaused == 0){
      scaleThrottle(randomTime, lastSign);
    }
    Serial.println("Waiting over, deciding next action");
    Serial.println(isPaused);
    // randomly set a new timeout
    int randomTimeout = random(5000,180000);
    timeToWait    = time + randomTimeout;    
  }
}

void switchMode(){
  mode = mode == 1 ? 2:1;
  Serial.println(mode);
}

int switchDirection(){
  mydirection = mydirection == 0 ? 1: 0;
  return mydirection;
}

void scaleThrottle(int runtime, int sign){
  // for first 5 seconds slowly increase the speed till at throttle
  // Use this equation for easing the throttle y=1250(1.01)^t
  float timeTaken = 0;
  float timeNeeded = (log(1.08)/log(1.01)) * 1000;
  float y;
  float growthRate;
   
  if (sign == 1){
    growthRate = 1.01;
  }
  else {growthRate = .99;}
  
  while (timeTaken <= timeNeeded){
    y = 1250 * pow(growthRate, timeTaken/1000);
    timeTaken += 100;
    esc.writeMicroseconds(y);  
    delay(100);
  }
  esc.writeMicroseconds(y);  
  delay(runtime);
}

void checkIRInput(){
 if (irrecv.decode(&results)) // have we received an IR signal?
  {
    Serial.println(String(results.value, HEX));
    //Serial.println(results.value);
    switch(results.value)
    {
      //case 0xFF629D: //fall
      case 0x511dbb:
      case 0xff629d: // UP button pressed
        if (mode == 1){
          // forward button pressed, send max throttle
          throttle = 1800;
          Serial.print("Forward\n");
        }
        else {
          //throttle = 1650;
          scaleThrottle(500, 1);
        }
        timeToWait = time + 5000; // Set the time to wait after hitting a button
        break;

      case 0xd7e84b1b:
      case 0xFF02FD: // >|| (play/pause) button pressed 
          throttle = 1250;
          Serial.print("Nuetral\n");
          timeToWait = time + 5000; // Set the time to wait after hitting a button
          if (mode == 2){
            if (isPaused == 1){
              isPaused=0;
              blinkArduino(2,500);
            }
            else { 
              isPaused=1; 
              blinkArduino(5,500);
            }
          }
          break;

      case 0xFFA857: // DOWN button pressed 
          // reverse button pressed, send max reverse
          if (mode == 1){
              throttle = 700;
          }
          else {
            throttle = 1100;
            scaleThrottle(500, -1);
            }
           
          Serial.print("Reverse\n ");
          timeToWait = time + 5000; // Set the time to wait after hitting a button
          break;

      case 0xEE886D7F: // Fall through
      case 0xFFE21D: // Func/Stop key pressed
          Serial.println("Mode Pressed");
          // Switch mode;
          switchMode();
          blinkArduino(mode,500);
          break;
     }
     irrecv.resume(); // receive the next value
  } 
}

void blinkArduino(int times, int delayMs){    
     for (int i=0; i < times; i++){
      digitalWrite(led,HIGH);
      delay(delayMs);
      digitalWrite(led,LOW);
      delay(delayMs);
     }
     delay(500);
}


