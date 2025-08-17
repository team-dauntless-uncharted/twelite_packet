# ARLISS 2025 Team TDU TwelitePacket Library

Arduino用Twelite無線モジュール通信ライブラリ(共通データ構造)

## インストール方法

1. `TwelitePacket.h`と`TwelitePacket.cpp`をArduinoプロジェクトフォルダにコピー
2. スケッチで`#include "TwelitePacket.h"`を追加

```cpp
#include "TwelitePacket.h"
```

## 基本的な使い方

### 1. ライブラリの初期化

```cpp
#include "TwelitePacket.h"

using namespace twelite;

TwelitePacket twelite;

void setup() {
    // シリアル通信を38400bpsで初期化
    twelite.begin(Serial1, 38400);
}
```

### 2. パケット送信

```cpp
void sendDeployComplete() {
    // ペイロードなしの簡単な信号送信
    Packet packet = TwelitePacket::makePacket(
        A_PARTS,           // 送信元: Aパーツ
        B_PARTS,           // 宛先: Bパーツ  
        DeployComplete,    // メッセージタイプ: 展開完了
        0,                 // ペイロード長: なし
        nullptr            // ペイロード: なし
    );
    
    twelite.sendPacket(packet);
}
```

### 3. パケット受信

```cpp
void loop() {
    Packet receivedPacket;
    
    if (twelite.receivePacket(receivedPacket)) {
        // パケット受信成功
        
        // 特定のメッセージを待つ場合
        if (TwelitePacket::match(receivedPacket, B_PARTS, A_PARTS, ReadyForCapture)) {
            Serial.println("撮影準備完了信号を受信!");
            // 応答処理...
        }
        
        // メッセージタイプで分岐処理
        switch(receivedPacket.messageType) {
            case DeployComplete:
                handleDeployComplete(receivedPacket);
                break;
            case ReadyForCapture:
                handleReadyForCapture(receivedPacket);
                break;
            // ...
        }
    }
}
```

### 4. ペイロード付きパケット送信

```cpp
void sendSensorData() {
    // センサーデータをペイロードとして送信
    uint8_t sensorData[4];
    sensorData[0] = temperature;  // 温度
    sensorData[1] = humidity;     // 湿度
    sensorData[2] = pressure >> 8;    // 気圧 上位
    sensorData[3] = pressure & 0xFF;  // 気圧 下位
    
    Packet packet = TwelitePacket::makePacket(
        A_PARTS,        // 送信元
        B_PARTS,        // 宛先
        TurnSignal,     // カスタムメッセージタイプ
        4,              // ペイロード長
        sensorData      // ペイロードデータ
    );
    
    twelite.sendPacket(packet);
}
```

### 5. ブロードキャスト送信

```cpp
void broadcastStatus() {
    uint8_t statusData = SYSTEM_READY;
    
    Packet packet = TwelitePacket::makePacket(
        A_PARTS,        // 送信元
        BROADCAST,      // 全デバイスに送信
        DeployComplete, // ステータス通知
        1,              // ペイロード長
        &statusData     // ステータスデータ
    );
    
    twelite.sendPacket(packet);
}
```

## APIリファレンス

### TwelitePacketクラス

#### `void begin(HardwareSerial &serial, int baud)`
シリアル通信を初期化します。

- `serial`: 使用するシリアルポート（Serial, Serial1等）
- `baud`: 通信速度（推奨: 38400）

#### `void sendPacket(Packet &packet)`
パケットを送信します。ヘッダとチェックサムは自動設定されます。

#### `bool receivePacket(Packet &packet)`
パケットを受信します。

- **戻り値**: 成功時`true`、失敗時`false`
- 失敗理由: データ不足、不正ヘッダ、チェックサム不一致等

#### `static Packet makePacket(...)`
パケットを作成するヘルパー関数です。

```cpp
static Packet makePacket(
    DEVICE_ID sender,      // 送信元ID
    DEVICE_ID receiver,    // 宛先ID
    MessageType msgType,   // メッセージタイプ
    uint8_t payloadLen,    // ペイロード長(0-32)
    uint8_t *payload       // ペイロードデータ
);
```

#### `static bool match(...)`
受信パケットが条件に一致するかを判定します。

```cpp
static bool match(
    const Packet& pkt,     // 判定するパケット
    DEVICE_ID sender,      // 期待する送信元
    DEVICE_ID receiver,    // 期待する宛先
    MessageType msgType    // 期待するメッセージタイプ
);
```

### 定義済み列挙型

