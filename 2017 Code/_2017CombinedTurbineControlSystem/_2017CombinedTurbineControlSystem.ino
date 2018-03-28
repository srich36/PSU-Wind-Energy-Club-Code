// Control system for the 2017 Penn State Collegiate Wind Competition test turbine.
// Authors: Patrick Nicodemus, Mitchell Proulx, Joseph Miscioscia.
// Date: April 17, 2017

#include <Servo.h>
#include <EEPROM.h>

Servo pitch; // Controls the pitch angle of the turbine blades.

const double MARGIN = 1.1;
const double RESISTOR_SCALING_FACTOR = 1.02;
const int BRAKE_PITCH = 145;
const byte RECOVERY = 0xFF;
const int CYCLES_FOR_ADJUSTMENT = 10;
const int MS_PER_CYCLE = 200;
const int STARTUP_PITCH = 75;
const double VOLTAGE_DIVIDER_TURBINE = 13.015;
const double VOLTAGE_DIVIDER_LOAD = 10.2933;//11.13;
const double MAX_VOLTAGE = 40; //Voltage observed at 11m/s
const double TURN_ON_VOLTAGE = 10.0;

double inferWindSpeed(int, double);
boolean voltageConsistentlyDifferent(double , double);
int optimumPitch(double);

void setup() {
  //Initialize Serial system output/input
  Serial.begin(9600);
  pitch.attach(3); // Control line on pin 3.

  if(EEPROM.read(0)){ // Byte 0 contains a startup-flag asking whether it has been used before.
    pitch.write((int) EEPROM.read(1)); //EEPROM[1] contains the previous blade pitch.
  } else {
    pitch.write(STARTUP_PITCH); // Should this be a generic value or the optimal startup angle or what?
  }
  /* pinMode(A1,OUTPUT);
  pinMode(A2,INPUT);
  digitalWrite(A0,HIGH);
  EEPROM.write(1,0);x
  EEPROM.write(1,85); */ // I don't know which pins our values are connected to on the new arduino. Will verify.
}

void loop() {
  double turbineVoltage;
  double loadVoltage;
  boolean killSwitch;
  boolean disconnected;
  static boolean disconnectedState = false;
  static boolean killState = false;
  static int adjustmentCounter = 0;
  int voltageDifferenceCounter;
  static int currentBladePitch = STARTUP_PITCH;
  double inferredWindSpeed;

  // Part 1 - Acquire data from sensors
  turbineVoltage = VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023.0;
  loadVoltage = RESISTOR_SCALING_FACTOR*VOLTAGE_DIVIDER_LOAD*((double)analogRead(A3))*5.0/1023.0;
  killSwitch = (digitalRead(A1) == LOW);

  // Part 2 - Business Logic.
  disconnected = voltageConsistentlyDifferent(turbineVoltage, loadVoltage);
  if(disconnected) { // Brake the turbine, and prepare for the Arduino to lose power.
    EEPROM.write(0,RECOVERY);                   //Save to the Arduino a flag that says it's mid competition.
    EEPROM.write(1, (byte) currentBladePitch);  //Save the current blade pitch for after startup.
    disconnectedState = true;
    Serial.print("Disconnect.");
    pitch.write(BRAKE_PITCH);
    delay(10000);
  } else if (killSwitch){
    Serial.print("Kill switch.\n");
    killState = true;
    pitch.write(BRAKE_PITCH);
    delay(2000);
  } else { // The circuit is closed and the kill switch is off.
    if(disconnectedState){ //The circuit was disconnected but has been closed; power has been restored.
      disconnectedState = false;
      pitch.write(currentBladePitch); // assuming currentBladePitch was positioned at before power was cut.
      delay(5000);
    } else if (killState){
      killState = false;
      pitch.write(currentBladePitch);
    }
    //Part 3 - Output
    if(!adjustmentCounter) { // Only execute every CYCLES_FOR_ADJUSTMENT cycles
      Serial.print("V: "); Serial.print(turbineVoltage); Serial.print(" P: "); Serial.print(currentBladePitch); Serial.print(' ');
      if( currentBladePitch > 85) { // The windspeed is high so the turbine has been braked
        if( turbineVoltage > MAX_VOLTAGE * 1.06 ){ 
          Serial.println("  <<< Still above max voltage");
          currentBladePitch += 2 ;
        } else if (turbineVoltage < MAX_VOLTAGE * 0.94 ){
          currentBladePitch -=  2 ;
          Serial.println("  <<< Voltage drop");
        } else {
          //do nothing
          Serial.println("  <<< Goldilocks");   
        }
      } else if (currentBladePitch == 70) {
        Serial.println(" Low speed.");
        currentBladePitch = (turbineVoltage < TURN_ON_VOLTAGE)? STARTUP_PITCH : 76;
      }  
      else { // Pitch is in the standard 76 - 85 range
        inferredWindSpeed = inferWindSpeed(currentBladePitch, turbineVoltage);
        currentBladePitch = optimumPitch(inferredWindSpeed);
      }
      pitch.write(currentBladePitch);
    }
    adjustmentCounter++;
    adjustmentCounter %= CYCLES_FOR_ADJUSTMENT;

    delay(MS_PER_CYCLE);
  }
}

