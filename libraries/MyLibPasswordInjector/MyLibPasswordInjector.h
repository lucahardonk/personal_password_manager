#ifndef MYLIBPASSWORDINJECTOR_H
#define MYLIBPASSWORDINJECTOR_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Define the software serial pins
#define RX_PIN 8
#define TX_PIN 9
#define BUZZER_PIN PD7

struct Credential {
    char title[40];
    char username[50];
    char password[50];
}; //234 passwords

#define fingerprintSerial Serial1
extern Adafruit_Fingerprint finger;
extern LiquidCrystal_I2C lcd;
extern SoftwareSerial mySerial;

extern int currentPasswordIndex;
extern Credential currentPasswordInDisplay;
extern bool refreshMenu;             
extern int arrowPointing;
extern volatile bool buzzerOn; // Declare buzzerOn as extern

#define EEPROM1_ADDRESS 0x50
#define EEPROM2_ADDRESS 0x51

#define EEPROM_SIZE_BYTES  32768 //cells of 1 byte
#define CREDENTIAL_SIZE  140    // Page size of the EEPROM
#define NUMBER_OF_PASSWORDS 234

#define BUTTONPIN 5 //A2
#define XAXISPIN 6 //A1
#define YAXISPIN 7 //A0
#define DEBAUNCE 200   //ms

// Function Declarations
void eraseEEPROM(int eepromAddress);
void replaceWhitespaceWithUnderscore(String &inputString);
void safeStrCopy(char* dest, const char* src, size_t maxLen);
void writeToEEPROM(int address, byte data, int eepromAddress);
byte readFromEEPROM(int address, int eepromAddress);
void writeCredentialToEEPROM(Credential &cred, int startAddress, int eepromAddress);
void readCredentialFromEEPROM(Credential &cred, int startAddress, int eepromAddress);
void printCredential(const Credential& credential);
int findFirstFreeIndex(int eepromAddress);
bool memoryEqual(int eepromAddress1, int eepromAddress2);
int numberPasswordSaved(int EEPROM_ADDRESS);
void readJoystick(int debaounceDelay);
int analogReadChannel(uint8_t channel);
void updateView(int arrow);
void loadPasswords(int request_index, int max_passwords, Credential* saveCredentials);
uint8_t getFingerprintID();
int getFingerprintIDez();
void printNumberingCredential(int EEPROM_ADDRESS);
void deleteCredentials(int index, int EEPROM_ADDRESS);
bool isNumber(String str);
void copyAintoB(int eepromAddressA, int eepromAddressB);
void printEEPROM(int eepromAddress);
void setupBuzzer(int pin, int freq);

#endif
