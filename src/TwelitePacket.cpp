#include "TwelitePacket.h"

namespace twelite {

/**
 * @brief シリアル通信の初期化
 * 
 * 指定されたシリアルポートを指定のボーレートで初期化します。
 * 
 * @param serial 使用するHardwareSerialオブジェクト
 * @param baud 通信速度（例：9600, 38400, 115200）
 */
void TwelitePacket::begin(HardwareSerial &serial, int baud) {
	_serial = &serial;
	_serial->begin(baud);
}

/**
 * @brief パケットのチェックサム計算
 * 
 * パケット内容（ヘッダ・チェックサムを除く）のXORチェックサムを計算します。
 * 送信時のチェックサム生成および受信時の検証に使用されます。
 * 
 * @param packet チェックサムを計算するパケット
 * @return uint8_t 計算されたチェックサム値
 */
uint8_t TwelitePacket::calculateChecksum(const Packet &packet) {
	// 送信元、宛先、メッセージタイプ、ペイロード長のXOR
	uint8_t sum = packet.sender ^ packet.receiver ^ packet.messageType ^ packet.payloadLength;
	
	// ペイロードの各バイトもXOR演算に含める
	for (int i = 0; i < packet.payloadLength; i++) {
		sum ^= packet.payload[i];
	}
	
	return sum;
}

/**
 * @brief パケット送信
 * 
 * パケット構造体の内容をシリアル経由でTweliteに送信します。
 * ヘッダとチェックサムは自動的に設定されます。
 * パケット終端にCR+LFを付加します。
 * 
 * @param packet 送信するパケット
 */
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

/**
 * @brief パケット受信
 * 
 * シリアルバッファからパケットデータを読み取り、検証を行います。
 * ヘッダチェック、ペイロード長チェック、チェックサム検証を実施します。
 * 
 * @param packet 受信データを格納するパケット構造体
 * @return bool 正常受信時true、エラー時false
 */
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

/**
 * @brief パケット作成のヘルパー関数
 * 
 * 指定されたパラメータからパケット構造体を作成します。
 * ヘッダとチェックサムは送信時に自動設定されます。
 * 
 * @param sender 送信元デバイスID
 * @param receiver 宛先デバイスID（BROADCAST可）
 * @param messageType メッセージタイプ
 * @param payloadLength ペイロード長（0-32）
 * @param payload ペイロードデータへのポインタ（nullptrの場合はペイロードなし）
 * @return Packet 作成されたパケット構造体
 */
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

/**
 * @brief パケット条件マッチング判定
 * 
 * 受信したパケットが指定した条件（送信元、宛先、メッセージタイプ）と
 * 一致するかを判定します。特定のパケットの受信待ちに使用します。
 * 
 * @param pkt 判定対象のパケット
 * @param sender 期待する送信元ID
 * @param receiver 期待する宛先ID
 * @param messageType 期待するメッセージタイプ
 * @return bool 全ての条件が一致した場合true
 */
bool TwelitePacket::match(const Packet& pkt, DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType) {
	return (pkt.sender == sender && pkt.receiver == receiver && pkt.messageType == messageType);
}

} // namespace twelite