boolean voltageConsistentlyDifferent(double turbineVoltage, double loadVoltage) {
  static double differences[7] = { 0, 0, 0, 0, 0, 0, 0};
  double voltageRatio;
  int count;
  
  voltageRatio = max( turbineVoltage/loadVoltage, loadVoltage/turbineVoltage );
  count = 0;
  for(int i = 0; i < 6; i++) {
    differences[i] = differences[i+1];
    if (!isnan(differences[i]) && differences[i] >= MARGIN ) {
      count++;
    }
  }
  differences[6] = voltageRatio;
  if(!isnan(voltageRatio) && voltageRatio >= MARGIN){
    count++;
  }
  return ( count >= 6 );
}

const double voltageData[8][10] =  { // 76 - 85
  { 13.73, 13.73, 13.73, 13.73, 13.73, 13.73, 13.73, 13.73, 13.73, 13.73 }, //5
  { 26.26, 26.26, 26.26, 26.26, 26.26, 26.26, 26.26, 26.26, 26.26, 26.26 }, //6
  { 31.51, 31.51, 31.51, 31.51, 31.51, 31.51, 31.51, 31.51, 31.51, 31.51 }, //7
  { 35.39, 35.39, 35.39, 35.39, 35.39, 35.39, 35.39, 35.39, 35.39, 35.39 }, //8
  { 36.50, 36.50, 36.50, 36.50, 36.50, 36.50, 36.50, 36.50, 36.50, 36.50 }, //9
  { 37.50, 37.50, 37.50, 37.50, 37.50, 37.50, 37.50, 37.50, 37.50, 37.50 }, //10
  { 38.50, 38.50, 38.50, 38.50, 38.50, 38.50, 38.50, 38.50, 38.50, 38.50 }, //11
  { 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0 }  //13
  
//  {  5.88,  6.16,  6.30,  6.32,  6.40,  6.45,  6.53,  6.58,  6.63,  6.68,  6.64,  6.70,  6.66,  6.68,  6.59,  5.83,  5.00,  4.70,  4.68,  4.70,  4.69,  4.62,  4.68,  4.65,  4.66,  4.63}, //5
//  {  8.62,  9.01,  9.21,  9.41,  9.51,  9.61,  9.78,  9.84,  9.98, 10.04, 10.16, 10.22, 10.26, 10.28, 10.28, 10.24, 10.26, 10.23, 10.15, 10.15, 10.08, 10.00,  9.95,  9.89,  9.75,  9.73}, //6
//  { 11.44, 11.77, 12.06, 12.12, 12.35, 12.52, 12.65, 12.85, 12.97, 13.11, 13.25, 13.33, 13.35, 13.43, 13.43, 13.37, 13.37, 13.28, 13.35, 13.25, 13.11, 13.05, 12.97, 12.86, 12.76, 12.62}, //7
//  { 14.28, 14.75, 15.06, 15.32, 15.60, 15.85, 16.05, 16.31, 16.46, 16.66, 16.81, 16.88, 16.73, 16.96, 16.94, 16.36, 16.85, 16.80, 16.68, 16.63, 16.56, 16.31, 16.11, 16.11, 15.81, 15.66}, //8
//  { 17.01, 18.45, 18.80, 19.10, 19.34, 19.35, 19.69, 19.94, 20.08, 20.23, 19.68, 20.31, 20.04, 20.24, 20.06, 20.03, 20.02, 19.68, 19.76, 19.44, 19.29, 18.99, 18.91, 18.46, 18.21, 17.87}, //9
//  { 19.25, 21.84, 22.05, 22.54, 21.73, 22.07, 21.25, 23.61, 23.73, 23.20, 23.84, 22.01, 21.63, 23.43, 22.35, 23.27, 22.50, 20.87, 22.48, 22.26, 21.83, 21.61, 21.24, 20.98, 20.78, 20.37}, //10
//  { 21.17, 22.65, 26.65, 23.37, 23.40, 24.08, 23.40, 23.47, 24.50, 24.01, 24.02, 23.60, 23.15, 23.80, 23.79, 22.82, 25.00, 21.73, 24.93, 22.88, 24.30, 22.95, 23.07, 23.34, 22.88, 22.60}, //11
//  { 26.36, 27.22, 26.87, 25.83, 25.73, 25.72, 25.31, 26.61, 26.52, 26.86, 26.21, 26.09, 26.07, 25.84, 24.92, 25.14, 24.76, 24.52, 27.04, 23.99, 23.75, 26.08, 25.64, 22.79, 24.91, 22.43}, //12
//  // { 22.49, 22.84, 23.69, 23.00, 23.95, 23.54, 23.24, 23.00, 22.76, 22.40, 22.00, 21.30, 21.46, 20.81 }
// { 29.84, 28.58, 28.11, 30.07, 30.20, 29.66, 28.53, 29.12, 29.25, 29.74, 29.92, 28.66, 28.48, 28.08, 27.44, 27.52, 26.82, 26.59, 26.25, 25.86, 25.49, 25.07, 24.59, 24.42, 23.98, 24.22}  //13
// { 23.57, 25.24, 23.64, 23.40, 24.67, 24.41, 23.61, 23.91, 23.55, 23.23, 22.98, 22.67, 22.16, 8.45, 7.53 }
};

