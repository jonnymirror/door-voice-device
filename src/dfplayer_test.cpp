#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

namespace {

// Arduino側のRX/TX。D11からDFPlayer RXへは1kΩ抵抗を挟む。
constexpr uint8_t DFPLAYER_RX_PIN = 10;
constexpr uint8_t DFPLAYER_TX_PIN = 11;
constexpr unsigned long SERIAL_BAUD_RATE = 9600;
constexpr unsigned long AUDIO_STARTUP_WAIT_MS = 3000;
constexpr uint8_t DEFAULT_VOLUME = 20;  // 0〜30
constexpr uint16_t TEST_TRACK_NUMBER = 1;  // microSD: /mp3/0001.wav

SoftwareSerial audioSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFRobotDFPlayerMini player;
bool audioReady = false;

void playTestVoice() {
  Serial.println(F("Playback requested: /mp3/0001.wav"));
  player.playMp3Folder(TEST_TRACK_NUMBER);
}

void reportAudioEvent(uint8_t type, int value) {
  Serial.print(F("DFPlayer event: type="));
  Serial.print(type);
  Serial.print(F(", value="));
  Serial.println(value);
  switch (type) {
    case DFPlayerPlayFinished:
      Serial.print(F("Playback finished. Track: "));
      Serial.println(value);
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayer error code: "));
      Serial.println(value);
      if (value == FileIndexOut || value == FileMismatch) {
        Serial.println(F("Check /mp3/0001.wav on the SD card."));
      } else if (value == Busy) {
        Serial.println(F("DFPlayer reports: card not found."));
      }
      break;
    case TimeOut:
      Serial.println(F("DFPlayer communication timed out."));
      break;
    case WrongStack:
      Serial.println(F("Invalid DFPlayer response."));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("SD card removed. Insert it and reset Arduino."));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("SD card insertion detected."));
      break;
    case DFPlayerCardOnline:
    case DFPlayerCardUSBOnline:
      Serial.println(F("SD card online notification received."));
      if (!audioReady) {
        Serial.println(F("Notification arrived after initialization failed; no playback requested."));
      }
      break;
    case DFPlayerFeedBack:
      Serial.println(F("Command acknowledgement received."));
      break;
    default:
      break;
  }
}

}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  audioSerial.begin(SERIAL_BAUD_RATE);
  Serial.println(F("DFPlayer standalone test (Phase 5)"));
  Serial.println(F("Diagnostics enabled. Arduino RX=D10, TX=D11."));
  Serial.println(F("Waiting 3 seconds for DFPlayer power-up..."));
  // 電源投入直後のモジュールへ、すぐにリセット命令を送らない。
  delay(AUDIO_STARTUP_WAIT_MS);
  Serial.println(F("Initializing DFPlayer... (may take several seconds)"));

  audioReady = player.begin(audioSerial, true, true);
  if (!audioReady) {
    Serial.println(F("DFPlayer initialization failed."));
    // begin()が保持している結果を表示する。available()で上書きする前に読む。
    const uint8_t type = player.readType();
    const int value = player.read();
    reportAudioEvent(type, value);
    Serial.println(F("Check power, TX/RX wiring and SD card, then reset Arduino."));
    Serial.println(F("Listening for late DFPlayer responses..."));
    return;
  }

  Serial.println(F("DFPlayer initialized."));
  player.volume(DEFAULT_VOLUME);
  // 起動時の設定反映待ち。再生要求はsetup()から1回だけ送る。
  delay(200);
  playTestVoice();
}

void loop() {
  // 初期化失敗後も、遅れて届くカード認識通知やエラーを記録する。
  if (player.available()) {
    const uint8_t type = player.readType();
    const int value = player.read();
    reportAudioEvent(type, value);
  }
}
