# Door Voice Device

Arduino Unoを使ったドア開閉音声デバイス。

ドアが開いたことを磁気センサーで検知し、
「いってらっしゃい」という音声を再生する。

現在は Phase 1〜4 まで実装・実機動作確認済み。Arduino Uno とリードスイッチでドアの
`CLOSED` / `OPEN` を判定し、状態が変化したときだけシリアルモニターへ出力する。
50msのチャタリング対策を実装済み。音声再生は未実装。

## Hardware

- Arduino Uno
- リードスイッチ
- 磁石
- DFPlayer Mini
- microSD
- スピーカー

## Status

Phase 1〜4 実装・実機動作確認済み（ユーザーによる確認）。

Arduino Uno とリードスイッチを使用し、開閉判定とシリアルモニターへの状態遷移出力の動作確認が完了した。
Phase 4 では、通常の1回の開閉でログが各1回出ることを実機で確認した。
音声再生は今後実装する。

## 配線（Phase 1〜4）

- Arduino D2 とリードスイッチの一方を接続する
- リードスイッチのもう一方を Arduino GND に接続する
- `INPUT_PULLUP` を使用するため、外付けプルアップ抵抗は不要

現在の `src/main.cpp` は `SENSOR_LOW_MEANS_CLOSED = false` に設定しており、
HIGH を `CLOSED`、LOW を `OPEN` として扱う。
実機で判定が逆になる場合は、`SENSOR_LOW_MEANS_CLOSED` を `true` に変更する。

## 動作確認

1. PlatformIO で Arduino Uno に書き込む。
2. シリアルモニターを 9600 baud で開く。
3. 磁石をリードスイッチへ近づけたり離したりする。

起動時には初期状態を出力する。以後は状態が変化したときだけ、次のように出力する。

```text
Door state changed: CLOSED -> OPEN
Door state changed: OPEN -> CLOSED
```

## Phase 4：チャタリング対策

入力が変化するたびに安定時間の計測をやり直し、同じ状態が50ms続いた場合のみ
状態変更を確定する。`millis()` を使用するため、待ち時間もセンサーの確認を続ける。
調整する場合は `src/main.cpp` の `DEBOUNCE_TIME_MS` を変更する。

実機確認手順：

1. `pio run -e uno -t upload` で書き込む。
2. `pio device monitor -b 9600` でシリアルモニターを開く。
3. 1回の開閉につき基本的に `CLOSED -> OPEN` と `OPEN -> CLOSED` が各1回出力されることを確認する。
4. 開いた状態・閉じた状態を維持し、ログが増え続けないことを確認する。

50ms未満の開閉は確定しない。逆に、各状態が50ms以上続く場合は状態変化として扱う。
磁石を小刻みに動かした場合、各状態が50ms以上続けばログが複数出るため、
ログの数だけではチャタリング対策の成否を判断しない。

## 次のステップ

次は Phase 5（DFPlayer Mini 単体での音声再生テスト）、
Phase 6（ドアの開閉検知と音声再生の統合）の順に進める。
