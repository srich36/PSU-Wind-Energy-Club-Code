#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

void setup() {
  // Open serial communications and wait for port to open:
  mySerial.begin(57600);
  Serial.begin(9600);
  
}

void loop(){
  Serial.println("testing to see if this works");
  mySerial.write(25);
  delay(10000);
}
