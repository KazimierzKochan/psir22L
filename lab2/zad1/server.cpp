#include <ZsutEthernet.h>       //niezbedne dla klasy 'ZsutEthernetUDP'
#include <ZsutEthernetUdp.h>    //sama klasa 'ZsutEthernetUDP'
#include <ZsutFeatures.h>       //for: ZsutMillis()

// zadany numer portu interwal cyklicznych raportow
#define PORT 12370
#define INTERVAL 3070

//dlugosc pakietu z danymi dla/z UDP
#define PACKET_BUFFER_LENGTH        2
unsigned char packetBuffer[PACKET_BUFFER_LENGTH];

uint16_t val;

unsigned long currentTime, lastTime;

ZsutEthernetUDP Udp;

ZsutIPAddress client = ZsutIPAddress(192,168,56,101);

char uint16ToCharArray(char *charArray, int length, uint16_t value) {
    int j;
    for(j = 0 ; j < length ; j++){
		//convert each 8bits to unsigned char
		charArray[j] = (unsigned char)(value >> 8*j) & 0xff;
	}
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("ZSUT eth UDP server init... "));
    Serial.println(F(__FILE__));
    Serial.print(F(__DATE__));
    Serial.print(F(", "));
    Serial.println(F(__TIME__));
    Serial.print("IP address: ");
    Serial.println(ZsutEthernet.localIP());

    currentTime = ZsutMillis();
    lastTime = 0;
    Udp.begin(PORT);

    for(;;){
        if(ZsutMillis() - currentTime < 1000)
            break;
    }
}

void loop() {
    currentTime = ZsutMillis();
    if(currentTime - lastTime >= INTERVAL) {
        val=ZsutAnalog0Read(); // odczyt z pinu Z0
        uint16ToCharArray(packetBuffer, PACKET_BUFFER_LENGTH, val);

        Udp.beginPacket(client, PORT);
        int r=Udp.write(packetBuffer, PACKET_BUFFER_LENGTH);
        Udp.endPacket();
        Serial.print("\n\n Send: ");
        Serial.print(val);
        Serial.print(".\n Using ");
        Serial.print(PACKET_BUFFER_LENGTH);
        Serial.print(" bytes: ");

	//print byte values
	for(int j=0; j < PACKET_BUFFER_LENGTH; j++){
		Serial.print(packetBuffer[j]);
		if(j < PACKET_BUFFER_LENGTH - 1){
			Serial.print(", ");
		}
	}

        lastTime = ZsutMillis();
    }
}
