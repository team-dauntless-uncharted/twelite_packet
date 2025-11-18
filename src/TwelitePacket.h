/**
 * @file TwelitePacket.h
 */

#pragma once

#include <Arduino.h>

// パケットヘッダの識別バイト
#define PACKET_HEADER 0xA5
// ペイロードの最大サイズ（バイト）
#define MAX_PAYLOAD_SIZE 32

namespace twelite {

/**
 * @brief メッセージタイプの定義
 * 
 * 各デバイス間で送受信するメッセージの種類を定義します。
 */
enum MessageType {
	DeployComplete      = 0x01, // 展開完了信号
    ReadyForCapture     = 0x02, // 撮影準備完了（離陸許可）
    ReadyForCaptureAck  = 0x03, // 撮影準備返信
    TurnSignal          = 0x04, // 旋回信号	
	MovementDirection 	= 0x05, // 進行方向（ヘディング・速度 等）
};

/**
 * @brief デバイスIDの定義
 * 
 * 通信に参加する各デバイスの識別子を定義します。
 */
enum DEVICE_ID {
	A_PARTS = 0x01,     // Aパーツ
	B_PARTS = 0x02,     // Bパーツ
	C_PARTS = 0x03,     // Cパーツ
	BROADCAST = 0xFF,   // ブロードキャスト(全デバイスに送信)
};

/**
 * @brief 通信パケットの構造体
 * 
 * Twelite間で送受信するデータパケットの構造を定義します。
 * パケット構造: [ヘッダ][送信元][宛先][メッセージ種別][ペイロード長][ペイロード][チェックサム][CR][LF]
 */
struct Packet {
	uint8_t header;						// ヘッダ(0xA5固定 - パケット開始の識別用)
	uint8_t sender;						// 送信元デバイスID
	uint8_t receiver;					// 宛先デバイスID
	uint8_t messageType;				// メッセージ種別（MessageType enum参照）
	uint8_t payloadLength;				// ペイロード長（0-32バイト）
	uint8_t payload[MAX_PAYLOAD_SIZE];	// ペイロードデータ
	uint8_t checksum;					// チェックサム（データ整合性確認用）
};

/**
 * @brief Tweliteパケット通信クラス
 * 
 * Twelite無線モジュール間でパケット通信を行うためのクラスです。
 * シリアル通信を使用してパケットの送受信、チェックサム計算などを行います。
 */
class TwelitePacket {
public:
	/**
	 * @brief コンストラクタ
	 */
	TwelitePacket(int resetPin) : _serial(nullptr), _resetPin(resetPin) {}

	/**
	 * @brief Tweliteをoffにする
	 */
	void off();

	/**
	 * @brief Tweliteをoffにする
	 */
	void on();
	
	/**
	 * @brief シリアル通信の初期化
	 * @param serial 使用するHardwareSerialオブジェクト（Serial, Serial1等）
	 */
	void begin(HardwareSerial &serial);
	
	/**
	 * @brief パケット送信
	 * @param packet 送信するパケット（ヘッダとチェックサムは自動設定）
	 */
	void sendPacket(Packet &packet);
	
	/**
	 * @brief パケット受信
	 * @param packet 受信したパケットを格納する構造体
	 * @return bool 受信成功時true、失敗時false
	 */
	bool receivePacket(Packet &packet);

	/**
	 * @brief パケット作成のヘルパー関数
	 * @param sender 送信元デバイスID
	 * @param receiver 宛先デバイスID
	 * @param messageType メッセージタイプ
	 * @param payloadLength ペイロード長
	 * @param payload ペイロードデータへのポインタ（nullptr可）
	 * @return Packet 作成されたパケット
	 */
	static Packet makePacket(DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType, uint8_t payloadLength, uint8_t *payload);
	
	/**
	 * @brief パケットマッチング判定
	 * @param pkt 判定するパケット
	 * @param sender 期待する送信元ID
	 * @param receiver 期待する宛先ID  
	 * @param messageType 期待するメッセージタイプ
	 * @return bool 全て一致した場合true
	 */
	static bool match(const Packet& pkt, DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType);

private:
	HardwareSerial *_serial;  // シリアル通信オブジェクトへのポインタ
	int _resetPin;
	
	/**
	 * @brief チェックサム計算（内部使用）
	 * @param packet チェックサムを計算するパケット
	 * @return uint8_t 計算されたチェックサム値
	 */
	uint8_t calculateChecksum(const Packet &packet);
};

} // namespace twelite