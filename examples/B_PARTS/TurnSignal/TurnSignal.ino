#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
	tweliteModule.begin(Serial2, 115200);
}

void loop() {
	float latitude = 35.681236;
	float longitude = 139.767125;
	
	uint8_t payloadLength = sizeof(float) * 2;
	uint8_t payload[payloadLength];
	memcpy(payload, &latitude, sizeof(float));
	memcpy(payload + sizeof(float), &longitude, sizeof(float));

	pkt = twelite::TwelitePacket::makePacket(twelite::B_PARTS, twelite::A_PARTS, twelite::TurnSignal, payloadLength, payload);
	tweliteModule.sendPacket(pkt);

	delay(1000);
}