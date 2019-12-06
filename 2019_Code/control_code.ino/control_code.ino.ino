

#include <Servo.h>
#include <EEPROM.h>



Servo pitch; 

const int PWM_PIN = 3;               //Digital
const int LOAD_ARDUINO_PIN = 8;      //Digital 
const int SERVO_PITCH_PIN = 9;        //Digital
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

const int BRAKE_PITCH = 90; //Verify with Adam
const int STARTUP_PITCH = 15; //Verify with Adam
const double VOLTAGE_DIVIDER_TURBINE = 13.015; //This should be the same as last year so we are good
const double VOLTAGE_DIVIDER_LOAD = 14.327;//This needs to be calculated for our new load
const double VOLTAGE_DIFFERENT_BUFFER = .5;
const double LOAD_VOTLAGE_BUFFER = .5;
const double MAX_VOLTAGE = 45;
const int RESISTANCE = 50; //Is this a constant or does it change? 
const double POWER_AT_11MS = 45.5;


void setup(){
  
  
  if(digitalRead(A0 == HIGH)){  //because the circuit is normally closed. High means the kill switch is not hit
    /*
    digitalWrite(LOAD_ARDUINO_PIN, HIGH);
    if(EEPROM.read(0)){
      pitch.write(EEPROM.read(0+1));
      currentPitch = EEPROM.read(0+1);
    }
    else{
      pitch.write(STARTUP_PITCH);
      currentPitch = STARTUP_PITCH;
    }
    */
    Serial.println("NOT HIT");
  }
  else{
    digitalWrite(LOAD_ARDUINO_PIN, LOW);
    Serial.println("KILL SWITCH HIT");
  }
  //if the kill switch is hit don't pitch at all. Does this work or do we need 8 Volts? 

}



void loop(){

  if(digitalRead(A2 == HIGH)){  //because the circuit is normally closed. High means the kill switch is not hit
    digitalWrite(LOAD_ARDUINO_PIN, HIGH);
    if(EEPROM.read(0)){
      pitch.write(EEPROM.read(0+1));
      currentPitch = EEPROM.read(0+1);
    }
    else{
      pitch.write(STARTUP_PITCH);
      currentPitch = STARTUP_PITCH;
    }
  }
  else{
    digitalWrite(LOAD_ARDUINO_PIN, LOW);
  }



}
