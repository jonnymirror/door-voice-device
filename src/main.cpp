#include <Arduino.h>

namespace {

constexpr uint8_t DOOR_SENSOR_PIN = 2;
constexpr unsigned long SERIAL_BAUD_RATE = 9600;
constexpr unsigned long DEBOUNCE_TIME_MS = 50;

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

void setupDoorSensor();
DoorState readDoorState();
void updateDoorState();
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

  logStateTransition(previousState, lastReadState);
  previousState = lastReadState;
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
  previousState = readDoorState();
  lastReadState = previousState;
  lastReadChangedAt = millis();

  Serial.print(F("Initial door state: "));
  Serial.println(doorStateToString(previousState));
  Serial.println(F("Ready."));
}

void loop() {
  updateDoorState();
}
