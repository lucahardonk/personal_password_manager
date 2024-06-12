#include <SoftwareSerial.h>
#include <Keyboard.h>

// Define the software serial pins
#define RX_PIN 8
#define TX_PIN 9

SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX

void setup() {
  // Start the USB serial communication for debugging
  //Serial.begin(9600);
  
  // Start the software serial communication
  mySerial.begin(9600);
  
  // Start the Keyboard library
  Keyboard.begin();
}

void loop() {
  // Check if data is available to read
  if (mySerial.available()) {
    // Read the incoming data until a newline character is encountered
    String data = mySerial.readStringUntil('\n');
    
    // Print the received data to the Serial Monitor for debugging
    Serial.print(data);
    
    // Send the received data as keyboard input
    Keyboard.print(data);
    Keyboard.print('\n'); // Ensure the end character is newline
    
    // Add a small delay for stability
    delay(5);
  }
}

