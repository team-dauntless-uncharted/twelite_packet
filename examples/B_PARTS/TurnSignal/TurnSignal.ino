/*
 * TurnSignal.ino - GPS座標送信サンプル
 * 
 * GPS座標データ（緯度・経度）を1秒間隔でTwelite経由で送信します
 * 送信元: B_PARTS -> 宛先: A_PARTS
 */

#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
	// Twelite通信を115200bpsで初期化
	tweliteModule.begin(Serial2, 115200);
}

void loop() {
	// 送信するGPS座標（例: 東京駅付近）
	float latitude = 35.681236;   // 緯度
	float longitude = 139.767125; // 経度
	
	// ペイロードサイズを計算（float型 × 2）
	uint8_t payloadLength = sizeof(float) * 2;
	uint8_t payload[payloadLength];
	
	// float値をバイト配列に変換
	memcpy(payload, &latitude, sizeof(float));
	memcpy(payload + sizeof(float), &longitude, sizeof(float));

	// パケット作成・送信
	pkt = twelite::TwelitePacket::makePacket(
		twelite::B_PARTS,     // 送信元
		twelite::A_PARTS,     // 宛先
		twelite::TurnSignal,  // メッセージタイプ
		payloadLength,        // ペイロード長
		payload               // GPS座標データ
	);
	tweliteModule.sendPacket(pkt);

	// 1秒待機
	delay(1000);
}