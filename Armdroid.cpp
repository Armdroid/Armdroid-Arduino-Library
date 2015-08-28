/********************************************************************************
This is the core Armdroid Library, providing low-level dynamics & control.
 
Copyright (c) 2014 - 2015 Richard Morris.  All rights reserved.
http://armdroid1.blogspot.co.uk

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************/

#include "Armdroid.h"

// un comment for Armdroid 1 prototype models:
//  1 = use direct drive method (recommended)
//  2 = use on-board logic drive
//#define INTERFACE_PROTOTYPE 1

// macro to make motor addresses according to interface type
#ifndef INTERFACE_PROTOTYPE
// standard addressing
#define MK_MTR_ADDR(x) (x << 1)
#else
// reverse addressing (prototype models only)
#define MK_MTR_ADDR(x) ((((x&1)<<2) + (x&2) + ((x&4)>>2)) << 1)
#endif

// motor control table - reassign your motor assignments as necessary
static MTR_CTRL mtr_control_table[] = {
  { MK_MTR_ADDR(1), 0, 0, 0 },
  { MK_MTR_ADDR(2), 0, 0, 0 },
  { MK_MTR_ADDR(3), 0, 0, 0 },
  { MK_MTR_ADDR(4), 0, 0, 0 },
  { MK_MTR_ADDR(5), 0, 0, 0 },
  { MK_MTR_ADDR(6), 0, 0, 0 }
};

// table of step waveforms (as published by Colne Robotics)
#define WAVE_TABLE_SIZE 4
static const uint8_t mtr_waveform_table[WAVE_TABLE_SIZE] = {
  192,      // 11000000
  144,      // 10010000
  48,       // 00110000
  96        // 01100000
};


/*
 * Standard constructor
 * Sets which wires should control the Armdroid.
 */
Armdroid::Armdroid( uint8_t pin_D1,
                    uint8_t pin_D2,
                    uint8_t pin_D3,
                    uint8_t pin_D4,
                    uint8_t pin_D5,
                    uint8_t pin_D6,
                    uint8_t pin_D7,
                    uint8_t pin_D8 )
{  
  // Arduino pins for interface connection:
  this->pin_D1 = pin_D1;
  this->pin_D2 = pin_D2;
  this->pin_D3 = pin_D3;
  this->pin_D4 = pin_D4;
  this->pin_D5 = pin_D5;
  this->pin_D6 = pin_D6;
  this->pin_D7 = pin_D7;
  this->pin_D8 = pin_D8;
  
  // setup the pins on the microcontroller:
  pinMode( this->pin_D1, OUTPUT );
  pinMode( this->pin_D2, OUTPUT );
  pinMode( this->pin_D3, OUTPUT );
  pinMode( this->pin_D4, OUTPUT );
  pinMode( this->pin_D5, OUTPUT );
  pinMode( this->pin_D6, OUTPUT );
  pinMode( this->pin_D7, OUTPUT );
  pinMode( this->pin_D8, OUTPUT );
  
  // start with Armdroid in input mode
  armdroid_write( STROBE );
}

/*
 * Writes value to Armdroid interface
 */
void Armdroid::armdroid_write(uint8_t output)
{
#ifdef DEBUG_VERBOSE_TRACE
  Serial.print( "armdroid_write = " );
  Serial.println( output, BIN );
#endif
  digitalWrite( this->pin_D8, output & 0x80 );
  digitalWrite( this->pin_D7, output & 0x40 );
  digitalWrite( this->pin_D6, output & 0x20 );
  digitalWrite( this->pin_D5, output & 0x10 );
  digitalWrite( this->pin_D4, output & 0x08 );
  digitalWrite( this->pin_D3, output & 0x04 );
  digitalWrite( this->pin_D2, output & 0x02 );
  digitalWrite( this->pin_D1, output & 0x01 );
}


/*
 * base ctor - initializes member variables
 */
ArmBase::ArmBase(void)
{
  this->asyncState    = ASYNC_DRIVE_STOPPED;
  this->previous_time = 0UL;
  setSpeed(100);
}

/*
 * Returns motor offset counters
 */
MTR_CHANNELS ArmBase::getOffsets(void)
{
  MTR_CHANNELS offset_counters;
  offset_counters.channel_1 = mtr_control_table[0].offset;
  offset_counters.channel_2 = mtr_control_table[1].offset;
  offset_counters.channel_3 = mtr_control_table[2].offset;
  offset_counters.channel_4 = mtr_control_table[3].offset;
  offset_counters.channel_5 = mtr_control_table[4].offset;
  offset_counters.channel_6 = mtr_control_table[5].offset;
  return offset_counters;
}

