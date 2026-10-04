#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

namespace {

constexpr uint8_t DOOR_SENSOR_PIN = 2;
constexpr uint8_t DFPLAYER_RX_PIN = 10;  // Arduino側の受信ピン
constexpr uint8_t DFPLAYER_TX_PIN = 11;  // Arduino側の送信ピン
constexpr unsigned long SERIAL_BAUD_RATE = 9600;
constexpr unsigned long DEBOUNCE_TIME_MS = 50;
constexpr unsigned long AUDIO_STARTUP_WAIT_MS = 3000;
constexpr uint8_t DEFAULT_VOLUME = 20;  // 0〜30
constexpr uint16_t LEAVING_TRACK_NUMBER = 1;  // microSD: /mp3/0001.wav

// INPUT_PULLUP を使い、リードスイッチは D2 と GND の間へ接続する。
// 現在の実機では HIGH を CLOSED、LOW を OPEN として扱う。
// 実機で OPEN / CLOSED が逆の場合は true に変更する。
constexpr bool SENSOR_LOW_MEANS_CLOSED = false;

enum class DoorState {
  Open,
  Closed,
};

DoorState previousState;
DoorState lastReadState;
unsigned long lastReadChangedAt;
SoftwareSerial audioSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFRobotDFPlayerMini player;
bool audioReady = false;

void setupDoorSensor();
DoorState readDoorState();
void updateDoorState();
void setupAudio();
void playLeavingVoice();
void reportAudioEvent();
const char* doorStateToString(DoorState state);
void logStateTransition(DoorState from, DoorState to);

void setupDoorSensor() {
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
}

DoorState readDoorState() {
  const bool sensorIsLow = digitalRead(DOOR_SENSOR_PIN) == LOW;
  const bool isClosed = SENSOR_LOW_MEANS_CLOSED ? sensorIsLow : !sensorIsLow;

  return isClosed ? DoorState::Closed : DoorState::Open;
}

void updateDoorState() {
  const DoorState currentState = readDoorState();
  const unsigned long now = millis();

  // 入力が変わるたびに、安定時間の計測をやり直す。
  if (currentState != lastReadState) {
    lastReadState = currentState;
    lastReadChangedAt = now;
  }

  // unsigned long の差分で、millis() の桁あふれにも対応する。
  if (lastReadState == previousState ||
      now - lastReadChangedAt < DEBOUNCE_TIME_MS) {
    return;
  }

  const DoorState oldState = previousState;
  previousState = lastReadState;
  logStateTransition(oldState, previousState);

  if (oldState == DoorState::Closed && previousState == DoorState::Open) {
    playLeavingVoice();
  }
}

void setupAudio() {
  audioSerial.begin(SERIAL_BAUD_RATE);
  Serial.println(F("Initializing DFPlayer..."));
  delay(AUDIO_STARTUP_WAIT_MS);

  audioReady = player.begin(audioSerial, true, true);
  if (!audioReady) {
    Serial.println(F("DFPlayer initialization failed. Door logs remain available."));
    return;
  }

  player.volume(DEFAULT_VOLUME);
  delay(200);
  Serial.println(F("DFPlayer initialized."));
}

void playLeavingVoice() {
  if (!audioReady) {
    Serial.println(F("Voice skipped: DFPlayer is not ready."));
    return;
  }

  Serial.println(F("Playing voice: leaving"));
  player.playMp3Folder(LEAVING_TRACK_NUMBER);
}

void reportAudioEvent() {
  if (!audioReady || !player.available()) {
    return;
  }

  const uint8_t type = player.readType();
  const int value = player.read();
  if (type == DFPlayerError) {
    Serial.print(F("DFPlayer error code: "));
    Serial.println(value);
  } else if (type == DFPlayerPlayFinished) {
    Serial.println(F("Voice playback finished."));
  }
}

const char* doorStateToString(DoorState state) {
  return state == DoorState::Closed ? "CLOSED" : "OPEN";
}

void logStateTransition(DoorState from, DoorState to) {
  Serial.print(F("Door state changed: "));
  Serial.print(doorStateToString(from));
  Serial.print(F(" -> "));
  Serial.println(doorStateToString(to));
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.println(F("Door Voice Device"));
  Serial.println(F("Initializing..."));

  setupDoorSensor();
  setupAudio();
  previousState = readDoorState();
  lastReadState = previousState;
  lastReadChangedAt = millis();

  Serial.print(F("Initial door state: "));
  Serial.println(doorStateToString(previousState));
  Serial.println(F("Ready."));
}

void loop() {
  updateDoorState();
  reportAudioEvent();
}
