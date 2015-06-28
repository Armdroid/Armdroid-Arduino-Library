/*
   Arduino Yun Wi-Fi-controller sketch
   Copyright Richard Morris 2015. All Rights Reserved
   http://armdroid1.blogspot.co.uk

   This sketch must be uploaded via wifi.
   REST API must be set to "open".

   Prepare your SD card with an empty folder in the SD root
   named "arduino" and a subfolder of that named "www".
   This will ensure that the Yún will create a link
   to the SD to the "/mnt/sd" path.

   Included with this sketch folder is a basic webpage.
   When you upload your sketch, these files
   will be placed in the /arduino/www/YunWebController folder on your SD card.
   
   You can then go to http://192.168.240.1/sd/YunWebController
   to interact with this sketch.
*/


#include <Armdroid.h>
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

// Armdroid channel definitions
#define ARMDROID_GRIPPER_CHANNEL       1
#define ARMDROID_LHS_WRIST_CHANNEL     2
#define ARMDROID_RHS_WRIST_CHANNEL     3
#define ARMDROID_ELBOW_CHANNEL         4
#define ARMDROID_SHOLDER_CHANNEL       5
#define ARMDROID_BASE_CHANNEL          6

#define DEF_MOTOR_SPEED           120

// Arduino Wiring Pins used by ArmdroidShield
const int armD1Pin = 2;
const int armD2Pin = 3;
const int armD3Pin = 4;
const int armD4Pin = 5;
const int armD5Pin = 6;
const int armD6Pin = 7;
const int armD7Pin = 8;
const int armD8Pin = 9;

// Create an Armdroid controller object
Armdroid armdroid(armD1Pin, armD2Pin, armD3Pin, armD4Pin, armD5Pin, armD6Pin, armD7Pin, armD8Pin);

// next revision of Armdroid Library will likly include a method to
// retrieve the current torque setting, for now, we'll simply maintain a boolean for this purpose:
bool bTorqued;

// Listen on default port 5555, the webserver on the Yun
// will forward there all HTTP requests for us.
YunServer server;

void setup() {
  // Armdroid
  bTorqued = true;
  armdroid.torqueMotors(bTorqued);

  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  
  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // Get clients coming from server
  YunClient client = server.accept();
  if (client)
  {
    client.setTimeout(5);
    process(client);
    client.stop();
  }
  delay(50); // Poll every 50ms
}

// process called when the REST API is called
void process(YunClient client)
{
  // read the command
  String command = client.readStringUntil('/');
  
  if (command.startsWith("armdroid"))
    armdroidCommand(client);
}

// process armdroid commands
void armdroidCommand(YunClient client)
{
  // read the command
  String command = client.readStringUntil('/');

  if (command.startsWith("base")) {
    baseCommand(client);
  }
  else if (command.startsWith("shoulder")) {
    shoulderCommand(client);
  }
  else if (command.startsWith("elbow")) {
    elbowCommand(client);
  }
  else if (command.startsWith("wrist")) {
    wristCommand(client);
  }
  else if (command.startsWith("gripper")) {
    gripperCommand(client);
  }
  else if (command.startsWith("torque")) {
    torqueCommand(client);
  }
  else if (command.startsWith("home")) {
    homeCommand(client);
  }
  else
    client.println(F("ERROR: invalid armdroid command"));
}

void baseCommand(YunClient client)
{
  String baseCmd = client.readStringUntil('/');
  if (baseCmd.startsWith("position")) {
    
    // read number of steps, if none have been specified, parseInt()
    // will simply return zero which will be ignored, and we'll
    // finish by feeding back current position to the client.
    const int steps = client.parseInt();
    if (steps != 0) {
      
      // if the URL includes a speed value, use it
      int whatSpeed = DEF_MOTOR_SPEED;
      if (client.read() == '/') {
        const int speedInRpm = client.parseInt();
        if (speedInRpm > 0)
          whatSpeed = speedInRpm;
      }
      
      // drive motor to rotate base clockwise/counterclockwise
      driveMotor( ARMDROID_BASE_CHANNEL, steps, whatSpeed );
    }
    
    // send feedback to client
    client.print(F("{\"base-position\":"));
    client.print(getPositionReading( ARMDROID_BASE_CHANNEL ));
    client.println(F("}"));
  }
  else if (baseCmd.startsWith("sensor")) {
    client.print(F("{\"base-sensor\":"));
    client.print(getSensorReading( ARMDROID_BASE_CHANNEL ));
    client.println(F("}"));
  }
  else
    client.println(F("ERROR: invalid base command"));
}

