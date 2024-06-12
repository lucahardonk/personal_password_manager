#include <MyLibPasswordInjector.h>

// Define the software serial pins
#define RX_PIN 8
#define TX_PIN 9
#define BUZZERPIN 6

#define fingerprintSerial Serial1
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprintSerial);


SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX

/*already present into library but useful to code*/
#define EEPROM1_ADDRESS 0x50
#define EEPROM2_ADDRESS 0x51

#define EEPROM_SIZE_BYTES 32768  //cells of 1 byte
#define CREDENTIAL_SIZE 140      // Page size of the EEPROM
#define NUMBER_OF_PASSWORDS 234

#define BUTTONPIN A2
#define XAXISPIN A1
#define YAXISPIN A0
#define DEBAUNCE 200   //ms


LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust the address if necessary
byte arrowIcon[8] = {
    B00000,
    B00100,
    B00010,
    B11111,
    B00010,
    B00100,
    B00000,
};
unsigned long lastScrollTime = 0;
unsigned long scrollInterval = 500; // Default scroll interval in milliseconds
String lastScrollingword;


int passowords_in_memory = 0;
int arrowPointing = 0;



int currentPasswordIndex = 0;
Credential currentPasswordInDisplay;  // on the diplay
bool refreshMenu = true;              // to not continously refresh the lcd


void displayMenu(bool refresh) {
  if (refresh) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Title 1");
    lcd.setCursor(0, 1);
    lcd.print("Title 2");
  }
}

void displayTitle1(bool refresh) {
  if (refresh) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Title 1");
    lcd.setCursor(0, 1);
    lcd.print("Username:");
    lcd.setCursor(0, 2);
    lcd.print("Password:");
  }
}

void displayTitle2(bool refresh) {
  if (refresh) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Title 2");
    lcd.setCursor(0, 1);
    lcd.print("Username:");
    lcd.setCursor(0, 2);
    lcd.print("Password:");
  }
}


void setup() {
  mySerial.begin(9600);
  
  Wire.begin();
  lcd.init();
  lcd.createChar(0, arrowIcon);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  //lcd.write((uint8_t)0); //test arrwow
  lcd.print("Pwd injector Ready");
  lcd.setCursor(0, 1);
  lcd.print("memory check");
  Serial.begin(115200);
  delay(4000);
  pinMode(BUZZERPIN, OUTPUT);

  Serial.print("memory check: ");
  /*
  if (memoryEqual(EEPROM1_ADDRESS, EEPROM2_ADDRESS)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pwd injector Ready");
    Serial.println("ready!");


  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("memory not synked");
    while (true) {
      Serial.println("please correct memory before going on");
      delay(1000);
    }
  }*/
  passowords_in_memory = numberPasswordSaved(EEPROM1_ADDRESS);


  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  /*
  Serial.println("testing retiriving function");
  loadPasswords(currentPasswordIndex, passowords_in_memory, currentPasswordInDisplay);
  printCredential(currentPasswordInDisplay[0]);
  Serial.println("------------------------");
  printCredential(currentPasswordInDisplay[1]);
*/

  delay(500);
}

void loop() {
  readJoystick(DEBAUNCE);

  if (refreshMenu) {
    loadPasswords(currentPasswordIndex, passowords_in_memory, &currentPasswordInDisplay);
    printCredential(currentPasswordInDisplay);
    Serial.println("------------------------");
    
    
    lcd.clear();
    if(arrowPointing != 3){
      lcd.setCursor(0,arrowPointing);
    }
    else{
      lcd.setCursor(7,1);
    }
    
    lcd.write((uint8_t)0); // Display custom character (arrow)


    lcd.setCursor(1,0);
    lcd.print(currentPasswordInDisplay.title);
    //bottom part of the display
    lcd.setCursor(1,1);
    lcd.print("user");
    lcd.setCursor(8,1);
    lcd.print("password");
    refreshMenu = false;
  }  
}



