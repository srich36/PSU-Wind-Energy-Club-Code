//TODO: 
//      1. figure out how to send the control signal to the arduino to break
//      2. Confirm both the voltage divider constants
//      3. Implement the kill switch in the hardware and test to make sure it is reading it correctly
//      4. Confirm constant buffer values
//     


//const double MOSFET_BUFFER_VOLTAGE = 8.1;
//const double VOLTAGE_DIVIDER_PRE_PCC= 14.327; //From last year
//const double VOLTAGE_DIVIDER_LOAD= 10; // Need to confirm both of these values
const int MOSFET_SIGNAL_PIN = 8;
const int CONTROL_ARDUINO_PIN = 10;
//const int KILL_SWITCH_PIN = 11;        //Generic pin values that we can figure out
const double VOLTAGE_DIFFERENT_BUFFER = .5;
const double LOAD_VOTLAGE_BUFFER = .5;
//static boolean killSwitchState = false;
//static boolean mosfetOn = false;



boolean determineDisconnect(double loadVoltage, double turbineVoltage);
void setup(){
  pinMode(MOSFET_SIGNAL_PIN, OUTPUT);
  //pinMode(KILL_SWITCH_PIN, INPUT);
  pinMode(CONTROL_ARDUINO_PIN, INPUT);
  digitalWrite(MOSFET_SIGNAL_PIN, LOW);
}

void loop(){
  //double prePCCVoltage= VOLTAGE_DIVIDER_PRE_PCC*((double)analogRead(A0))*5.0/1023.0; //Attach the turbine voltage reading on pin A0
  //double loadVoltage = VOLTAGE_DIVIDER_LOAD*((double)analogRead(A1))*5.0/1023.0;  //Attach the load voltage reading on A1
  
  /*
  //For debugging
  Serial.print("Reading the pre pcc Voltage as: ");
  Serial.println(prePCCVoltage);
  Serial.print("Reading the load voltage as: ");
  Serial.println(loadVoltage);
  //For debugging
  */
  //Test this part first initially.
  
  //boolean pccDisconnect = determineDisconnect(loadVoltage, prePCCVoltage);
  boolean braked = (digitalRead(CONTROL_ARDUINO_PIN) == LOW); //Has to be low because when the other arduino loses power it will be low and know it's braked
  if(braked){
    digitalWrite(MOSFET_SIGNAL_PIN, HIGH);
    
  }
  
  
  
  //******************************************************************************//
  //******************************************************************************//
  //*******************************KILL SWITCH BLOCK******************************//
  //******************************************************************************//
  //******************************************************************************//
  
  /*
  
  if(killSwitchHit){
    killSwitchState = true;
    digitalWrite(CONTROL_ARDUINO_PIN, HIGH);
    //send a signal to the control arduino to break.
  }
  else{
    if(killSwitchState!=killSwitchHit){
      //This means the kill switch was flipped back off and we are ready to begin pitching and producing power
      
      digitalWrite(MOSFET_SIGNAL_PIN, HIGH); //turn on the mosfet
      
      //For debugging
      Serial.println("Turning the Mosfet on");
      //For debugging
      
      delay(10000);                         //Wait 10 secons
      digitalWrite(MOSFET_SIGNAL_PIN, LOW); //turn mosfet back off
      
      //For debugging
      Serial.println("Turbin the mosfet off");
      //For debugging
      
      killSwitchState = killSwitchHit;      //reset the states so that they are equal
      */
    //}
  }
  
  
  
  //******************************************************************************//
  //******************************************************************************//
  //*****************************PCC DISCONNECT BLOCK*****************************//
  //******************************************************************************//
  //******************************************************************************//
/*  
  if(pccDisconnect){
    //send a signal to the control arduino to break
    mosfetOn = true;
    
    //For debugging
    Serial.println("PCC Disconnected. Turning the Mosfet back on. ");
    //For debugging
    
    
    digitalWrite(MOSFET_SIGNAL_PIN, HIGH);
  }
  else{
    if(mosfetOn){
      if(prePCCVoltage-loadVoltage > VOLTAGE_DIFFERENT_BUFFER && loadVoltage > MOSFET_BUFFER_VOLTAGE){
        digitalWrite(MOSFET_SIGNAL_PIN, LOW);
        mosfetOn = false;
      }
    }
  }
  
}


boolean determineDisconnect(double loadVoltage, double turbineVoltage){
  if(turbineVoltage-loadVoltage > VOLTAGE_DIFFERENT_BUFFER && loadVoltage < LOAD_VOTLAGE_BUFFER){
    return true;
  }
  return false;
}

*/
