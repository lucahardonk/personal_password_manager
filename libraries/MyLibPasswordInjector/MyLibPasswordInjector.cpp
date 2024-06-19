#include "MyLibPasswordInjector.h"

// Function to erase the entire EEPROM
void eraseEEPROM(int eepromAddress) {
    Serial.println("Starting EEPROM erase...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("erasing all memory");

    // Loop through each byte of EEPROM and write a space character
    for(unsigned int i = 0; i < EEPROM_SIZE_BYTES; i++) {
        writeToEEPROM(i, ' ', eepromAddress); 
    }

    Serial.println("Erase complete");
}

// Function to replace all whitespace characters in a string with underscores
void replaceWhitespaceWithUnderscore(String &inputString) {
    for (int i = 0; i < inputString.length(); i++) {
        if (inputString[i] == ' ') {
            inputString[i] = '_'; // Replace whitespace with underscore
        }
    }
}

// Function to safely copy a C-string with a maximum length
void safeStrCopy(char* dest, const char* src, size_t maxLen) {
    strncpy(dest, src, maxLen - 1);
    dest[maxLen - 1] = '\0'; // Ensure null termination
}

// Function to write a byte of data to a specific address in EEPROM
void writeToEEPROM(int address, byte data, int eepromAddress) {
    Wire.beginTransmission(eepromAddress); // Start communication with EEPROM
    Wire.write((int)(address >> 8));       // High byte of address
    Wire.write((int)(address & 0xFF));     // Low byte of address
    Wire.write(data);                      // Data to be written
    Wire.endTransmission();                // End communication
    delay(5);                              // Delay for EEPROM to finish writing
}

// Function to read a byte of data from a specific address in EEPROM
byte readFromEEPROM(int address, int eepromAddress) {
    byte data = 0;                          // Variable to store read data
    Wire.beginTransmission(eepromAddress);   // Start communication with EEPROM
    Wire.write((int)(address >> 8));         // High byte of address
    Wire.write((int)(address & 0xFF));       // Low byte of address
    Wire.endTransmission(false);             // End transmission with repeated start
    Wire.requestFrom(eepromAddress, 1);      // Request 1 byte of data from EEPROM
    if (Wire.available()) {
        data = Wire.read();                  // Read the data
    }
    return data;                             // Return read data
}

// Function to write a Credential structure to EEPROM
void writeCredentialToEEPROM(Credential cred, int startAddress, int eepromAddress) {
    byte* data = (byte*)&cred;
    for (int i = 0; i < sizeof(Credential); i++) {
        writeToEEPROM(startAddress + i, data[i], eepromAddress);
    }
}

// Function to read a Credential structure from EEPROM
void readCredentialFromEEPROM(Credential &cred, int startAddress, int eepromAddress) {
    byte* data = (byte*)&cred;
    for (int i = 0; i < sizeof(Credential); i++) {
        data[i] = readFromEEPROM(startAddress + i, eepromAddress);
    }
    // Ensure null-termination for strings
    cred.title[sizeof(cred.title) - 1] = '\0';
    cred.username[sizeof(cred.username) - 1] = '\0';
    cred.password[sizeof(cred.password) - 1] = '\0';
}

// Function to print the details of a Credential structure to Serial
void printCredential(const Credential& credential) {
    Serial.print("Title: ");
    Serial.println(credential.title);
    Serial.print("Username: ");
    Serial.println(credential.username);
    Serial.print("Password: ");
    Serial.println(credential.password);
}

// Function to find the first free index in EEPROM
int findFirstFreeIndex(int eepromAddress) {
    // Iterate through each page
    for (unsigned int page = 0; page < NUMBER_OF_PASSWORDS; page++) {
        char firstbyte = readFromEEPROM(page * CREDENTIAL_SIZE, eepromAddress);
        //Serial.print("char found: "); Serial.println(firstbyte);
        if (firstbyte == ' ') {
            return page * CREDENTIAL_SIZE; // Return the index of the first page with an empty title
        }
    }
    // If no free page is found, return -1 to indicate no free index
    return -1;
}

// Function to check if two EEPROM memories are equal
bool memoryEqual(int eepromAddress1, int eepromAddress2) {
    int lastFreeIndex = findFirstFreeIndex(eepromAddress1);
    if (findFirstFreeIndex(eepromAddress2) == lastFreeIndex) {
        for (int i = 0; i < lastFreeIndex; i++) {
            if (readFromEEPROM(i, EEPROM1_ADDRESS) != readFromEEPROM(i, EEPROM2_ADDRESS)) {
                return false;
            }
        }
    } else {
        return false;
    }
    return true;
}

// Function to get the number of passwords saved in EEPROM
int numberPasswordSaved(int EEPROM_ADDRESS) {
    int passwords_in_memory = findFirstFreeIndex(EEPROM_ADDRESS);
    if (passwords_in_memory == -1) {
        passwords_in_memory = NUMBER_OF_PASSWORDS;
    } else {
        passwords_in_memory = passwords_in_memory / 140;
    }
    return passwords_in_memory;
}

// Function to read the joystick input with a debounce delay
void readJoystick(int debounceDelay) {
    if (analogRead(XAXISPIN) > 1000) {
        arrowPointing++;
        updateView(arrowPointing);
        delay(debounceDelay);
    } else if (analogRead(XAXISPIN) < 20) {
        arrowPointing--;
        updateView(arrowPointing);
        delay(debounceDelay);
    }

    if (analogRead(YAXISPIN) > 1000) {
        if(arrowPointing == 3) {
            arrowPointing = 1;
        } else {
            arrowPointing++;
        }
        updateView(arrowPointing);
        delay(debounceDelay);
    } else if (analogRead(YAXISPIN) < 20) {
        if(arrowPointing == 1) {
            arrowPointing = 3;
        } else {
            arrowPointing--;
        }
        updateView(arrowPointing);
        delay(debounceDelay);
    }

    if (analogRead(BUTTONPIN) < 10) {
        if(arrowPointing == 1) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("scan fingerprint");
            // Ask for fingerprint
            while(getFingerprintID() == 2);
            Serial.println("Injecting Username");
            mySerial.println(currentPasswordInDisplay.username);
            tone(BUZZERPIN, 1500, 500);
            refreshMenu = true;
        } else if(arrowPointing == 3) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("scan fingerprint");
            // Ask for authentication
            while(getFingerprintID() == 2);
            Serial.println("Injecting Password");
            mySerial.println(currentPasswordInDisplay.password);
            tone(BUZZERPIN, 1500, 500);
            refreshMenu = true;
        }
        delay(debounceDelay);
    }
}

