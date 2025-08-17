#include "TwelitePacket.h"

namespace twelite {

void TwelitePacket::begin(HardwareSerial &serial, int baud) {
	_serial = &serial;
	_serial->begin(baud);
}

uint8_t TwelitePacket::calculateChecksum(const Packet &packet) {
	uint8_t sum = packet.sender ^ packet.receiver ^ packet.messageType ^ packet.payloadLength;
	for (int i = 0; i < packet.payloadLength; i++) {
		sum ^= packet.payload[i];
	}
	return sum;
}

void TwelitePacket::sendPacket(Packet &packet) {
	packet.header = PACKET_HEADER;
	packet.checksum = calculateChecksum(packet);

	_serial->write(packet.header);
	_serial->write(packet.sender);
	_serial->write(packet.receiver);
	_serial->write(packet.messageType);
	_serial->write(packet.payloadLength);
	for (int i = 0; i < packet.payloadLength; i++) {
		_serial->write(packet.payload[i]);
	}
	_serial->write(packet.checksum);
	_serial->write('\r');
	_serial->write('\n');
}

bool TwelitePacket::receivePacket(Packet &packet) {
	if (_serial->available() < 6) {
		return false;
	}
	if (_serial->read() != PACKET_HEADER) {
		return false;
	}

	packet.header = PACKET_HEADER;
	packet.sender = _serial->read();
	packet.receiver = _serial->read();
	packet.messageType = _serial->read();
	packet.payloadLength = _serial->read();

	if (packet.payloadLength > MAX_PAYLOAD_SIZE) {
		return false;
	}

	for (int i = 0; i < packet.payloadLength; i++) {
		if (!_serial->available()) {
			return false;
		}
		packet.payload[i] = _serial->read();
	}

	packet.checksum = _serial->read();

	if (_serial->peek() == '\r') _serial->read();
	if (_serial->peek() == '\n') _serial->read();

	if (packet.checksum != calculateChecksum(packet)) {
		return false;
	}

	return true;
}

Packet TwelitePacket::makePacket(DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType, uint8_t payloadLength, uint8_t *payload) {
	Packet pkt;
	pkt.sender = sender;
	pkt.receiver = receiver;
	pkt.messageType = messageType;
	pkt.payloadLength = payloadLength;
	if (payload && payloadLength > 0) {
		memcpy(pkt.payload, payload, payloadLength);
	}
	return pkt;
}

bool TwelitePacket::match(const Packet& pkt, DEVICE_ID sender, DEVICE_ID receiver, MessageType messageType) {
	return (pkt.sender == sender && pkt.receiver == receiver && pkt.messageType == messageType);
}

} // namespace twelite