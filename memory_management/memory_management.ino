
#include <MyLibPasswordInjector.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the address if necessary

void setup() {
    Wire.begin();
    delay(1500);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("EEPROM CLI Ready");
    Serial.begin(115200);
    delay(4000);
    Serial.println("EEPROM CLI Ready");
    Serial.println("Available commands:");
    Serial.println("1) erase - Erase all EEPROM memory");
    Serial.println("2) add <password> - Add a new password");
    Serial.println("3) read - Read EEPROM contents");   
}

void loop() {
    static String command;

    if (Serial.available()) {
        char incomingChar = Serial.read();
        if (incomingChar == '\n') {
            command.trim();
            processCommand(command);
            command = ""; // Clear the command string for the next command
        } else {
            command += incomingChar;
        }
    }
}

void processCommand(String command) {
    lcd.clear();
    if (command == "erase") {
        Serial.println("Erasing EEPROM 1...");
        eraseEEPROM(EEPROM1_ADDRESS);
        Serial.println("Erasing EEPROM 2...");
        eraseEEPROM(EEPROM2_ADDRESS);
        lcd.setCursor(0, 0);
        lcd.print("EEPROM erased");
        Serial.println("EEPROM erased");
    } else if (command == "add") {
        int firstFreeIndex = findFirstFreeIndex(EEPROM1_ADDRESS);
        if (firstFreeIndex != -1) {
            // Create a new credential
            Credential newCredential;
            String inputString;

            // Read the title, username, and password from Serial input
            Serial.println("No white spaces please ;(");


            Serial.println("Enter title:");
            while (!Serial.available()); // Wait for user input
            inputString = Serial.readStringUntil('\n');
            inputString.trim();
            replaceWhitespaceWithUnderscore(inputString);
            strncpy(newCredential.title, inputString.c_str(), sizeof(newCredential.title));
            //----------------------------->
            Serial.println("Enter username:");
            while (!Serial.available()); // Wait for user input
            inputString = Serial.readStringUntil('\n');
            inputString.trim();
            replaceWhitespaceWithUnderscore(inputString);
            strncpy(newCredential.username, inputString.c_str(), sizeof(newCredential.username));

            Serial.println("Enter password:");
            while (!Serial.available()); // Wait for user input
            inputString = Serial.readStringUntil('\n');
            inputString.trim();
            replaceWhitespaceWithUnderscore(inputString);
            strncpy(newCredential.password, inputString.c_str(), sizeof(newCredential.password));


            printCredential(newCredential);
            // Write the new credential to EEPROM
            writeCredentialToEEPROM(newCredential, firstFreeIndex,EEPROM1_ADDRESS );
            writeCredentialToEEPROM(newCredential, firstFreeIndex,EEPROM2_ADDRESS );
            

            // Display success message
            lcd.setCursor(0, 0);
            lcd.print("Password saved");
            Serial.println("Password added");
        } else {
            // EEPROM memory is full
            lcd.setCursor(0, 0);
            lcd.print("EEPROM is full");
            Serial.println("EEPROM is full");
        }
    } else if (command == "read") {
        lcd.setCursor(0, 0);
        lcd.print("Reading EEPROM");

        

        // Read the Credential structures from EEPROM1 and EEPROM2
        Credential readCred;
        Serial.println("reading from mem 1: ");
        for(int a = 0; a < NUMBER_OF_PASSWORDS; a++){
          
          readCredentialFromEEPROM(readCred, CREDENTIAL_SIZE*a, EEPROM1_ADDRESS);
          printCredential(readCred);
        }
        Serial.println("-------------------------------------");
        Serial.println("reading from mem 2:  ");
        for(int a = 0; a < NUMBER_OF_PASSWORDS; a++){
          readCredentialFromEEPROM(readCred, CREDENTIAL_SIZE*a, EEPROM2_ADDRESS);
          printCredential(readCred);
        }
        lcd.setCursor(0, 1);
        lcd.print("Read complete");
    } else {
        lcd.setCursor(0, 0);
        lcd.print("Invalid command");
        Serial.println("Invalid command");
    }
}