void shoulderCommand(YunClient client)
{
  String shldrCmd = client.readStringUntil('/');
  if (shldrCmd.startsWith("position")) {
    
    // read number of steps
    const int steps = client.parseInt();
    if (steps != 0) {
      
      int whatSpeed = DEF_MOTOR_SPEED;
      if (client.read() == '/')
      {
        const int speedInRpm = client.parseInt();
        if (speedInRpm > 0)
          whatSpeed = speedInRpm;
      }
      
      // drive motor to raise/lower shoulder function
      driveMotor( ARMDROID_SHOLDER_CHANNEL, steps, whatSpeed );
    }
    
    client.print(F("{\"shoulder-position\":"));
    client.print(getPositionReading( ARMDROID_SHOLDER_CHANNEL ));
    client.println(F("}"));
  }
  else if (shldrCmd.startsWith("sensor")) {
    client.print(F("{\"shoulder-sensor\":"));
    client.print(getSensorReading( ARMDROID_SHOLDER_CHANNEL ));
    client.println(F("}"));
  }
  else
    client.println(F("ERROR: invalid shoulder command"));
}

void elbowCommand(YunClient client)
{
  String elbowCmd = client.readStringUntil('/');
  if (elbowCmd.startsWith("position")) {
    
    // read number of steps
    const int steps = client.parseInt();
    if (steps != 0) {
      
      int whatSpeed = DEF_MOTOR_SPEED;
      if (client.read() == '/') {
        const int speedInRpm = client.parseInt();
        if (speedInRpm > 0)
          whatSpeed = speedInRpm;
      }
      
      // drive motor to raise/lower elbow function
      driveMotor( ARMDROID_ELBOW_CHANNEL, steps, whatSpeed );
    }
    
    client.print(F("{\"elbow-position\":"));
    client.print(getPositionReading( ARMDROID_ELBOW_CHANNEL ));
    client.println(F("}"));
  }
  else if (elbowCmd.startsWith("sensor")) {
    client.print(F("{\"elbow-sensor\":"));
    client.print(getSensorReading( ARMDROID_ELBOW_CHANNEL ));
    client.println(F("}"));
  }
  else
    client.println(F("ERROR: invalid elbow command"));
}

void wristCommand(YunClient client)
{
  String wristCmd = client.readStringUntil('/');
  if (wristCmd.startsWith("pitch")) {
    
    // read number of steps
    const int steps = client.parseInt();
    if (steps != 0) {
      
      int whatSpeed = DEF_MOTOR_SPEED;
      if (client.read() == '/') {
        const int speedInRpm = client.parseInt();
        if (speedInRpm > 0)
          whatSpeed = speedInRpm;
      }
      
      // drive both wrist motors to perform pitch motion
      driveWristMotors(steps, -steps, whatSpeed);
    }
    
    // send feedback to client
    client.print(F("{\"left-wrist-position\":"));
    client.print(getPositionReading( ARMDROID_LHS_WRIST_CHANNEL ));
    client.print(F(",\"right-wrist-position\":"));
    client.print(getPositionReading( ARMDROID_RHS_WRIST_CHANNEL ));
    client.println(F("}"));
  }
  else if (wristCmd.startsWith("rotate")) {

    // read number of steps
    const int steps = client.parseInt();
    if (steps != 0) {
      
      int whatSpeed = DEF_MOTOR_SPEED;
      if (client.read() == '/') {
        const int speedInRpm = client.parseInt();
        if (speedInRpm > 0)
          whatSpeed = speedInRpm;
      }
      
      // drive both wrist motors to perform roll motion
      driveWristMotors(steps, steps, whatSpeed);
    }
    
    // send feedback to client
    client.print(F("{\"left-wrist-position\":"));
    client.print(getPositionReading( ARMDROID_LHS_WRIST_CHANNEL ));
    client.print(F(",\"right-wrist-position\":"));
    client.print(getPositionReading( ARMDROID_RHS_WRIST_CHANNEL ));
    client.println(F("}"));
  }
  else if (wristCmd.startsWith("left"))
  {    
    String lhsWristSubCmd = client.readStringUntil('/');
    if (lhsWristSubCmd.startsWith("position")) {
      
      // read number of steps
      const int steps = client.parseInt();
      if (steps != 0) {
        
        int whatSpeed = DEF_MOTOR_SPEED;
        if (client.read() == '/') {
          const int speedInRpm = client.parseInt();
          if (speedInRpm > 0)
            whatSpeed = speedInRpm;
        }
        
        // drive left-hand-side wrist motor
        driveMotor( ARMDROID_LHS_WRIST_CHANNEL, steps, whatSpeed );
      }
      
      // send feedback to client
      client.print(F("{\"left-wrist-position\":"));
      client.print(getPositionReading( ARMDROID_LHS_WRIST_CHANNEL ));
      client.println(F("}"));
    }
    else if (lhsWristSubCmd.startsWith("sensor")) {
      client.print(F("{\"left-wrist-sensor\":"));
      client.print(getSensorReading( ARMDROID_LHS_WRIST_CHANNEL ));
      client.println(F("}"));
    }
    else
      client.println(F("ERROR: invalid left wrist action"));
  }
  else if (wristCmd == "right") {
    String rhsWristSubCmd = client.readStringUntil('/');
    if (rhsWristSubCmd.startsWith("position")) {
      
      // read number of steps
      const int steps = client.parseInt();
      if (steps != 0) {

        int whatSpeed = DEF_MOTOR_SPEED;
        if (client.read() == '/') {
          const int speedInRpm = client.parseInt();
          if (speedInRpm > 0)
            whatSpeed = speedInRpm;
        }

        // drive right-hand-side wrist motor
        driveMotor( ARMDROID_RHS_WRIST_CHANNEL, steps, whatSpeed );
      }
      
      // send feedback to client
      client.print(F("{\"right-wrist-position\":"));
      client.print(getPositionReading( ARMDROID_RHS_WRIST_CHANNEL ));
      client.println(F("}"));
    }
    else if (rhsWristSubCmd.startsWith("sensor")) {
      client.print(F("{\"right-wrist-sensor\":"));
      client.print(getSensorReading( ARMDROID_RHS_WRIST_CHANNEL ));
      client.println(F("}"));
    }
    else
      client.println(F("ERROR: invalid right wrist action"));
  }
  else
    client.println(F("ERROR: invalid wrist command"));
}