// Function to update the view based on the arrow position
void updateView(int arrow) {
    refreshMenu = true;
    if(arrow == -1) {
        currentPasswordIndex--;
        arrowPointing = 0;
    } else if(arrow == 2) {
        currentPasswordIndex++;
        arrowPointing = 0;
    }
}

// Function to load passwords from EEPROM into a given array of credentials
void loadPasswords(int request_index, int max_passwords, Credential* saveCredentials) {
    int retrieveIndex = abs(request_index) % max_passwords;
    Serial.println("1 index retrieved: " + String(retrieveIndex));
    readCredentialFromEEPROM(*saveCredentials, retrieveIndex * CREDENTIAL_SIZE, EEPROM1_ADDRESS);
}

// Function to get the fingerprint ID
uint8_t getFingerprintID() {
    uint8_t p = finger.getImage();
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println("No finger detected");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    // OK success!
    p = finger.image2Tz();
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image converted");
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FINGERPRINT_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    // OK converted!
    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK) {
        Serial.println("Found a print match!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_NOTFOUND) {
        Serial.println("Did not find a match");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);

    return finger.fingerID;
}

// Function to get the fingerprint ID with easy return values
int getFingerprintIDez() {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)  return -1;

    // found a match!
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    return finger.fingerID;
}

// Function to print all credentials saved in EEPROM with numbering
void printNumberingCredential(int EEPROM_ADDRESS) {
    int passwords = numberPasswordSaved(EEPROM_ADDRESS);
    Credential temp;
    for(int a = 0; a < passwords; a++ ) {
        Serial.println("");
        readCredentialFromEEPROM(temp, a * CREDENTIAL_SIZE, EEPROM_ADDRESS );
        Serial.println("Credential saved at:" +  String(a));
        printCredential(temp);
    }
}

// Function to delete a credential from EEPROM
void deleteCredentials(int index, int EEPROM_ADDRESS) {
    Credential temp;
    readCredentialFromEEPROM(temp, findFirstFreeIndex(EEPROM_ADDRESS) - CREDENTIAL_SIZE, EEPROM_ADDRESS);
    int lastIndex = (numberPasswordSaved(EEPROM_ADDRESS) - 1);

    for(int a = 0; a < CREDENTIAL_SIZE; a++) { // Delete first credential
        writeToEEPROM(a + (index * CREDENTIAL_SIZE), ' ', EEPROM_ADDRESS);
    }

    //Serial.println("lastIndex: " + String(lastIndex));
    for(int a = 0; a < CREDENTIAL_SIZE; a++) { // Delete last credential
        writeToEEPROM(a + (lastIndex * CREDENTIAL_SIZE), ' ', EEPROM_ADDRESS);
    }

    if(index != lastIndex) {
        writeCredentialToEEPROM(temp, index * CREDENTIAL_SIZE, EEPROM_ADDRESS);
    }
}

// Function to check if a string is a number
bool isNumber(String str) {
    for (unsigned int i = 0; i < str.length(); i++) {
        if (!isDigit(str.charAt(i))) {
            return false;
        }
    }
    return true;
}

// Function to copy contents of one EEPROM to another
void copyAintoB(int eepromAddressA, int eepromAddressB) {
    for(long i = 0; i < EEPROM_SIZE_BYTES; i++) {
        writeToEEPROM(i, readFromEEPROM(i, eepromAddressA), eepromAddressB);
    }
}

// Function to print the contents of an EEPROM to Serial
void printEEPROM(int eepromAddress) {
    Serial.println("reading eeprom: " + String(eepromAddress));
    for(long i = 0; i < EEPROM_SIZE_BYTES; i++) {
        Serial.print(char(readFromEEPROM(i, eepromAddress)));
    }
    Serial.println("");
}
