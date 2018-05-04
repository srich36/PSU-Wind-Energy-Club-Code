const int KILL_SWITCH_PIN = 11;
const int CONTROL_ARDUINO_PIN = 8;
static int counter = 0;

void setup(){
  Serial.begin(9600);
  pinMode(KILL_SWITCH_PIN, INPUT);
  pinMode(CONTROL_ARDUINO_PIN, OUTPUT);
  counter = 0;
  Serial.println("Starting. ");
  
}

void loop(){
  
  counter++;
  if(counter % 3 == 0){
    digitalWrite(CONTROL_ARDUINO_PIN, HIGH);
    Serial.println("Sending a high voltage");
  }
  else{
    digitalWrite(CONTROL_ARDUINO_PIN, LOW);
    Serial.println("Sending a low voltage");
  }
  delay(5000);
  
  Serial.println("In the loop. ");
  
  
}
