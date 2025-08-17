#include <TwelitePacket.h>

twelite::TwelitePacket tweliteModule;
twelite::Packet pkt;

void setup() {
	tweliteModule.begin(Serial2, 115200);
}

void loop() {
	pkt = twelite::TwelitePacket::makePacket(twelite::C_PARTS, twelite::BROADCAST, twelite::DeployComplete, 0, NULL);
	tweliteModule.sendPacket(pkt);

	delay(10000);
}