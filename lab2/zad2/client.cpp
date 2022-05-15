#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>

#define HELLO_MESSAGE 0b10000000U
#define RESPONSE_MESSAGE 0b00000000U
#define REQUEST_MESSAGE 0b01000000U
#define REMOTE_PORT 8276
#define MESSAGE_COUNT 0b00111100U
#define RESPONSE_PACKET_SIZE 2
#define REQUEST_PACKET_SIZE 1
#define WAIT_TIME 3380
#define AWAIT_HELLO_STATE 0
#define AWAIT_RESPONSE_STATE 1
#define READY_FOR_REQUEST 2


ZsutEthernetUDP Udp;

byte mac[] = {0xb8,0x27,0xeb,0xfb,0xeb,0x80};

byte packetNum = 0;
ZsutIPAddress serverIp = ZsutIPAddress(192,168,0,50);
unsigned long beginTime;

char requestPacketBuffer[1];
char incomingPacketBuffer[2];

uint16_t outputValue;
int packetSize;
int currentState = AWAIT_HELLO_STATE;

void setup() {
  Serial.begin(9600);
  Serial.println();

  //ZsutEthernet.begin(mac);

  Serial.println(ZsutEthernet.localIP());
  Udp.begin(REMOTE_PORT);
  beginTime = ZsutMillis();
}

void loop() {

  switch (currentState){
    case AWAIT_HELLO_STATE:
	  if(Udp.parsePacket()){
		Udp.read(incomingPacketBuffer, sizeof(incomingPacketBuffer));
        if(incomingPacketBuffer[0] & HELLO_MESSAGE){
	      Serial.println("Received hello message");
          currentState = READY_FOR_REQUEST;
        } 
	  }

      break;
    case AWAIT_RESPONSE_STATE:
	  
	  //Serial.print("IP i port w obiekcie Udp ");
	  //Serial.print(Udp.remoteIP());
	  //Serial.print(" ");
	  //Serial.print(Udp.remotePort());
	  //Serial.println();
	  
	  Udp.read(incomingPacketBuffer, 2);
	  

      if(Udp.parsePacket()){

        Serial.print("Client has received response. time: ");
        Serial.print(ZsutMillis());
		
        packetNum = (incomingPacketBuffer[1] >> 2) & 0b00001111;
        outputValue = ((incomingPacketBuffer[1] & 0b00000011) << 8) | incomingPacketBuffer[0];
        Serial.print("\nPacket number: ");
        Serial.print(packetNum);
        Serial.print("\nValue: ");
        Serial.print(outputValue);
		Serial.print("\n");
        currentState = READY_FOR_REQUEST;
      }
      break;
    case READY_FOR_REQUEST:
      if(ZsutMillis() - beginTime >= WAIT_TIME){
		
        requestPacketBuffer[0] = (char)REQUEST_MESSAGE;
		packetNum++;
        requestPacketBuffer[0] = requestPacketBuffer[0] | (packetNum << 2);
		
        Serial.print("Sending request = 0b");
        Serial.print(requestPacketBuffer[0], BIN);
		Serial.print("\nPacket number: ");
        Serial.print(packetNum);
		Serial.print("\n");
		
        //Udp.beginPacket(serverIp, REMOTE_PORT);
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
		
        Udp.write(requestPacketBuffer, 1);
        Udp.endPacket();
		Serial.println("Wyslano?");
		
        beginTime = ZsutMillis();

        currentState = AWAIT_RESPONSE_STATE;
      }
      break;
  }
}
