#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>

#define HELLO_MESSAGE 0b10000000U
#define RESPONSE_MESSAGE 0b11000000U
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

byte packetNum = 0;
ZsutIPAddress serverIp = ZsutIPAddress(192,168,0,57);
unsigned long beginTime;

unsigned char requestPacketBuffer[1];
unsigned char incomingPacketBuffer[2];

byte mac[] = {0xb8,0x27,0xeb,0xfb,0xeb,0x80};

uint16_t outputValue;
int packetSize;
int currentState = AWAIT_HELLO_STATE;

void setup() {
  Serial.begin(9600);
  Serial.println("");
  
  ZsutEthernet.begin(mac); //proforma
  
  Serial.println(ZsutEthernet.localIP());
  Udp.begin(REMOTE_PORT);
  beginTime = ZsutMillis();
}

void loop() {
	  
  if(currentState == AWAIT_HELLO_STATE){
	if(Udp.parsePacket()){
	  Udp.read(incomingPacketBuffer, 2);
	  
      if(incomingPacketBuffer[1] & HELLO_MESSAGE){
	    Serial.println("+++Received hello message");
        currentState = READY_FOR_REQUEST;
		packetNum = (incomingPacketBuffer[1] >> 2) & 0b00001111;
		Serial.print("Received packet number: ");
        Serial.print(packetNum);
		Serial.print("\n+++\n");
      } 
	}
  }else if(currentState ==  AWAIT_RESPONSE_STATE){
	
    if(Udp.parsePacket()){
      //what value should be received
      if(packetNum == 15){
        packetNum = 1;		  
	  }else{
		packetNum++;
	  }
	  
      Udp.read(incomingPacketBuffer, 2);
	  
      Serial.print("===Client has received response. time: ");
      Serial.println(ZsutMillis());
	  
	  outputValue = ((uint16_t)(incomingPacketBuffer[1] & 0b00000011) << 8) | (uint16_t)incomingPacketBuffer[0];
		
      int receivedPacketNum = (incomingPacketBuffer[1] >> 2) & 0b00001111;
	  
	  if(receivedPacketNum == packetNum){
		Serial.print("Packet number: ");
        Serial.print(packetNum);
	  }else{
		Serial.print("Received and local packet numbers don't match");  
	  }

      Serial.print("\nPotentiometer value: ");
      Serial.print(outputValue);
      Serial.print("\n===\n");
      currentState = READY_FOR_REQUEST;
    }
  }else if(currentState == READY_FOR_REQUEST){
	  
      if(ZsutMillis() - beginTime >= WAIT_TIME){
		//flag
        requestPacketBuffer[0] = (char)REQUEST_MESSAGE;
		//packet number
		if(packetNum == 15){
          Serial.println("Max packet num (15), starting from 1");
          packetNum = 1;		  
		}else{
		  packetNum++;
		}
		
        requestPacketBuffer[0] = requestPacketBuffer[0] | (packetNum << 2);
		
        Serial.print("---Sending request\n");
		Serial.print("Packet number: ");
        Serial.print(packetNum);
		Serial.print("\n---\n");
		
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
		
        Udp.write(requestPacketBuffer, 1);
        Udp.endPacket();
		
        beginTime = ZsutMillis();

        currentState = AWAIT_RESPONSE_STATE;
      }
  }
}