#include <Servo.h>


//****************************************************//
//******************Arduino Pin constants*************//
//****************************************************//
const int pwmPin = 3;
const int powerToGateDrivePin = 8;
const int servoPitchPin = 9;
const int killSwitchPin = 1;
const int turbineVoltagePin = 0;
const int loadVoltagePin = 5; // This value is to be changed when we make the circuit
const double VoltageBuffer = 1.5;


//****************************************************//
//****************Control System Constants************//
//****************************************************//
const int optimalPitchAngleData[7] = {60, 63, 55, 54, 55, 47, 47}; //Optimal pitch angles from 5m/s to 11m/s

const int BRAKE_PITCH = 145; //Need to verify pitch for new turbine
const int STARTUP_PITCH = 45; //Need to verify pitch for new turbine
const double VOLTAGE_DIVIDER_TURBINE = 13.015; //This should be the same as last year so we are good
const double VOLTAGE_DIVIDER_LOAD = 11.13;//This needs to be calculated for our new load
const double MAX_VOLTAGE = 45;
static double inferredWindSpeed;

Servo pitch;

//****************************************************//
//****************Function Prototypes*****************//
//****************************************************//

double inferWindSpeed();
boolean determinePCCDisconnect(double turbineVoltage, double loadVoltage);
int determineOptimumPitch(double);
void processDisconnectedState(boolean disconnected);
void processKillState(boolean killSwitch);


void setup(){
  Serial.begin(9600);
  pinMode(pwmPin, OUTPUT);                 //Configures PWM pin as output
  //pinMode(servoPitch, OUTPUT);  Don't think I need this //configures servo pin as output
  pinMode(loadVoltagePin, INPUT);
  pinMode(turbineVoltagePin, INPUT);
  pinMode(killSwitchPin, INPUT);
  pitch.attach(servoPitchPin);
  digitalWrite(powerToGateDrivePin, HIGH);
  Serial.println("Beginning the control system code");
}

void loop(){

  //****************************************************//
  //*****Variables to be used in the control system*****//
  //****************************************************//

  double turbineVoltage;
  double loadVoltage;
  double rotorRPM;
  double power;
  double resistorValue;
  int optimalDutyRatio;
  boolean killSwitch;
  boolean disconnected;
  static boolean disconnectedState = false;
  static boolean killState = false;

  turbineVoltage = VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023.0;

  //*****************************************************************//
  //**********Will read load voltage here when it is done************//
  //*****************************************************************//
  killSwitch = (digitalRead(A1) == LOW);
  disconnectedState = determinePCCDisconnect(turbineVoltage, 5); //5 is a placeholder here for load voltage
  processDisconnectedState(disconnectedState);
  processKillState(killSwitch);

}

void processDisconnectedState(boolean disconnected){
  if(disconnected)
    pitch.write(BRAKE_PITCH);
  delay(1000);
  return;
}

void processKillState(boolean killSwitchState){
  if(killSwitchState){
    Serial.println("Kill switch is hit. Braking the turbine.");
  }
}

boolean determinePCCDisconnect(double turbineVoltage, double loadVoltage){
  if(turbineVoltage-loadVoltage > VoltageBuffer)
    return true;
  return false;
}

