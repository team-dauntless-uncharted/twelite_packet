/*
 * ReadyForCapture.ino - 撮影準備信号送信・応答受信
 * 
 * 撮影準備信号を送信し、相手からの応答（Ack）を待機します
 * 送信元: B_PARTS -> 宛先: A_PARTS
 * 10秒間隔で撮影準備信号を送信し続けます
 */

#include <TwelitePacket.h>

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
		twelite::B_PARTS,         // 送信元
		twelite::A_PARTS,         // 宛先
		twelite::ReadyForCapture, // 撮影準備信号
		0,                        // ペイロード長（なし）
		NULL                      // ペイロード（なし）
	);
	tweliteModule.sendPacket(pkt);

	// 応答パケットの受信を試行
	if (tweliteModule.receivePacket(pkt)) {
		// 撮影準備応答かチェック
		if (twelite::TwelitePacket::match(pkt, twelite::A_PARTS, twelite::B_PARTS, twelite::ReadyForCaptureAck)) {
			Serial.println("received ReadyForCaptureAck");
		}
	}
}