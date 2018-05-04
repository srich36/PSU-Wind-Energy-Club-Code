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
//const int KILL_SWITCH_PIN = 2      //Analog
const int TURBINE_VOLTAGE_PIN = 0;    //Analog
const int PWM_CONVERSION = 255;       //the arduino operates on a 0-255 scale for pwm so the duty cycle needs to be within this range
const int FIVE_TO_SEVEN_PITCH_ANGLE = 61;
const int SEVEN_TO_TEN_PITCH_ANGLE = 57;
const int TEN_PLUS_INITIAL_PITCH_ANGLE = 48;
const double SIGNAL_BUFFER = .01;
const double THEORETICAL_VS_ACTUAL_VOLTAGE_BUFFER = .3;
static int currentPitch;
static boolean breakedInCompetition = false;

//****************************************************//
//****************Control System Constants************//
//****************************************************//

const int BRAKE_PITCH = 110; //Need to verify pitch for new turbine -> should be verified now
const int STARTUP_PITCH = 45; //Need to verify pitch for new turbine -> should be verified now
const double VOLTAGE_DIVIDER_TURBINE = 13.015; //This should be the same as last year so we are good
const double VOLTAGE_DIVIDER_LOAD = 14.327;//This needs to be calculated for our new load
const double VOLTAGE_DIFFERENT_BUFFER = .5;
const double LOAD_VOTLAGE_BUFFER = .5;
const double MAX_VOLTAGE = 45;
const int RESISTANCE = 50; //Is this a constant or does it change? 
const double POWER_AT_11MS = 45.5;

Servo pitch;

//****************************************************//
//****************Function Prototypes*****************//
//****************************************************//

double inferWindSpeed(double voltageIn, double RPM, double Power);
//int determineOptimumPitch(double);
void processDisconnectedState();
int calculateDutyCycle11(double resistance, double power, double turbineVoltage);
double getRPMfromVoltageIn(double VoltageIn);
double calculatePowerFromRPM(double RPM);
void pitchToPitchAngleBucket(double windSpeed);
boolean determineDisconnect(double, double);
double calculateTheoreticalOutputVoltage(double voltageIn, int dutyCycle);
void stabilizeVoltageGivenDutyCycle(int dutyCycle, double desiredVoltage); 


void setup(){
  Serial.begin(9600);
  
  //Change the timer settings
  TCCR2B = TCCR2B & 0b11111000 | 0x01;
  
  pinMode(PWM_PIN, OUTPUT);  //Configures PWM pin as output
  pinMode(LOAD_ARDUINO_PIN, OUTPUT);
  pinMode(A0, INPUT);        //For the turbine voltage
  pinMode(A1, INPUT);        //For the load voltage
  
  //pinMode(servoPitch, OUTPUT);  Don't think I need this //configures servo pin as output
  //pinMode(A0, INPUT);   //But don't think so because I'm calling analog read on them
  pitch.attach(SERVO_PITCH_PIN);
  Serial.println("Beginning the control system code");
  
  //CHECKING IF KILL SWITCH IS HIT
  if(digitalRead(A2 == HIGH)){  //because the circuit is normally closed. High means the kill switch is not hit
    if(EEPROM.read(0)){
      pitch.write(EEPROM.read(0+1));
      currentPitch = EEPROM.read(0+1);
    }
    else{
      pitch.write(STARTUP_PITCH);
      currentPitch = STARTUP_PITCH;
    }
  }
  //CHECKING IF KILL SWITCH IS HIT
}