/*
 * Resets all motor offset counters
 */
void ArmBase::resetOffsetCounts(void)
{
  for(uint8_t motor = 0; motor < 6; motor++) {
    mtr_control_table[motor].offset = 0;
  }
}

/*
 * Sets speed in Revolutions per Minute
 */
void ArmBase::setSpeed(uint32_t whatSpeed)
{
  this->step_interval = 60L * 1000L / ID35_STEPS / whatSpeed;
}

/*
 * Drives a single motor channel.
 * steps can be specified as +/- for CW/CCW rotations
 */
void ArmBase::driveMotor(uint8_t motor, int16_t steps)
{
#ifdef DEBUG_VERBOSE_TRACE
  Serial.print( "armdroid_drive_motor - motor " );
  Serial.print( motor );
  Serial.print( " steps " );
  Serial.println( steps );
#endif

  if (motor < 1 || motor > 6) {
#ifdef DEBUG_VERBOSE_TRACE
    Serial.println("motor out of range");
#endif
    return;
  }
  if (steps == 0) {
#ifdef DEBUG_VERBOSE_TRACE
    Serial.println("no steps specified");
#endif
    return;
  }
  
  MTR_CTRL* const mtr_ctrl = &mtr_control_table[ motor - 1 ];
  mtr_ctrl->steps_left = abs(steps);
  mtr_ctrl->dir = (steps > 0);
  
  unsigned long current_time = 0UL;
  
  while ( mtr_ctrl->steps_left > 0 )
  {
    // pulse motor only if the appropriate delay has passed:
    current_time = millis();
    if ((current_time - previous_time) >= step_interval)
    {
      // save last time we stepped the motor
      previous_time = current_time;

      pulse_stepper_motor( mtr_ctrl );
    }
  }
  
  // ensure Armdroid is returned to Input mode
  armdroid_write( STROBE );
}

/*
 * Drives multiple motor channels at same time.
 * Assign number of steps required in MTR_CHANNEL structure (inputs)
 * for each channel.
 * zero = motor should not be moved
 */
void ArmBase::driveAllMotors(MTR_CHANNELS target)
{
  // setup internal table from channel control structure
  set_target( target );
  
  // calculate maximum number of steps
  uint16_t max_step_index = 0;
  for(uint8_t motor = 0; motor < 6; motor++)
  {
    if (mtr_control_table[motor].steps_left > mtr_control_table[max_step_index].steps_left)
    {
      max_step_index = motor;
    }

#ifdef DEBUG_VERBOSE_TRACE
    Serial.print("channel ");
    Serial.print(motor);
    Serial.print(" steps_left ");
    Serial.print(mtr_control_table[motor].steps_left);
    Serial.print(" dir ");
    Serial.print(mtr_control_table[motor].dir);
    Serial.print(" step_index ");
    Serial.print(mtr_control_table[motor].step_index);
    Serial.print(" offset ");
    Serial.print(mtr_control_table[motor].offset);
    Serial.println();
#endif
  }
  
  unsigned long current_time = 0UL;
  MTR_CTRL* mtr_ctrl;
  
  const uint16_t * const total_steps_left = &mtr_control_table[max_step_index].steps_left;
  while ( *total_steps_left > 0 )
  {
    // wait until appropriate delay has passed:
    current_time = millis();
    if ((current_time - previous_time) >= step_interval)
    {
      // save last time we stepped the motor
      previous_time = current_time;
     
      // step active channels  
      for(uint8_t motor=0; motor<6; motor++)
      {
        mtr_ctrl = &mtr_control_table[ motor ];
        if ( mtr_ctrl->steps_left > 0 )
        {
          pulse_stepper_motor( mtr_ctrl );
        }
      }
    }
  }
  
  // ensure Armdroid is returned to Input mode
  armdroid_write( STROBE );  
}

/*
 * Drives multiple motors asynchronously
 */
void ArmBase::driveMotorsAsynchronous(void)
{
	if (isRunning())
	{
		unsigned long current_time = millis();
		if ((current_time - previous_time) >= step_interval)
    	{
			previous_time = current_time;
			
			uint32_t steps_left = 0;
			for(uint8_t motor=0; motor<6; motor++)
			{
				MTR_CTRL* const mtr_ctrl = &mtr_control_table[ motor ];
				if ( mtr_ctrl->steps_left > 0 )
				{
					pulse_stepper_motor( mtr_ctrl );
				}
				
				steps_left += mtr_ctrl->steps_left;
			}
			
			if (steps_left == 0)
			{
				asyncState = ASYNC_DRIVE_STOPPED;

				// ensure Armdroid is returned to Input mode
  				armdroid_write( STROBE );
			}
		}
	}
}

