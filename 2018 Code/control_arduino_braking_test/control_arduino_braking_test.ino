const int LOAD_ARDUINO_PIN = 8;

void setup(){
  Serial.begin(9600);
  pinMode(LOAD_ARDUINO_PIN, INPUT);
  Serial.println("Starting. ");
}

void loop(){
  Serial.println("In the loop: ");
  Serial.println("Reading the signal from the load arduino: ");
  double Voltage = digitalRead(LOAD_ARDUINO_PIN);
  Serial.println(Voltage);
  if(digitalRead(LOAD_ARDUINO_PIN)){
    Serial.println("Receiving a high signal. ");
  }
  else{
    Serial.println("Receiving a low signal. ");
  }
  delay(5000);
  
  
}
