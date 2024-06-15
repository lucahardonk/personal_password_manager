#include "MyLibPasswordInjector.h"



void eraseEEPROM(int eepromAddress) {
    Serial.println("Starting EEPROM erase...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("erasing all memory");

    for(unsigned int i = 0; i < EEPROM_SIZE_BYTES; i++) {
        writeToEEPROM(i, ' ', eepromAddress); 
    }

    Serial.println("Erase complete");
}

void replaceWhitespaceWithUnderscore(String &inputString) {
    for (int i = 0; i < inputString.length(); i++) {
        if (inputString[i] == ' ') {
            inputString[i] = '_'; // Replace whitespace with underscore
        }
    }
}

void safeStrCopy(char* dest, const char* src, size_t maxLen) {
    strncpy(dest, src, maxLen - 1);
    dest[maxLen - 1] = '\0'; // Ensure null termination
}

void writeToEEPROM(int address, byte data, int eepromAddress) {
    Wire.beginTransmission(eepromAddress); // Start communication with EEPROM
    Wire.write((int)(address >> 8));       // High byte of address
    Wire.write((int)(address & 0xFF));     // Low byte of address
    Wire.write(data);                      // Data to be written
    Wire.endTransmission();                // End communication
    delay(5);                              // Delay for EEPROM to finish writing
}

byte readFromEEPROM(int address, int eepromAddress) {
    byte data = 0;                          // Variable to store read data
    Wire.beginTransmission(eepromAddress);   // Start communication with EEPROM
    Wire.write((int)(address >> 8));         // High byte of address
    Wire.write((int)(address & 0xFF));       // Low byte of address
    Wire.endTransmission(false);             // End transmission with repeated start
    Wire.requestFrom(eepromAddress, 1);      // Request 1 byte of data from EEPROM
    if (Wire.available()) {
        data = Wire.read();                    // Read the data
    }
    return data;                             // Return read data
}

void writeCredentialToEEPROM(Credential cred, int startAddress, int eepromAddress) {
    byte* data = (byte*)&cred;
    for (int i = 0; i < sizeof(Credential); i++) {
        writeToEEPROM(startAddress + i, data[i], eepromAddress);
    }
}

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

void printCredential(const Credential& credential) {
    Serial.print("Title: ");
    Serial.println(credential.title);
    Serial.print("Username: ");
    Serial.println(credential.username);
    Serial.print("Password: ");
    Serial.println(credential.password);
}

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


//-----------------------------------------------------------------> new utilities functions
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


int numberPasswordSaved(int EEPROM_ADDRESS) {
  int passowords_in_memory = findFirstFreeIndex(EEPROM_ADDRESS);
  if (passowords_in_memory == -1) {
    passowords_in_memory = NUMBER_OF_PASSWORDS;
  } else {
    passowords_in_memory = (passowords_in_memory) / 140;
  }
  return passowords_in_memory;
}


void readJoystick(int debaounceDelay) {
  
  if (analogRead(XAXISPIN) > 1000) {
    
    arrowPointing++;
    updateView(arrowPointing);
    delay(debaounceDelay);
  } else if (analogRead(XAXISPIN) < 20) {
    
    arrowPointing--;
    updateView(arrowPointing);
    delay(debaounceDelay);
  }
  
  if (analogRead(YAXISPIN) > 1000) {
    
    if(arrowPointing == 3){arrowPointing = 1;}
    else{arrowPointing++;}
    updateView(arrowPointing);
    delay(debaounceDelay);
  } else if (analogRead(YAXISPIN) < 20) {
    
    if(arrowPointing == 1){arrowPointing = 3;}
    else{arrowPointing--;}
    updateView(arrowPointing);
    delay(debaounceDelay);
  }
  
  if (analogRead(BUTTONPIN) < 10) {
    if(arrowPointing == 1){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("scan fingerprint");
      //ask for fingerprint
      while(getFingerprintID() == 2);
      Serial.println("Injecting Username");
      mySerial.println(currentPasswordInDisplay.username);
      tone(BUZZERPIN, 1500, 500);
      refreshMenu = true;
    }
    else if(arrowPointing == 3){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("scan fingerprint");
      //ask for autentication
      while(getFingerprintID() == 2);
      Serial.println("Injecting Password");
      mySerial.println(currentPasswordInDisplay.password);
      tone(BUZZERPIN, 1500, 500);
      refreshMenu = true;

    }
    delay(debaounceDelay);
  }
}

void updateView(int arrow){
  refreshMenu = true;
  if(arrow == -1){
    currentPasswordIndex--;
    arrowPointing = 0;
  }
  else if(arrow == 2){
    currentPasswordIndex++;
    arrowPointing = 0;
  }
}

void loadPasswords(int request_index, int max_passwords, Credential* saveCredentials) {

  int retriveIndex = abs(request_index) % max_passwords;
  Serial.println("1 index retrived: " + String(retriveIndex));
  readCredentialFromEEPROM(*saveCredentials, retriveIndex * CREDENTIAL_SIZE, EEPROM1_ADDRESS);
}


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

// returns -1 if failed, otherwise returns ID #
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

void printNumberingCredential(int EEPROM_ADDRESS){
  int passwords = numberPasswordSaved(EEPROM_ADDRESS);
  Credential temp;
  for(int a = 0; a < passwords; a++  ){
    Serial.println("");
    readCredentialFromEEPROM(temp, a*CREDENTIAL_SIZE, EEPROM_ADDRESS );
    Serial.println("Credential saved at:" +  String(a));
    printCredential (temp);

  }


}


void deleteCredentials(int index, int EEPROM_ADDRESS){
  Credential temp;
  readCredentialFromEEPROM(temp, findFirstFreeIndex(EEPROM_ADDRESS)-CREDENTIAL_SIZE, EEPROM_ADDRESS);
  int lastIndex = (numberPasswordSaved(EEPROM_ADDRESS)-1);
  
  for(int a=0; a<CREDENTIAL_SIZE; a++){ //delete first credential
      writeToEEPROM(a+(index*CREDENTIAL_SIZE), ' ', EEPROM_ADDRESS);
    }
    
    //Serial.println("lastIndex: " + String(lastIndex));
  for(int a=0; a<CREDENTIAL_SIZE; a++){ //delete last credential
    writeToEEPROM(a+(lastIndex*CREDENTIAL_SIZE) , ' ', EEPROM_ADDRESS);
  }

  if(index != lastIndex){writeCredentialToEEPROM(temp, index*CREDENTIAL_SIZE, EEPROM_ADDRESS);}
}






bool isNumber(String str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    if (!isDigit(str.charAt(i))) {
      return false;
    }
  }
  return true;
}


void copyAintoB(int eepromAddressA, int eepromAddressB){
  for(long i = 0; i < EEPROM_SIZE_BYTES; i++){
    writeToEEPROM(i, readFromEEPROM(i, eepromAddressA), eepromAddressB);
  }

}


void printEEPROM(int eepromAddress){
  Serial.println("reading eeprom: " + String(eepromAddress));
  for(long i = 0; i < EEPROM_SIZE_BYTES; i++){
    Serial.print(char(readFromEEPROM(i, eepromAddress)));
  }
  Serial.println("");
}

