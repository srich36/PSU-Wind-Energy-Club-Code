#include <Servo.h>

//All pins are by default input so you don't have to specify input,and you do not on analog pins.

//You do not have to configure for analogread input when reading a voltage from something. If you want to read a voltage it has to be connected to these
//pins because they can have a range of values.

//however, it is still good to confiure as input to see what your pins are, because even though it configures them as digital input, analogread converts them
//to analog

//Analogwrite also sets the pin to output when called

//****************************************************//
//******************Questions to Ask:*************//
//****************************************************//
//
//1. Is resistance constant? If not, which resistance should I use? Because it is always changing in the equation and optimal power outputs. 
//
//2. How are we determining wind speed? Using the data that has a given Vin and given power output at
//
//3. On the resistance power curve data plot, are those optimal pitch angles at the given wind speed all at a 50ohm resistance? Is that what we are operating at constantly?
//
//4. So we want to fix power, do we need to find the RPM at the optimal pitch angle at 11 m/s and calculate that power
// Then keep that is constant in the duty cycle equation? Because if so, then we need to test more at 11m/s because we don't have
// data there at a 50 ohm resistance.



////////////////////////////////////
/////Note to self: analogWrite and read are analog functions and thus default to analog pins
///// while digitalWrite and read are digital functions and thus expect digital pins as default
///// however, you can say A0 in digitalread to make it read the analog pin
/////////////




//****************************************************//
//******************Arduino Pin constants*************//
//****************************************************//
const int pwmPin = 3;               //Digital
const int powerToGateDrivePin = 8;  //Digital
const int servoPitchPin = 9;        //Digital
const int killSwitchPin = 1;        //Digital
const int turbineVoltagePin = 0;    //Analog
const int loadVoltagePin = 5; // This value is to be changed when we make the circuit

Servo pitch;

//****************************************************//
//****************Control System Constants************//
//****************************************************//

const int BRAKE_PITCH = 110; //Need to verify pitch for new turbine -> should be verified now
const int STARTUP_PITCH = 45; //Need to verify pitch for new turbine -> should be verified now
const double VOLTAGE_DIVIDER_TURBINE = 13.015; //This should be the same as last year so we are good
const double VOLTAGE_DIVIDER_LOAD = 11.13;//This needs to be calculated for our new load
const double MAX_VOLTAGE = 45;
const int RESISTANCE = 50; //Is this a constant or does it change? 



//****************************************************//
//****************Function Prototypes*****************//
//****************************************************//

double inferWindSpeed(double voltageIn);
boolean determinePCCDisconnect(double turbineVoltage, double loadVoltage);
int determineOptimumPitch(double);
void processDisconnectedState();
void processKillState(boolean killSwitch);
int calculateDutyCycle11(double resistance, double power, double turbineVoltage);
double getRPMfromVoltageIn(double VoltageIn);
double calculatePowerFromRPM(double RPM);


void setup(){
  Serial.begin(9600);
  pinMode(pwmPin, OUTPUT);                 //Configures PWM pin as output
  //pinMode(servoPitch, OUTPUT);  Don't think I need this //configures servo pin as output
  //pinMode(loadVoltagePin, INPUT);      //Might need to put A's in front of these because they are analog pins
  //pinMode(A0, INPUT);   //But don't think so because I'm calling analog read on them
  //pinMode(killSwitchPin, INPUT);
  pitch.attach(servoPitchPin);
  digitalWrite(powerToGateDrivePin, HIGH);
  Serial.println("Beginning the control system code");
  pitch.write(STARTUP_PITCH);
}

void loop(){

  //****************************************************//
  //*****Variables to be used in the control system*****//
  //****************************************************//

  double turbineVoltage;
  double loadVoltage;
  double inferredWindSpeed;
  boolean killSwitch;
  boolean disconnected;
  
  double RPM;
  int dutyCycle;
  static boolean disconnectedState = false;
  static boolean killState = false;

  turbineVoltage = VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023.0;
  RPM = getRPMfromVoltageIn(turbineVoltage);

  //*****************************************************************//
  //**********Will read load voltage here when it is done************//
  //*****************************************************************//
  
  
  killSwitch = (digitalRead(A1) == LOW);
  disconnectedState = determinePCCDisconnect(turbineVoltage, loadVoltage);
  processDisconnectedState(disconnectedState);                                  //May need to make sure it doesn't pitch away from brake pitch immediately
  processKillState(killSwitch);
  inferredWindSpeed = inferWindSpeed(turbineVoltage);
  if(inferredWindSpeed < 11){
    dutyCycle = calculateDutyCycle(RESISTANCE, calculatePowerFromRPM(RPM), turbineVoltage);
    analogWrite(pwmPin, dutyCycle);
  }
  else{
    dutyCycle = -1; //Placeholder to find out duty cycle given constant power. Depends on answers to the above questions
  }


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

double inferWindSpeed(double voltageIn){
  return 0;
}

boolean determinePCCDisconnect(double turbineVoltage, double loadVoltage){
  return false;
}

double getRPMfromVoltageIn(double turbineVoltage){
  return 1; //Placeholder for more data that needs to go in here.
}

double calculatePowerFromRPM(double RPM){
  return -7.97*10E-11*pow(RPM,3)+1.11*10E-5*pow(RPM,2)-4.97*10E-3*RPM-1.91;
}

int calculateDutyCycle(double resistance, double power, double turbineVoltage){
  return int(sqrt(resistance*power)/turbineVoltage);
}