void gripperCommand(YunClient client)
{
  String gripCmd = client.readStringUntil('/');
  if (gripCmd.startsWith("position")) {
    
    // read number of steps
    const int steps = client.parseInt();
    if (steps != 0) {
      
      int whatSpeed = DEF_MOTOR_SPEED;
      if (client.read() == '/') {
        const int speedInRpm = client.parseInt();
        if (speedInRpm > 0)
          whatSpeed = speedInRpm;
      }
      
      // drive motor to open/close gripper
      driveMotor( ARMDROID_GRIPPER_CHANNEL, steps, whatSpeed );
    }
    
    client.print(F("{\"gripper-position\":"));
    client.print(getPositionReading( ARMDROID_GRIPPER_CHANNEL ));
    client.println(F("}"));
  }
  else if (gripCmd.startsWith("sensor")) {
    client.print(F("{\"gripper-sensor\":"));
    client.print(getSensorReading( ARMDROID_GRIPPER_CHANNEL ));
    client.println(F("}"));
  }
  else
    client.println(F("ERROR: invalid gripper command"));
}

// apply or release torque for all motors
void torqueCommand(YunClient client)
{
  String torqueCmd = client.readStringUntil('/');
  if (torqueCmd.startsWith("enabled")) {
    // torque enabled if we receive >= 1
    const int value = client.parseInt();
    bTorqued = (value >= 1);
    // release or apply torque here
    armdroid.torqueMotors( bTorqued );
    // reset offset counters when torque is re-applied
    if (bTorqued)
      armdroid.resetOffsetCounts();
  }
  
  // send feedback to client
  client.print(F("{\"torque\":\""));
  client.print(bTorqued);
  client.println(F("\"}"));
}

// return to home position
void homeCommand(YunClient client)
{
  driveToHomePosition();
  client.println(F("{\"home-position\":\"OK\"}"));
}

// wrist mover method - supports pitch/rotation
void driveWristMotors(int leftWrist, int rightWrist, int speedRpm)
{
  const int stepRatio = 1;
  const MTR_CHANNELS channels = {0, leftWrist*stepRatio, rightWrist*stepRatio, 0, 0, 0};
  armdroid.setSpeed( speedRpm );
  armdroid.driveAllMotors( channels );
}

// main mover method
void driveMotor(int channel, int relativeSteps, int speedRpm)
{
  armdroid.setSpeed( speedRpm );
  armdroid.driveMotor( channel, relativeSteps );
}

// returns all functions to home position
void driveToHomePosition(void)
{
  MTR_CHANNELS offsets = armdroid.getOffsets();
  offsets.channel_1 = -offsets.channel_1;
  offsets.channel_2 = -offsets.channel_2;
  offsets.channel_3 = -offsets.channel_3;
  offsets.channel_4 = -offsets.channel_4;
  offsets.channel_5 = -offsets.channel_5;
  offsets.channel_6 = -offsets.channel_6;
  armdroid.setSpeed( DEF_MOTOR_SPEED );
  armdroid.driveAllMotors( offsets );
}

// returns the current position for a given motor channel
int getPositionReading(int channel)
{
  const MTR_CHANNELS offsets = armdroid.getOffsets();
  switch (channel)
  {
    case 1:  return offsets.channel_1;
    case 2:  return offsets.channel_2;
    case 3:  return offsets.channel_3;
    case 4:  return offsets.channel_4;
    case 5:  return offsets.channel_5;
    case 6:  return offsets.channel_6;
    default: return 0;   
  }
}

int getSensorReading(int channel)
{
  // TODO not currently implemented
  return 0;
}

