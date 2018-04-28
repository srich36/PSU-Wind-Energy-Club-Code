// Control system for the 2017 Penn State Collegiate Wind Competition test turbine.
// Authors: Patrick Nicodemus, Joseph Miscioscia, Mitchell Proulx.
// Date: April 17, 2017

#include <Servo.h>
#include <EEPROM.h>

Servo pitch; // Controls the pitch angle of the turbine blades.

const double MARGIN = 3.5;
const double RESISTOR_SCALING_FACTOR = 1.02;
const int BRAKE_PITCH = 145;
const byte RECOVERY = 0xFF;
const int CYCLES_FOR_ADJUSTMENT = 6;
const int MS_PER_CYCLE = 200;
const int STARTUP_PITCH = 75;
const double VOLTAGE_DIVIDER_TURBINE = 13.015;
const double VOLTAGE_DIVIDER_LOAD = 11.13;//11.13 for new load, 10.29 for old one;
const double MAX_VOLTAGE = 45; //Voltage observed at 11m/s
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
  static double inferredWindSpeed = 5;

  // Part 1 - Acquire data from sensors
  turbineVoltage = VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023.0;
  //loadVoltage = RESISTOR_SCALING_FACTOR*VOLTAGE_DIVIDER_LOAD*((double)analogRead(A3))*5.0/1023.0;
  killSwitch = (digitalRead(A1) == LOW);

  // Part 2 - Business Logic.
  disconnected = voltageConsistentlyDifferent(turbineVoltage, loadVoltage);
 // disconnected = false;
  if(disconnected) { // Brake the turbine, and prepare for the Arduino to lose power.
    EEPROM.write(0,RECOVERY);                   //Save to the Arduino a flag that says it's mid competition.
    EEPROM.write(1, (byte) currentBladePitch);  //Save the current blade pitch for after startup.
    disconnectedState = true;
    Serial.println("Disconnect.");
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
      if( currentBladePitch < 75 || turbineVoltage > MAX_VOLTAGE ) { // The windspeed is high so the blades have been pitched more sharply.
        if( turbineVoltage > MAX_VOLTAGE ){ 
          Serial.println("  <<< TOO CLOSE to max voltage");
          int pitchDegrees = ceil(2*(turbineVoltage-MAX_VOLTAGE-2));
          currentBladePitch -= pitchDegrees;
        } else if (turbineVoltage < MAX_VOLTAGE-2 ){
          int pitchDegrees = floor(2*(turbineVoltage-MAX_VOLTAGE+2));
          currentBladePitch -= pitchDegrees;
          Serial.println("  <<< Voltage drop");
        } else {
          //do nothing
          Serial.println("  <<< Goldilocks");   
        }
      }
      else { // Pitch is in the standard range
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

//Don't worry about this function it doesn't do anything; they wrote it and never fully implemented it
boolean voltageConsistentlyDifferent(double turbineVoltage, double loadVoltage) {
  static double differences[7] = { 0, 0, 0, 0, 0, 0, 0};
  double voltageRatio;
  int count;
  Serial.print(turbineVoltage);
  Serial.print("     ");
  Serial.print(loadVoltage);
  Serial.print("     ");
  Serial.println(turbineVoltage - loadVoltage);
  voltageRatio = max( turbineVoltage-loadVoltage, loadVoltage-turbineVoltage );
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

//Pretty sure this is voltage produced at a given wind speed with pitch angles from 75-85
const double voltageData[8][11] =  { // 75 - 85
  { 14.12,  16.66, 17.83, 17.91, 18.62, 18.76, 18.86, 19.02, 19.23, 19.23, 19.53 }, //5
  { 23.55,  23.91, 24.02, 24.22, 24.24, 24.75, 24.50, 24.41, 24.51, 24.27, 24.25 }, //6
  { 29.29,  29.52, 29.45, 29.25, 29.25, 29.49, 29.17, 28.30, 28.81, 28.58, 28.07 }, //7
  { 34.70,  33.84, 33.45, 33.40, 34.04, 33.87, 33.36, 32.77, 33.28, 33.45, 32.51 }, //8
  { 38.47,  38.12, 38.24, 38.92, 37.97, 37.93, 37.87, 37.12, 37.28, 37.15, 36.76 }, //9
  { 42.62,  42.27, 42.71, 42.36, 42.16, 41.84, 41.49, 40.81, 41.39, 40.41, 40.15 }, //10
  { 46.87,  46.25, 46.14, 45.84, 45.86, 45.34, 44.91, 44.09, 44.33, 43.88, 43.24 }, //11
  { 50,     49,    49,    49,    48.86, 48.34, 47.91, 47.09, 47.33, 46.88, 46.24 }  // Guesses at 12 (add three)
};



double inferWindSpeed(int currentBladePitch, double turbineVoltage){
  int i = -1;
  while( i < 6 && turbineVoltage >= voltageData[i+1][currentBladePitch-75]) {
    i++;
  }
  double x;
  if(i == -1) { // Lower than all boxes
    return turbineVoltage/voltageData[0][currentBladePitch-75] * 5;
  } else if (i<6){
    x =  5 + i + (turbineVoltage - voltageData[i][currentBladePitch-75])/( voltageData[i+1][currentBladePitch-75] - voltageData[i][currentBladePitch-75] );
  } else {   //i >= 6
    //double dw_dv = 2.0/(voltageData[7][currentBladePitch-75] - voltageData[6][currentBladePitch-75]);
    //x =  11.0 + (turbineVoltage - voltageData[6][currentBladePitch-75])*dw_dv;
    return 10;
  }
  Serial.print(x);
  return x;  
}

int optimumPitch( double windSpeed ) {
  double optim[7] = { 85, 80, 80, 79, 78, 77, 75} ; // Optimal pitches at wind speeds 5 through 11 m/s.
  if( windSpeed < 3) {
    Serial.print(' ');  Serial.println(STARTUP_PITCH);
    return STARTUP_PITCH;
  } else if (windSpeed < 5) {
    Serial.print(' '); Serial.println(optim[0]);
    return optim[0];
  } else if (windSpeed > 11.3) {      //Too high, brake
    Serial.print(' ');  Serial.println();
    return 74;//?????
  } else {
    double x = optim[(int)(windSpeed - 5)] + (optim[(int)(windSpeed - 5) + 1] - optim[(int)(windSpeed - 5)])*(windSpeed - floor(windSpeed));
    Serial.print(' '); Serial.println(round(x));
    return (int) round(x);
  }
}