int pitchData(int i, int j) {
  return j+65;
}

double inferWindSpeed(int currentBladePitch, double turbineVoltage){
  int i = -1;
  while( i < 6 && turbineVoltage >= voltageData[i+1][currentBladePitch-76]) {
    i++;
  }
  double x;
  if(i == -1) { // Lower than all boxes
    x = 0;    
  } else if (i<6){
    x =  5 + i + (turbineVoltage - voltageData[i][currentBladePitch-76])/( voltageData[i+1][currentBladePitch-76] - voltageData[i][currentBladePitch-76] );
  } else {   //i >= 6
    double dw_dv = 2.0/(voltageData[7][currentBladePitch-76] - voltageData[6][currentBladePitch-76]);
    x =  11.0 + (turbineVoltage - voltageData[6][currentBladePitch-76])*dw_dv;
  }
  Serial.print(x);
  return x;  
}

int optimumPitch( double windSpeed ) {
  double optim[7] = { 78, 78, 79, 79, 80, 80, 80 };
  if( windSpeed < 5) {
    Serial.print(' ');  Serial.println(STARTUP_PITCH);
    return STARTUP_PITCH;
  } else if (windSpeed > 11.3) {      //Too high! Brake.
    Serial.print(' ');  Serial.println(92);
    return 92;    
  } else {
    double x = optim[(int)(windSpeed - 5)] + (optim[(int)(windSpeed - 5) + 1] - optim[(int)(windSpeed - 5)])*(windSpeed - floor(windSpeed));
    Serial.print(' '); Serial.println(round(x));
    return (int) round(x);
  }
}

