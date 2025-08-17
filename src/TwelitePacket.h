#pragma once

#include <Arduino.h>

#define PACKET_HEADER 0xA5
#define MAX_PAYLOAD_SIZE 32

namespace twelite {

enum MessageType {
	DeployComplete      = 0x01, // 展開完了信号
    ReadyForCapture     = 0x02, // 撮影準備完了（離陸許可）
    ReadyForCaptureAck  = 0x03, // 撮影準備返信
    TurnSignal          = 0x04, // 旋回信号	
};

enum DEVICE_ID {
	A_PARTS = 0x01,
	B_PARTS = 0x02,
	C_PARTS = 0x03,
	BROADCAST = 0xFF,	// ブロードキャスト(送信専用)
};

struct Packet {
	uint8_t header;						// ヘッダ(開始の識別用バイト)
	uint8_t sender;						// 送信元
	uint8_t receiver;					// 宛先
	uint8_t messageType;				// メッセージ種別
	uint8_t payloadLength;				// ペイロード長
	uint8_t payload[MAX_PAYLOAD_SIZE];	// ペイロード
	uint8_t checksum;					// チェックサム
};

class TwelitePacket {
public:
	TwelitePacket() : _serial(nullptr) {}
	void begin(HardwareSerial &serial, int baud);
	void sendPacket(Packet &packet);
	bool receivePacket(Packet &packet);

	static Packet makePacket(DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType, uint8_t payloadLength, uint8_t *payload);
	static bool match(const Packet& pkt, DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType);

private:
	HardwareSerial *_serial;
	uint8_t calculateChecksum(const Packet &pakcet);
};

} // namespace twelite