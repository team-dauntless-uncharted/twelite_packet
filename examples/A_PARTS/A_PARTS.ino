/*
 * ReadyForCaptureAck.ino - 撮影準備信号受信・応答送信
 * 
 * 撮影準備信号を受信したら、自動的に応答(Ack)を返します
 * 受信待ち: B_PARTS からの ReadyForCapture
 * 応答送信: A_PARTS -> B_PARTS の ReadyForCaptureAck
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
	// パケット受信を試行
	if (tweliteModule.receivePacket(pkt)) {
		// 撮影準備信号を受信したかチェック
		if (twelite::TwelitePacket::match(pkt, twelite::B_PARTS, twelite::A_PARTS, twelite::ReadyForCapture)) {
			Serial.println("received ReadyForCapture");
			
			// 撮影準備応答を自動送信
			pkt = twelite::TwelitePacket::makePacket(
				twelite::A_PARTS,            // 送信元
				twelite::B_PARTS,            // 宛先（元の送信者）
				twelite::ReadyForCaptureAck, // 撮影準備応答
				0,                           // ペイロード長（なし）
				NULL                         // ペイロード（なし）
			);
			tweliteModule.sendPacket(pkt);
		}
	}
}