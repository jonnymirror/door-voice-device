# Door Voice Device

Arduino Unoを使ったドア開閉音声デバイス。

ドアが開いたことを磁気センサーで検知し、
「いってらっしゃい」という音声を再生する。

Phase 1〜6 は実装・実機動作確認済み。
Arduino Uno とリードスイッチでドアの `CLOSED` / `OPEN` を判定し、
50ms安定した `CLOSED → OPEN` の変化があったとき、DFPlayer Miniから音声を1回再生する。

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
Phase 5 は2026年10月4日にユーザーが実機で初期化成功、音声の1回再生、再生後に繰り返さないことを確認した。
Phase 6 はドア検知と音声再生を統合し、ユーザーが実機で動作確認した。

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

## Phase 5：DFPlayer Mini 単体テスト

`src/dfplayer_test.cpp` を `dfplayer_test` 環境で書き込むと、起動時に音声を1回だけ再生する。
この環境ではドアセンサー処理は実行しない。`uno` 環境は統合版の `src/main.cpp` を使用する。

### 配線（Phase 5）

用意するもの：Arduino Uno、DFPlayer Mini、4〜8Ωの小型スピーカー、
1kΩ抵抗1本、ジャンパーワイヤ、ブレッドボード、音声入りmicroSD、USBケーブル。
ここではArduinoをUSBで給電し、Arduinoの5VピンからDFPlayerへ給電する。