bool ArmBase::Start(MTR_CHANNELS target)
{
	set_target( target );
	asyncState = ASYNC_DRIVE_RUNNING;
	return true;
}

bool ArmBase::Resume(void)
{
	if (asyncState == ASYNC_DRIVE_RUNNING)
		return true;
	else if (asyncState == ASYNC_DRIVE_PAUSED)
	{
		asyncState = ASYNC_DRIVE_RUNNING;
		return true;
	}
	return false;
}

bool ArmBase::Pause(void)
{
	if (asyncState == ASYNC_DRIVE_PAUSED)
		return true;
	else if (asyncState == ASYNC_DRIVE_RUNNING)
	{
		asyncState = ASYNC_DRIVE_PAUSED;
		return true;
	}
	return false;
}

bool ArmBase::Stop(void)
{
	asyncState = ASYNC_DRIVE_STOPPED;
	return true;
}


/*
 * Allows the user to manually reposition all joints
 * by freeing holding torque.
 *
 * To free motors, call with torqueEnabled = false 
 */
void ArmBase::torqueMotors(boolean torqueEnabled)
{
#ifndef INTERFACE_PROTOTYPE
  for(uint8_t motor = 0; motor < 6; motor++)
  {
    MTR_CTRL* const mtr_ctrl = &mtr_control_table[ motor ];
    
    // combine with coils off pattern + control bits if disabling holding torque, otherwise
    // reinstate coil pattern from last step index
    const uint8_t output = (torqueEnabled ? mtr_waveform_table[ mtr_ctrl->step_index ] : FREEARM) + mtr_ctrl->address + STROBE;
    
    // write command to Armdroid port
    armdroid_write( output );
    armdroid_write( output - STROBE );
    
    delay(1);
  }
  
  // ensure Armdroid is returned to Input mode
  armdroid_write( STROBE );
#endif
}

/*
 * Private method to pulse a stepper motor by precisely one step.
 * Maintains all internal counters
 */
void ArmBase::pulse_stepper_motor(MTR_CTRL *mtr_ctrl)
{
	// output byte initially contains motor address and strobe HIGH
	uint8_t output = mtr_ctrl->address + STROBE;

#if defined(INTERFACE_PROTOTYPE) && INTERFACE_PROTOTYPE > 1
  // generate clock pulse for on-board logic drive
  if (mtr_ctrl->step_index != 0)
    output += CCLK;
  mtr_ctrl->step_index = !mtr_ctrl->step_index;
  // direction control
  if (mtr_ctrl->dir == 0) {
    // counter-clockwise
    mtr_ctrl->offset--;
    output += CDIR;
  }
  else {
    // clockwise
    mtr_ctrl->offset++;
  }
#else
  // calculate next step from waveform table
  if (mtr_ctrl->dir == 1) {
    // increment offset counter
    mtr_ctrl->offset++;
    // increment waveform index 
    mtr_ctrl->step_index++;
    if (mtr_ctrl->step_index == WAVE_TABLE_SIZE)
       mtr_ctrl->step_index = 0;
  }
  else {
    // decrement offset counter
    mtr_ctrl->offset--;
    // decrement waveform index
    if (mtr_ctrl->step_index == 0)
       mtr_ctrl->step_index = WAVE_TABLE_SIZE;
    mtr_ctrl->step_index--;
  }

  // combine motor coil data + control bits
  output += mtr_waveform_table[ mtr_ctrl->step_index ];
#endif

  // write command to Armdroid port
  armdroid_write( output );
  armdroid_write( output - STROBE );
  
  // decrement steps remaining
  mtr_ctrl->steps_left--;
}

/*
 * Private method to set channel steps & direction
 */
void ArmBase::set_target(MTR_CHANNELS target)
{
	mtr_control_table[0].steps_left = abs(target.channel_1);
	mtr_control_table[0].dir        = (target.channel_1 > 0);
	mtr_control_table[1].steps_left = abs(target.channel_2);
	mtr_control_table[1].dir        = (target.channel_2 > 0);
	mtr_control_table[2].steps_left = abs(target.channel_3);
	mtr_control_table[2].dir        = (target.channel_3 > 0);
	mtr_control_table[3].steps_left = abs(target.channel_4);
	mtr_control_table[3].dir        = (target.channel_4 > 0);
	mtr_control_table[4].steps_left = abs(target.channel_5);
	mtr_control_table[4].dir        = (target.channel_5 > 0);
	mtr_control_table[5].steps_left = abs(target.channel_6);
	mtr_control_table[5].dir        = (target.channel_6 > 0);
}
