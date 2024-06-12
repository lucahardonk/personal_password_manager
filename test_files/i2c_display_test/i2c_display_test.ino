#include <Wire.h>
#include <LiquidCrystal_I2C.h>


bool debug = true;
int joystickArray[3];      // 0 -> axisx, 1-> axisy , 2 -> buttonPressed
#define BUTTONPIN A2
#define XAXISPIN A1
#define YAXISPIN A0


// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initial position of the arrow
int arrowPosition = 0; // 0 for "Add Password", 1 for "Enroll Finger"

byte customChar[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
  B11111
};


// Function to display the arrow at the current position
void displayArrow() {
  lcd.setCursor(arrowPosition, 1);
  lcd.write(">");
}

// Function to erase the arrow
void eraseArrow() {
  lcd.setCursor(arrowPosition, 1);
  lcd.write(" ");
}


void setup() {
  // Initialize the LCD
  Serial.begin(9600);
  Serial.println("hello");
  //init_jopystick
  pinMode(BUTTONPIN, INPUT);
  pinMode(XAXISPIN, INPUT);
  pinMode(YAXISPIN, INPUT);



  lcd.init();

  // Turn on the backlight
  lcd.backlight();
  // Turn off the backlight
  //lcd.noBacklight();

  // Print a message to the LCD
  lcd.setCursor(0, 0);
  lcd.print("Hy");
/*
  lcd.createChar(0, customChar);
  lcd.begin(16, 2);
  lcd.setCursor(0, 1);
  //lcd.write(byte(0)); // Stampa il carattere personalizzato*/

}

void loop() {
  readJoystick(joystickArray);
   delay(100);

  // Move arrow based on joystick input
  if (joystickArray[0] == -1 && arrowPosition > 0) {
    arrowPosition = 0;
  } else if (joystickArray[0] == 1 && arrowPosition < 1) {
    arrowPosition = 8;
  }

  // Display arrow at the new position
  //displayArrow();

  // Display buttons
  /*lcd.setCursor(0, 1);
  lcd.print(" Add Password ");

  lcd.setCursor(8, 1);
  lcd.print(" Enroll Finger ");
*/

}



void readJoystick(int* a){
  a[0] = 0;
  if(analogRead(XAXISPIN) > 900){
    a[0] = 1; 
  }
  else if(analogRead(XAXISPIN) < 10){
    a[0] = -1; 
  }

  a[1] = 0;
  if(analogRead(YAXISPIN) > 900){
    a[1] = 1; 
  }
  else if(analogRead(YAXISPIN) < 10){
    a[1] = -1; 
  }
  
  a[2] = 1 ;
  if(analogRead(BUTTONPIN) > 10){a[2] = 0;}
  if (debug) {
    Serial.print("axis x: ");
    Serial.println(a[0]);
    Serial.print("axis y: ");
    Serial.println(a[1]);
    Serial.print("button: ");
    Serial.println(a[2]);
  }

}