まずUSBケーブルなどの電源をすべて外してから配線する。
DFPlayerの端子は基板の印字で確認する。表裏を反転すると左右も反転するため、
位置だけで判断せず、[公式ピン配置図](https://wiki.dfrobot.com/dfr0299#pinout) と照合する。
以下の番号は [公式データシートのピン番号](https://image.dfrobot.com/image/data/DFR0299/DFPlayer%20Mini%20Manul.pdf)（2ページ目）。

| 接続元 | 接続先（DFPlayer Mini） | 役割 |
| --- | --- | --- |
| ArduinoのPOWER端子にある5V | VCC（1番） | 電源供給 |
| ArduinoのGND（どのGNDでも可） | GND（7番） | 電源・通信の共通基準 |
| Arduinoのデジタル10番（D10） | TX（3番） | DFPlayerからArduinoへ応答を送る |
| Arduinoのデジタル11番（D11）→ 1kΩ抵抗 | RX（2番） | ArduinoからDFPlayerへ命令を送る |
| スピーカーの＋端子 | SPK1（8番） | スピーカー出力 |
| スピーカーの－端子 | SPK2（6番） | スピーカー出力 |

接続全体は次のとおり。

```text
Arduino Uno                         DFPlayer Mini
  5V  ----------------------------- VCC (1)
  GND ----------------------------- GND (7)
  D10 (受信) <--------------------- TX  (3)
  D11 (送信) ----[ 1kΩ抵抗 ]-------> RX  (2)

                                    SPK1 (8) ---- スピーカー ＋
                                    SPK2 (6) ---- スピーカー －
```

**1. 電源をつなぐ**

Arduinoの `5V` とDFPlayerの `VCC`、Arduinoの `GND` とDFPlayerの `GND` をつなぐ。
Arduinoの `VIN` や `3.3V` ではなく、今回は `5V` を使う。
DFPlayerにはGNDが2本（7番・10番）あるが、この配線では7番を使用する。

**2. 通信線をつなぐ**

DFPlayerの `TX` をArduinoの `D10` へ直接つなぐ。
Arduinoの `D11` は、1kΩ抵抗を1本通してDFPlayerの `RX` へつなぐ。
抵抗には向きがない。D11→抵抗の片脚→抵抗のもう片脚→RXという直列接続にする。
ブレッドボードでは、抵抗の両脚を電気的につながっていない別々の穴グループへ挿す。
一般的なブレッドボードでは同じ番号のa〜eがつながっているため、同じ側の異なる番号の行を使う。
両脚を同じ穴グループに挿すと抵抗が短絡される。

`TX` は送信、`RX` は受信。送信側と受信側を組み合わせるため、TX同士・RX同士ではつながない。
コードの `DFPLAYER_RX_PIN = 10` は「Arduino側の受信ピン」を意味する。
Unoに印字された `RX/0`・`TX/1` は今回は使用しない。
1kΩ抵抗の直列接続は [DFRobot公式の接続説明](https://wiki.dfrobot.com/dfr0299/docs/20906) に基づく。

**3. スピーカーをつなぐ**

アンプを内蔵しない2端子の小型スピーカーを、`SPK1` と `SPK2` の間へつなぐ。
スピーカーの－端子もArduinoのGNDへは接続しない。
`DAC_L`・`DAC_R` は今回のスピーカー接続には使用しない。
`BUSY`・`IO1`・`IO2`・`ADKEY1`・`ADKEY2`・`USB+`・`USB-` も未接続でよい。

**4. microSDを挿し、USBで給電する**

下記の構成で音声を保存したmicroSDを、電源OFFの状態でDFPlayerへ挿入する。
配線後にArduinoをUSBでPCへ接続し、後述のコマンドで書き込む。
Phase 5ではリードスイッチは不要。Phase 6ではD2–GNDの配線も使用する。

ブレッドボードの電源レールを使う場合は、5Vを＋レール、GNDを－レールへつなぐ。
途中で分断されている電源レールもあるため、実際の導通を確認する。

### 音声ファイル

microSDは32GB以下・FAT16またはFAT32を使用する。
配線・カード仕様は [DFRobot公式資料](https://wiki.dfrobot.com/dfr0299) を参照。

作成済みの `0001.wav`（プロジェクト直下）を、microSDの次の位置へコピーする。
音声形式は24kHz・16bit・モノラルの非圧縮PCM WAV、長さは約1.07秒。拡張子は変更しない。
音声：VOICEVOX:春日部つむぎ

```text
microSD/
└── mp3/
    └── 0001.wav
```

`playMp3Folder(1)` でこのファイルを指定する。ルート直下には置かない。
音量は `src/dfplayer_test.cpp` の `DEFAULT_VOLUME = 20`（範囲0〜30）で調整できる。

### ビルド・書き込み・確認

```powershell
pio run -e dfplayer_test
pio run -e dfplayer_test -t upload
pio device monitor -e dfplayer_test
```

モニターを開いてからArduinoのRESETボタンを押すと、起動ログと再生を確認できる。
リセットするたびに1回再生する。

```text
DFPlayer standalone test (Phase 5)
Initializing DFPlayer... (may take several seconds)
DFPlayer initialized.
Playback requested: /mp3/0001.wav
```

`Playback requested` は再生コマンドを送ったことを示す。実際に音声が聞こえることと、
終了後に自動で繰り返し再生しないことを確認する。終了通知を受信すると `Playback finished` が出る。
初期化に失敗した場合は再生せず、配線・電源・カードの確認を促すログを出す。
再生中の通信エラーやファイルエラーもログへ出力する。修正後はRESETで再試行する。

初期化タイムアウトの調査用に、起動後3秒待ってから初期化を開始する。
`dfplayer_test` 環境では `-D_DEBUG=0` により、ライブラリが `sending:`（送信）と
`received:`（受信）の生データを16進数で表示する。
DFPlayerライブラリはマクロの定義有無でログを有効にする。値を0にしてSoftwareSerial側の定義と合わせている。
`sending:` だけではDFPlayerが命令を受信したとは判断できない。
`received:` があっても、ノイズや不正な応答の場合があるため、内容とイベントログを併せて確認する。
初期化失敗後も遅れて届く通知を記録するが、成功扱いにして再生は行わない。

ドアセンサーのテストへ戻す場合は `pio run -e uno -t upload` で書き直す。
`pio run` の既定環境も引き続き `uno`。

### 実機確認時の接触不良

初期化時に `DFPlayer communication timed out.` が出て、受信ログがない状態が発生した。
DFPlayerのブレッドボードへの差し込みが浅く、挿し直した後に初期化と音声再生が成功した。
同じ症状では、USBを抜いてからピンの差し込みを確認する。
ブレッドボード上の電源用の穴で電圧を測れても、DFPlayerのピンまで接触しているとは限らない。

## Phase 6：ドア開閉と音声再生の統合

Phase 1〜4のリードスイッチ（D2–GND）と、Phase 5のDFPlayer・スピーカーの配線を同時に接続する。
両方のGNDはArduinoで共通にする。配線を変更する際はUSBを抜く。
microSDの `/mp3/0001.wav` をそのまま使う。

既定の `uno` 環境をArduinoへ書き込み、シリアルモニターを開く。

```powershell
pio run -e uno -t upload
pio device monitor -e uno
```

起動時はDFPlayerの初期化を待ってからドアの初期状態を読み取る。
起動時にドアが開いていても音声は再生しない。
DFPlayerの初期化に失敗した場合も開閉ログは出るが、音声再生は行わない。

1. ドアを閉じた状態でArduinoをRESETし、`Initial door state: CLOSED` と `DFPlayer initialized.` を確認する。
2. ドアを1回開く。`Door state changed: CLOSED -> OPEN` と `Playing voice: leaving` が1回ずつ出て、音声が1回流れることを確認する。
3. 開いたまま待ち、音声が繰り返されないことを確認する。
4. ドアを閉じ、`Door state changed: OPEN -> CLOSED` が出て、音声が流れないことを確認する。
5. 再度ドアを開き、音声が再び1回流れることを確認する。

状態が50ms以上安定した場合に開閉を確定する。通常の手動操作で短時間に何度も開閉すると、
それぞれが50ms以上続いた場合には複数の状態遷移として扱う。

Phase 6 の実機動作確認済み。上記の手順は再確認時にも使用する。

## 次のステップ

実際のドアに取り付ける位置と配線を固定し、開閉を繰り返して安定動作を確認する。
