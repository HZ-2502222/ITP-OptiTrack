#include <SoftwareSerial.h>

// Password: 1234

// Serial Monitor Settings: 9600 baud,Both NL & CR
// Send AT commands to the HC-05 module to configure it
// AT
// AT+UART=115200,0,0

// Route the HC-05 through pins 9 and 8
SoftwareSerial BTSerial(9, 8); // RX to TXD(Orange| TX to RXD(Blue)

void setup() {
  Serial.begin(9600);
  Serial.println("Enter AT Commands:");
  
  // HC-05 default baud rate for AT mode is ALWAYS 38400
  BTSerial.begin(38400);  
}

void loop() {
  // Read from HC-05 and print to PC
  if (BTSerial.available()) {
    Serial.write(BTSerial.read());
  }
  // Read from PC and print to HC-05
  if (Serial.available()) {
    BTSerial.write(Serial.read());
  }
}