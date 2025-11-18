/*
 * ReadyForCapture.ino - 撮影準備信号送信・応答受信
 * 
 * 撮影準備信号を送信し、相手からの応答（Ack）を待機します
 * 送信元: B_PARTS -> 宛先: A_PARTS
 * 10秒間隔で撮影準備信号を送信し続けます
 */

#include <src/TwelitePacket.h>

twelite::TwelitePacket tweliteModule(11);
twelite::Packet pkt;

void setup() {
	// シリアルモニター用の通信初期化
	Serial.begin(115200);
	
	// Twelite通信を115200bpsで初期化
	Serial2.begin(115200);
	tweliteModule.begin(Serial2);
	tweliteModule.on();
}

void loop() {
	// 撮影準備信号を送信（ペイロードなし）
	pkt = twelite::TwelitePacket::makePacket(
		twelite::C_PARTS,        // 送信元: Cパーツ
		twelite::BROADCAST,      // 宛先: 全デバイス
		twelite::DeployComplete, // 展開完了信号
		0,                       // ペイロード長（なし）
		NULL                     // ペイロード（なし）
	);
	tweliteModule.sendPacket(pkt);

	// 10秒待機後に再送信
	delay(10000);
}