#### MessageType
```cpp
enum MessageType {
    DeployComplete      = 0x01, // 展開完了信号
    ReadyForCapture     = 0x02, // 撮影準備完了（離陸許可）
    ReadyForCaptureAck  = 0x03, // 撮影準備返信
    TurnSignal          = 0x04, // 旋回信号
};
```

#### DEVICE_ID
```cpp
enum DEVICE_ID {
    A_PARTS = 0x01,     // Aパーツ
    B_PARTS = 0x02,     // Bパーツ
    C_PARTS = 0x03,     // Cパーツ
    BROADCAST = 0xFF,   // ブロードキャスト
};
```

### Packet構造体

```cpp
struct Packet {
    uint8_t header;                     // ヘッダ(0xA5)
    uint8_t sender;                     // 送信元ID
    uint8_t receiver;                   // 宛先ID
    uint8_t messageType;                // メッセージタイプ
    uint8_t payloadLength;              // ペイロード長
    uint8_t payload[MAX_PAYLOAD_SIZE];  // ペイロード(最大32バイト)
    uint8_t checksum;                   // チェックサム
};
```

## パケット構造

```
[Header][Sender][Receiver][MsgType][PayloadLen][Payload...][Checksum][CR][LF]
  0xA5    1byte    1byte     1byte     1byte    0-32bytes   1byte   1byte 1byte
```

## 完全なサンプルコード

### 送信側デバイス (B_PARTS) - GPS座標送信

```cpp
#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
    // Twelite通信を115200bpsで初期化
    tweliteModule.begin(Serial2, 115200);
}

void loop() {
    // GPS座標データ（例: 東京駅付近）
    float latitude = 35.681236;   // 緯度
    float longitude = 139.767125; // 経度
    
    // float型2つ分のペイロードサイズを計算
    uint8_t payloadLength = sizeof(float) * 2;
    uint8_t payload[payloadLength];
    
    // float値をバイト配列にコピー
    memcpy(payload, &latitude, sizeof(float));
    memcpy(payload + sizeof(float), &longitude, sizeof(float));
    
    // パケット作成
    pkt = twelite::TwelitePacket::makePacket(
        twelite::B_PARTS,     // 送信元: Bパーツ
        twelite::A_PARTS,     // 宛先: Aパーツ
        twelite::TurnSignal,  // メッセージタイプ: 旋回信号
        payloadLength,        // ペイロード長
        payload               // GPS座標データ
    );
    
    // パケット送信
    tweliteModule.sendPacket(pkt);
    
    // 1秒間隔で送信
    delay(1000);
}
```

### 受信側デバイス (A_PARTS) - GPS座標受信

```cpp
#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
    // デバッグ用シリアル通信初期化
    Serial.begin(115200);
    
    // Twelite通信を115200bpsで初期化
    tweliteModule.begin(Serial2, 115200);
}

void loop() {
    // パケット受信を試行
    if (tweliteModule.receivePacket(pkt)) {
        Serial.println("received packet");
        
        // 特定の送信元・宛先・メッセージタイプをチェック
        if (twelite::TwelitePacket::match(pkt, twelite::B_PARTS, twelite::A_PARTS, twelite::TurnSignal)) {
            // ペイロードからfloat値を復元
            float latitude, longitude;
            memcpy(&latitude, pkt.payload, sizeof(float));
            memcpy(&longitude, pkt.payload + sizeof(float), sizeof(float));
            
            // 座標情報を表示
            Serial.printf("latitude: %f, longitude: %f\n\n", latitude, longitude);
        }
    } else {
        Serial.println("no packet");
    }
    
    // 1秒間隔でチェック
    delay(1000);
}
```

## トラブルシューティング

### 受信が失敗する場合

1. **Twelite設定確認**: **最も重要** - 以下の設定が全デバイスで一致している必要があります
   - **チャンネル(Channel)**: 通信に使用する周波数チャンネル（11-26）
   - **アプリケーションID(Application ID)**: 同じネットワークのデバイスを識別するID（0x00000000-0xFFFFFFFF）
   - **Device ID**: 各デバイスの個別識別子（0x01-0x64, 0x78-0x7F）
   
   ```
   設定例:
   - Channel: 18
   - Application ID: 0x67726305
   - Device ID: 送信側=0x01, 受信側=0x02
   ```

2. **ボーレート確認**: 送受信デバイスで同じボーレートを使用
3. **配線確認**: TweliteとArduinoの接続を確認
4. **シリアルポート確認**: 正しいSerialポートを指定
5. **電波環境**: 障害物や距離による通信障害をチェック

### チェックサムエラーの場合

1. **電波ノイズ**: 通信環境を改善
2. **電源ノイズ**: 適切な電源フィルタリング
3. **タイミング**: 送信間隔を調整