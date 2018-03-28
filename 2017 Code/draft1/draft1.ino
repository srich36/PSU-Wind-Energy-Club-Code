// Patrick Nicodemus (gadget142@gmail,com), Joey Miscioscia, Mitch Proulx
// Spring 2017
// Penn State Collegiate Wind Competition
// Load control code

// This code performs these basic functions.
// 1. Latching mechanism: The rules require that the turbine should draw no power at the beginning of the competition,
//    but the controller on the turbine will eventually require external power in the event that the turbine brakes
//    and cannot supply its own power. We resolve this by a latch that turns on at about midway through the competition
//    once nce the turbine starts supplying power to the load.
// 2. "Variable-load" - there is a voltage regulator on the turbine side whose output is varied by altering its feedback loop.
 

const double SUPPLY_VOLTAGE = 8; // Supplied by a power supply hooked up to the turbine on the load side.
const double R1 = 98.3; //kOhms
const double R2 = 9.71; // kOhms
const double VOLTAGE_DIVISION_RATIO = R2/(R1+R2)*29.3/30;
const double TURN_ON_VOLTAGE = 12; // At what voltage should the power latch turn on and stay on?
const double BEGIN_DRAWING_POWER = 5; // At what voltage should the regulator begin drawing power?
const double BAND_GAP_VOLTAGE = 1.23; //The voltage regulator uses this number as a reference.

const int latchWritePin = 7;
const int PWMPin = 3;
const int regulatorPin = 5;

bool latch, regulatorOn = false;

double sampleBusVoltage() {
  double maxVoltage = 5; // Between 0 and 5V
  double resolution = 1024; // analogRead returns an integer between 0 and 1023
  double voltage = analogRead(A0)* maxVoltage / (resolution * VOLTAGE_DIVISION_RATIO);
  Serial.print(voltage);
  Serial.println();
  return voltage;
}

void writePWMVoltage(double voltage){
  int PWMdutycycle = (int) ((voltage/5.0) * 255);
  analogWrite(PWMPin,PWMdutycycle);
}

double voltageMap(double v_in){
  // This function is a map between the input voltage of the regulator and the desired output voltage across the resistor.
  // The output voltage must be lower than the voltage in.
  // The ideal function will be determined experimentallly through our testing.
  // For now, let
  double v_out;

  v_out = (v_in*v_in)/48;
  if(v_out > v_in - 1.5){
    v_out = v_in - 1.5;
  }
  return v_out;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  latch = false;
  regulatorOn = false;
  digitalWrite(latchWritePin, LOW);
  digitalWrite(regulatorPin, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:  
  double v_in = sampleBusVoltage();

  // If the voltage exceeds the power-source-enabled threshold, turn on the latch.
  if(!latch && v_in > TURN_ON_VOLTAGE){
    digitalWrite(latchWritePin, HIGH);
    latch = true;
  }

  // If the voltage exceeds the draw-power-from-turbine threshold, turn on the regulator.
  if(!regulatorOn && v_in > BEGIN_DRAWING_POWER){
    digitalWrite(regulatorPin, LOW);
    regulatorOn = true;+
  }
    
  double v_load = voltageMap(v_in);
  if( v_load < BAND_GAP_VOLTAGE) {
    writePWMVoltage(0);
  } else {
    double opAmpBias = (v_load - BAND_GAP_VOLTAGE)/12.0;
    if (opAmpBias >= 5){
      writePWMVoltage(5);
    } else {
      writePWMVoltage(opAmpBias);
    }
  }


  delay(10);
}

