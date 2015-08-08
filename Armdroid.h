// ensure this library description is only included once
#ifndef _ARMDROID_H
#define _ARMDROID_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "ArmTypes.h"


// Arduino Wiring Pins used by ArmdroidShield
#define ARM_SHIELD_D1_PIN			2
#define ARM_SHIELD_D2_PIN			3
#define ARM_SHIELD_D3_PIN			4
#define ARM_SHIELD_D4_PIN			5
#define ARM_SHIELD_D5_PIN			6
#define ARM_SHIELD_D6_PIN			7
#define ARM_SHIELD_D7_PIN			8
#define ARM_SHIELD_D8_PIN			9

// Armdroid Axis
#define ARM_AXIS_GRIPPER			1
#define ARM_AXIS_LEFT_WRIST			2
#define ARM_AXIS_RIGHT_WRIST		3
#define ARM_AXIS_ELBOW				4
#define ARM_AXIS_SHOLDER			5
#define ARM_AXIS_BASE				6


// interface definitions
#define ID35_STEPS  48                    // number of steps/revolution
#define STROBE      0x01                  // output (from microcontroller) LOW
                                          // input (to microcontroller) HIGH
#define FREEARM     0xF0                  // pattern for motors off

#define CDIR        0x10                  // direction bit (prototype models only)
#define CCLK        0x20                  // clock         (prototype models only)


// Asynchronous State Enumeration
typedef enum
{
	ASYNC_DRIVE_RUNNING  = 0,
	ASYNC_DRIVE_STOPPED  = 1,
	ASYNC_DRIVE_PAUSED   = 2
} ArmAsyncState;


// abstract base class
class ArmBase {
public:

  // default constructor:
  ArmBase(void);

  // speed setter method:
  void setSpeed(uint32_t whatSpeed);

  // offset counter methods:
  MTR_CHANNELS getOffsets(void);
  void resetOffsetCounts(void);

  // main mover methods:
  void driveMotor(uint8_t motor, int16_t steps);
  void driveAllMotors(MTR_CHANNELS target);
  void driveMotorsAsynchronous(void);

  // asynchronous control:
  inline ArmAsyncState getAsyncState(void) { return(asyncState); }
  inline bool isRunning() { return(asyncState == ASYNC_DRIVE_RUNNING); }
   bool Start(MTR_CHANNELS target);
   bool Pause();
   bool Resume();
   bool Stop();

  // torque control:
  void torqueMotors(boolean torqueEnabled = true);

protected:
  // This MUST be defined by the subclass:
  virtual void armdroid_write(uint8_t output) = 0;

private:
  void pulse_stepper_motor(MTR_CTRL *mtr_ctrl);
  inline void set_target(MTR_CHANNELS target);

  unsigned long step_interval;      // interval between steps, in milliseconds based on speed
  unsigned long previous_time;      // time in milliseconds when last step was taken

  ArmAsyncState asyncState;			// async state
};


// Arduino standard wiring (8-wires) class
// ( wiring defaults to ArmdroidShield pins 2-9 )
class Armdroid : public ArmBase {
  public:
  // constructors:
  Armdroid( uint8_t pin_D1 = ARM_SHIELD_D1_PIN,
            uint8_t pin_D2 = ARM_SHIELD_D2_PIN,
            uint8_t pin_D3 = ARM_SHIELD_D3_PIN,
            uint8_t pin_D4 = ARM_SHIELD_D4_PIN,
            uint8_t pin_D5 = ARM_SHIELD_D5_PIN,
            uint8_t pin_D6 = ARM_SHIELD_D6_PIN,
            uint8_t pin_D7 = ARM_SHIELD_D7_PIN,
            uint8_t pin_D8 = ARM_SHIELD_D8_PIN );
 
  protected:
  // writes to Armdroid interface:
  void armdroid_write(uint8_t output);
 
  private: 
  // interface pin numbers:
  uint8_t pin_D1;
  uint8_t pin_D2;
  uint8_t pin_D3;
  uint8_t pin_D4;
  uint8_t pin_D5;
  uint8_t pin_D6;
  uint8_t pin_D7;
  uint8_t pin_D8;
};


#endif // _ARMDROID_H
