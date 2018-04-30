//TODO: 
//     1. Test EEPROM memory. 
//     2. Figure out how to write the infer wind speed function
//     3. Confirm pin constants
//     4. Figure out more logic to control how pitch works after 10m/s



#include <Servo.h>
#include <EEPROM.h>

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
//A: the effective resistance is changing through the duty cycle. The r in the equations is that constant 50
//
//2. How are we determining wind speed? Using the data that has a given Vin and given power output at
//A: Gven inputs of voltage, RPM, and power, we can estimate the wind speed 
//
//3. On the resistance power curve data plot, are those optimal pitch angles at the given wind speed all at a 50ohm resistance? Is that what we are operating at constantly?
//A: We are just doing buckets of pitch angles for a given wind speed
//
//4. So we want to fix power, do we need to find the RPM at the optimal pitch angle at 11 m/s and calculate that power
// Then keep that is constant in the duty cycle equation? Because if so, then we need to test more at 11m/s because we don't have
// data there at a 50 ohm resistance.
//A: Yes the power at 11m/s is a constant we will maintain.



////////////////////////////////////
/////Note to self: analogWrite and read are analog functions and thus default to analog pins
///// while digitalWrite and read are digital functions and thus expect digital pins as default
///// however, you can say A0 in digitalread to make it read the analog pin
/////////////




//****************************************************//
//******************Arduino Pin constants*************//
//****************************************************//
const int PWM_PIN = 3;               //Digital
const int LOAD_ARDUINO_PIN = 8;      //Digital 
const int SERVO_PITCH_PIN = 9;        //Digital
const int KILL_SWITCH_PIN = 1;        //Digital
const int TURBINE_vOLTAGE_PIN = 0;    //Analog
const int PWM_CONVERSION = 255;       //the arduino operates on a 0-255 scale for pwm so the duty cycle needs to be within this range
const int FIVE_TO_SEVEN_PITCH_ANGLE = 61;
const int SEVEN_TO_TEN_PITCH_ANGLE = 57;
const int TEN_PLUS_INITIAL_PITCH_ANGLE = 48;
static int currentPitch;
static boolean breakedInCompetition = false;

//****************************************************//
//****************Control System Constants************//
//****************************************************//

const int BRAKE_PITCH = 110; //Need to verify pitch for new turbine -> should be verified now
const int STARTUP_PITCH = 45; //Need to verify pitch for new turbine -> should be verified now
const double VOLTAGE_DIVIDER_TURBINE = 13.015; //This should be the same as last year so we are good
const double VOLTAGE_DIVIDER_LOAD = 11.13;//This needs to be calculated for our new load
const double MAX_VOLTAGE = 45;
const int RESISTANCE = 50; //Is this a constant or does it change? 
const double POWER_AT_11MS = 45.5;

Servo pitch;

//****************************************************//
//****************Function Prototypes*****************//
//****************************************************//

double inferWindSpeed(double voltageIn, double RPM, double Power);
//int determineOptimumPitch(double);
//void processDisconnectedState();
int calculateDutyCycle11(double resistance, double power, double turbineVoltage);
double getRPMfromVoltageIn(double VoltageIn);
double calculatePowerFromRPM(double RPM);
void pitchToPitchAngleBucket(double windSpeed);

void setup(){
  Serial.begin(9600);
  pinMode(PWM_PIN, OUTPUT);  //Configures PWM pin as output
  pinMode(LOAD_ARDUINO_PIN, INPUT);
  //pinMode(servoPitch, OUTPUT);  Don't think I need this //configures servo pin as output
  //pinMode(A0, INPUT);   //But don't think so because I'm calling analog read on them
  pitch.attach(SERVO_PITCH_PIN);
  //digitalWrite(powerToGateDrivePin, HIGH);             //Not sure if we need this because the gate drive might not be working
  Serial.println("Beginning the control system code");
  if(EEPROM.read(0)){
    pitch.write(EEPROM.read(0+1));
  }
  else{
    pitch.write(STARTUP_PITCH);
  }
}

void loop(){

  //****************************************************//
  //*****Variables to be used in the control system*****//
  //****************************************************//

  double turbineVoltage;
  double inferredWindSpeed;
  boolean breakNeeded;
  double RPM;
  double power;
  int dutyCycle;

  if(digitalRead(LOAD_ARDUINO_PIN) == HIGH){
    breakNeeded = true;
    processDisconnectedState(breakNeeded);
  }
  else
    breakNeeded = false; 
  turbineVoltage = VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023.0;
  RPM = getRPMfromVoltageIn(turbineVoltage);
  power = calculatePowerFromRPM(RPM);                           
  inferredWindSpeed = inferWindSpeed(turbineVoltage, RPM, power);
  if(inferredWindSpeed < 11){
    dutyCycle = calculateDutyCycle(RESISTANCE, calculatePowerFromRPM(RPM), turbineVoltage);
    analogWrite(PWM_PIN, dutyCycle);
  }
  else{
    dutyCycle = calculateDutyCycle(RESISTANCE, POWER_AT_11MS, turbineVoltage); //Placeholder to find out duty cycle given constant power. Depends on answers to the above questions
    analogWrite(PWM_PIN, dutyCycle);
  }


}

void processDisconnectedState(boolean disconnected){
  if(disconnected){
    breakedInCompetition = true;
    EEPROM.write(0,breakedInCompetition);
    EEPROM.write(0+1,currentPitch);
    pitch.write(BRAKE_PITCH);
  }
  else{
    breakedInCompetition = false;
    EEPROM.write(0,breakedInCompetition);
  }
  delay(10000);
  return;
}

double inferWindSpeed(double voltageIn, double RPM, double power){
  return 0;
}



double getRPMfromVoltageIn(double turbineVoltage){
  return 108*turbineVoltage-462; //Placeholder for more data that needs to go in here.
}

double calculatePowerFromRPM(double RPM){
  return -7.97*10E-11*pow(RPM,3)+1.11*10E-5*pow(RPM,2)-4.97*10E-3*RPM-1.91;
}

int calculateDutyCycle(double resistance, double power, double turbineVoltage){
  int dutyCycle = PWM_CONVERSION*int(sqrt(resistance*power)/turbineVoltage);
  if(dutyCycle > 255){
    Serial.print("Over 255 duty cycle. Something is wrong. Duty cycle is equal to: ");
    Serial.println(dutyCycle);
  }
  else{
    Serial.print("Sending a duty cycle of: ");
    Serial.println(dutyCycle);
  }
  
  return dutyCycle;
}

void pitchToPitchAngleBucket(double windSpeed){
  if(windSpeed > 5 && windSpeed < 7){
    pitch.write(FIVE_TO_SEVEN_PITCH_ANGLE);
    currentPitch = FIVE_TO_SEVEN_PITCH_ANGLE;
  }
  else if(windSpeed>=7 && windSpeed < 10){
    pitch.write(SEVEN_TO_TEN_PITCH_ANGLE);
    currentPitch = SEVEN_TO_TEN_PITCH_ANGLE;
  }
  else if(windSpeed > 10){
    pitch.write(TEN_PLUS_INITIAL_PITCH_ANGLE);
    currentPitch = TEN_PLUS_INITIAL_PITCH_ANGLE;
  }

}

