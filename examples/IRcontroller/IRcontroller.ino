/*
   Armdroid IR remote controller sketch
   Infrared remote control signals are decoded to control Armdroid functions
   using a standard Sky remote control.  Other remotes may be used, but you'll
   need to change the key codes to match your hardware.

   Copyright Richard Morris 2014 - 2015. All Rights Reserved
   http://armdroid1.blogspot.co.uk

   Arrow Keys = base rotation & shoulder up/down
   Channel Keys = Elbow up/down
   back up = return Armdroid to home position
   Red = gripper pitch up
   Green = gripper pitch down
   Yellow = gripper roll ccw
   Blue = gripper roll cw
   text = gripper open
   help = gripper close

   sketch requires the IRremote library downloadable from:
   https://github.com/shirriff/Arduino-IRremote
   
   for sensor wiring and more information:
   http://armdroid1.blogspot.co.uk/2015/01/infrared-remote-control.html
*/

#include <Armdroid.h>
#include <IRremote.h>

// Arduino Wiring Pins
const int armD1Pin      = 2;
const int armD2Pin      = 3;
const int armD3Pin      = 4;
const int armD4Pin      = 5;
const int armD5Pin      = 6;
const int armD6Pin      = 7;
const int armD7Pin      = 8;
const int armD8Pin      = 9;
const int irReceiverPin = 10;

/* direction macros */
#define DIR_CW    (+1)
#define DIR_CCW   (-1)

/* command table, maps remote key codes to command routines */
struct key_cmd_table {
  long irKeyCode;
  void (*cmd_fxn)(int,int);
  int  cmd_arg1;
  int  cmd_arg2;
} commands[] =
{
  /* Elbow */
  0xC00020, armdroid_mover, 4, DIR_CW,    // channel up
  0xC00021, armdroid_mover, 4, DIR_CCW,   // channel down
  
  /* Shoulder */
  0xC00058, armdroid_mover, 5, DIR_CCW,   // arrow up
  0xC00059, armdroid_mover, 5, DIR_CW,    // arrow down
  
  /* Base */
  0xC0005A, armdroid_mover, 6, DIR_CW,    // arrow left
  0xC0005B, armdroid_mover, 6, DIR_CCW,   // arrow right
  
  /* Gripper */
  0xC0003C, armdroid_mover, 1, DIR_CW,  
  0xC0006D, armdroid_gripper_mover, DIR_CW, DIR_CCW,
  0xC0006E, armdroid_gripper_mover, DIR_CCW, DIR_CW,
  0xC0006F, armdroid_gripper_mover, DIR_CW, DIR_CW,
  0xC00070, armdroid_gripper_mover, DIR_CCW, DIR_CCW,
  0xC00081, armdroid_mover, 1, DIR_CCW,
  
  /* preset positions */
  0xC000CC, armdroid_goto_point, 0, 0,    // tvguide
  0xC0007D, armdroid_goto_point, 1, 0,    // box office
  0xC0007E, armdroid_goto_point, 2, 0,    // services
  0xC000F5, armdroid_goto_point, 3, 0,    // interactive
  0xC00001, armdroid_store_point, 0, 0,
  0xC00002, armdroid_store_point, 1, 0,
  0xC00003, armdroid_store_point, 2, 0,
  0xC00004, armdroid_store_point, 3, 0,
  
  0xC00083, armdroid_goto_home, 0, 0,     // backup - home
};
const int numKeyCmds = (sizeof(commands) / sizeof(key_cmd_table));

// storage for custom preset positions
MTR_CHANNELS preset_point_store[4] = {};

/* create an Armdroid controller object */
Armdroid armdroid(armD1Pin, armD2Pin, armD3Pin, armD4Pin, armD5Pin, armD6Pin, armD7Pin, armD8Pin);

/* create IR receiver objects */
IRrecv irrecv(irReceiverPin);
decode_results irData;      // IR data goes here

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn();
}

void loop()
{
  if (irrecv.decode(&irData) == true)
  {
    Serial.print("decoded signal = ");
    Serial.println(irData.value, HEX);
  
    invokeKeyCommand(irData.value);
    
    irrecv.resume();
  }
}

