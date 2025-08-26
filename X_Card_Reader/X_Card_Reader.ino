// #include <Wire.h>
// #include <NfcAdapter.h>
// #include <PN532.h>
// #include <PN532_I2C.h>

// PN532_I2C* My_PN532;
// NfcAdapter* NFC_Adapter;

// void setup() {
//   Wire.setClock(100000);
//   Wire.begin(21, 22);
//   My_PN532 = new PN532_I2C(Wire);
//   NFC_Adapter = new NfcAdapter(*My_PN532);
//   Serial.begin(115200);
//   Serial.print("Program pokrenut\n");
//   NFC_Adapter->begin();
// }

// void loop() {
//   if (NFC_Adapter->tagPresent()) {
//     Serial.print("\nPronadjen NFC tag:");
//     NfcTag card = NFC_Adapter->read();
//     Serial.print("Duljina jedinstvenog ID-a: ");
//     Serial.print(card.getUidLength());
//     Serial.print(" bytova\n");
//     Serial.print("\nJedinstveni ID: ");
//     Serial.print(card.getUidString());
//     Serial.println("\n");
//   }
//   delay(500);
// }

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

const char* ssid = "ssid";
const char* password = "pass";
const char* serverBaseUrl = "http://ipv4:5000";

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_PN532 nfc(-1, -1, &Wire);

//NFC
uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t challengeBlock = 8;
uint8_t responseBlock = 9;

void connectWiFi() {
  WiFi.begin(ssid,password);
  Serial.print("Connection to WiFi");
  while(WiFi.status() != WL_CONNECTED){
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected!");
}

String getUIDString(uint8_t* uid, uint8_t length) {
  String uidStr = "";
  for (int i = 0; i<length; i++){
    if(uid[i] < 0x10) uidStr += "0";
    uidStr += String(uid[i], HEX);
  }
  return uidStr;
}

// bool writeChallengeToCard(uint8_t* uid, uint8_t uidLength, uint32_t challenge){
//   if (!nfc.mifareclassic_AuthenticateBlock(uid, uidLength, challengeBlock, 0, keyA))
//     return false;
  
//   uint8_t buffer[16] = {0};
//   buffer[0] = challenge & 0xFF;
//   buffer[1] = (challenge >> 8) & 0xFF;
//   buffer[2] = (challenge >> 16) & 0xFF;
//   buffer[3] = (challenge >> 24) & 0xFF;

//   return nfc.mifareclassic_WriteDataBlock(challengeBlock, buffer);

// }

// uint32_t readResponseFromCard(uint8_t* uid, uint8_t uidLength) {
//   if (!nfc.mifareclassic_AuthenticateBlock(uid, uidLength, responseBlock, 0, keyA)) {
//     Serial.println("Failed to authenticate response block");
//     return 0;
//   }
//   uint8_t buffer[16];
//   if (!nfc.mifareclassic_ReadDataBlock(responseBlock, buffer)) {
//     Serial.println("Failed to read response");
//     return 0;
//   }
//   return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
// }

void setup() {
  Serial.begin(115200);
  connectWiFi();
  
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN532");
    while (1);
  }
  nfc.SAMConfig();
  Serial.println("Waiting for a card...");
}

void loop() {
  uint8_t uid[7];
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    String uidStr = getUIDString(uid, uidLength);
    Serial.print("Card detected UID: "); Serial.println(uidStr);

    // Step 1: GET challenge from server
    HTTPClient http;
    String url = String(serverBaseUrl) + "/get_challenge?uid=" + uidStr;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != 200) {
      Serial.println("Failed to get challenge");
      http.end();
      return;
    }

    String challengeStr = http.getString();
    http.end();
    // uint32_t challenge = strtoul(challengeStr.c_str(), NULL, 16);
    // Serial.print("Received challenge: 0x"); Serial.println(challenge, HEX);
    Serial.print("Received challenge: "); Serial.println(challengeStr);

    // Step 2: User challenge signing
    Serial.println("Give this challenge to phone signer app:");
    Serial.print("UID="); Serial.print(uidStr);
    Serial.print("  Challenge="); Serial.println(challengeStr);

    delay(5000); // simulate external response processor

    // Step 3: Read response
    // uint32_t response = readResponseFromCard(uid, uidLength);
    // Serial.print("Read response: 0x"); Serial.println(response, HEX);

    // // Step 4: POST to server for verification
    // http.begin(String(serverBaseUrl) + "/verify");
    // http.addHeader("Content-Type", "application/json");
    // String jsonBody = "{\"uid\":\"" + uidStr + "\",\"challenge\":\"" + challengeStr + "\",\"response\":\"" + String(response, HEX) + "\"}";
    // httpCode = http.POST(jsonBody);

    // String serverReply = http.getString();
    // Serial.print("Server reply: "); Serial.println(serverReply);

    // http.end();
    // delay(5000);
  }
}