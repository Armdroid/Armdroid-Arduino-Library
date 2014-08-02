// ensure this library description is only included once
#ifndef _ARMDROID_H
#define _ARMDROID_H

#include "Arduino.h"
#include "Armtypes.h"

#define ID35_STEPS  48                    // number of steps/revolution
#define STROBE      0x01                  // output (from microcontroller) LOW
                                          // input (to microcontroller) HIGH
#define FREEARM     0xF0                  // pattern for motors off

#define CDIR        0x10                  // direction bit (prototype models only)
#define CCLK        0x20                  // clock         (prototype models only)


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
  void driveAllMotors(MTR_CHANNELS channels);
  
  // torque control
  void torqueMotors(boolean torqueEnabled);

protected:
  // This MUST be defined by the subclass:
  virtual void armdroid_write(uint8_t output) = 0;

private:
  void pulse_stepper_motor(MTR_CTRL *mtr_ctrl);
  
  unsigned long step_interval;      // interval between steps, in milliseconds based on speed
  unsigned long previous_time;      // time in milliseconds when last step was taken
};


// Arduino standard wiring (8-wires) class
class Armdroid : public ArmBase {
  public:
  // constructors:
  Armdroid( uint8_t pin_D1,
            uint8_t pin_D2,
            uint8_t pin_D3,
            uint8_t pin_D4,
            uint8_t pin_D5,
            uint8_t pin_D6,
            uint8_t pin_D7,
            uint8_t pin_D8 );
 
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
