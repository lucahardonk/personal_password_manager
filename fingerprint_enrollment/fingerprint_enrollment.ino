#include <Adafruit_Fingerprint.h>

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);
const uint8_t MAX_ENROLLED_IDS = 127;
bool enrolledIDs[MAX_ENROLLED_IDS] = {false}; // Initialize all IDs as unoccupied

void setup() {
  Serial.begin(9600);
  Serial1.begin(57600);
  
  while (!Serial);  // Wait for Serial to be ready
  delay(100);
  
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
  
  if (finger.verifyPassword()) {
    Serial1.println("Found fingerprint sensor!");
  } else {
    Serial1.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial1.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial1.print(F("Status: 0x")); Serial1.println(finger.status_reg, HEX);
  Serial1.print(F("Sys ID: 0x")); Serial1.println(finger.system_id, HEX);
  Serial1.print(F("Capacity: ")); Serial1.println(finger.capacity);
  Serial1.print(F("Security level: ")); Serial1.println(finger.security_level);
  Serial1.print(F("Device address: ")); Serial1.println(finger.device_addr, HEX);
  Serial1.print(F("Packet len: ")); Serial1.println(finger.packet_len);
  Serial1.print(F("Baud rate: ")); Serial1.println(finger.baud_rate);

  // Display previously enrolled fingerprints
  Serial.println("Occupied IDs:");
  for (uint8_t i = 1; i <= 127; i++) {
    if (finger.getTemplateCount() > 0) {
      Serial.print(i);
      Serial.print(", ");
    }
  }
  Serial.println();
}



uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  uint8_t id = readnumber();
  if (id == 0 || id > MAX_ENROLLED_IDS) {
    Serial.println("Invalid ID. Please enter a number between 1 and 127.");
    return;
  }
  if (enrolledIDs[id - 1]) {
    Serial.println("ID already occupied. Please choose another ID.");
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!getFingerprintEnroll(id));
}

uint8_t getFingerprintEnroll(uint8_t id) {
  int p = -1;
  Serial.println("Waiting for valid finger to enroll...");
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // Image taken successfully, continue with enrollment process
  p = finger.image2Tz(1);
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

  // Ask user to remove finger
  Serial.println("Remove finger");
  delay(2000); // Give time to remove finger

  // Wait until finger is removed
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  // Mark ID as occupied
  enrolledIDs[id - 1] = true;

  Serial.print("ID: "); 
  Serial.println(id);

  // Ask user to place the same finger again
  Serial.println("Place the same finger again");
  delay(1000); // Delay before next fingerprint capture

  // Capture image of the finger again
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communicationerror");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // Image captured successfully, continue with enrollment process
  p = finger.image2Tz(2);
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

  // Image converted successfully, create model for fingerprint
  Serial.println("Creating fingerprint model...");
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint model created");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // Store the fingerprint model
  Serial.println("Storing fingerprint model...");
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint stored successfully!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