void loop(){
  
  //For debugging
  Serial.print("Starting a new loop current pitch is currently equal to: ");
  Serial.print(currentPitch);
  //For debugging
  

  //****************************************************//
  //*****Variables to be used in the control system*****//
  //****************************************************//

  double turbineVoltage;
  double loadVoltage;
  double theoreticalOutputVoltage;
  double inferredWindSpeed;
  boolean breakNeeded;
  double RPM;
  double power;
  int dutyCycle;

  if(digitalRead(A2 == LOW) ){ //Kill switch is hit, don't do anything on this loop
    
    //NEEDS TESTING
    
    Serial.println("Kill switch is hit. Turbine is braking or is already braked.");
    breakNeeded = true;
    processDisconnectedState(breakNeeded);
  }
  else{
    //Kill switch not hit
    breakNeeded = false;
    //Kill switch not hit
    
    //Reading in turbine and load voltage
    turbineVoltage = VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023.0;
    loadVoltage = VOLTAGE_DIVIDER_LOAD*((double)analogRead(A3))*5.0/1023.0;
    //Reading in turbine and load voltage
    
    
    //For testing
    Serial.print("Reading in a turbine voltage of: ");
    Serial.println(turbineVoltage);
    Serial.print("Reading in a load voltage of: ");
    Serial.println(loadVoltage);
    //For testing
    
    //PCC Disconnect
    breakNeeded = determineDisconnect(loadVoltage, turbineVoltage);
    processDisconnectedState(breakNeeded);
    //PCC Disconnect block

    //Calculating RPM and Power from turbine voltage
    RPM = getRPMfromVoltageIn(turbineVoltage);
    power = calculatePowerFromRPM(RPM); 
    //Calculating RPM and Power from turbine Voltage
  
    //For testing
    Serial.print("Calculated power to be: ");
    Serial.println(power);                         //Only need to output power because power comes from RPM
    //For testing
   
    //Getting the estimated wind speed
    inferredWindSpeed = inferWindSpeed(turbineVoltage, RPM, power);
    //Getting the estimated wind speed
    
    //Processing the estimated wind speed. PROTECTED AGAINST OVERFLOWS < 0 AND > 255
    if(inferredWindSpeed < 11){
      dutyCycle = calculateDutyCycle(RESISTANCE, calculatePowerFromRPM(RPM), turbineVoltage);
      theoreticalOutputVoltage = calculateTheoreticalOutputVoltage(turbineVoltage, dutyCycle);
      if(dutyCycle >= 0 && dutyCycle <= 255){
        analogWrite(PWM_PIN, dutyCycle);
      }
      else{
        if(dutyCycle < 0){
          dutyCycle = 0;
          theoreticalOutputVoltage = 0;
          analogWrite(PWM_PIN, 0);
        }
        if(dutyCycle > 255){
          dutyCycle = 255;
          theoreticalOutputVoltage = turbineVoltage;
          analogWrite(PWM_PIN, 255);
        } 
      stabilizeVoltageGivenDutyCycle(dutyCycle, theoreticalOutputVoltage);
      }
    } 
    else{
      dutyCycle = calculateDutyCycle(RESISTANCE, POWER_AT_11MS, turbineVoltage);
      theoreticalOutputVoltage = dutyCycle*turbineVoltage;
      if(dutyCycle > 255){
        dutyCycle = 255; 
        theoreticalOutputVoltage = turbineVoltage;
        analogWrite(PWM_PIN, dutyCycle);
      }
      else if(dutyCycle < 0){
        dutyCycle = 0;
        theoreticalOutputVoltage = 0;
        analogWrite(PWM_PIN, 0);
      }
      else{
        analogWrite(PWM_PIN, dutyCycle);
      //NEED A STABILIZE POWER AT 11 M/S
      }
    }
    //Processing the estimated wind speed. NEED TO EDIT TO ADJUST DUTY CYCLE
  
  }
}






//********************************************************************************************************************//
//********************************************************************************************************************//
//********************************************************************************************************************//
//*********************************************ADDITIONAL FUNCTIONS***************************************************//
//********************************************************************************************************************//
//********************************************************************************************************************//
//********************************************************************************************************************//





double calculateTheoreticalOutputVoltage(double voltageIn, int PWMdutyCycle){
  return voltageIn*double(PWMdutyCycle/255.0);
}


