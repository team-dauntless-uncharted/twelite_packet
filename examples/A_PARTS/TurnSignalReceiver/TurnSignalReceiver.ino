/*
 * TurnSignalReceiver.ino - GPS座標受信サンプル
 * 
 * Twelite経由でGPS座標データを受信し、シリアルモニターに表示します
 * 送信元: B_PARTS からの受信を待機
 */

#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
	// シリアルモニター用の通信初期化
	Serial.begin(115200);
	
	// Twelite通信を115200bpsで初期化
	tweliteModule.begin(Serial2, 115200);
}

void loop() {
	// パケット受信を試行
	if (tweliteModule.receivePacket(pkt)) {
		Serial.println("received packet");
		
		// 特定の送信者・メッセージタイプかチェック
		if (twelite::TwelitePacket::match(pkt, twelite::B_PARTS, twelite::A_PARTS, twelite::TurnSignal)) {
			// ペイロードからGPS座標を復元
			float latitude, longitude;
			memcpy(&latitude, pkt.payload, sizeof(float));
			memcpy(&longitude, pkt.payload + sizeof(float), sizeof(float));
			
			// 座標をシリアルモニターに表示
			Serial.printf("latitude: %f, longitude: %f\n\n", latitude, longitude);
		}
	} else {
		Serial.println("no packet");
	}

	// 1秒待機
	delay(1000);
}