#include "SerialCommand.h"
#include "SDPArduino.h"
#include <Wire.h>
#include <Arduino.h>
#include <I2CPort.h>


//Kickers in back
//define FRONT 1
//define RIGHT 7
//define BACK 5
//define LEFT 3

//Kickers in front
#define FRONT 4
#define RIGHT 2
#define BACK 5
#define LEFT 3

#define KICKERS 1
#define GRABBER 0

#define OPADDR 0x5A
#define REGADDR 0x04

#define KICKERDELAY 10


boolean requestStopKick = 0;
boolean kickerStatus = 0;

int zeroPosition;

#define ROTARY_KICK_PULSES 18
#define ROTARY_KICKER_POSITION 1
#define ROTARY_SLAVE_ADDRESS 5
#define ROTARY_COUNT 6
#define PRINT_DELAY 200

// Initial motor position is 0.
int positions[ROTARY_COUNT] = {0};

int run = 0;

SerialCommand sCmd;

void setup(){
  Wire.begin();
  sCmd.addCommand("f", dontMove); 
  sCmd.addCommand("h", completeHalt); 
  sCmd.addCommand("motor", spinmotor); 
  sCmd.addCommand("r", rationalMotors); 
  sCmd.addCommand("ping", pingMethod); 
  sCmd.addCommand("kick", kick);
  sCmd.addCommand("kick_continuously", kickerStart);
  sCmd.addCommand("stop_kicking", kickerStop);
  sCmd.addCommand("grab", grab); 
  sCmd.addCommand("ungrab", ungrab); 
  sCmd.addCommand("mux", muxTest);
  SDPsetup();
  helloWorld();
  printMotorPositions();
}

void loop(){
  sCmd.readSerial();
}

void muxTest(){
  int motor = atoi(sCmd.next());
  int dir  = atoi(sCmd.next());
  int pow  = atoi(sCmd.next());
  Wire.beginTransmission(OPADDR);
  Wire.write(motor);
  Wire.write(dir);
  Serial.println(Wire.endTransmission());
  Wire.beginTransmission(OPADDR);
  Wire.write(motor+1);
  Wire.write(pow);
  Serial.println(Wire.endTransmission());
  delay(2000);
  Wire.beginTransmission(OPADDR);
  Wire.write(motor);
  Wire.write(0);
  Serial.println(Wire.endTransmission());
}

void kickerStart(){
  motorBackward(KICKERS, 100);
}

void kickerStop(){
  motorStop(KICKERS);
}

void dontMove(){
  motorStop(FRONT);
  motorStop(BACK);
  motorStop(LEFT);
  motorStop(RIGHT);
}

void spinmotor(){
  int motor = atoi(sCmd.next());
  int power = atoi(sCmd.next());
  motorForward(motor, power);
}

void motorControl(int motor, int power){
  if(power == 0){
    motorStop(motor);
  } else if(power > 0){
    motorForward(motor, power);
  } else {
    motorBackward(motor, -power);
  }
}


void rationalMotors(){
  int front = atoi(sCmd.next());
  int back  = atoi(sCmd.next());
  int left  = atoi(sCmd.next());
  int right = atoi(sCmd.next());
  Serial.println(front);
  Serial.println(back);
  Serial.println(left);
  Serial.println(right);

  motorControl(FRONT, -front);
  motorControl(BACK, back);
  motorControl(LEFT, left);
  motorControl(RIGHT, -right);
}

void pingMethod(){
  Serial.println("pang");
}

void kicker(){
  int type = atoi(sCmd.next());
  if(type == 0){
    motorStop(KICKERS);
  } else if (type == 1){
    Serial.print("Starting From: ");
    Serial.println(positions[0] % 40);
    motorForward(KICKERS, 100);
    kickerStatus = 1;
  } else {
    motorBackward(KICKERS, 100);
    kickerStatus = -1;
  }
}

void kick(){
  updateMotorPositions();
  int targetPosition = positions[ROTARY_KICKER_POSITION] - ROTARY_KICK_PULSES;
  printMotorPositions();
  motorBackward(KICKERS, 100);
  while(abs(positions[ROTARY_KICKER_POSITION]) < abs(targetPosition)){
    updateMotorPositions();
  }
  motorStop(KICKERS);
  printMotorPositions();
}

void grab(){
  motorForward(GRABBER, 100);
  delay(500);
  motorStop(GRABBER);
}

void ungrab(){
  motorBackward(GRABBER, 100);
  delay(300);
  motorStop(GRABBER);
}


void completeHalt(){
  motorAllStop();
  //motorControl(FRONT, 0);
  //motorControl(BACK, 0);
  //motorControl(LEFT, 0);
  //motorControl(RIGHT, 0);
}


void updateMotorPositions() {
  // Request motor position deltas from rotary slave board
  Wire.requestFrom(ROTARY_SLAVE_ADDRESS, ROTARY_COUNT);
  
  // Update the recorded motor positions
  for (int i = 0; i < ROTARY_COUNT; i++) {
    positions[i] += (int8_t) Wire.read();  // Must cast to signed 8-bit type
  }
}

void printMotorPositions() {
  Serial.print("Motor positions: ");
  for (int i = 0; i < ROTARY_COUNT; i++) {
    Serial.print(positions[i]);
    Serial.print(' ');
  }
  Serial.println();
  delay(PRINT_DELAY);  // Delay to avoid flooding serial out
}


