#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t GREEN_LED = 2;
constexpr uint8_t RED_LED = 3;
constexpr uint8_t BUZZER = 4;
constexpr uint8_t BUTTON = 5;
constexpr uint8_t RFID_SS = 6;
constexpr uint8_t RFID_RST = 7;
constexpr uint8_t SPI_MOSI_PIN = 10;
constexpr uint8_t SPI_MISO_PIN = 9;
constexpr uint8_t SPI_SCK_PIN = 8;
constexpr uint8_t RELAY = 21;

constexpr unsigned long UNLOCK_TIME_MS = 5000;
constexpr unsigned long AUTHORIZED_BEEP_MS = 150;
constexpr unsigned long UNAUTHORIZED_BEEP_MS = 500;
constexpr unsigned int AUTHORIZED_TONE_HZ = 2000;
constexpr unsigned int UNAUTHORIZED_TONE_HZ = 400;

// Replace this example UID with the UID of the card that should unlock the
// relay. The UID length must match the card's UID length.
constexpr byte AUTHORIZED_UID[] = {0xDE, 0xAD, 0xBE, 0xEF};
constexpr byte AUTHORIZED_UID_LENGTH = sizeof(AUTHORIZED_UID);

MFRC522 rfid(RFID_SS, RFID_RST);
bool relayActive = false;
unsigned long relayStartedAt = 0;
unsigned long feedbackStartedAt = 0;
unsigned long feedbackDuration = 0;
bool feedbackActive = false;
bool cardPresent = false;

bool isAuthorized(const MFRC522::Uid &uid) {
  if (uid.size != AUTHORIZED_UID_LENGTH) {
    return false;
  }

  for (byte index = 0; index < uid.size; ++index) {
    if (uid.uidByte[index] != AUTHORIZED_UID[index]) {
      return false;
    }
  }
  return true;
}

void startFeedback(bool authorized) {
  digitalWrite(GREEN_LED, authorized ? HIGH : LOW);
  digitalWrite(RED_LED, authorized ? LOW : HIGH);
  tone(BUZZER, authorized ? AUTHORIZED_TONE_HZ : UNAUTHORIZED_TONE_HZ);
  feedbackStartedAt = millis();
  feedbackDuration = authorized ? AUTHORIZED_BEEP_MS : UNAUTHORIZED_BEEP_MS;
  feedbackActive = true;
}

void stopFeedback() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);
  feedbackActive = false;
}

void unlockRelay() {
  digitalWrite(RELAY, HIGH);
  relayActive = true;
  relayStartedAt = millis();
  startFeedback(true);
}

void rejectCard() {
  startFeedback(false);
}

void stopRelayWhenDue() {
  if (relayActive && millis() - relayStartedAt >= UNLOCK_TIME_MS) {
    digitalWrite(RELAY, LOW);
    relayActive = false;
  }
}

void stopFeedbackWhenDue() {
  if (feedbackActive && millis() - feedbackStartedAt >= feedbackDuration) {
    stopFeedback();
  }
}

void readCard() {
  if (!rfid.PICC_IsNewCardPresent()) {
    cardPresent = false;
    return;
  }

  if (cardPresent || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  cardPresent = true;
  if (isAuthorized(rfid.uid)) {
    unlockRelay();
  } else {
    rejectCard();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(RELAY, OUTPUT);

  digitalWrite(RELAY, LOW);
  stopFeedback();

  SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, RFID_SS);
  rfid.PCD_Init();
  delay(4);
}

void loop() {
  readCard();
  stopRelayWhenDue();
  stopFeedbackWhenDue();
}