#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
	Serial.begin(115200);
	tweliteModule.begin(Serial2, 115200);
}

void loop() {
	if (tweliteModule.receivePacket(pkt)) {
		Serial.println("received packet");
		if (twelite::TwelitePacket::match(pkt, twelite::B_PARTS, twelite::A_PARTS, twelite::TurnSignal)) {
			float latitude, longitude;
			memcpy(&latitude, pkt.payload, sizeof(float));
			memcpy(&longitude, pkt.payload + sizeof(float), sizeof(float));
			Serial.printf("latitude: %f, longitude: %f\n\n", latitude, longitude);
		}
	} else {
		Serial.println("no packet");
	}

	delay(1000);
}