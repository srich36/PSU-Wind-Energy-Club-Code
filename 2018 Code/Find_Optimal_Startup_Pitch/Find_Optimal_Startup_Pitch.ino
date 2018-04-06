#include <Servo.h>

//break pitch is 140.

//Limits are from 30 to 140 in arduino code

const double VOLTAGE_DIVIDER_TURBINE = 13.1;
const double windSpeed = 5;
const double startPitch = 30;
const double endPitch = 45;
const int voltageReadingsPerPitch = 5;
const double voltageBuffer = .3;

Servo pitch; //don't forget to include a servo object

void setup() {
  //Initialize Serial system output/input
  Serial.begin(9600);
  pitch.attach(3); // Control line on pin 3.

}

void loop() {
  double turbineVoltage, averageVoltage;
  double voltages[voltageReadingsPerPitch];
  int optimalPitchAngle;
  Serial.print("Testing at wind speed: "); Serial.println(windSpeed);
  pitch.write(startPitch);
  delay(5000);
  for(int i = startPitch; i < endPitch+1; i++){
    pitch.write(i);
    Serial.print("Pitch angle set to: "); Serial.println(i);
    delay(2500);
    for(int j = 0; j < voltageReadingsPerPitch; j++){
      turbineVoltage = (VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023); //make sure you do NOT do integer division here
      voltages[j] = turbineVoltage;
      delay(500);
    }
    double voltageTotal = 0;
    for(int k = 0; k < voltageReadingsPerPitch; k++){
      voltageTotal+=voltages[k];
    }
    averageVoltage = voltageTotal/voltageReadingsPerPitch;
    if(averageVoltage > voltageBuffer){
      optimalPitchAngle = i;
      Serial.print("Turbine producing a voltage. Optimal pitch angle determined to be "); Serial.println(optimalPitchAngle);
    }
    else{
      Serial.println("No significant voltage detected. Moving on to the next pitch. ");
    }
  }
}

