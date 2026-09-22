# Door Voice Device

Arduino Unoを使ったドア開閉音声デバイス。

ドアが開いたことを磁気センサーで検知し、
「いってらっしゃい」という音声を再生する。

現在は Phase 1〜3 まで実装済み。Arduino Uno とリードスイッチでドアの
`CLOSED` / `OPEN` を判定し、状態が変化したときだけシリアルモニターへ出力する。
音声再生とチャタリング対策は未実装。

## Hardware

- Arduino Uno
- リードスイッチ
- 磁石
- DFPlayer Mini
- microSD
- スピーカー

## Status

Phase 1〜3 実装済み

## 配線（Phase 1〜3）

- Arduino D2 とリードスイッチの一方を接続する
- リードスイッチのもう一方を Arduino GND に接続する
- `INPUT_PULLUP` を使用するため、外付けプルアップ抵抗は不要

磁石が近い（リードスイッチが閉じる）状態を `CLOSED` として扱う。
実機で判定が逆になる場合は、`src/main.cpp` の
`SENSOR_LOW_MEANS_CLOSED` を `false` に変更する。

## 動作確認

1. PlatformIO で Arduino Uno に書き込む。
2. シリアルモニターを 9600 baud で開く。
3. 磁石をリードスイッチへ近づけたり離したりする。

起動時には初期状態を出力する。以後は状態が変化したときだけ、次のように出力する。

```text
Door state changed: CLOSED -> OPEN
Door state changed: OPEN -> CLOSED
```
