/**
 * @file TwelitePacket.cpp
 */

#include "TwelitePacket.h"

namespace twelite {

void TwelitePacket::off() {
	digitalWrite(_resetPin, LOW);
	delay(100);
}

void TwelitePacket::on() {
	digitalWrite(_resetPin, HIGH);
	delay(100);
}

void TwelitePacket::begin(HardwareSerial &serial) {
	pinMode(_resetPin, OUTPUT);
	delay(100);
	_serial = &serial;
}

uint8_t TwelitePacket::calculateChecksum(const Packet &packet) {
	// 送信元、宛先、メッセージタイプ、ペイロード長のXOR
	uint8_t sum = packet.sender ^ packet.receiver ^ packet.messageType ^ packet.payloadLength;
	
	// ペイロードの各バイトもXOR演算に含める
	for (int i = 0; i < packet.payloadLength; i++) {
		sum ^= packet.payload[i];
	}
	
	return sum;
}

void TwelitePacket::sendPacket(Packet &packet) {
	// ヘッダを設定
	packet.header = PACKET_HEADER;
	
	// チェックサムを計算・設定
	packet.checksum = calculateChecksum(packet);

	// パケットデータをシリアル送信
	_serial->write(packet.header);         // ヘッダ
	_serial->write(packet.sender);         // 送信元ID
	_serial->write(packet.receiver);       // 宛先ID
	_serial->write(packet.messageType);    // メッセージタイプ
	_serial->write(packet.payloadLength);  // ペイロード長
	
	// ペイロードデータを送信
	for (int i = 0; i < packet.payloadLength; i++) {
		_serial->write(packet.payload[i]);
	}
	
	_serial->write(packet.checksum);       // チェックサム
	_serial->write('\r');                  // キャリッジリターン
	_serial->write('\n');                  // ラインフィード
}

bool TwelitePacket::receivePacket(Packet &packet) {
	// 最低限必要なバイト数（ヘッダ+送信元+宛先+メッセージ種別+ペイロード長+チェックサム）をチェック
	if (_serial->available() < 6) {
		return false;  // データ不足
	}
	
	// ヘッダバイトの確認
	if (_serial->read() != PACKET_HEADER) {
		return false;  // 不正なヘッダ
	}

	// パケット情報を読み取り
	packet.header = PACKET_HEADER;
	packet.sender = _serial->read();       // 送信元ID
	packet.receiver = _serial->read();     // 宛先ID  
	packet.messageType = _serial->read();  // メッセージタイプ
	packet.payloadLength = _serial->read(); // ペイロード長

	// ペイロード長の妥当性チェック
	if (packet.payloadLength > MAX_PAYLOAD_SIZE) {
		return false;  // ペイロードサイズが上限を超過
	}

	// ペイロードデータを読み取り
	for (int i = 0; i < packet.payloadLength; i++) {
		if (!_serial->available()) {
			return false;  // ペイロードデータ不足
		}
		packet.payload[i] = _serial->read();
	}

	// チェックサム読み取り
	packet.checksum = _serial->read();

	// 終端文字（CR, LF）の処理（あれば読み捨て）
	if (_serial->peek() == '\r') _serial->read();
	if (_serial->peek() == '\n') _serial->read();

	// チェックサム検証
	if (packet.checksum != calculateChecksum(packet)) {
		return false;  // チェックサム不一致（データ破損の可能性）
	}

	return true;  // 正常受信完了
}

Packet TwelitePacket::makePacket(DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType, uint8_t payloadLength, uint8_t *payload) {
	Packet pkt;
	pkt.sender = sender;
	pkt.receiver = receiver;
	pkt.messageType = messageType;
	pkt.payloadLength = payloadLength;
	
	// ペイロードがある場合はコピー
	if (payload && payloadLength > 0) {
		memcpy(pkt.payload, payload, payloadLength);
	}
	
	return pkt;
}

bool TwelitePacket::match(const Packet& pkt, DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType) {
	return (pkt.sender == sender && pkt.receiver == receiver && pkt.messageType == messageType);
}

} // namespace twelite