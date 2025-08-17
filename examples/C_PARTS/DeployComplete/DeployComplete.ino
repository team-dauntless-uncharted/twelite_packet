/*
 * DeployComplete.ino - 展開完了ブロードキャスト
 * 
 * 展開完了信号を全デバイスにブロードキャスト送信します
 * 送信元: C_PARTS -> 宛先: BROADCAST（全デバイス）
 * 10秒間隔で展開完了信号を送信し続けます
 */

#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
	// Twelite通信を115200bpsで初期化
	// シリアルモニターは不要（送信専用）
	tweliteModule.begin(Serial2, 115200);
}

void loop() {
	// 展開完了信号をブロードキャスト送信
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