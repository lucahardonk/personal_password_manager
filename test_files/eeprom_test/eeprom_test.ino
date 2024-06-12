#include <Wire.h>

#define EEPROM1_ADDRESS 0x50
#define EEPROM2_ADDRESS 0x51

struct Credential {
    char title[40];
    char username[50];
    char password[50];
};

// Function to safely copy a string into a fixed-length character array
void safeStrCopy(char* dest, const char* src, size_t maxLen) {
    strncpy(dest, src, maxLen - 1);
    dest[maxLen - 1] = '\0'; // Ensure null termination
}

// Function to write a byte to EEPROM
void writeToEEPROM(int address, byte data, int eepromAddress) {
    Wire.beginTransmission(eepromAddress); // Start communication with EEPROM
    Wire.write((int)(address >> 8));       // High byte of address
    Wire.write((int)(address & 0xFF));     // Low byte of address
    Wire.write(data);                      // Data to be written
    Wire.endTransmission();                // End communication
    delay(5);                              // Delay for EEPROM to finish writing
}

// Function to read a byte from EEPROM
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

// Function to write the Credential structure to EEPROM
void writeCredentialToEEPROM(Credential cred, int startAddress, int eepromAddress) {
    byte* data = (byte*)&cred;
    for (int i = 0; i < sizeof(Credential); i++) {
        writeToEEPROM(startAddress + i, data[i], eepromAddress);
    }
}

// Function to read the Credential structure from EEPROM
void readCredentialFromEEPROM(Credential &cred, int startAddress, int eepromAddress) {
    byte* data = (byte*)&cred;
    for (int i = 0; i < sizeof(Credential); i++) {
        data[i] = readFromEEPROM(startAddress + i, eepromAddress);
    }
}

void setup() {
    Wire.begin();        // Initialize I2C communication
    Serial.begin(115200);  // Initialize Serial communication

    // Create a Credential structure
    Credential cred1;
    safeStrCopy(cred1.title, "This is a very long title that exceeds the maximum length", sizeof(cred1.title));
    safeStrCopy(cred1.username, "User1", sizeof(cred1.username));
    safeStrCopy(cred1.password, "Pass1", sizeof(cred1.password));

    Credential cred2;
    safeStrCopy(cred2.title, "Title2", sizeof(cred2.title));
    safeStrCopy(cred2.username, "User2", sizeof(cred2.username));
    safeStrCopy(cred2.password, "Pass2", sizeof(cred2.password));

    // Write the Credential structures to EEPROM1 and EEPROM2
    writeCredentialToEEPROM(cred1, 0, EEPROM1_ADDRESS);
    writeCredentialToEEPROM(cred2, 0, EEPROM2_ADDRESS);

    // Create empty Credential structures to read data into
    Credential readCred1;
    Credential readCred2;

    // Read the Credential structures from EEPROM1 and EEPROM2
    readCredentialFromEEPROM(readCred1, 0, EEPROM1_ADDRESS);
    readCredentialFromEEPROM(readCred2, 0, EEPROM2_ADDRESS);

    // Print the read Credential structures to the Serial Monitor
    Serial.println("Credential read from EEPROM1:");
    Serial.print("Title: "); Serial.println(readCred1.title);
    Serial.print("Username: "); Serial.println(readCred1.username);
    Serial.print("Password: "); Serial.println(readCred1.password);

    Serial.println("\nCredential read from EEPROM2:");
    Serial.print("Title: "); Serial.println(readCred2.title);
    Serial.print("Username: "); Serial.println(readCred2.username);
    Serial.print("Password: "); Serial.println(readCred2.password);
}

void loop() {
    // Do nothing in loop
}