/* Searches the command table, if a match is found,
   runs the associated routine */
int invokeKeyCommand(const long code)
{
  for (int i=0; i<numKeyCmds; i++)
  {
    if (commands[i].irKeyCode == code)
    {
      Serial.println("key found");
      (*commands[i].cmd_fxn)(commands[i].cmd_arg1, commands[i].cmd_arg2);
      return 1;
    }
  }
  Serial.println("key not found");
  return 0;
}

/* Armdroid mover method */
void armdroid_mover(int channel, int rotation)
{
  Serial.print("armdroid_mover - channel ");
  Serial.print(channel);
  Serial.print(" rotation ");
  Serial.println(rotation);
  
  const int stepRatio = 20;
  armdroid.setSpeed(50);
  armdroid.driveMotor( channel, 1*rotation*stepRatio );
  SerialPrintMotorOffsets();
}

/* Armdroid specialized gripper mover (pitch/roll) */
void armdroid_gripper_mover(int leftWrist, int rightWrist)
{
  Serial.print("armdroid_gripper_mover - lhs ");
  Serial.print(leftWrist);
  Serial.print(" rhs ");
  Serial.println(rightWrist);
  
  const int stepRatio = 30;
  const MTR_CHANNELS channels = {0, leftWrist*stepRatio, rightWrist*stepRatio, 0, 0, 0};
  armdroid.setSpeed(50);
  armdroid.driveAllMotors(channels);
  SerialPrintMotorOffsets();
}

/* Armdroid return to home position */
void armdroid_goto_home(int notUsed1, int notUsed2)
{
  Serial.println("armdroid_goto_home");
  
  MTR_CHANNELS offsets = armdroid.getOffsets();
  offsets.channel_1 = -offsets.channel_1;
  offsets.channel_2 = -offsets.channel_2;
  offsets.channel_3 = -offsets.channel_3;
  offsets.channel_4 = -offsets.channel_4;
  offsets.channel_5 = -offsets.channel_5;
  offsets.channel_6 = -offsets.channel_6;
  armdroid.setSpeed(150);
  armdroid.driveAllMotors(offsets);
  SerialPrintMotorOffsets();
}

/* routine to calculate relative offsets to target position */
static int calc_steps(int target, int current)
{
  if (target == current)
    return 0;
  else if (target > 0)
    return current > 0 ? target - current : target + (-current);
  else if (target < 0)
    return current < 0 ? target - current : -(current - target);
  else
    return -current;
}

/* Armdroid goto position (move to absolute offset) */
void armdroid_goto_point(int preset, int notUsed2)
{
  Serial.print("armdroid_goto_point - preset ");
  Serial.println(preset);
  
  const MTR_CHANNELS target = preset_point_store[preset];
  const MTR_CHANNELS current = armdroid.getOffsets();

  MTR_CHANNELS offsets;   
  offsets.channel_1 = calc_steps(target.channel_1, current.channel_1);
  offsets.channel_2 = calc_steps(target.channel_2, current.channel_2);
  offsets.channel_3 = calc_steps(target.channel_3, current.channel_3);
  offsets.channel_4 = calc_steps(target.channel_4, current.channel_4);
  offsets.channel_5 = calc_steps(target.channel_5, current.channel_5);
  offsets.channel_6 = calc_steps(target.channel_6, current.channel_6);

  armdroid.setSpeed(150);
  armdroid.driveAllMotors(offsets);
  SerialPrintMotorOffsets();
}

/* Armdroid store custom position */
void armdroid_store_point(int preset, int notUsed)
{
  Serial.print("armdroid_store_point - preset");
  Serial.println(preset);
  
  const MTR_CHANNELS offsets = armdroid.getOffsets();
  preset_point_store[preset] = offsets;
}

/* debugging */
void SerialPrintMotorOffsets()
{
  static char message[80];
  const MTR_CHANNELS offsets = armdroid.getOffsets();
  sprintf(message, "offsets = %d,%d,%d,%d,%d,%d", offsets.channel_1,
                                                  offsets.channel_2,
                                                  offsets.channel_3,
                                                  offsets.channel_4,
                                                  offsets.channel_5,
                                                  offsets.channel_6);
  Serial.println(message);
}

