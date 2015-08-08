/*
 * Asynchronous Drive Demonstration
 * Copyright Richard Morris 2015. All Rights Reserved
 * http://armdroid1.blogspot.co.uk
 *
 */

#include "Armdroid.h"

// Arduino Wiring Pins:
const int armD1Pin = 2;
const int armD2Pin = 3;
const int armD3Pin = 4;
const int armD4Pin = 5;
const int armD5Pin = 6;
const int armD6Pin = 7;
const int armD7Pin = 8;
const int armD8Pin = 9;

// initialize Armdroid library:
Armdroid myArm( armD1Pin, armD2Pin, armD3Pin, armD4Pin, armD5Pin, armD6Pin, armD7Pin, armD8Pin );

// variables for receiving and decoding commands:
unsigned int rxCmdPos;
int rxCmdVal;
int rxCmdArg[5];

// variable to store previous state:
ArmAsyncState previous;

void setup()
{
  Serial.begin(9600);

  myArm.setSpeed(120);
  myArm.torqueMotors(true);
}

void loop()
{
  if (!Serial) {
    // while the serial stream is not open, do nothing:
    while(!Serial);
    Serial.println(F("Welcome, Armdroid!"));

    // reset command variables
    rxCmdPos = rxCmdVal = 0;
    memset(rxCmdArg, 0, sizeof(rxCmdArg));
  }

  if (Serial.available()) {
    const char ch = Serial.read();
    if (isDigit(ch))
      rxCmdVal = (rxCmdVal * 10) + (ch  - '0');  // accumulate value
    else if (ch == '-')
      rxCmdVal = rxCmdVal * -1;
    else if (ch == ',') {
      rxCmdArg[rxCmdPos++] = rxCmdVal;           // shift received value into
                                                 //  arguments array
      if (rxCmdPos == 6)
        rxCmdPos = 0;                            // wrap argument index
      
      rxCmdVal = 0;                              // reset accumulator
    }
    else if (ch == 'D') {
      Serial.println( F("drive motors") );

      MTR_CHANNELS channels = { rxCmdArg[0], rxCmdArg[1], rxCmdArg[2], rxCmdArg[3], rxCmdArg[4], rxCmdVal };
      myArm.Start(channels);

      // reset accumulator
      rxCmdPos = rxCmdVal = 0;
    }
    else if (ch == 'P') {
      Serial.println( F("pause running motors") );
      myArm.Pause();
    }
    else if (ch == 'C') {
      Serial.println( F("continue driving motors") );
      myArm.Resume();
    }
    else if (ch == 'S') {
      Serial.println( F("stop dead all motors!") );
      myArm.Stop();
    }
  }

  //  method called every loop iteration:
  myArm.driveMotorsAsynchronous();
  
  // report state changes:
  ArmAsyncState current = myArm.getAsyncState();
  if (current != previous) {

  	Serial.print( F("DRIVE STATUS = ") );
  	switch (current)
  	{
  		case ASYNC_DRIVE_STOPPED:
  			Serial.println( F("STOPPED") );
  			break;
  		case ASYNC_DRIVE_RUNNING:
  			Serial.println( F("RUNNING") );
  			break;
  		case ASYNC_DRIVE_PAUSED:
  			Serial.println( F("PAUSED") );
  			break;
  	}
  	
  	previous = current;
  }
  
}
