#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
	Serial.begin(115200);
	tweliteModule.begin(Serial2, 115200);
}

void loop() {
	if (tweliteModule.receivePacket(pkt)) {
		if (twelite::TwelitePacket::match(pkt, twelite::B_PARTS, twelite::A_PARTS, twelite::ReadyForCapture)) {
			Serial.println("received ReadyForCapture");
			pkt = twelite::TwelitePacket::makePacket(twelite::A_PARTS, twelite::B_PARTS, twelite::ReadyForCaptureAck, 0, NULL);
			tweliteModule.sendPacket(pkt);

			delay(10000);
		}
	}
}