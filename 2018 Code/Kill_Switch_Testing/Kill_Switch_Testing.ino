
void setup(){
  pinMode(A1, INPUT);
  Serial.begin(9600);
  Serial.println("Starting. ");
  
}


void loop(){
  boolean killed = (digitalRead(A1) == LOW);
  double voltage = digitalRead(A1);
  Serial.print("Voltage equals: ");
  if(digitalRead(A1) == HIGH){
    Serial.println("Returned high");
    
  }
  else{
    Serial.println("Returned low");
  }
  Serial.println(voltage);
  Serial.println(killed);
  delay(5000);
}
