#include <Servo.h>

const double VOLTAGE_DIVIDER_TURBINE = 13.1;
const double windSpeed = 5;
const double startPitch = 75;
const double endPitch = 85;
const int pitchIncrement = 1;
const int voltageReadingsPerPitch = 5;

Servo pitch; //don't forget to include a servo object

void setup() {
  //Initialize Serial system output/input
  Serial.begin(9600);
  pitch.attach(3); // Control line on pin 3.

  /* pinMode(A1,OUTPUT);
  pinMode(A2,INPUT);
  digitalWrite(A0,HIGH);
  EEPROM.write(1,0);x
  EEPROM.write(1,85); */ // I don't know which pins our values are connected to on the new arduino. Will verify.
}

void loop(){
  double turbineVoltage;
  int totalVoltages = (endPitch-startPitch)*voltageReadingsPerPitch/pitchIncrement;
  double voltages[totalVoltages];
  double sampleVoltages[voltageReadingsPerPitch];
  delay(5000);
  Serial.print("Testing at wind speed: "); Serial.println(windSpeed);
  //loop through and read the voltage at each wind speed from start pitch to end spitch
  for(int i = startPitch; i < endPitch + 1; i+=pitchIncrement){
    pitch.write(i);
    Serial.print("Testing at pitch angle "); Serial.println(i);
    delay(5000);
    for(int j =  0; j < voltageReadingsPerPitch; j++){
      turbineVoltage = (VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023); //make sure you do NOT do integer division here
      sampleVoltages[j] = turbineVoltage;
      Serial.print("Voltage read "); Serial.println(turbineVoltage);
      delay(5000);
    }
    
    double totalVoltage = 0;
    for(int k = 0; k < voltageReadingsPerPitch; k++){
      totalVoltage+=sampleVoltages[k];
    }
    double averageVoltage = totalVoltage/voltageReadingsPerPitch;
     Serial.print("Average voltage at pitch angle "); Serial.print(i); Serial.print(" is: "); Serial.println(averageVoltage); 
   }
  
}

