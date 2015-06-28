/*
 * Armdroid Serial Driver Sketch
 * Copyright (C) Richard Morris 2014 - 2015.  All Rights Reserved
 * http://armdroid1.blogspot.co.uk
 *
 * 1.0 Initial version
 * 1.1 Added variable speed control
 * 1.2 Added support for driving multiple motors
 * 1.3 Moved all Armdroid routines into new class library
 * 1.4 Added Torque control enhancements
 */

#include "Armdroid.h"

#define PIN_D1      2                             // declare I/O pins we're using
#define PIN_D2      3
#define PIN_D3      4
#define PIN_D4      5
#define PIN_D5      6
#define PIN_D6      7
#define PIN_D7      8
#define PIN_D8      9

// initialize the Armdroid library on pins 2 through 9:
Armdroid myArm(PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7, PIN_D8);   

// variables for recieving and decoding commands:
unsigned int rxCmdPos;
int rxCmdVal;
int rxCmdArg[5];

// temporary buffer for string formating:
char message[80];

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  if (!Serial) {
    // while the serial stream is not open, do nothing:
    while(!Serial);
    Serial.println("Welcome, Armdroid!");
 
    // reset command variables for session
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

    else if (ch == 'd') {
      // drive motor (channel specified in Arg[0], steps in Accumulator)
      myArm.driveMotor(rxCmdArg[0], rxCmdVal);
      
      // return offsets for feedback
      SerialPrintMotorOffsets();
      
      // reset accumulator
      rxCmdPos = rxCmdVal = 0;
    }
    else if (ch == 'D') {
      // drive all motors specified in Arg[0]..[4] and Accumulator 
      MTR_CHANNELS channels = { rxCmdArg[0], rxCmdArg[1], rxCmdArg[2], rxCmdArg[3], rxCmdArg[4], rxCmdVal };
      myArm.driveAllMotors(channels);
      
      // return offsets for feedback
      SerialPrintMotorOffsets();
      
      // reset accumulator
      rxCmdPos = rxCmdVal = 0;
    }
    else if (ch == 'g') {
      // for debugging command args
      Serial.print("debug = ");
      for(unsigned int i = 0; i < 5; i++)
      {
        Serial.print(rxCmdArg[i]);
        Serial.print(' ');
      }
      Serial.println(rxCmdVal);
    }
    else if (ch == 'h') {
      // returns to home position
      MTR_CHANNELS offs = myArm.getOffsets();
      offs.channel_1 = -offs.channel_1;
      offs.channel_2 = -offs.channel_2;
      offs.channel_3 = -offs.channel_3;
      offs.channel_4 = -offs.channel_4;
      offs.channel_5 = -offs.channel_5;
      offs.channel_6 = -offs.channel_6;
      myArm.driveAllMotors(offs);
      SerialPrintMotorOffsets();
    }
    else if (ch == 'o') {
      // returns current offset counters
      SerialPrintMotorOffsets();
    }
    else if (ch == 'r') {
      // resets home position
      myArm.resetOffsetCounts();
      SerialPrintMotorOffsets();
      // reset all command variables
      rxCmdPos = rxCmdVal = 0;
      memset(rxCmdArg, 0, sizeof(rxCmdArg));
    }
    else if (ch == 's') {
      // set desired speed in RPM
      myArm.setSpeed(rxCmdVal);
      Serial.print("speed = ");
      Serial.println(rxCmdVal);
      rxCmdVal = 0;
    }
    else if (ch == 't') {
      // torque enablement
      myArm.torqueMotors( (boolean) rxCmdVal );
      Serial.print("torque = ");
      Serial.println(rxCmdVal ? "enabled" : "disabled");
      rxCmdVal = 0;
    }
    else if (ch == 'v') {
      // returns firmware/protocol version
      Serial.println("firmware version = 2.0/protocol = 1.0a");
    }
    else if (ch == 'V') {
      // returns interface this firmware was configured to use
#ifdef INTERFACE_PROTOTYPE
      Serial.println("interface = logic-drive");
#else
      Serial.println("interface = direct-drive");
#endif
    }
  }
}

void SerialPrintMotorOffsets()
{
  MTR_CHANNELS offsets = myArm.getOffsets();
  sprintf(message, "offsets = %d,%d,%d,%d,%d,%d", offsets.channel_1,
                                                  offsets.channel_2,
                                                  offsets.channel_3,
                                                  offsets.channel_4,
                                                  offsets.channel_5,
                                                  offsets.channel_6);
  Serial.println(message);
}


