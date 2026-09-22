#include <Arduino.h>

namespace {

constexpr uint8_t DOOR_SENSOR_PIN = 2;
constexpr unsigned long SERIAL_BAUD_RATE = 9600;

// INPUT_PULLUP を使い、リードスイッチは D2 と GND の間へ接続する。
// 磁石が近く、スイッチが閉じたときに LOW になる配置を想定する。
// 実機で OPEN / CLOSED が逆の場合は false に変更する。
constexpr bool SENSOR_LOW_MEANS_CLOSED = true;

enum class DoorState {
  Open,
  Closed,
};

DoorState previousState;

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

  if (currentState == previousState) {
    return;
  }

  logStateTransition(previousState, currentState);
  previousState = currentState;
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

  Serial.print(F("Initial door state: "));
  Serial.println(doorStateToString(previousState));
  Serial.println(F("Ready."));
}

void loop() {
  updateDoorState();
}
