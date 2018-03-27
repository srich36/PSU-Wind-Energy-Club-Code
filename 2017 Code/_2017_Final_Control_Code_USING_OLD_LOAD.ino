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
const int CYCLES_FOR_ADJUSTMENT = 10;
const int MS_PER_CYCLE = 200;
const int STARTUP_PITCH = 75;
const double VOLTAGE_DIVIDER_TURBINE = 13.015*1.032;
const double VOLTAGE_DIVIDER_LOAD = 10.2933;//11.13 for new load, 10.29 for old one;
const double MAX_VOLTAGE = 45; //Voltage observed at 11m/s
const double TURN_ON_VOLTAGE = 6.0;

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
  loadVoltage = RESISTOR_SCALING_FACTOR*VOLTAGE_DIVIDER_LOAD*((double)analogRead(A3))*5.0/1023.0;
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
      if( currentBladePitch < 75 || turbineVoltage > 45 ) { // The windspeed is high so the turbine has been braked
        if( turbineVoltage > 45 ){ 
          Serial.println("  <<< TOO CLOSE to max voltage");
          //inferredWindSpeed++;
          //currentBladePitch = optimumPitch(inferredWindSpeed);
          int pitchDegrees = floor((turbineVoltage-42.5));
          currentBladePitch -= pitchDegrees;
        } else if (turbineVoltage < 40 ){
          //inferredWindSpeed--;
          //currentBladePitch = optimumPitch(inferredWindSpeed);
          int pitchDegrees = ceil((turbineVoltage-42.5)/2.5);
          currentBladePitch -= pitchDegrees;
          Serial.println("  <<< Voltage drop");
        } else {
          //do nothing
          Serial.println("  <<< Goldilocks");   
        }
          //pitch.write(currentBladePitch);
          //for(int bj = 0; bj < 10; bj++){
          //  delay(250);
          //  Serial.println(VOLTAGE_DIVIDER_TURBINE*((double)analogRead(A0))*5.0/1023.0);
          //}
      } else if (currentBladePitch == STARTUP_PITCH) {
        Serial.println(" Low speed.");
        currentBladePitch = (turbineVoltage < TURN_ON_VOLTAGE)? STARTUP_PITCH : 80;
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

boolean voltageConsistentlyDifferent(double turbineVoltage, double loadVoltage) {
  static double differences[7] = { 0, 0, 0, 0, 0, 0, 0};
  double voltageRatio;
  int count;
 // Serial.print(turbineVoltage);
 // Serial.print("     ");
 // Serial.print(loadVoltage);
 // Serial.print("     ");
 // Serial.println(turbineVoltage - loadVoltage);
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

const double voltageData[8][11] =  { // 75 - 85
  { 12.43, 12.74, 12.96, 13.34, 13.37, 13.56, 13.66, 13.73, 13.7, 13.87, 13.89}, //5
  { 19.57, 19.99, 20.07, 20.27, 20.57, 21.28, 21.04, 21.09, 21.71, 21.5, 21.46}, //6
  { 27.81, 28.12, 28.26, 28.57, 28.99, 28.54, 27.3, 26.2, 27.97, 27.31, 27.24}, //7
  { 35.91, 35.62, 36.18, 35.52, 35.2, 35.51, 34.97, 34.66, 34.25, 34.19, 33.95}, //8
  { 41.21, 41.21, 41.4, 41.99, 41.2, 40.22, 41.03, 40.17, 40.4, 38.8, 39.4}, //9
  { 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44}, //10
  { 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44}, //11
  { 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44}  //13
  
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
    x = 5 * turbineVoltage / voltageData[0][currentBladePitch - 76];
  } else if (i<6){
    x =  5 + i + (turbineVoltage - voltageData[i][currentBladePitch-76])/( voltageData[i+1][currentBladePitch-76] - voltageData[i][currentBladePitch-76] );
  } else {   //i >= 6
    //double dw_dv = 2.0/(voltageData[7][currentBladePitch-76] - voltageData[6][currentBladePitch-76]);
    //x =  11.0 + (turbineVoltage - voltageData[6][currentBladePitch-76])*dw_dv;
    return 10;
  }
  Serial.print(x);
  return x;  
}

int optimumPitch( double windSpeed ) {
  double optim[12] = { 85, 80, 79, 78, 78, 62, 56, 50, 48, 44, 40, 36};//9
  if( windSpeed < 5) {
    if( windSpeed > 3.75) {
      return 85   ; // magic number smd
    }
    Serial.print(' ');  Serial.println(STARTUP_PITCH);
    return STARTUP_PITCH;
  } else if (windSpeed > 13.3) {      //Too high, brake
    Serial.print(' ');  Serial.println();
    return 40;
  } else {
    double x = optim[(int)(windSpeed - 5)] + (optim[(int)(windSpeed - 5) + 1] - optim[(int)(windSpeed - 5)])*(windSpeed - floor(windSpeed));
    Serial.print(' '); Serial.println(round(x));
    return (int) round(x);
  }
}

