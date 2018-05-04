//TODO: 
//PROTECT AGAINST CASE WHERE DUTY CYCLE IS 1 AND THEORETICAL VOLTAGE CAN NEVER BE ACTUAL VOLTAGE BECAUSE OF SYSTEM LOSSES


#include <Servo.h>

const int PWM_PIN = 3; 
const int SERVO_PITCH_PIN = 9; 
const int LOWER_VOLTAGE_LIMIT = 10;
const int UPPER_VOLTAGE_LIMIT = 15;
const int MAINTAIN_VOLTAGE_IN = 13;
const double VOLTAGE_DIVIDER_LOAD = 14.327;
const int DUTY_CYCLE_RATIO = 5;
static int currentPitch;


double turbineVoltageBefore;
const int STARTUP_PITCH = 45; //Need to verify pitch for new turbine -> should be verified now
const double VOLTAGE_DIVIDER_TURBINE = 13.015; //This should be the same as last year so we are good
const double THEORETICAL_VS_ACTUAL_VOLTAGE_BUFFER = .3;

void determinePitch(double turbineVoltage);
int calculateDutyCycle(double);
void stabilizeVoltageGivenDutyCycle(int dutyCycle, double desiredVoltage);


Servo pitch;

void setup(){
  pinMode(PWM_PIN, OUTPUT);
  turbineVoltageBefore = -1;
  pitch.attach(SERVO_PITCH_PIN);
  pitch.write(STARTUP_PITCH);
  currentPitch = STARTUP_PITCH;
}

void loop(){
  double turbineVoltage;
  int dutyCycle;
  double theoreticalVoltage;
  turbineVoltage = VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023.0;
  if(turbineVoltageBefore!= -1){ //This is not the first voltage reading.
    determinePitch(turbineVoltage);
  }
  //Else do nothing because it's only the first loop
  //Now calculate the duty cycle
  
  
  dutyCycle = calculateDutyCycle(turbineVoltage);
  theoreticalVoltage = turbineVoltage*double(dutyCycle)/255.0;
  if(dutyCycle >= 0 && dutyCycle <= 255){
    analogWrite(PWM_PIN, dutyCycle);
  }
  else{
    if(dutyCycle < 0){
      dutyCycle = 0;
      theoreticalVoltage = 0;
      analogWrite(PWM_PIN, 0);
    }
    if(dutyCycle > 255){
      dutyCycle = 255;
      theoreticalVoltage = turbineVoltage;
      analogWrite(PWM_PIN, 255);
    }
  }
  stabilizeVoltageGivenDutyCycle(dutyCycle, theoreticalVoltage);
  
  
  //Move to the t+1 loop domain
  turbineVoltageBefore = turbineVoltage;
}

void determinePitch(double turbineVoltage){
  
  //Case 1:
  if(turbineVoltage < LOWER_VOLTAGE_LIMIT){
    pitch.write(currentPitch+1);
    currentPitch++;
  }
  else if(turbineVoltage> LOWER_VOLTAGE_LIMIT && turbineVoltage < MAINTAIN_VOLTAGE_IN){ //Case 2
    if(turbineVoltage>=turbineVoltageBefore){
      //Do nothing
    }
    else{
      pitch.write(currentPitch+1);
      currentPitch++;
    }
    
  }
  else if(turbineVoltage >= MAINTAIN_VOLTAGE_IN && turbineVoltage < UPPER_VOLTAGE_LIMIT){  //Case 3
    
    if(turbineVoltage>=turbineVoltageBefore){
      pitch.write(currentPitch-1);
      currentPitch--;
    }
    else{
      //Do nothing
    }
    
  }
  else if(turbineVoltage>=UPPER_VOLTAGE_LIMIT){
    pitch.write(currentPitch - 3);
    currentPitch -= 3;
  }
  
  else{
    
    //For debugging
    Serial.println("Edge case we have not accounted for. ");
    Serial.print("Turbine Voltage is: ");
    Serial.println(turbineVoltage);
    Serial.print("And voltage before is: ");
    Serial.println(turbineVoltageBefore);
    //For debugging
  }
  
}

int calculateDutyCycle(double turbineVoltage){
  if(turbineVoltage > 5){
    
    
    //For debugging
    Serial.print("Sending a duty cycle of: ");
    double theoreticalDutyCycle = (DUTY_CYCLE_RATIO)/turbineVoltage;
    //For debugging
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
  else{
    
    //For debugging
    Serial.println("Not sure if this should ever get in this else loop. Sending a fully on duty cycle just in case. ");
    //For debugging
    
    return 255; //Max duty cycle 
  }
  
  
}



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