void processDisconnectedState(boolean disconnected){
  if(disconnected){
    Serial.println("Braking the turbine!");
    breakedInCompetition = true;
    EEPROM.write(0,breakedInCompetition);
    EEPROM.write(0+1,currentPitch);
    digitalWrite(LOAD_ARDUINO_PIN, LOW);
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
  
  
   //For debugging
  Serial.print("Input voltage of: ");
  Serial.println(voltageIn);
  Serial.print("Calculating a power of: ");
  Serial.println(power);
  Serial.print("Estimating a wind speed of: ");
  Serial.println(4.09+5.16*power+.077*pow(power,2)+.0462*pow(power,3));
  //For debugging
  
  return 4.09+5.16*power+.077*pow(power,2)+.0462*pow(power,3);
}



double getRPMfromVoltageIn(double turbineVoltage){
  
  //For debugging
  Serial.print("Input voltage of: ");
  Serial.println(turbineVoltage);
  Serial.print("Calculating an RPM of: ");
  Serial.println(108*turbineVoltage-462);
  //For debugging
  
  
  return 108*turbineVoltage-462; //Placeholder for more data that needs to go in here.
}

double calculatePowerFromRPM(double RPM){
  return -7.97*10E-11*pow(RPM,3)+1.11*10E-5*pow(RPM,2)-4.97*10E-3*RPM-1.91;
}

int calculateDutyCycle(double resistance, double power, double turbineVoltage){
  double theoreticalDutyCycle = PWM_CONVERSION*int(sqrt(resistance*power)/turbineVoltage);
  if(theoreticalDutyCycle < 25){
     return int((255)*(.000162-.114*theoreticalDutyCycle+5.68*theoreticalDutyCycle*theoreticalDutyCycle));
  }
  else if(theoreticalDutyCycle < .87){
     return int((255)*(.035+1.31*theoreticalDutyCycle-.58*theoreticalDutyCycle*theoreticalDutyCycle));
  }
  else if(theoreticalDutyCycle <= 1){
     return int((255)*(7.51-15.6*theoreticalDutyCycle+8.92*theoreticalDutyCycle*theoreticalDutyCycle));
  }
  else{
     Serial.println("What the heck happened here? ");
     return 255;
      
  }
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


boolean determineDisconnect(double loadVoltage, double turbineVoltage){
  if(turbineVoltage-loadVoltage > VOLTAGE_DIFFERENT_BUFFER && loadVoltage < LOAD_VOTLAGE_BUFFER){
    return true;
  }
  return false;
}

//INPUTS: Duty Cycle and desired output voltage
//ALGORITHM: Checks if the actual output voltage is within a buffer of the theoretical one. If it is greater
//           than the desired output + buffer, drop the duty cycle by one and test again. If it is less than
//           the desired output - buffer, increase the duty cycle by one and test again. 
//
//CONSTRAINTS: DUTY CYCLE CAN NEVER BE MORE THAN 255 OR LESS THAN 0

void stabilizeVoltageGivenDutyCycle(int dutyCycle, double desiredVoltage){
  
  
  int iterations = 0;
  bool flag = true;
  while(iterations < 50 && flag == true){
    flag = false;
    if(VOLTAGE_DIVIDER_LOAD*((double)analogRead(A2))*5.0/1023.0 < desiredVoltage - THEORETICAL_VS_ACTUAL_VOLTAGE_BUFFER){
      flag = true;
      if(dutyCycle > 0){
        analogWrite(PWM_PIN, --dutyCycle);
      }
      else{
        break;
      }
    }
    else if(VOLTAGE_DIVIDER_LOAD*((double)analogRead(A2))*5.0/1023.0 > desiredVoltage + THEORETICAL_VS_ACTUAL_VOLTAGE_BUFFER){
      flag = true;
      if(dutyCycle < 255){
        analogWrite(PWM_PIN, ++dutyCycle);
      }
      else{
        break;
      }
    }
       
    iterations++;
  }
}
