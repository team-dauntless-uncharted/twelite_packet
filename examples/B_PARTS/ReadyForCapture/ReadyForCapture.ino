#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
	Serial.begin(115200);
	tweliteModule.begin(Serial2, 115200);
}

void loop() {
	pkt = twelite::TwelitePacket::makePacket(twelite::B_PARTS, twelite::A_PARTS, twelite::ReadyForCapture, 0, NULL);
	tweliteModule.sendPacket(pkt);

	if (tweliteModule.receivePacket(pkt)) {
		if (twelite::TwelitePacket::match(pkt, twelite::A_PARTS, twelite::B_PARTS, twelite::ReadyForCaptureAck)) {
			Serial.println("received ReadyForCaptureAck");
		}
	}

	delay(10000);
}